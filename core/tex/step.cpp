#include "route.hpp"
#include "round.hpp"
#include "utils/sys/sys.hpp"

#include <cmath> // exp(double)

const int Route_t::UnitStep() const {
	return (1<<StepOffset()) + StepBase();
}

int Route_t::step_cost( const Step_t& step_from, int to_index ) const {
	if( is_wire( step_from.index(), to_index ) )
		return step_from.step_cost();

	int BoundaryCost = 0;
	Tex_Man_t::Pos_t pos = _pTex->getPos(to_index);
	//if( _pTex->ynum()-1==pos._y || 0==pos._y )\
		BoundaryCost += 500;\
	else\
	if( _pTex->xnum()-1==pos._x || 0==pos._x )\
		BoundaryCost += 500;

	int RsmtCost = - is_rsmt( step_from.index(), to_index );
	int PRLCost  = 0;
	if( _pGdb->PrlFactor() )
		PRLCost = _pTex->getTex(to_index).isPRL2D()? 2: 0;
	return step_from.step_cost() +  UnitStep() + RsmtCost + PRLCost + BoundaryCost;
}
const Step_t::cost_t Route_t::turning_cost( const Step_t& step ) const {
	if( step.nstep()<2 )
		return 0;
	if( is_wire( step.src_index(), step.index() ) )
		return 0;
	int edge_prev = srcEid(step.src_index());
	int edge_cur  = _pTex->getTeid( step.src_index(), step.index() );

	//if( using_round()? _pRnd->using_post_route(): false )\
		return _pTex->is_veid(edge_cur) \
		!= _pTex->is_veid(edge_prev)? 100: 0;
	return _pTex->is_veid(edge_cur) 
	!= _pTex->is_veid(edge_prev)? (UnitStep()<<TurnOffset()): 0;
}

const Step_t::cost_t Route_t::pinPrefill_cost( const Step_t& step ) const {
	if( step.nstep()<1 )
		return 0;

	Tex_Man_t::Pos_t pos = _pTex->getPos(step.index());
	const Tex_t& tex = _pTex->getTex(step.index());
	int capV=_pTex->get_Cap( true, pos._x );
	int capH=_pTex->get_Cap(false, pos._y );
	int obsV = tex.obsLoad( true);
	int obsH = tex.obsLoad(false);
	float PenaltyTh = 0.5f;
	int PinPrefill = _pTex->getTex(step.index()).pinPrefill();
	bool fPenalty  = false;//(PenaltyTh <  (float)obsV/(capV+1)) || (PenaltyTh < (float)obsH/(capH+1));
	//if( fPenalty )\
		assert(!fPenalty);
	//if( is_wire( step.src_index(), step.index() ) )\
		return 0;
	//return (_pTex->getTex(step.index()).pinPrefill() * UnitStep()) << PinPrefillOffset();
	return (Step_t::cost_t) (int) (( PinPrefill+ ((PinPrefill && fPenalty)? UnitStep(): 0)) <<  PinPrefillOffset());
}

const int Route_t::srcEid( const Step_t& step ) const {
	return srcEid(step.index());
}

const int Route_t::srcEid( const int index ) const {
	const Tex_t& tex = _pTex->getTex(index);
	switch( stepSrc(index) ){
		case Tex_Man_t::Pos_t::UP   : return tex.vnext_eid();
		case Tex_Man_t::Pos_t::DOWN : return tex.vprev_eid();
		case Tex_Man_t::Pos_t::LEFT : return tex.hprev_eid();
		case Tex_Man_t::Pos_t::RIGHT: return tex.hnext_eid();
	}
	return lem::dev()? -1: 0;
}

const int Route_t::trackCap( const int eid ) const {
	return _pTex->trackCap(eid);
}

const Step_t::cost_t Route_t::cost( const int eid ) const {
	if( -1==eid && !lem::dev() )
		return 0;
	//if( is_wire( _pTex->Edge(eid).first, _pTex->Edge(eid).second ) )\
		return 0;
	int load= _pTex->getEdgeLoad(eid) + !is_wire( _pTex->Edge(eid).first, _pTex->Edge(eid).second );
	int cap = trackCap(eid);                 // 0 <= 0 
	cap = (float) cap * (using_round()? _pRnd->kPrefill(): _pGdb->Prefill2D());

	// obstruction cost
	if( 0==cap )\
		return 1<<CapzOffset();
	
	// rescue mode
	if( using_round()? _pRnd->using_rescue(): false )
		if( cap < load )
			return 1<<OvflOffset();
	
	// post route
	if( using_round()? _pRnd->using_post_route(): false )
		return (Step_t::cost_t) ( load <= cap ? 0.1: 100000 ) * (load)/(1+cap);

	Step_t::cost_t cong_cost = (Step_t::cost_t) 150/(1+exp( 0.3*( cap- load ) ));
	float dah = using_round()? _pRnd->dah(eid): 0;
	return (Step_t::cost_t) (1 + dah ) * cong_cost;
//	if( cap < load )
//		return 1<<OvflOffset();
//	int ratio = (float) 10*(load/cap);
//	return 1<<(ratio>>1);
	//return (Step_t::cost_t) 1 + ( load <= cap ? 0.1: 100000 ) * load/(1+cap);
}
const Step_t::cost_t Route_t::cost( const Step_t& step ) const {
	if( step.isFirstStep() )
		return 0;
	int edge_between = _pTex->getTeid( step.src_index(), step.index() );
	return cost(edge_between);
}

float StepQueueCmptor_t::cost( const Step_t& step ) const {
	return pRoute()? pRoute()->cost(step): 0;
}
const Tex_Man_t * StepQueueCmptor_t::pTex() const { return _pStepQueue? _pStepQueue->argpk().pTex: _pTex; }
const Route_t * StepQueueCmptor_t::pRoute() const { return _pStepQueue? _pStepQueue->argpk().pRoute: _pRoute; }
const Tex_Man_t::Pos_t StepQueueCmptor_t::pos1() const { return _pStepQueue? _pStepQueue->argpk().pos1: _pos1; }
const Tex_Man_t::Pos_t StepQueueCmptor_t::pos2() const { return _pStepQueue? _pStepQueue->argpk().pos2: _pos2; }
const int StepQueueCmptor_t::StepOffset() const { return pRoute()? pRoute()->StepOffset(): 0; }
const int StepQueueCmptor_t::StepLimit () const { return pRoute()? pRoute()->StepLimit (): 0; }
const int StepQueueCmptor_t::PredOffset() const { return pRoute()? pRoute()->PredOffset(): 0; }
const int StepQueueCmptor_t::  UnitStep() const { return pRoute()? pRoute()->  UnitStep(): 1; }
const bool StepQueueCmptor_t::UseStepLimit() const { return pRoute()? pRoute()->fUseStepLimit(): true; }
const bool StepQueueCmptor_t::UseCost     () const { return pRoute()? pRoute()->fUseCost     (): true; }
const bool StepQueueCmptor_t::is_vstep( const Step_t& step ) const { return pRoute()? pRoute()->is_vstep(step): false; }
bool StepQueueCmptor_t::operator()( const Step_t& s1, const Step_t& s2 )const{
	bool fUseStepLimit = UseStepLimit();
	bool fUseCost      = UseCost     ();
	const int manh1 = ( pTex()->valid(pos2())? manh( pTex()->getPos(s1.index()), pos2() ) :0 );
	const int manh2 = ( pTex()->valid(pos2())? manh( pTex()->getPos(s2.index()), pos2() ) :0 );
	if( StepLimit() && fUseStepLimit ){
		const int predwl1 = s1.nstep() + manh1;
		const int predwl2 = s2.nstep() + manh2;
		if( predwl1 <= StepLimit() && StepLimit() < predwl2 )
			return true ;
		if( predwl2 <= StepLimit() && StepLimit() < predwl1 )
			return false;
		if( StepLimit() < predwl1  && StepLimit() < predwl2 )
			return predwl1 < predwl2;
	}
	const int pred1 = (manh1 * UnitStep())<<PredOffset();
	const int pred2 = (manh2 * UnitStep())<<PredOffset();
	const int dist1 = s1.step_cost() + pred1;
	const int dist2 = s2.step_cost() + pred2;

	if( fUseCost ){
		Step_t::cost_t c1 = s1.cost();
		Step_t::cost_t c2 = s2.cost();
		const int cost1 = dist1 + c1;
		const int cost2 = dist2 + c2;
		//printf("%d %d %f %f\n", cost1, cost2, c1, c2);
		if( cost1 != cost2 )\
			return cost1 < cost2;
	}

	//int dist1 = s1.step_cost() + ( pTex()->valid(pos2())? manh( pTex()->getPos(s1.index()), pos2() )<<StepOffset() :0 );
	//int dist2 = s2.step_cost() + ( pTex()->valid(pos2())? manh( pTex()->getPos(s2.index()), pos2() )<<StepOffset() :0 );
	if( dist1 != dist2 )
		return dist1 < dist2;
	if( pred1 != pred2 )
		return pred1 < pred2;
	//bool vstep1 = is_vstep(s1);\
	bool vstep2 = is_vstep(s2);\
	if( vstep1 != vstep2 )\
		return vstep1 > vstep2;
	return s1.index() > s2.index();
}


const bool Route_t::is_rsmt( int from_index, int to_index ) const {
	if( using_round() ){
		//return _pRnd->RndTcl(_tcl).RsmtGuide.has(from_index) && _pRnd->RndTcl(_tcl).RsmtGuide.has(to_index);
		return _pRnd->RndTcl(_tcl).RsmtGuide.has(to_index);
	}
	//std::cout<<"is_rsmt? "<< from_index <<","<< to_index<<": "<<  RsmtGuide.val(from_index) << RsmtGuide.val(to_index) <<"\n";
	//return RsmtGuide.val(from_index) && RsmtGuide.val(to_index);
	return RsmtGuide.val(to_index);
}
const bool Route_t::is_wire( int from_index, int to_index ) const {
	return _is_wire_tex(to_index);
}

const bool Route_t::_is_wire( int edge_between ) const {
	if( using_round() ){
		return _pRnd->RndTcl(_tcl).EdgeGuide.has(edge_between);
	}
	return WireGuide.val(edge_between);// && WireGuide.val(to_index);
}

const bool Route_t::_is_wire_tex( int step_index ) const {
	int eids[4] = 
		{ _pTex->getTex(step_index).hnext_eid() 
		, _pTex->getTex(step_index).vnext_eid() 
		, _pTex->getTex(step_index).hprev_eid() 
		, _pTex->getTex(step_index).vprev_eid() };
	return
		   ( ( eids[0]!=Tex_t::NullEdge() ) ? _is_wire(eids[0]): false )
		|| ( ( eids[1]!=Tex_t::NullEdge() ) ? _is_wire(eids[1]): false )
		|| ( ( eids[2]!=Tex_t::NullEdge() ) ? _is_wire(eids[2]): false )
		|| ( ( eids[3]!=Tex_t::NullEdge() ) ? _is_wire(eids[3]): false );
}