#include <math.h>
#include <iomanip>
#include "lya.hpp"
#include "lya_tcl.hpp"
#include "db/db.hpp"
#include "core/tex/tex.hpp"
#include "lem_mac.hpp"
#include "utils/sys/thread.hpp"

void Lem_TclPath2Segment( Lem_Gdb_t * pGdb, Tex_Man_t * pTex );
void Lem_AssignLayerRace( Lem_Gdb_t * pGdb, Tex_Man_t * pTex ){
	std::cout<<"AssignLayer"<<std::endl;
	Lem_TclPath2Segment(pGdb,pTex);
	Lya_t lya(pGdb,pTex);
	//for(int i=0; i<1; i++){\
		lya.init_assign();\
		lya.print_cube();\
	}
	//lya.head_statistic();
	lya.init_race();
	int nRound = 0;
	while( lya.round() )
		nRound++;
	int nVia = lya.num_via();
	std::cout<<"Lya Round Total = "<<nRound<<", nVia="<<nVia<<std::endl;
	
	if( lem::dev() )
		lya.round_stats(true );
//return ;
//	int nOverflowSegHard = lya.num_ovfl(false);
	if( lem::dev() )
		for(int i=pGdb->StartLya(); i<pTex->layerV().size(); i++)
			std::cout<<pTex->layerName()[i]<<" Ovfl# "<< lya.num_ovfl_layer(i,true)<<" Hard# "<< lya.num_ovfl_layer(i,false) <<std::endl;
//	if( 0==nOverflowSegHard )\
		return ;
	lya.relax();
	if( pGdb->AuxGuide() )
		lya.auxguide();
	if( lem::dev() )
		for(int i=pGdb->StartLya(); i<pTex->layerV().size(); i++)
			std::cout<<pTex->layerName()[i]<<" Ovfl# "<< lya.num_ovfl_layer(i,true)<<" Hard# "<< lya.num_ovfl_layer(i,false) <<std::endl;
}

int  Lya_t::relax(){
	int nVia = num_via();
	int nOverflowSeg = num_ovfl(true);
	int nOverflowSegHard = num_ovfl(false);

	set_relax();
	_nRelaxRound = 0;
	_HardLimit = 12;
	bool fStopAtNextDec = false;
	for(int i=0; (i<_HardLimit || (1==i%2)) && i < _HardLimit ; i++){
		_nRelaxRound++;
		round();
		int nVia_i = num_via();
		int nOverflowSeg_i = num_ovfl(true);
		int nOverflowSegHard_i = num_ovfl(false);
		//lya.round_stats(true );
		std::cout<<"relax round = "<<i<<", nVia="<<nVia_i<<", nOverflowSeg="<<nOverflowSeg_i<<", nOverflowSegHard="<< nOverflowSegHard_i <<std::endl;
		//if(nVia_i > nVia && nOverflowSeg_i > nOverflowSeg && nOverflowSegHard_i > nOverflowSegHard && i > 2)\
			break;
		if( fStopAtNextDec )\
			if( (nOverflowSeg>nOverflowSeg_i || nVia>nVia_i) && nOverflowSegHard>=nOverflowSegHard_i )\
				break;
		if( (nOverflowSeg<nOverflowSeg_i && nVia<nVia_i) || nOverflowSegHard<nOverflowSegHard_i ){\
			fStopAtNextDec = true;\
		}
		if( nOverflowSeg==nOverflowSeg_i && nVia==nVia_i && nOverflowSegHard==nOverflowSegHard_i )
			break;
		nVia = nVia_i;
		nOverflowSeg = nOverflowSeg_i;
		nOverflowSegHard= nOverflowSegHard_i;

		if( 0==nOverflowSeg_i && 0==nOverflowSegHard_i )\
			break;
	}
	if( lem::dev() )
		round_stats(true );

	del_relax();
	//check_load_equal();
}

bool Lya_t::check_load_equal()  {
	for(int i=0; i<_vSegPtr.size(); i++)
		race_detachLayer(*_vSegPtr[i]);
	for(int i=0; i<data().size(); i++){
		if(data()[i].load())
			std::cout<<data()[i].load()<<std::endl;
		assert(data()[i].load()==0);
	}
	exit(0);
	return true;
}

static bool fPrintProgress = false;
void Lya_t::DirectionalAssign(const bool isV){
	mSegPtr_t& mSegTex = isV? SegTexV: SegTexH;
	vInt_t vJid( mSegTex.size() );
	for(int i=0; i<vJid.size(); i++)
		vJid[i] = i;
	const int nChildThread = _pGdb->nThread()-1;
	std::vector<std::thread> vChildThread(nChildThread);
	lem::Jidep_t<> Jidep(vJid, lem::Deptor_t(true));
	lem::Progress_t<int> prog( 50, vJid.size());
	for(int i=0; i<nChildThread; i++)
		vChildThread[i] = std::thread( &Lya_t::assign_tex_array, this, &isV, &Jidep, &prog );
	assign_tex_array(&isV,&Jidep,&prog);
	for(int i=0; i<nChildThread; i++)
		vChildThread[i].join();
	if( fPrintProgress ){
		prog.set(Jidep.nDone()).print(std::cout);
		std::cout<<std::endl; // end of progress bar 
	}
}

struct SegPtrCmptor{
	const Tex_Man_t * _pTex;
	const std::vector<Seg_t*> * _pvSegPtr;
	bool _fLongFirst;
	Tex_Man_t::Pos_t BoundaryPos( const Seg_t * pSeg, bool isHead ) const { return _pTex->getPos(isHead? pSeg->front(): pSeg->back ());}
	int headIndex( const Seg_t * pSeg ) const { return pSeg->isV()? BoundaryPos(pSeg,true )._y: BoundaryPos(pSeg,true )._x; }
	int tailIndex( const Seg_t * pSeg ) const { return pSeg->isV()? BoundaryPos(pSeg,false)._y: BoundaryPos(pSeg,false)._x; }
	SegPtrCmptor(const Tex_Man_t * pTex, const std::vector<Seg_t*> * pvSegPtr, bool fLongFirst):_pTex(pTex),_pvSegPtr(pvSegPtr),_fLongFirst(fLongFirst){}
	bool operator()( const int& s1, const int& s2 ) const {
		const Seg_t& Seg1 = *(*_pvSegPtr)[s1];
		const Seg_t& Seg2 = *(*_pvSegPtr)[s2];
		//if( Seg1.vTeg().size() != Seg2.vTeg().size() )\
			return Seg1.vTeg().size() > Seg2.vTeg().size();
		if( Seg1.size() != Seg2.size() )
			return _fLongFirst? Seg1.size() > Seg2.size(): Seg1.size() < Seg2.size();
		return Seg1.tcl() < Seg2.tcl();
	}
};
static int countExtraVia( const IntPair_t& layerRange, int layer ){
	if(Seg_t::NullLayer() != layerRange.first ? layer<=layerRange.first : false)
		return layerRange.first - layer;
	if(Seg_t::NullLayer() != layerRange.second? layer>=layerRange.second: false)
		return layer -layerRange.second;
	return 0;
}
static IntPair_t layerRangeAssume( const IntPair_t& layerRange, int layer ){
	IntPair_t ret = layerRange;
	if(Seg_t::NullLayer() == layerRange.first || layer<layerRange.first )
		ret.first = layer;
	if(Seg_t::NullLayer() == layerRange.second|| layer>layerRange.second)
		ret.second= layer;
	return ret;
}
static bool layerRangeContain( const IntPair_t& layerRange, int layer ){
	if( Seg_t::NullLayer()!=layerRange.first && Seg_t::NullLayer()!=layerRange.second )
		return layerRange.first < layer && layer < layerRange.second;
	return false;
}

static bool layerRangeOverlap( const IntPair_t& layerRange, int layer ){
	if( Seg_t::NullLayer()!=layerRange.first && Seg_t::NullLayer()!=layerRange.second )
		return layerRange.first <= layer && layer <= layerRange.second;
	return false;
}

int  Lya_t::congestCost(Seg_t& Seg, int index, int layer){
	const bool UsingPinPrefill = true ;
	//if( Seg.isV()!=_pTex->layerV()[layer] )\
		return _WrongWayCost;
	//if( is_wpot(Seg,index) )\
		return 0;
	int cap = get_cap(Seg[index],layer);
	int obs = get_obsLoad(Seg[index],layer);
	int net = get_netLoad(Seg[index],layer);
	
	int pinLoaded  = 0;
	int pinPrefill = _pTex->getTex(Seg[index]).pinPrefill();
	int stdPinload = get_stdPinLoad(Seg[index],layer);
	bool fpinPrefill = UsingPinPrefill && 1==layer && 0<pinPrefill;

	if( fpinPrefill ){
		//int nStdPin = num_std_pin(Seg,index);\
		pinLoaded  = nStdPin? 0:( (fpinPrefill? pinPrefill: 0) - (fpinPrefill? stdPinload: 0) + 1);
		pinLoaded  = (fpinPrefill? pinPrefill: 0) - (fpinPrefill? stdPinload: 0);
	}

	//assert(0<=net && 0<=obs && 0<=stdPinload && 0<=pinPrefill);
	bool fWPOT = is_wpot(Seg,index);
	if( 0==cap || (!fWPOT && cap <= obs + pinLoaded) )
		return 1<<25;//<<overflow_offset(false);
	if( stdPinload )
		return 0;
	cap = cap - obs;// - pinLoaded;
	int load= net + pinLoaded + (stdPinload? 0: 1);
	float load2capRatio = (float) load/cap;
	//if( layer==1 && obs && pinPrefill && load2capRatio > 1.0f ){\
		std::cout <<"pinPrefill="<< pinPrefill <<", obs="<< obs<< ", net="<<net<<", cap="<<cap<<": "<<load2capRatio<<std::endl;\
	}
	if( _pGdb->OvflWall()? cap <= load: false )\
		return load2capRatio * (_StepCost<<11) * (net+1);
	const float PinTilePrefill = 0.6;
	const float PrefillRate= _pGdb->DynPrefill3D()? _DPrefill[Seg[index]]: (fpinPrefill? PinTilePrefill: _pGdb->Prefill3D());
	
	float prefillCap = (float) cap * PrefillRate;
	
	float ratio_prefill = (float) 1.41422f - PrefillRate;
	bool fPRL = _pGdb->PrlFactor()? _pTex->getTex(Seg[index]).layerPRL(layer): false;
	int StepWt = _StepCost + (_pGdb->LayerFactor()? layer: 0 )+ (fPRL? _ViaCost: 0);
	int ViaWt  = _ViaCost  + (_pGdb->LayerFactor()? layer: 0 )+ (fPRL? _ViaCost: 0);
	int M2Cost = 0;//(fpinPrefill && !is_wpot(Seg,index) )? _ViaCost: 0;
	if( load <= _pGdb->LowCostTh() * prefillCap )\
		return M2Cost + (float) (load2capRatio + fPRL) * StepWt * _pGdb->Lv2CostWt();
	float ret = M2Cost + (float) (pow(load2capRatio+ratio_prefill,4) + fPRL) * (ViaWt);
	//if( layer==1 && obs && pinPrefill ){\
		std::cout <<"pinPrefill="<< pinPrefill <<", obs="<< obs<< ", net="<<net<<", cap="<<cap<<": "<<load2capRatio<<",  ret="<<ret<<std::endl;\
	}
	return ret;
}

int Lya_t::countVia(const Tcl_t& Tcl) const {
	int nVia = 0;
	std::set<int> SeenTex;
	Lem_Iterate(Tcl.vSeg(),pSeg)
		nVia += pSeg->countVia(SeenTex, _pGdb);
	return nVia;
}

int  Lya_t::throughCost (const IntPair_t& thRange, Seg_t& Seg, int index){
	int ret = 0;
	for(int i=thRange.first+1; i<thRange.second; i++)
		ret += congestCost(Seg, index, i);
	return ret;
}

IntPair_t Lya_t::throughRange(const IntPair_t& layerRange, const int& dist_layer ){
	IntPair_t ret(dist_layer, dist_layer);
	if(Seg_t::NullLayer() != layerRange.second && dist_layer > layerRange.second)
		ret.first = layerRange.second;
	if(Seg_t::NullLayer() != layerRange.first  && dist_layer < layerRange.first )
		ret.second= layerRange.first ;
	return ret;
}

void Lya_t::race_detachLayer(Seg_t& Seg){
	const bool isWire = true;
	for(int i=0; i<Seg.size(); i++){
		IntPair_t layerRange_i  = Seg.layerRange(i,_pGdb,false);
		if( !layerRangeOverlap(layerRange_i,Seg.Layer(i)) ){

			IntPair_t ThroughRange = throughRange(layerRange_i, Seg.Layer(i));
			for(int j=ThroughRange.first+1; j<ThroughRange.second; j++)
				dec_netLoad(Seg, i, j, !isWire);

			dec_netLoad(Seg, i, Seg.Layer(i), isWire);
		}
		//if( 0<i? Seg.Layer(i-1)!=Seg.Layer(i): false )\
			dec_netLoad(Seg[i-1], Seg.Layer(i));\
		continue;
		if( 0<i? Seg.Layer(i-1)!=Seg.Layer(i): false ){
			IntPair_t layerRange_i  = Seg.layerRange(i-1,_pGdb);
			if( layerRangeOverlap(layerRange_i,Seg.Layer(i)) )
				continue;
			dec_netLoad(Seg, i-1, Seg.Layer(i), isWire);
			IntPair_t ThroughRange = throughRange(layerRange_i, Seg.Layer(i-1));
			for(int j=ThroughRange.first+1; j<ThroughRange.second; j++)
				dec_netLoad(Seg, i-1, j, !isWire);
		}
	}
	Seg.resetLayer();
}
void Lya_t::race_attachLayer(Seg_t& Seg, mInt_t& mCost, mInt_t& mSel ){
	int sel_layer = _start_layer;
	for(int i=_start_layer+1; i<mCost.back().size(); i++)
		if( mCost.back()[i] < mCost.back()[sel_layer] )
			sel_layer = i;
	int index_cur = Seg.size()-1;
	while(sel_layer>=0){
		//Seg.setLayer(index_cur, Seg.layer());
		Seg.setLayer(index_cur, sel_layer);
		sel_layer = mSel[index_cur][sel_layer];
		--index_cur;
	}
	const bool isWire = true;
	for(int i=0; i<Seg.size(); i++){
		IntPair_t layerRange_i  = Seg.layerRange(i,_pGdb,false);
		if( !layerRangeOverlap(layerRange_i,Seg.Layer(i)) ){

			IntPair_t ThroughRange = throughRange(layerRange_i, Seg.Layer(i));
			for(int j=ThroughRange.first+1; j<ThroughRange.second; j++)
				inc_netLoad(Seg, i, j, !isWire);

			inc_netLoad(Seg, i, Seg.Layer(i), isWire);
		}
		//if( 0<i? Seg.Layer(i-1)!=Seg.Layer(i): false )\
			inc_netLoad(Seg[i-1], Seg.Layer(i));\
		continue;
		if( 0<i? Seg.Layer(i-1)!=Seg.Layer(i): false ){
			IntPair_t layerRange_i  = Seg.layerRange(i-1,_pGdb);
			if( layerRangeOverlap(layerRange_i,Seg.Layer(i)) )
				continue;
			inc_netLoad(Seg, i-1, Seg.Layer(i), isWire);
			IntPair_t ThroughRange = throughRange(layerRange_i, Seg.Layer(i-1));
			for(int j=ThroughRange.first+1; j<ThroughRange.second; j++)
				inc_netLoad(Seg, i-1, j, !isWire);
		}
	}
//	if( "pin1"==_pGdb->vTcl[Seg.tcl()].name() && !Seg.isV() && Seg.size()>80 && Seg.size()<103 && Seg.vTeg().size()
//		){
//		std::cout<<_pTex->getCoord(Seg[0],false)<<","<<_pTex->getCoord(Seg[0],true)<<std::endl;
//		std::cout<<"Layer    ";
//		for(int i=0; i<_pTex->layerV().size(); i++){
//			std::cout<<std::setw(12)<< _pTex->layerName()[i] <<" ";
//		}std::cout<<std::endl;
//		for(int i=0; i<mCost.size(); i++){
//			std::cout<<"Step="<<std::setw(2)<<i<<": ";
//			for(int j=0; j<mCost[i].size(); j++)
//				std::cout<<std::setw(12)<<mCost[i][j]<<" ";
//			std::cout<<std::endl;
//		}
//		for(int i=0; i<mSel.size(); i++){
//			std::cout<<"Step="<<std::setw(2)<<i<<": ";
//			for(int j=0; j<mSel[i].size(); j++)
//				std::cout<<std::setw(12)<<(mSel[i][j]>=0? _pTex->layerName()[mSel[i][j]]: "-" )<<" ";
//			std::cout<<std::endl;
//		}
//		std::cout<<Seg<<std::endl;
//		exit(0);
//	}
}
void Lya_t::assign_seg(Seg_t& Seg){
	if( using_relax() )
		race_detachLayer(Seg);
	mInt_t mSel ( Seg.size() );
	mInt_t mCost( Seg.size() );
	for(int i=0; i<Seg.size(); i++){
		mSel [i].resize(_pTex->layerV().size(),-1);
		mCost[i].resize(_pTex->layerV().size(),_WrongWayCost );
	}
	IntPair_t layerRange_prev(Seg_t::NullLayer(), Seg_t::NullLayer());
	float ViaCostWt= _pGdb->DynViaCost()? 0.2 + sqrt((float)(1+_nRelaxRound)/_HardLimit): 1;
	for(int i=0; i<Seg.size(); i++){
		//assign_seg_pos(Seg, i, mSel, mCost);
		IntPair_t layerRange_cur  = Seg.layerRange(i,_pGdb,false);
		IntPair_t layerRange_i    = Seg.layerRange(i,_pGdb,false);
		// compute cost of current tex
		for(int j=_start_layer; j<_pTex->layerV().size(); j++){
			if( Seg.isV()!=_pTex->layerV()[j] ){
				mCost[i][j] = 1<<22;\
				continue;
			}
			int ViaCost_j = (float) ViaCostWt * _ViaCost * countExtraVia(layerRange_cur, j);
			int CongestCost = congestCost(Seg,i,j);
			IntPair_t ThroughRange = throughRange(layerRange_i, j);
			int ThroughCost = throughCost(ThroughRange, Seg, i);
			mCost[i][j] = CongestCost + ThroughCost+ ViaCost_j;

//	if( _pGdb->vTcl[Seg.tcl()].name()=="net16548" && !Seg.isV() && 2==Seg.vTeg().size() && 1==Seg.Teg(i).size() && 1==j ){
//		std::cout<<"\t~~~~~~~Teg="<<Seg.Teg(i)[0]<<" on i"<<i<<std::endl;
//		std::cout<<"\t"<<"CongestCost="<<CongestCost<<", ThroughCost="<<ThroughCost<<", ViaCost_j="<<ViaCost_j<<", nExtraVia="<<countExtraVia(layerRange_cur, j)<< ", mCost[i][j]="<<mCost[i][j]<<std::endl;
//	}
//	if( _pGdb->vTcl[Seg.tcl()].name()=="net16548" && !Seg.isV() && 2==Seg.vTeg().size() && i==0 ){
//		std::cout<<"><On i="<<i<<"CongestCost="<<CongestCost<<", ThroughCost="<<ThroughCost<<", ViaCost_j="<<ViaCost_j<<", nExtraVia="<<countExtraVia(layerRange_cur, j)<< ", mCost[i][j]="<<mCost[i][j]<<std::endl;
//	}
		}

		if(0==i){
			for(int j=_start_layer; j<_pTex->layerV().size(); j++)
				mSel[i][j] = -1;

//	if( _pGdb->vTcl[Seg.tcl()].name()=="net16548" && !Seg.isV() && 2==Seg.vTeg().size() ){
//		std::cout<<"><Skip i="<<0<<"mCost[i][j]="<<mCost[0][1]<<std::endl;
//	}
			goto SkipPrev;
		}
		// compute cost of selected previous solution
		for(int j=_start_layer; j<_pTex->layerV().size(); j++){
			int CongestCost_prev = congestCost(Seg,i-1,j);
			vInt_t vSelCost( _pTex->layerV().size(), 0 );
			for(int k=_start_layer; k<_pTex->layerV().size(); k++){
				if( k==j ){
					vSelCost[k] = mCost[i-1][k];// + prlCost(Seg, i, k);
					continue;
				}
				if( _pTex->layerV()[j]!=_pTex->layerV()[k] )
					vSelCost[k] += 1<<22;
				IntPair_t layerRange_k = layerRangeAssume(layerRange_prev, k);
				IntPair_t ThroughRange = throughRange(layerRange_k, j);
				int ThroughCost = throughCost(ThroughRange, Seg, i-1);
				int ViaCost_k =  (float) ViaCostWt * _ViaCost * countExtraVia(layerRange_k, j);
				//if(ViaCost_k)\
				std::cout<<(ViaCost_k? ViaCost_k: 0)<<":"<< _ViaCost <<":"<< countExtraVia(layerRange_k, j)<<":"<<sqrt((float)(1+_nRelaxRound)/_HardLimit) <<std::endl;
				int prev_cost = (ThroughCost + (layerRangeContain(layerRange_k,k)? 0: CongestCost_prev));
				vSelCost[k] = ViaCost_k + mCost[i-1][k] + prev_cost;
			}
			int sel_k = _start_layer;
			for(int k=_start_layer+1; k<vSelCost.size(); k++){
				if( vSelCost[k]<vSelCost[sel_k] )
					sel_k = k;
			}

//	if( _pGdb->vTcl[Seg.tcl()].name()=="net16548" && !Seg.isV() && 2==Seg.vTeg().size() && (1==Seg.Teg(i).size()||i==0) ){
//		std::cout<<"Done i= "<<i<<", mCost[i][j]"<<mCost[i][1]<<", vSelCost[sel_k]="<<vSelCost[sel_k]<<std::endl;
//	}
			mSel [i][j] = sel_k;
			mCost[i][j] = mCost[i][j] + vSelCost[sel_k];//std::min(_WrongWayCost, mCost[i][j] + vSelCost[sel_k]);
		}

		SkipPrev:
		layerRange_prev = layerRange_cur;
	}


//	if( _pGdb->vTcl[Seg.tcl()].name()=="net16548" && !Seg.isV() && 2==Seg.vTeg().size() && 1==Seg.Teg(0).size() ){
//		std::cout<<"\tEnd i="<<0<<" mCost[i][j]="<<mCost[0][1]<<std::endl;
//	}
//
//	if( _pGdb->vTcl[Seg.tcl()].name()=="net16548" && !Seg.isV() && 2==Seg.vTeg().size() && 1==Seg.Teg(0).size() ){
//		std::cout<<"\tEnd i="<<(Seg.size()-1)<<" mCost[i][j]="<<mCost[Seg.size()-1][1]<<std::endl;
//	}
	//for(int i=0; i<mCost.back().size(); i++)\
		std::cout<<std::setw(11)<<mCost.back()[i]<<"("<<std::setw(1)<<mSel.back()[i] <<") ";\
	std::cout<<std::endl;
	race_attachLayer(Seg,mCost,mSel);
	
	int nType = 0;
	vBool_t vType(_pTex->layerV().size(),false);
	if(lem::dev()){
		for(int i=0; i<Seg.size(); i++){
			assert( Seg.Layer(i)!=Seg_t::NullLayer() );
			if( !vType[Seg.Layer(i)] )
				nType ++ ;
			vType[Seg.Layer(i)] = true;
		}
	}
	//if(nType>1){\
		for(int i=0; i<Seg.size(); i++)\
			std::cout<< Seg.Layer(i)<<" ";\
		std::cout<<std::endl;\
	}
	
}
void Lya_t::assign_seg_array(vSegPtr_t& vSegPtr){
	vSegPtr_t vSegPtrLocal;
	Lem_Iterate(vSegPtr, pSegPtr){
		if( _pGdb->Lya1Bfs() )
			if( (*pSegPtr)->race_assigned() && !using_relax() )
					continue;
		if( _pGdb->Lya1Bfs()? (*pSegPtr)->adj_race_assigned(_pGdb): true )
			vSegPtrLocal.push_back(*pSegPtr);
	}
	if( vSegPtrLocal.empty() )
		return ;
	// update prefill rate

	vInt_t vOrder( vSegPtrLocal.size() );
	int Top = 0;
	Lem_Iterate(vSegPtrLocal, pSegPtr){
		if( !using_relax() )
			(*pSegPtr)->resetLayer();
		vOrder[Top] = Top++;
	}
	std::sort(vOrder.begin(), vOrder.end(), SegPtrCmptor(_pTex, &vSegPtrLocal, _pGdb->IntlvSort()&&(_nRelaxRound%2)) );
	for(int i=0; i<vOrder.size(); i++)
		assign_seg(*vSegPtrLocal[vOrder[i]]);
}

void Lya_t::assign_tex_array(const bool * pisV, lem::Jidep_t<>* pJidep, lem::Progress_t<int>* pProg){
	const bool isV = *pisV;
	mSegPtr_t& mSegTex = isV? SegTexV: SegTexH;
	lem::Jidep_t<>& Jidep = *pJidep;
	int jid = lem::Jidep_t<>::jobWait();
	while( lem::Jidep_t<>::jobNull() != (jid=Jidep.getJob()) ){
		vSegPtr_t& vSegPtr = mSegTex[jid];
		if( using_aux_guide() )
			auxguide_seg_array(vSegPtr, isV);
		else
			assign_seg_array(vSegPtr);

		Jidep.done(jid);
		_progSpin.lock();
		if( fPrintProgress && (0==(Jidep.done()%100) || jid==(mSegTex.size()-1)) )
			pProg->set(Jidep.nDone()).print(std::cout);
		_progSpin.unlock();
	}
}

void Lya_t::init_race(){
	if( _pTex->layerV().empty() )
		return;
	SegTexV.clear();
	SegTexH.clear();
	SegTexV.resize( _pTex->vGridX().size()-1 ); // segments go vertically accessed by x index of tex
	SegTexH.resize( _pTex->vGridY().size()-1 );
	Lem_Iterate(_pGdb->vTcl, pTcl)
		Lem_Iterate(pTcl->vSeg(),pSeg){
			if( pSeg->isV() )
				SegTexV[ _pTex->getPos(pSeg->front())._x ].push_back(&*pSeg);
			else
				SegTexH[ _pTex->getPos(pSeg->front())._y ].push_back(&*pSeg);
		}
}

int  Lya_t::round(){
	DirectionalAssign( !_pTex->layerV().front() );
	DirectionalAssign(  _pTex->layerV().front() );
	return round_stats(false);
}

int  Lya_t::auxguide(){
	set_aux_guide();
	DirectionalAssign( !_pTex->layerV().front() );
	DirectionalAssign(  _pTex->layerV().front() );
	del_aux_guide();
	return 1;
}

int  Lya_t::num_via() const {
	int nVia = 0;
	Lem_Iterate(_pGdb->vTcl, pTcl)
		nVia += countVia(*pTcl);
	return nVia;
}

//int  Lya_t::prlCost(Seg_t& Seg, int index, int layer){
//	if( Seg.size() <60 )
//		return 0;
//	int cap = get_cap(Seg[index],layer);
//	int obs = get_obsLoad(Seg[index],layer);
//	int net = get_netLoad(Seg[index],layer);
//	const int _prlCost = _ViaCost/6;
//	return ((cap>>1) <= obs + net )? _prlCost: 0;
//}

int  Lya_t::round_stats(bool fPrint){

	vInt_t vFreq(_pTex->layerV().size(), 0);
	//vInt_t vFreqOld(_pTex->layerV().size(), 0);
	int nWait = 0, nOverflowSeg = 0, nOverflowHardSeg = 0;
	for(int i=0; i<_vSegPtr.size(); i++){
		if( !_vSegPtr[i]->race_assigned() ){
			nWait ++ ;
			continue;
		}
		nOverflowSeg     += 0<get_nOverflow(*_vSegPtr[i],true );
		nOverflowHardSeg += 0<get_nOverflow(*_vSegPtr[i],false);
		for(int j=0; j<_vSegPtr[i]->size(); j++){
			//if(_vSegPtr[i]->Layer(j)==Seg_t::NullLayer())\
				continue;
			assert( _vSegPtr[i]->Layer(j)!=Seg_t::NullLayer() );
			vFreq[_vSegPtr[i]->Layer(j)] ++ ;
		}
		//vFreqOld[_vSegPtr[i]->layer()] += _vSegPtr[i]->size() ;
	}
	if( fPrint ){
		std::cout<<"nViaTotal="<<num_via() <<std::endl;

		for(int i=0; i<vFreq.size(); i++)
			std::cout<< _pTex->layerName()[i] <<": "<<std::setw(12)<< vFreq[i]<<std::endl;
		std::cout<<"nOverflowSeg #= "<< nOverflowSeg<<", hard="<<nOverflowHardSeg<<std::endl;
	}
	return nWait;
}