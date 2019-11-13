#include <math.h>
#include <unistd.h>
#include <iomanip>
#include <thread>

#include "db/db.hpp"
#include "round.hpp"


int  Round_t::num_tex_overflow( int tcl, int pair_index, float SoftOverflow ) const {
	const Tcl_t& Tcl = _pGdb->vTcl[tcl];
	int nOverflow     = _pTex->num_overflow(Tcl.mTegTeid()[pair_index], SoftOverflow);
	int nIntrinsicOvfl= 
		_pTex->intrinsic_overflow(Tcl.vTexPos()[Tcl.vTegPair()[pair_index].first ], SoftOverflow) +
		_pTex->intrinsic_overflow(Tcl.vTexPos()[Tcl.vTegPair()[pair_index].second], SoftOverflow);
	return std::max(nOverflow - nIntrinsicOvfl, 0);
}

int  Round_t::num_net_overflow( float SoftOverflow ) const {
	int nJob = 0;
	for(int i=0; i<_vPosPair.size(); i++){
		const int tcl        = _vPosPair[i]._tcl;
		const int pair_index = _vPosPair[i]._pair_index;
		if( 0<num_tex_overflow(tcl, pair_index, SoftOverflow) )
			nJob++;
	}
	return nJob;
}

void Round_t::set_as_base_length(const bool fOnlyFreezed){
	for(int i=0; i<_vPosPair.size(); i++){
		Tcl_t& Tcl = _pGdb->vTcl[_vPosPair[i]._tcl];
		const int pair_index = _vPosPair[i]._pair_index;
		if( fOnlyFreezed && !Tcl.isFreezed(pair_index) )
			continue;
		//if( Tcl.mTegPath()[pair_index].size() < Tcl.TegBaseLen()[pair_index] )\
			continue;
		Tcl.TegBaseLen()[pair_index] = Tcl.mTegPath()[pair_index].size();
	}
}
void PathGuide_t::add   ( const vInt_t& vPath ){
	insert(vPath.begin(), vPath.end());
}
void PathGuide_t::add   ( const int& val ){
	insert(val);
}
void PathGuide_t::remove( const vInt_t& vPath ){
	for(int i=0; i<vPath.size(); i++){
		iterator itr = find(vPath[i]);
		if( end()!=itr )
			erase (itr);
	}
}
void PathGuide_t::remove( const int& step ){
	iterator itr = find(step);
	if( end()!=itr )
		erase (itr);
}
bool PathGuide_t::has( int index ) const {
	bool ret = end() != find(index);
	return ret;
}
void Skeleton_t ::add   ( const int& val ){
	insert(val);
}
void Skeleton_t ::remove( const int& val ){
	iterator itr = find(val);
	if( end()!=itr )
		erase (itr);
}

bool Skeleton_t ::has( int index ) const {
	return end() != find(index);
}
Round_t::Round_t( Lem_Gdb_t * pGdb, Tex_Man_t * pTex ):
Jidep(300){
	_pGdb = pGdb;
	_pTex = pTex;
	para_init();

	_vRndTcl.resize(_pGdb->vTcl.size());
	int nNet = 0;

	if( lem::dev() )
		std::cout<<"Minimal prefill rate="<<minimal_prefill_rate()<<std::endl;
	
	for(vTcl_t::iterator pTcl = pGdb->vTcl.begin(); pTcl!=pGdb->vTcl.end(); pTcl++, nNet++){
		//for(int j=0; j<pTcl->vPath().size(); j++)\
			pTex->getTex(pTcl->vPath()[j]).inc_netLoad();
		for(int j=0; j<pTcl->mTegPath().size(); j++){
			for(int k=0; k<pTcl->TegPath(j).size(); k++){
				int tex = pTcl->TegPath(j)[k];
				if( _pGdb->TexLoad() )
					_vRndTcl[nNet].try_incStepLoad( tex, _pTex );
				_vRndTcl[nNet].StepGuide.add( tex );
			}
		}
		for(int j=0; j<pTcl->mTegTeid().size(); j++)
			_vRndTcl[nNet].EdgeGuide.add( pTcl->TegTeid(j) );

		Lem_Iterate(pTcl->Rsmt().vEdge(), pEdge){\
			Rect_t rect( pTcl->Rsmt().vPoint()[pEdge->first], pTcl->Rsmt().vPoint()[pEdge->second] );
			for(int j=rect.p1.x; j<rect.p2.x; j++){
				int index1 = _pTex->Pos2Index( Tex_Man_t::Pos_t( j, rect.p1.y ) );
				int index2 = _pTex->Pos2Index( Tex_Man_t::Pos_t( j, rect.p2.y ) );
				if( lem::dev() ){
					assert(0 <= index1 && index1 < _pTex->data().size() );\
					assert(0 <= index2 && index2 < _pTex->data().size() );
				}
				_vRndTcl[nNet].RsmtGuide.add( index1 );
				_vRndTcl[nNet].RsmtGuide.add( index2 );
			}
			for(int j=rect.p1.y; j<rect.p2.y; j++){
				int index1 = _pTex->Pos2Index( Tex_Man_t::Pos_t( rect.p1.x, j ) );
				int index2 = _pTex->Pos2Index( Tex_Man_t::Pos_t( rect.p2.x, j ) );
				if( lem::dev() ){
					assert(0 <= index1 && index1 < _pTex->data().size() );\
					assert(0 <= index2 && index2 < _pTex->data().size() );
				}
				_vRndTcl[nNet].RsmtGuide.add( index1 );
				_vRndTcl[nNet].RsmtGuide.add( index2 );
			}
		}
		//std::cout <<_vRndTcl[nNet].WireGuide.size_() <<" w "<< std::endl;\
		std::cout <<_vRndTcl[nNet].RsmtGuide.size_() << std::endl;
	}
	
	_nThread = _pGdb->nThread();
	vRoutePtr.resize(_nThread, NULL);
	Route_t::argpk_t argpk(pGdb,pTex);
	argpk._StepBase   = StepBase();
	argpk._StepOffset = 5;
	argpk._PredOffset = 0;
	argpk._CostOffset = 0;
	argpk._pRnd = this;
	for(int i=0; i<vRoutePtr.size(); i++)
		vRoutePtr[i] = new Route_t(argpk);

	resetDah();
}

void Round_t::resetDah(){
	_vDah.clear();
	_vDah.resize( _pTex->Edge().size(), 1 );
}

Round_t::~Round_t(){
	for(int i=0; i<vRoutePtr.size(); i++)
		delete vRoutePtr[i];
	vRoutePtr.clear();
}

const vInt_t& Round_t::TegPath( const PosPair_t& pospair ) const {
	int tcl = pospair._tcl;
	int pair_index = pospair._pair_index;
	return _pGdb->vTcl[tcl].TegPath(pair_index);
}
const vInt_t& Round_t::TegTeid( const PosPair_t& pospair ) const {
	int tcl = pospair._tcl;
	int pair_index = pospair._pair_index;
	return _pGdb->vTcl[tcl].TegTeid(pair_index);
}

vInt_t& Round_t::TegPath( const PosPair_t& pospair ){
	int tcl = pospair._tcl;
	int pair_index = pospair._pair_index;
	return _pGdb->vTcl[tcl].TegPath(pair_index);
}
vInt_t& Round_t::TegTeid( const PosPair_t& pospair ){
	int tcl = pospair._tcl;
	int pair_index = pospair._pair_index;
	return _pGdb->vTcl[tcl].TegTeid(pair_index);
}

PathGuide_t& Round_t::StepGuide(const PosPair_t& pospair ){
	return _vRndTcl[pospair._tcl].StepGuide;
}
PathGuide_t& Round_t::EdgeGuide(const PosPair_t& pospair ){
	return _vRndTcl[pospair._tcl].EdgeGuide;
}

//bool Round_t::overflow( int teid ) const {\
	return 0==_pTex->trackCap(teid);\
}

int Round_t::getJob( int pid ){
	int jid = lem::Jidep_t<>::jobWait();
	int nround = 0;
	while( jid == lem::Jidep_t<>::jobWait() ){
		//std::cout<<jid<<std::endl;
		jid = Jidep.getJob();
	}
	return jid;
}

float Round_t::lenRatio() const {
	return 1 + atan(_k-9)+1.5;// + atan(_k-14)+1.5;
}

Rect_t Round_t::lenBox( const PosPair_t& pospair ) const {
	int _tcl = pospair._tcl;
	int _pair_index = pospair._pair_index;
	int p1 = _pGdb->vTcl[_tcl].vTegPair()[_pair_index].first ;
	int p2 = _pGdb->vTcl[_tcl].vTegPair()[_pair_index].second;
	int lenLim = (float) _pGdb->vTcl[_tcl]. TegBaseLen(_pair_index) * lenRatio();
	Rect_t rect( _pGdb->vTcl[_tcl].vTexPos()[p1].point(), _pGdb->vTcl[_tcl].vTexPos()[p2].point() );

	int max_h = lenLim - rect.rangeY().scalar();
	int max_v = lenLim - rect.rangeX().scalar();

	int remainder_h = (float) max_h /2 + 1.0f;
	int remainder_v = (float) max_v /2 + 1.0f;
	Rect_t ret(rect);
	ret.p1.x -= remainder_h;
	ret.p2.x += remainder_h;
	ret.p1.y -= remainder_v;
	ret.p2.y += remainder_v;
	return ret;
}
void Round_t::initJob(){
	const bool fRouteAll = isInitRoute() && !_pGdb->ReuseTrial();
	_jobTop = 0;
	RoundRouteTarget(_vPosPair, _vOrder);
	RoundRouteOrder (_vPosPair, _vOrder, fRouteAll );

	_vChildThread.clear();
	_vChildThread.resize( nChildThread() );
	_vJid.clear();
	_vJid.resize(_nThread, -1);

	_JobStart.store(false);
	for(int i=0; i<nChildThread(); i++)
		_vChildThread[i] = std::thread(&Round_t::race_router, this, i);
	
	_vJobNum.clear();
	_vJobNum.resize(_nThread, 0);

	_vGoing.clear();
	_vGoing.resize(_nThread, -1);

	_vEdgeOvfl.clear();
	_vEdgeOvfl.resize( _pTex->Edge().size(), 0 );

	Jidep.init(_vOrder,NetDeptor_t(_pGdb,this,&_vPosPair));
	_JobStart.store(true );
}
void Round_t::joinJob(){
	race_router( nChildThread() );
	for(int i=0; i<nChildThread(); i++)
		_vChildThread[i].join();

	int nTotal = 0;
	std::cout<<"\t(Loads/thread: ";
	for(int i=0; i<_vJobNum.size(); i++){
		nTotal += _vJobNum[i];
		std::cout<< _vJobNum[i]<<" ";
	}

	if( using_post_route() )
		;
	else {
		for(int i=0; i<_vEdgeOvfl.size(); i++)
			_vDah[i] += _vEdgeOvfl[i];
	}

	int nTeid = total_teid();
	std::cout<<"): ";
	std::cout<<"(nTeid   ="<< nTeid <<", ";
	std::cout<<"nWaitLv2="<< Jidep.nWaitLv2() <<", ";
	std::cout<<"nTotal ="<< nTotal <<")\n";
	//std::cout<<"nDep="<< Jidep.nDep() <<"\n";
	//std::cout<<" nThread="<< _nThread <<"\n";
	//std::cout<<" nScheduled="<< _vOrder.size() <<"\n";
	assert( nTotal== _vOrder.size() );
}

float Round_t::dah( int teid ) const {
	//return 1;
	//return (float) _vDah[teid]/(7+4*sqrt(_k));
	return (float) _vDah[teid]/(7+4*sqrt(_k));
}

int Round_t::total_teid() const {
	int nTeid = 0;
	for(int i=0; i<_pGdb->vTcl.size(); i++)
		nTeid += _pGdb->vTcl[i].vTeid().size();
	return nTeid;
}

double Round_t::minimal_prefill_rate() const {
	int nTeid = total_teid();
	int TeidCap = _pTex->total_teid_cap();
	return (double) nTeid/(1+TeidCap);
}

bool Rnd_Tcl_t::has_dirLoad(int tex, bool isV, Tex_Man_t * pTex) const {
	// skip if tex is not in current path
	if( ! StepGuide.has(tex) )
		return false;
	int adj[2] = {pTex->getTidPrev(tex, isV), pTex->getTidNext(tex, isV)};
	bool fHit = false;
	for(int j=0; j<2; j++){
		int tex_j = adj[j];
		if( Tex_Man_t::NullTex()==tex_j ) // skip if no adj
			continue;
		if( StepGuide.has(tex_j) ) // already added because tex_j to tex connection
			return true;
	}
	return false;
}

// call before the step on tex is added to the StepGuide
void Rnd_Tcl_t::try_incStepLoad( int tex, Tex_Man_t * pTex ){
	if( StepGuide.has(tex) ) // load of this tex has been added
		return;
	
	for(int i=0; i<2; i++){
		bool isV = 0<i;
		int adj[2] = {pTex->getTidPrev(tex, isV), pTex->getTidNext(tex, isV)};
		bool fHit = false;
		for(int j=0; j<2; j++){
			int tex_j = adj[j];
			if( Tex_Man_t::NullTex()==tex_j )
				continue;
			if( ! StepGuide.has(tex_j) )
				continue;
			fHit = true; // tex should has dir load
			if( has_dirLoad(tex_j, isV, pTex) )
				continue;
			pTex->getTex(tex_j).inc_netLoad(isV);
		}
		pTex->getTex(tex).inc_netLoad(isV);
	}
}

// call after the step on tex is deleted from the StepGuide
void Rnd_Tcl_t::try_decStepLoad( int tex, Tex_Man_t * pTex ){
	if( StepGuide.has(tex) ) // load of this tex will be remained due to multiple walk of subnets on the tex
		return;
	for(int i=0; i<2; i++){
		bool isV = 0<i;
		int adj[2] = {pTex->getTidPrev(tex, isV), pTex->getTidNext(tex, isV)};
		bool fHit = false;
		for(int j=0; j<2; j++){
			int tex_j = adj[j];
			if( Tex_Man_t::NullTex()==tex_j )
				continue;
			if( ! StepGuide.has(tex_j) )
				continue;
			fHit = true; // tex should has dir load
			if( has_dirLoad(tex_j, isV, pTex) )
				continue;
			// tex is the only adj of tex_j, but tex has been deleted
			pTex->getTex(tex_j).dec_netLoad(isV);
		}
		pTex->getTex(tex).dec_netLoad(isV);
	}
}
