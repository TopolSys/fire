#include <math.h>
#include <unistd.h>
#include <iomanip>
#include <thread>

#include "db/db.hpp"
#include "round.hpp"

static void _RoundRouteTarget( Lem_Gdb_t * pGdb, Tex_Man_t * pTex, vPosPair_t& vPosPair, vInt_t& vOrder ){
	int nPosPair = 0;
	for(vTcl_t::iterator pTcl = pGdb->vTcl.begin(); pTcl!=pGdb->vTcl.end(); pTcl++)
		nPosPair += pTcl->vTegPair().size();
	vPosPair.resize(nPosPair);
	int nTcl = 0;
	nPosPair = 0;
	for(vTcl_t::iterator pTcl = pGdb->vTcl.begin(); pTcl!=pGdb->vTcl.end(); pTcl++, nTcl++)
		for(int i=0; i<pTcl->vTegPair().size(); i++)
			vPosPair[ nPosPair++ ] = PosPair_t(nTcl, i);
}


static void _RoundRouteOrder(Lem_Gdb_t * pGdb, Tex_Man_t * pTex, Round_t * pRnd, vPosPair_t& vPosPair, vInt_t& vOrder, float Prefill, bool fInitRoute, bool fPostRoute, bool fRescue, bool fSkipFreezed ){
	bool fAllRoute = false;
	int nJob = 0;
	int nIntrinsicOvflTotal = 0;
	//float Prefill = pRnd? pRnd->kPrefill(): pGdb->Prefill2D();
	for(int i=0; i<vPosPair.size(); i++){
		const Tcl_t& Tcl = pGdb->vTcl[vPosPair[i]._tcl];
		const int pair_index = vPosPair[i]._pair_index;
		int nOverflow     = pTex->num_overflow(Tcl.mTegTeid()[pair_index], Prefill);
		int nHardOverflow = pTex->num_overflow(Tcl.mTegTeid()[pair_index], 1);
		int nIntrinsicOvfl= 
			pTex->intrinsic_overflow(Tcl.vTexPos()[Tcl.vTegPair()[pair_index].first ], Prefill) +
			pTex->intrinsic_overflow(Tcl.vTexPos()[Tcl.vTegPair()[pair_index].second], Prefill);
		nHardOverflow -= nIntrinsicOvfl;
		nOverflow     -= nIntrinsicOvfl;
		nIntrinsicOvflTotal += nIntrinsicOvfl;
		int routed_len = Tcl.mTegPath()[pair_index].size();
		int base_len = Tcl.TegBaseLen(pair_index);
		//if( 0==length )\
			length = Tcl.TegBaseLen(pair_index);
		int nSoftOverflow = nOverflow - nHardOverflow;
		int score = (nHardOverflow<<18)+ (nSoftOverflow<<12) + std::max(routed_len, base_len);
		if( fInitRoute )
			score = -base_len;
		if( fPostRoute ){
			score = -base_len;// (float) 100000* routed_len/(1 + base_len);;//100*sqrt(routed_len)*routed_len/(1 + base_len);// - base_len;
		}
		vPosPair[i]._score    = score;
		vPosPair[i]._overflow = std::max(nOverflow,0);
		vPosPair[i]._hard_overflow = nHardOverflow;
		vPosPair[i]._nstep    = Tcl.mTegTeid()[pair_index].size();
		if( pRnd )
			vPosPair[i]._lenBox   = pRnd->lenBox(vPosPair[i]);
		if( Tcl.isFreezed(pair_index) && fSkipFreezed )
			continue;
		if( fPostRoute || vPosPair[i]._overflow || fInitRoute || (fAllRoute&&!fRescue) )
			nJob++;
	}
	//std::cout<<"intrinsic overflow = "<<nIntrinsicOvflTotal << std::endl;
	vOrder.clear();
	vOrder.resize( nJob );
	int OrderTop = 0;
	for(int i=0; i<vPosPair.size(); i++){
		const Tcl_t& Tcl = pGdb->vTcl[vPosPair[i]._tcl];
		const int pair_index = vPosPair[i]._pair_index;
		if( Tcl.isFreezed(vPosPair[i]._pair_index) && fSkipFreezed )
			continue;
		if( fPostRoute || vPosPair[i]._overflow || fInitRoute || (fAllRoute&&!fRescue) )
			vOrder[OrderTop++] = i;
	}
		
	std::sort( vOrder.begin(), vOrder.end(), vPosPairComptor_t(vPosPair) );
}


void Round_t::RoundRouteOrder( vPosPair_t& vPosPair, vInt_t& vOrder, bool fInitRoute ){
	_RoundRouteOrder(_pGdb,_pTex,this,vPosPair,vOrder,kPrefill(), fInitRoute, using_post_route(), using_rescue(), false );
}

void Round_t::RoundRouteTarget( vPosPair_t& vPosPair, vInt_t& vOrder ){
	_RoundRouteTarget(_pGdb,_pTex,vPosPair,vOrder);
}

void Lem_RoundRouteReport( Lem_Gdb_t * pGdb, Tex_Man_t * pTex, vPosPair_t& vPosPair, vInt_t& vOrder, float SoftOverflow ){
	_RoundRouteTarget(pGdb,pTex,vPosPair,vOrder);
	_RoundRouteOrder (pGdb,pTex,NULL,vPosPair,vOrder,SoftOverflow, false, false, false, false);
}

NetDeptor_t::NetDeptor_t( const Lem_Gdb_t * pGdb, const Round_t * pRnd, const vPosPair_t * pvPosPair )
:_pGdb(pGdb),_pvPosPair(pvPosPair),_pRnd(pRnd){}
bool NetDeptor_t::operator()(const int& n1, const int& n2) const {
	if( ! _pvPosPair || ! _pGdb )
		return false;
	const PosPair_t& pos1 = (*_pvPosPair)[n1];
	const PosPair_t& pos2 = (*_pvPosPair)[n2];
	if( pos1._tcl == pos2._tcl )
		return true;
	//Rect_t rect1 = (*_pvPosPair)[n1].box( _pGdb );
	//Rect_t rect2 = (*_pvPosPair)[n2].box( _pGdb );
	//// length constraint 
	return overlap(pos1._lenBox, pos2._lenBox);
}

std::ostream& operator<<(std::ostream& ostr, const PosPair_t& p ){
	using namespace std;
	ostr<<"tcl "<<setw(7)<<p._tcl <<", pid " <<setw(5)<< p._pair_index<<" overflow# "<<setw(6)<< p._overflow <<" step# "<<setw(9)<<p._nstep;
	return ostr;
}


bool  Round_t::CompressEnd () const {
	int prefillPercent     = floor(kPrefill    ()*100);
	int prefillPercentGoal = floor(kPrefillGoal()*100);
	return prefillPercent <= prefillPercentGoal;
}
int   Round_t::CompressFail() const {
	return num_net_overflow(kPrefill());
}

int  Round_t::tagFreezed(){
	int nFreeze = 0;
	float Prefill = kPrefill()+0.01;

	for(int i=0; i<_vPosPair.size(); i++){
		const int tcl        = _vPosPair[i]._tcl;
		const int pair_index = _vPosPair[i]._pair_index;
		if( 0==num_tex_overflow(tcl, pair_index, kPrefill()) )
			continue;
		if( _pGdb->vTcl[tcl].isFreezed(pair_index) )
			continue;
		_pGdb->vTcl[tcl].setFreeze(pair_index);
		nFreeze++;
	}
	return nFreeze;
}

void Round_t::report_overflow( float SoftOverflow ) const {

	if( 0<num_net_overflow(SoftOverflow) ){
		//std::cout<<"Overflow still exist after "<< RescueLim <<" rescue rounds"<<std::endl;
		vPosPair_t vPosPair;
		vInt_t vOrder;
		Lem_RoundRouteReport(_pGdb, _pTex, vPosPair, vOrder, SoftOverflow);
		std::cout<< vOrder.size() <<" Nets are "<< SoftOverflow*100 <<"\% full after round route: ";
		for(int i=0; i<vOrder.size() && i<5; i++){
			int index = vOrder[i];
			int tcl = vPosPair[index]._tcl;
			std::cout<<(0<i? ", ": "")<<_pGdb->vTcl[tcl].name();
		}
		std::cout<< " ... " << std::endl;
	}
}