#include <map>       // point to index in terminal parition
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "utils/sys/sys.hpp" // error log

#include "db/db.hpp"
#include "core/tex/tex.hpp"
#include "route.hpp"
#include "round.hpp"

#include "lem_mac.hpp"
#include "lem_type.hpp"

struct TegPairCmptor {
	const Tcl_t& _Tcl;
	TegPairCmptor( const Tcl_t& Tcl ): _Tcl(Tcl){}
	int dist( const int& index ) const {
		const Teg_t& t1 = _Tcl[_Tcl.vTegPair()[index].first ];
		const Teg_t& t2 = _Tcl[_Tcl.vTegPair()[index].second];
		return manh( Tex_Man_t::Pos_t(t1.gposX(), t1.gposY()), Tex_Man_t::Pos_t(t2.gposX(), t2.gposY()) );
	}
	bool operator()( const int& p1, const int& p2 ) const {
		return dist(p1) < dist(p2);
	}
};

int Route_t::minlen_obs_cross( Tcl_t& Tcl ){
	_guide_init (Tcl);
	if( Tcl.mTegPath().size() < Tcl.vTegPair().size() ){
		Tcl.mTegPath().clear();
		Tcl.mTegPath().resize( Tcl.vTegPair().size() );
	}
	if( Tcl.mTegTeid().size() < Tcl.vTegPair().size() ){
		Tcl.mTegTeid().clear();
		Tcl.mTegTeid().resize( Tcl.vTegPair().size() );
	}
	if( Tcl.TegBaseLen().size() < Tcl.vTegPair().size() ){
		Tcl.TegBaseLen().clear();
		Tcl.TegBaseLen().resize( Tcl.vTegPair().size(), 0 );
	}
	vInt_t vPath, vPathPrev;
	vInt_t vTeid, vTeidPrev;
	int accu_len = 0;
	
	if( Tcl.vTexPos().size()==1 ){
		int index = _pTex->Pos2Index(Tex_Man_t::Pos_t(Tcl[0].gposX(), Tcl[0].gposY()));
		vPath.push_back(index);
	} else {
		// RMST pin pair
		vInt_t vTegPairOrder( Tcl.vTegPair().size() );
		for(int i=0; i<Tcl.vTegPair().size(); i++)
			vTegPairOrder[i] = i;
		std::sort( vTegPairOrder.begin(), vTegPairOrder.end(), TegPairCmptor(Tcl) );
		//for(int i=0; i<Tcl.vTegPair().size(); i++){
		for(int i=0; i<vTegPairOrder.size(); i++){
			int pair_index = vTegPairOrder[i];
			//const Teg_t& t1 = Tcl[Tcl.vTegPair()[ pair_index ].first ];
			//const Teg_t& t2 = Tcl[Tcl.vTegPair()[ pair_index ].second];
			const TexPos_t& tp1 = Tcl.vTexPos()[ Tcl.vTegPair()[ pair_index ].first  ];
			const TexPos_t& tp2 = Tcl.vTexPos()[ Tcl.vTegPair()[ pair_index ].second ];
			//trial_route(t1, t2, Tcl.TegPath( pair_index ), Tcl.TegTeid( pair_index ) );

			_trial_route(tp1, tp2, Tcl.TegPath( pair_index ), Tcl.TegTeid( pair_index ) );
			Tcl.TegBaseLen()[pair_index] = Tcl.TegPath( pair_index ).size()-1;
			//std::cout<< Tcl.TegPath( pair_index ).size()<<std::endl;
//_GuideClk.start();
			WireGuide.fillLine(Tcl.TegTeid( pair_index ), 1);
//_GuideClk.check();
			vPathPrev.swap(vPath);
			Lem_Union(Tcl.TegPath( pair_index ), vPathPrev, vPath);
			vTeidPrev.swap(vTeid);
			Lem_Union(Tcl.TegTeid( pair_index ), vTeidPrev, vTeid);
			//Tcl.TegPath( pair_index ).clear();\
			Tcl.TegTeid( pair_index ).clear();
//			peak_queue = std::max((int)StepQueue.size(), peak_queue);
//			max_manh_area = std::max( abs(tp1._x-tp2._x)*abs(tp1._y-tp2._y), max_manh_area);
//			accu_len  += Tcl.TegTeid( pair_index ).size();
		}
	}
	_guide_erase(Tcl, vTeid);
//	accu_teid += vTeid.size();
//	max_teid = std::max( max_teid, (int) vTeid.size() );
//	max_rsmt_improve = std::max( max_rsmt_improve,  accu_len - (int)vTeid.size() );
//	accu_rsmt_improve += accu_len - vTeid.size();
//
//	//std::cout<<Tcl.size()<<", "<< vTeid.size() <<", "<< StepQueue.size() <<", "<<  (accu_len - (int)vTeid.size() ) <<std::endl;
	Tcl.vPath().swap( vPath );
	Tcl.vTeid().swap( vTeid );
	return 1;
}

int Route_t::round_route(int tcl, const Tex_Man_t::Pos_t& pos_src, const Tex_Man_t::Pos_t& pos_dst, vInt_t& Path, vInt_t& vTegTeid, int LenLim, bool fFreezed){
	if( lem::dev() )
		assert( _pRnd );
	_tcl = tcl;
	_trial_route(pos_src, pos_dst, Path, vTegTeid, LenLim, fFreezed);
}
int Route_t::trial_route( Tcl_t& Tcl ){
	_guide_init (Tcl);
	if( Tcl.mTegPath().size() < Tcl.vTegPair().size() ){
		Tcl.mTegPath().clear();
		Tcl.mTegPath().resize( Tcl.vTegPair().size() );
	}
	if( Tcl.mTegTeid().size() < Tcl.vTegPair().size() ){
		Tcl.mTegTeid().clear();
		Tcl.mTegTeid().resize( Tcl.vTegPair().size() );
	}
	vInt_t vPath, vPathPrev;
	vInt_t vTeid, vTeidPrev;
	int accu_len = 0;
	
	if( Tcl.vTexPos().size()==1 ){
		int index = _pTex->Pos2Index(Tex_Man_t::Pos_t(Tcl[0].gposX(), Tcl[0].gposY()));
		vPath.push_back(index);
	} else {
		// RMST pin pair
		vInt_t vTegPairOrder( Tcl.vTegPair().size() );
		for(int i=0; i<Tcl.vTegPair().size(); i++)
			vTegPairOrder[i] = i;
		std::sort( vTegPairOrder.begin(), vTegPairOrder.end(), TegPairCmptor(Tcl) );
		//for(int i=0; i<Tcl.vTegPair().size(); i++){
		for(int i=0; i<vTegPairOrder.size(); i++){
			int pair_index = vTegPairOrder[i];
			//const Teg_t& t1 = Tcl[Tcl.vTegPair()[ pair_index ].first ];
			//const Teg_t& t2 = Tcl[Tcl.vTegPair()[ pair_index ].second];
			const TexPos_t& tp1 = Tcl.vTexPos()[ Tcl.vTegPair()[ pair_index ].first  ];
			const TexPos_t& tp2 = Tcl.vTexPos()[ Tcl.vTegPair()[ pair_index ].second ];
			//trial_route(t1, t2, Tcl.TegPath( pair_index ), Tcl.TegTeid( pair_index ) );
			_trial_route(tp1, tp2, Tcl.TegPath( pair_index ), Tcl.TegTeid( pair_index ) );
_GuideClk.start();
			WireGuide.fillLine(Tcl.TegTeid( pair_index ), 1);
_GuideClk.check();
			vPathPrev.swap(vPath);
			Lem_Union(Tcl.TegPath( pair_index ), vPathPrev, vPath);
			vTeidPrev.swap(vTeid);
			Lem_Union(Tcl.TegTeid( pair_index ), vTeidPrev, vTeid);
			peak_queue = std::max((int)StepQueue.size(), peak_queue);
			max_manh_area = std::max( abs(tp1._x-tp2._x)*abs(tp1._y-tp2._y), max_manh_area);
			accu_len  += Tcl.TegTeid( pair_index ).size();
		}
	}
//	for(int i=0; i<vTeid.size(); i++){\
		assert(WireGuide.val(vTeid[i]));\
	}
	_guide_erase(Tcl, vTeid);
	demand_add(vTeid);
	accu_teid += vTeid.size();
	max_teid = std::max( max_teid, (int) vTeid.size() );
	max_rsmt_improve = std::max( max_rsmt_improve,  accu_len - (int)vTeid.size() );
	accu_rsmt_improve += accu_len - vTeid.size();

	//std::cout<<Tcl.size()<<", "<< vTeid.size() <<", "<< StepQueue.size() <<", "<<  (accu_len - (int)vTeid.size() ) <<std::endl;
	Tcl.vPath().swap( vPath );
	Tcl.vTeid().swap( vTeid );
	return 1;
}
struct TexPosCmp_t {
	bool operator()( const TexPos_t& p1, const TexPos_t& p2 ) const {
		return p1._x != p2._x? p1._x < p2._x: p1._y < p2._y;
	}
};
bool operator!=( const TexPos_t& p1, const TexPos_t& p2 ){
	return p1._x != p2._x || p1._y != p2._y;
}
void Lem_Tcl2TexPos( Tcl_t& Tcl ){
	vTexPos_t vTexPos;
	Lem_Iterate( Tcl, pPin )
		vTexPos.push_back( TexPos_t(pPin->gposX(), pPin->gposY()) );
	std::sort( vTexPos.begin(), vTexPos.end(), TexPosCmp_t() );
	Tcl.vTexPos().clear();
	for(int i=0; i<vTexPos.size(); i++)
		if( i>0? vTexPos[i]!=vTexPos[i-1]: true )
			Tcl.vTexPos().push_back( vTexPos[i] );
}

void Lem_InitRoute( Lem_Gdb_t * pGdb, Tex_Man_t * pTex ){
	int Lem_NetDecomp( Tcl_t& Tcl, vIntPair_t& vPinPair );
	int Lem_NetSteinerPoint( Tcl_t& Tcl, Rsmt_t& rsmt );

	std::cout<<"InitRoute"<<std::endl;
	Route_t::argpk_t argpk(pGdb,pTex);
	argpk._StepBase   = 200;
	argpk._StepOffset = 5;
	argpk._PredOffset = 0;
	argpk._CostOffset = 0;
	Route_t route(argpk);

	lem::Progress_t<int> prog( 50, pGdb->vTcl.size());
	int nNet = 0;
	for(vTcl_t::iterator pTcl = pGdb->vTcl.begin(); pTcl!=pGdb->vTcl.end(); pTcl++, nNet++){
		if( 0==(nNet%10) || pTcl==--pGdb->vTcl.end())
			prog.set(nNet+1).print(std::cout);
		Lem_Tcl2TexPos( *pTcl );
		Lem_NetDecomp( *pTcl, pTcl->vTegPair() );
		if( ! Lem_NetSteinerPoint( *pTcl, pTcl->Rsmt() ) ){
			char buff[1024];
			sprintf(buff, "net_id = %d steiner tree failure",nNet);
			lem::error.add(buff);
			pTcl->notOk();
			continue;
		}
		if(!route.trial_route(*pTcl))
			std::cout<<"X"<<std::endl;
	}
	std::cout<<std::endl;
	std::cout<<std::setw(20)<<std::left<<          " max teid"<<" = "<<std::right<<std::setw(10)<< route. max_teid<<std::endl;
	std::cout<<std::setw(20)<<std::left<<          "accu teid"<<" = "<<std::right<<std::setw(10)<< route.accu_teid <<std::endl;
	std::cout<<std::setw(20)<<std::left<<         "peak queue"<<" = "<<std::right<<std::setw(10)<< route.peak_queue<<std::endl;
	std::cout<<std::setw(20)<<std::left<<          " max push"<<" = "<<std::right<<std::setw(10)<< route. max_push<<std::endl;
	std::cout<<std::setw(20)<<std::left<<     " max manh area"<<" = "<<std::right<<std::setw(10)<< route. max_manh_area<<std::endl;
	std::cout<<std::setw(20)<<std::left<<  " max rsmt improve"<<" = "<<std::right<<std::setw(10)<< route. max_rsmt_improve<<std::endl;
	std::cout<<std::setw(20)<<std::left<<  "accu rsmt improve"<<" = "<<std::right<<std::setw(10)<< route.accu_rsmt_improve<<std::endl;
	std::cout<<std::setw(20)<<std::left<< "guide elapsed time"<<" = "<<std::right<<std::setw(10)<< route._GuideClk.elapsed()<<std::endl;
	pTex->print_edge();
}
