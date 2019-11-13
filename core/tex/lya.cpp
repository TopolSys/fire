#include "lya.hpp"
#include "lya_tcl.hpp"
#include "db/db.hpp"
#include "core/tex/tex.hpp"

Lya_t::Lya_t( Lem_Gdb_t * pGdb, Tex_Man_t * pTex ){
	_pGdb = pGdb;
	_pTex = pTex;
	resize(pTex->layerV().size(), pTex->ynum(), pTex->xnum());
	_OVERFLOW_OFFSET = 8;
	_SOFT_OVERFLOW_OFFSET = 3;
	_VIA_OFFSET = 0;
	_start_layer = pGdb->StartLya();
	_PinFillCost = 0;

	int nSegTotal = 0;
	for(vTcl_t::iterator pTcl = _pGdb->vTcl.begin(); pTcl!=_pGdb->vTcl.end(); pTcl++)
		nSegTotal += pTcl->vSeg().size();
	
	_vSegPtr.clear();
	_vSegPtr.resize(nSegTotal, NULL);
	if( nSegTotal )
		std::cout<<"nSegTotal="<<nSegTotal<<" (#seg/net="<< (float)nSegTotal/_pGdb->vTcl.size() <<")\n";

	int SegTop = 0;
	for(vTcl_t::iterator pTcl = _pGdb->vTcl.begin(); pTcl!=_pGdb->vTcl.end(); pTcl++)
		for(vSeg_t::iterator pSeg = pTcl->vSeg().begin(); pSeg!=pTcl->vSeg().end(); pSeg++)
			_vSegPtr[ SegTop++ ] = &*pSeg;

	//
	_StepCost = 1<<7;
	_ViaCost  = _StepCost<<1;
	_WrongWayCost = 1<<27;
	_HardLimit = 11;
	_nRelaxRound = 0;

	_fRelax    = false;
	_fAuxGuide = false;

	if( _pGdb->DynPrefill3D() ){
//		int nLow = 0;
		int nMid = 0;
		int nHigh= 0;
		float     MidLoad = 0.45;
//		float    HighLoad = 0.6;
		float  MidPrefill = 0.7;
		float HighPrefill = _pGdb->Prefill3D();
		_DPrefill.clear();
		_DPrefill.resize( pTex->data().size(), _pGdb->Prefill3D() );
		for(int i=0; i<_DPrefill.size(); i++){
			int loadRatio = pTex->tex_fillrate(i);
			if( loadRatio<MidLoad ){
				_DPrefill[i] = MidPrefill;
				nMid++;
			} else {
				_DPrefill[i] = HighPrefill;
				nHigh++;
			}
			//_DPrefill[i] = (_DPrefill[i]<=LowLoad)? LowPrefill: HighPrefill;
		}
		if( lem::dev() ){
			std::cout<<" Mid-th: load < "<<MidLoad<<", prefill="<< MidPrefill <<std::endl;
			std::cout<<"High-th prefill="<< HighPrefill <<std::endl;
			std::cout<<((float) 100.0f*nMid/((float)nHigh+nMid+1) )<<"\% tex in low prefill rate \n";
		}
	}
}


static void _TclPath2Segment( Lem_Gdb_t * pGdb, Tex_Man_t * pTex, int pid, int nThread ){
	Lya_Tcl_t lt(pGdb, pTex);
	for(int i=pid; i<pGdb->vTcl.size(); i+= nThread ){
		Tcl_t& Tcl = pGdb->vTcl[i];
		lt.init(Tcl); 		// tag texs and edges
		lt.decompose(Tcl); 	// decompose net into create segments
		if( pGdb->SegChunk() )
			lt.chunk(Tcl); 		// into small chunks
		lt.update(Tcl); 		// store result
		lt.erase(Tcl);
	}
}

void Lem_TclPath2Segment( Lem_Gdb_t * pGdb, Tex_Man_t * pTex ){
	const int nThread = pGdb->nThread();
	const int nChildThread = nThread-1;
	std::vector<std::thread> vChildThread(nChildThread);
	for(int i=0; i<nChildThread; i++)
		vChildThread[i] = std::thread( &_TclPath2Segment, pGdb, pTex, i, nThread );
	_TclPath2Segment( pGdb, pTex, nThread-1, nThread );
	for(int i=0; i<nChildThread; i++)
		vChildThread[i].join();
//	bool fPrintProgress = 1;
//	Lya_Tcl_t lt(pGdb, pTex);
//	lem::Progress_t<int> prog( 50, pGdb->vTcl.size());
//	int nNet = 0;
//	int nSegTotal = 0;
//	for(vTcl_t::iterator pTcl = pGdb->vTcl.begin(); pTcl!=pGdb->vTcl.end(); pTcl++, nNet++){
//		if( !pTcl->ok() )
//			continue;
//		if( fPrintProgress && (0==(nNet%100) || pTcl==--pGdb->vTcl.end()) )\
//			prog.set(nNet+1).print(std::cout);
//		lt.init(*pTcl); 		// tag texs and edges
//		lt.decompose(*pTcl); 	// decompose net into create segments
//		if( pGdb->SegChunk() )
//			lt.chunk(*pTcl); 		// into small chunks
//		lt.update(*pTcl); 		// store result
//		lt.erase(*pTcl);
//		nSegTotal += pTcl->vSeg().size();
//	}
//	if( fPrintProgress )
//		std::cout<<"\n";
}

struct SegPtrCmptor {
	bool operator()( const Seg_t* pSeg1, const Seg_t* pSeg2 ) const {
		const Seg_t& s1 = *pSeg1;
		const Seg_t& s2 = *pSeg2;
		//if( s1.cost() != s2.cost() )\
			return s1.cost() > s2.cost();
		if( s1.vTeg().size() != s2.vTeg().size() )\
			return s1.vTeg().size() > s2.vTeg().size();
		if( s1.isPinFill() != s2.isPinFill() )
			return s1.isPinFill() > s2.isPinFill();
		if( s1.length() != s2.length() )
			return s1.length() < s2.length();
		return false;
	}
};

void Lya_t::init_assign(){
	for(int i=0; i<_vSegPtr.size(); i++){
		Seg_t& seg = *_vSegPtr[i];
		if( seg.layer() == Seg_t::NullLayer() ){
			seg.set_cost(0);
			continue;
		}
		seg.set_cost( get_segCost(seg, seg.layer()) );
	}
	std::sort( _vSegPtr.begin(), _vSegPtr.end(), SegPtrCmptor() );
	//for(int i=0; i<_vSegPtr.size(); i++)\
		std::cout<< *_vSegPtr[i]<<" ";\
	std::cout<<"\n";

	vInt_t vLayerUtilNum( _pTex->layerV().size(), 0 );
	int nOverflowSeg = 0;
	Lem_Iterate(_vSegPtr, pSeg){
		Seg_t& Seg = **pSeg;
		int layer = 0;
		int minCost = 0;
		for(int i=_start_layer; i<_pTex->layerV().size(); i++){
			int cost = get_segCost(Seg,i);
			if( _start_layer==i || cost < minCost ){
				layer = i;
				minCost = cost;
			}
		}
		if( get_nOverflow(Seg,layer,false) )
			nOverflowSeg++;
		assign_layer(Seg,layer);
		vLayerUtilNum[ layer ]++;
	}
	if( nOverflowSeg )
		printf("OverflowSeg %d\n", nOverflowSeg );
	if( !_vSegPtr.empty() )
		for(int i=0; i<vLayerUtilNum.size(); i++){
			if( vLayerUtilNum[i] )
				printf("%9s:%9d (%4.1f\%)\n", _pTex->layerName(i).c_str(), vLayerUtilNum[i], (float) 100*vLayerUtilNum[i]/_vSegPtr.size() );
		}
}

void Lya_t::print_cube() const {
	std::cout<<"layer load report\n\t0-cap: zero capacity & load\n\t0full: loads on zero capacity.\n";
	const int nCategory = 14;
	mInt_t mLoadRatio(zdim());
	for(int i=0; i<zdim(); i++)
		mLoadRatio[i].resize(nCategory,0);
	int nTotalLoad = 0;
	for(int i=0; i<zdim(); i++){
		bool isV = _pTex->layerV()[i];
		for(int j=0; j<ydim(); j++){
			for(int k=0; k<xdim(); k++){
				Tex_Man_t::Pos_t pos = Tex_Man_t::Pos_t(k,j);
				int tex_index = _pTex->Pos2Index(pos);
//				int cap = _pTex->get_layerCap(isV, i, isV? k: j );
//				int obs = get_obsLoad(tex_index, i);
//				int load= entry(i,j,k).load();

				int obs = get_obsLoad(tex_index, i);
				int cap = get_cap    (tex_index, i);
				int net = get_netLoad(tex_index, i);
				nTotalLoad += net;

				if( 0>=cap-obs ){
					if( 0!=net )
						mLoadRatio[i][12]++;
					else 
						mLoadRatio[i][13]++;
					continue;
				}
				//if( 0==net )\
					continue;

				float ratio = (float) net/(cap-obs);
				if( 1.0f < ratio ){
					mLoadRatio[i][11]++;
					continue;
				}
				int index = ratio*10;
				mLoadRatio[i][index] ++ ;
			}
		}
	}
	printf("%8s%8s", "Layer", "prefer" );
	for(int i=0; i<10; i++)
		printf("%4s%3d%%", ">", i*10 );
	printf("%4s%3d%%", "==", 100 );
	printf("%4s%3d%%", ">", 100 );
	printf("%8s", "0full" );
	printf("%8s\n", "0-cap" );
	for(int i=0; i<zdim(); i++){
		printf("%8s%8s", _pTex->layerName()[i].c_str(), _pTex->layerV()[i]? "V": "H" );
		for(int j=0; j<mLoadRatio[i].size(); j++)
			printf("%8d", mLoadRatio[i][j]);
		printf("\n");
	}
	printf("%15s%d\n", "nTotalLoad=", nTotalLoad );
}

void Lya_t::round_assign(){
	;
}

void Lya_t::head_statistic() const {
	int nTotal = 0;
	for(int i=0; i<_vSegPtr.size(); i++){
		if( _vSegPtr[i]->head() != Seg_t::NullIndex() )
			continue;
		if( _vSegPtr[i]->tail() == Seg_t::NullIndex() )
			continue;
		nTotal ++ ;
	}

	vSegPtr_t _vSegHeadPtr;
	_vSegHeadPtr.clear();
	_vSegHeadPtr.resize( nTotal, NULL );
	int Top = 0;
	for(int i=0; i<_vSegPtr.size(); i++){
		if( _vSegPtr[i]->head() != Seg_t::NullIndex() )
			continue;
		if( _vSegPtr[i]->tail() == Seg_t::NullIndex() )
			continue;
		_vSegHeadPtr[Top++] = _vSegPtr[i];
	}

	int nMultiSwitch = 0;
	const int nChangeTimes = 15;
	vInt_t vChange(nChangeTimes+1, 0);
	for(int i=0; i<_vSegPtr.size(); i++){
		int prev_layer = -1;
		int nChange = 0;
		for(Seg_t * pS = _vSegPtr[i]; NULL!=pS; pS=pS->next(_pGdb)){
			if( -1!=prev_layer && pS->layer()!=prev_layer )
				nChange ++;
			prev_layer = pS->layer();
		}
		if( nChange<nChangeTimes )
			vChange[nChange] ++ ;
		else
			vChange[nChangeTimes] ++;
	}
	printf("%s", "Long Segment Switch Statistics\n");
	printf("%8s", "Times");
	for(int i=0; i<vChange.size()-1; i++)
		printf("%8d", i);
	printf("%7s%d\n", ">=", nChangeTimes);
	printf("%8s", "Counts");
	for(int i=0; i<vChange.size(); i++)
		printf("%8d", vChange[i]);
	printf("\n");
}

void Lem_AssignLayer( Lem_Gdb_t * pGdb, Tex_Man_t * pTex ){
	std::cout<<"AssignLayer"<<std::endl;
	Lem_TclPath2Segment(pGdb,pTex);
	Lya_t lya(pGdb,pTex);
	for(int i=0; i<1; i++){
		lya.init_assign();
		lya.print_cube();
	}
	lya.head_statistic();
}
