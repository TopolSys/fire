#include <iomanip>

#include "db/db.hpp"
#include "round.hpp"


void Round_t::para_init(){
	_StepBaseBegin = 256;
	_fRescue = false;
	_fPostRoute = false;
	_nThread = 1;
	_CompressFailLimit = 0;

	_j = 0;
	_k = 0;
	_nCompressFail = 0;
	_StepBase = _StepBaseBegin;
	_kPrefill = 0.96;\
	_kPrefillGoal = _pGdb->Prefill2D();
	_kPrefillStep = _pGdb->RoundStep();
	_FreezeWireLength = true;
	_fStopOnReflex = true;
}

int Round_t::compress(){
	lem::Clk_t clk;
	clk.start();
	std::cout<<"Compress "<<_j<<", PrefillRate = "<< kPrefill()<<std::endl;
	const bool fOnlyFreezed = false;
	if( !isInitRoute() )
		set_as_base_length(fOnlyFreezed);
	// reset
	_k = 0;
	_StepBase = _StepBaseBegin;
	// clean history cost 
	if( _pGdb->DahReset() )
		resetDah();

	int nPrevJob = 0;
	while( route() ){
		//if( 1<_k && _fStopOnReflex && nPrevJob < _vOrder.size() && _kPrefillGoal!=_kPrefill )\
			break;
		nPrevJob = _vOrder.size();
	}
	int nOvfl = num_net_overflow(kPrefill());
	int nFreezed = tagFreezed();
	clk.check();
	std::cout<<"> End Compress "<<_j<<" by "<<_k<<" rounds "<<", Elapsed Time = "<< clk.elapsed()<<", nOvfl="<<nOvfl<<", nFreezed="<<nFreezed <<std::endl;
	if( 0<nOvfl )
		_nCompressFail ++ ;
	if( CompressEnd() /**/|| (_CompressFailLimit > 0 && _nCompressFail >= _CompressFailLimit) /**/){
		std::cout<<"Terminate compressing process, reach limit="<< _CompressFailLimit <<std::endl;
		return 0;
	}
	// for next iteration 
	_kPrefill = std::max(kPrefill() - kPrefillStep(), kPrefillGoal() );
	_j++;
	return 1;
}

int Round_t::route(){
	initJob();
	std::cout<<"\tRound "<<std::setw(2)<<_k<<" StepBase="<<StepBase()<<" lenRatio="<<lenRatio()<<" nScheduled="<<_vOrder.size()<<"\n";
	joinJob();
	//_pTex->print_edge();
	_StepBase = _StepBaseBegin>>_k;
	return (0!=num_net_overflow(kPrefill())) && (_k++ < _pGdb->RoundLim());
}

int  Round_t::post_route(){
	std::cout<<"PostRoute"<<std::endl;
	int LenBefore = 0;
	for(int i=0; i<_pGdb->vTcl.size(); i++)
		LenBefore += _pGdb->vTcl[i].vPath().size();
	set_post_route();
	initJob();
	joinJob();
	del_post_route();
	int LenAfter  = 0;
	for(int i=0; i<_pGdb->vTcl.size(); i++)
		LenAfter += _pGdb->vTcl[i].vPath().size();
	std::cout<<"LenBefore="<< LenBefore<<", LenAfter="<<LenAfter<<", Diff="<< LenBefore - LenAfter <<std::endl;
	return num_net_overflow(kPrefill());
}


void Lem_RoundRoute( Lem_Gdb_t * pGdb, Tex_Man_t * pTex ){
	std::cout<<"RoundRoute"<<std::endl;
	Round_t round(pGdb,pTex);
	while( round.compress() )
		;
	//while( round.route() )\
		;
	if( lem::dev() ){
		round.report_overflow(0.4);
		round.report_overflow(0.45);
		round.report_overflow(0.5);
		round.report_overflow(0.55);
		round.report_overflow(0.6);
		round.report_overflow(0.65);
		round.report_overflow(0.7);
		round.report_overflow(0.75);
		round.report_overflow(0.8);
		round.report_overflow(0.85);
		round.report_overflow(0.9);
		round.report_overflow(0.95);
		round.report_overflow(1);
	}
	//int OvflAfterPost = round.post_route();\
	std::cout<<"OvflAfterPost="<<OvflAfterPost<<std::endl;
	return;

//	int nRescue = 0;
//	round.set_rescue();
//	round.round_reset();
//	round.set_as_base_length();
//	while( round.route() )
//		;
//	round.del_rescue();
}
//int Round_t::rescue(){
//	_fRescue = true ;
//	initJob();
//	joinJob();
//	_pTex->print_edge();
//	_fRescue = false;
//	return 0!=num_net_overflow(kPrefill());
//}

void Round_t::ripup( const PosPair_t& pospair ){
	// remove non repeat edge load 
	for(int i=0; i<TegTeid(pospair).size(); i++)
		if( EdgeGuide(pospair).unique(TegTeid(pospair)[i]) )
			_pTex->Edge(TegTeid(pospair)[i]).decLoad();
	EdgeGuide(pospair).remove( TegTeid(pospair) );

//	// remove non repeat tile load
	for(int i=0; i<TegPath(pospair).size(); i++){
		int tex = TegPath(pospair)[i];
		StepGuide(pospair).remove( tex );
		if( _pGdb->TexLoad() )
			_vRndTcl[pospair._tcl].try_decStepLoad( tex, _pTex ); // call-after deletion
	}
}

void Round_t::demand_add( const PosPair_t& pospair ){
	for(int i=0; i<TegTeid(pospair).size(); i++){
		int teid = TegTeid(pospair)[i];
		if( ! EdgeGuide(pospair).has(teid) )
			_pTex->Edge(teid).incLoad();
		if( _pTex->is_overflow(teid, kPrefill() ) )
			_vEdgeOvfl[teid] ++ ;
		EdgeGuide(pospair).add(teid);
	}

	for(int i=0; i<TegPath(pospair).size(); i++){
		int tex = TegPath(pospair)[i];
		if( _pGdb->TexLoad() )
			_vRndTcl[pospair._tcl].try_incStepLoad( tex, _pTex ); // call-before insertion
		StepGuide(pospair).add( tex );
	}
}

lem::Spin_t cspin;
int Round_t::route_on( Route_t& route, const PosPair_t& pospair ){
	int tcl = pospair._tcl;
	int pair_index = pospair._pair_index;
	Tcl_t& Tcl = _pGdb->vTcl[tcl];
	int p1_index = Tcl.vTegPair() [pair_index].first ;
	int p2_index = Tcl.vTegPair() [pair_index].second;
	int lenLim   = Tcl. TegBaseLen(pair_index);
	const Tex_Man_t::Pos_t& pos1 = Tcl.vTexPos()[p1_index];
	const Tex_Man_t::Pos_t& pos2 = Tcl.vTexPos()[p2_index];

	if( using_post_route() )
		lenLim = Tcl.mTegPath()[pair_index].size()-1;
	route.round_route( tcl, pos1, pos2, TegPath( pospair ), TegTeid( pospair ), lenLim, Tcl.isFreezed(pair_index) );
	
	
	// add non repeat load
	demand_add(pospair);
	return 1;
}

int Round_t::race_router(int id){
	Route_t::argpk_t argpk(_pGdb, _pTex);
	argpk._StepBase   = StepBase();
	argpk._StepOffset = 5;
	argpk._PredOffset = 0;
	argpk._CostOffset = 0;
	argpk._pRnd       = this;
	//argpk._fUseStepLimit = !_fRescue;
	Route_t route(argpk);
	while( !_JobStart.load() );
	int jid = lem::Jidep_t<>::jobWait();
	while( lem::Jidep_t<>::jobNull() != (jid = getJob(id)) ){
		_vGoing[id] = jid;
		const PosPair_t& pospair = _vPosPair[jid];
		if( (using_post_route()||TegPath(pospair).empty())? true: (0 < num_tex_overflow( pospair._tcl, pospair._pair_index, kPrefill() )) ){
			ripup(pospair);   // remove non-repeat load
			route_on( route, pospair); // add non-repeat load
		}

		Jidep.done(jid);
		_vJobNum[id] ++ ;
	}
	// finalize
	for(int i=id; i<_pGdb->vTcl.size(); i+=_nThread ){
		Tcl_t& Tcl = _pGdb->vTcl[i];
		_vRndTcl[i].StepGuide.member2univec(Tcl.vPath());
		std::sort( Tcl.vPath().begin(), Tcl.vPath().end() );

		vInt_t vTeid, vTeidPrev;
		for(int j=0; j<Tcl.vTegPair().size(); j++){
			vTeidPrev.swap(vTeid);
			Lem_Union(Tcl.TegTeid(j), vTeidPrev, vTeid);
		}
		Tcl.vTeid().swap( vTeid );
	}
	return 0;
}
