#ifndef CORE_TEX_ROUTE_HPP
#define CORE_TEX_ROUTE_HPP

#include <set>

#include "lem_mem.hpp"
#include "lem_type.hpp"
#include "alg/rsmt/rsmt.hpp"
#include "core/tex/tex.hpp"
#include "core/tex/teg.hpp"
class Lem_Gdb_t;
class Tex_Man_t;
typedef std::vector<Rsmt_t> vRsmt_t;

// Step 
class Step_t {
public:
	typedef float cost_t;
	Step_t( int src_index, int index, int nstep, int step_cost, cost_t cost ):_src_index(src_index),_index(index),_nstep(nstep),_step_cost(step_cost),_cost(cost){}
	const Tex_t& getTex( Tex_Man_t * pTex ) const { return pTex->getTex(_index); }
	const int index() const { return _index; }
	const int nstep() const { return _nstep; }
	const int step_cost() const { return _step_cost; }
	const int src_index() const { return _src_index; }
	const cost_t cost() const { return _cost; }
	void addCost( cost_t val ){ _cost += val; }
	const bool isFirstStep() const { return NullSrc()==src_index(); }
	static Step_t init_step( int index, int nstep, int step_cost, cost_t cost ){
		return Step_t( NullSrc(), index, nstep, step_cost, cost);
	}
private:
	static int NullSrc(){ return -1 ;}
	int _index, _src_index;
	int _nstep; 	// steps from origin
	int _step_cost; // step related cost
	cost_t _cost;
};
inline std::ostream& operator<<( std::ostream& ostr, const Step_t& st ){
	return ostr<<"tex= "<<st.index()<<" nstep="<<st.nstep()<<" step_cost="<<st.step_cost()<<" all cost="<<st.cost();
}

class Route_t;
class StepQueue_t;
class StepQueueCmptor_t {
public:
	bool operator()( const Step_t& s1, const Step_t& s2 )const;
	struct argpk_t {
		Tex_Man_t * pTex;
		Tex_Man_t::Pos_t pos1, pos2;
		Route_t * pRoute;
		StepQueue_t * pStepQueue;
		argpk_t():pTex(NULL),pRoute(NULL),pStepQueue(NULL){}
	};
	StepQueueCmptor_t( StepQueue_t * pStepQueue ):_pTex(NULL),_pRoute(NULL),_pStepQueue(pStepQueue){}
	StepQueueCmptor_t( Tex_Man_t * pTex, const Tex_Man_t::Pos_t& pos1 ):_pTex(pTex), _pos1(pos1),_pRoute(NULL),_pStepQueue(NULL){}
	StepQueueCmptor_t( argpk_t argpk ){
		_pTex = argpk.pTex;
		_pos1 = argpk.pos1;
		_pos2 = argpk.pos2;
		_pRoute = argpk.pRoute;
		_pStepQueue = argpk.pStepQueue;
	}
	const Tex_Man_t * pTex() const ;
	const Route_t * pRoute() const ;
	const Tex_Man_t::Pos_t pos1() const ;
	const Tex_Man_t::Pos_t pos2() const ;
	const int StepOffset() const ;
	const int PredOffset() const ;
	const int StepLimit () const ;
	const int   UnitStep() const ;
	const bool UseStepLimit() const ;
	const bool UseCost     () const ;
	const bool is_vstep( const Step_t& step ) const ;
private:
	StepQueue_t * _pStepQueue;
	float cost( const Step_t& step ) const;
	Tex_Man_t * _pTex;
	Tex_Man_t::Pos_t _pos1, _pos2;
	Route_t * _pRoute;
};

class StepQueue_t: public std::set<Step_t,StepQueueCmptor_t,Lem_Alloc<Step_t> > {
//class StepQueue_t: public std::set<Step_t,StepQueueCmptor_t > {
public:
	StepQueue_t(): std::set<Step_t,StepQueueCmptor_t,Lem_Alloc<Step_t> >(StepQueueCmptor_t(this)){}
	//StepQueue_t(): std::set<Step_t,StepQueueCmptor_t >(StepQueueCmptor_t(this)){}
	const Step_t& front() const { return *begin(); }
	void push( const Step_t& step ){ insert(step); }
	Step_t pop(){ Step_t step = front(); erase(begin()); return step; }
	void reset( const StepQueueCmptor_t::argpk_t& argpk ){
		clear();
		_argpk = argpk;
		_argpk.pStepQueue = this;
		//StepQueueCmptor_t cmptor(_argpk);\
		dynamic_cast<std::set<Step_t,StepQueueCmptor_t>&>(*this) = std::set<Step_t,StepQueueCmptor_t>(cmptor);
	}
	const StepQueueCmptor_t::argpk_t& argpk()const{ return _argpk; };
private:
	StepQueueCmptor_t::argpk_t _argpk;
};

class StepRecord_t: public vLembit_t<2> {
public:
	typedef Tex_Man_t::Pos_t::DIR_t DIR_t;
	void stepFromTop   ( int index ){ stepFrom(index, Tex_Man_t::Pos_t::   UP);}
	void stepFromBottom( int index ){ stepFrom(index, Tex_Man_t::Pos_t:: DOWN);}
	void stepFromLeft  ( int index ){ stepFrom(index, Tex_Man_t::Pos_t:: LEFT);}
	void stepFromRight ( int index ){ stepFrom(index, Tex_Man_t::Pos_t::RIGHT);}
	void stepFrom      ( int index, char src ){ set(index, src); }
	void derive        ( Tex_Man_t * pTex, int src, int dst, vInt_t& vStep, vInt_t * pvTeid ){
		vStep.clear();
		if( pvTeid ) pvTeid->clear();
		int cur = dst;
		int top = 0;
		while( cur != src ){
			vStep.push_back(cur);

			const Tex_t& tex = pTex->getTex(cur);
			Tex_Man_t::Pos_t pos = pTex->getPos(cur);
			const int dir = val(cur);
			if( dir == Tex_Man_t::Pos_t::UP ){
				pos._y ++; // go back to up
				assert( -1!=tex.vnext_eid() );
				if( pvTeid ) pvTeid->push_back( tex.vnext_eid() );
			} else 
			if( dir == Tex_Man_t::Pos_t::DOWN  ){
				pos._y --; // go back to down
				assert( -1!=tex.vprev_eid() );
				if( pvTeid ) pvTeid->push_back( tex.vprev_eid() );
			} else 
			if( dir == Tex_Man_t::Pos_t::LEFT  ){
				pos._x --; // go back to left
				assert( -1!=tex.hprev_eid() );
				if( pvTeid ) pvTeid->push_back( tex.hprev_eid() );
			} else 
			if( dir == Tex_Man_t::Pos_t::RIGHT ){
				pos._x ++; // go back from right
				assert( -1!=tex.hnext_eid() );
				if( pvTeid ) pvTeid->push_back( tex.hnext_eid() );
			}
			cur = pTex->Pos2Index(pos);
		}
		vStep.push_back(cur);
	}
};
class RsmtGuide_t: public vLembit_t<1> {
public:
	void fillRect( Tex_Man_t * pTex, const Rect_t& rect, bool fOne=true ){
//		IntPair_t rangeY = Lem_CoveredTex( rect.rangeY(), pTex->vGridY() );
//		IntPair_t rangeX = Lem_CoveredTex( rect.rangeX(), pTex->vGridX() );
//		for(int j=rangeY.first; j<rangeY.second; j++){
//			int base = pTex->xnum() * j;
//			fill( base + rangeX.first, base + rangeX.second, fOne );
//		}
		for(int j=rect.p1.y; j<rect.p2.y; j++){
			int base = pTex->xnum() * j;
			fill( base + rect.p1.x, base + rect.p2.x, fOne );
		}
	}
};
class WireGuide_t: public vLembit_t<1> {
public:
	void fillLine( const vInt_t& vStep, bool fOne=true ){
		// assume vStep sorted in ascending order
		for(int i=0; i<vStep.size(); i++){
			int j;
			for(j=i; j<vStep.size()-1; j++){
				if( vStep[j]+1!=vStep[j+1] )
					break;
			}
			fill( vStep[i], vStep[j]+1, fOne );
			i=j;
		}
	}
};
class TexCost_t {
public:
	TexCost_t( Tex_Man_t * pTex ):_pTex(pTex){}
	float operator()( const Tex_Man_t::Pos_t& pos, bool passV ) const {
		const Tex_t& tex = _pTex->getTex(pos);
		int cap = passV? _pTex->get_Cap(passV,pos._x): _pTex->get_Cap(passV,pos._y);
		return 0;
	}
private:
	Tex_Man_t * _pTex;
};

class Round_t;
class Route_t {
public:
	// observer 
	void print_edge() const ;
	struct argpk_t {
		argpk_t( Lem_Gdb_t * pGdb, Tex_Man_t * pTex ):_pGdb(pGdb),_pTex(pTex),_StepOffset(0),_PredOffset(0),_CostOffset(0),_StepBase(0)
		,_pRnd(NULL),_fUseStepLimit(true), _fUseCost(true){}
		Lem_Gdb_t * _pGdb;
		Tex_Man_t * _pTex;
		Round_t * _pRnd;
		int _StepOffset;
		int _PredOffset;
		int _CostOffset;
		int _StepBase;
		bool _fUseStepLimit, _fUseCost;
	};
	struct roundarg_t {
		roundarg_t():_StepLimit(0){}
		int _StepLimit;
	};
public:
	Route_t( const argpk_t& argpk );
	Route_t( Lem_Gdb_t * pGdb, Tex_Man_t * pTex );
	
	int minlen_obs_cross( Tcl_t& Tcl );
	int trial_route( Tcl_t& Tcl );
	int round_route( int tcl, const Tex_Man_t::Pos_t& pos_src, const Tex_Man_t::Pos_t& pos_dst, vInt_t& Path, vInt_t& vTegTeid, int lenLim, bool fFreezed );

	const StepRecord_t::DIR_t stepSrc( const Step_t& step ) const { return stepSrc(step.index()); }
	const StepRecord_t::DIR_t stepSrc( const int index ) const { return (StepRecord_t::DIR_t) StepRecord.val(index); }
	const int srcEid( const Step_t& step ) const ;
	const int srcEid( const    int index ) const ;
	const int trackCap( const    int   eid ) const ; // capacity of an edge
	const Step_t::cost_t turning_cost( const Step_t& step ) const ;
	const Step_t::cost_t pinPrefill_cost( const Step_t& step ) const ;
	const Step_t::cost_t cost( const Step_t& step ) const ;
	const Step_t::cost_t cost( const      int eid ) const ;
	const bool is_vstep( const Step_t& step ) const ;
	const bool is_rsmt( int from_index, int to_index ) const ;
	const bool is_wire( int from_index, int to_index ) const ;
	const bool _is_wire( int edge_between ) const ;
	const bool _is_wire_tex( int step_index ) const ;

	const int UnitStep() const ;

	int accu_teid, max_teid, max_push, max_rsmt_improve, accu_rsmt_improve, max_manh_area;
	int peak_queue;
	lem::Clk_t _GuideClk;
	int StepBase  () const ;
	int StepOffset() const { return _StepOffset; }
	int PredOffset() const { return _PredOffset; }
	int StepLimit () const { return _StepLimit ; }
	int CapzOffset() const { return _CapzOffset; }
	int OvflOffset() const { return _OvflOffset; }
	int TurnOffset() const { return _TurnOffset; }
	int PinPrefillOffset() const { return _PinPrefillOffset; }
	bool fUseStepLimit() const { return _fUseStepLimit; }
	bool fUseCost     () const { return _fUseCost     ; }

	static const int NullIndex(){ return -1; }

	void demand_add_extern( const vInt_t& vTeid ){ demand_add(vTeid);}
private:

	int _trial_route( const Tex_Man_t::Pos_t& pos_src, const Tex_Man_t::Pos_t& pos_dst, vInt_t& Path, vInt_t& vTegTeid, int lenLim=0, bool fFreezed=false );
	bool using_round() const ;

	void _data_init();
	void _guide_init ( const Tcl_t& Tcl );
	void _guide_erase( const Tcl_t& Tcl, const vInt_t& vPath );
	void demand_add( const vInt_t& vTeid );
	void Stepped( const Step_t& step );
	Step_t NextStep();
	int try_push( const Step_t& step_from, const Tex_Man_t::Pos_t& from, const Tex_Man_t::Pos_t& to );
	int step_cost( const Step_t& step_from, int to_index ) const ;
	void push_step( const Step_t& step );
	void _ResetStepQueue( const Tex_Man_t::Pos_t& pos1, const Tex_Man_t::Pos_t& pos2, int lenLim=0, bool fFreezed=false );
	//bool order_ok( const Teg_t& t1, const Teg_t& t2 ) const ;
	bool order_ok( const Tex_Man_t::Pos_t& pos1, const Tex_Man_t::Pos_t& pos2 ) const ;
	
	/* change from net-to-net */
	StepQueue_t StepQueue;
	StepRecord_t StepRecord;
	RsmtGuide_t RsmtGuide;
	WireGuide_t WireGuide;
	TravTag_t _EdgeTrav;
	TravTag_t _TexLeave;
	int _StepLimit;

	/* change from round-to-round */
	int _StepOffset, _PredOffset, _CostOffset;
	int _StepBase;
	int _CapzOffset, _OvflOffset;
	int _TurnOffset, _PinPrefillOffset;

	bool _fUseStepLimit, _fUseCost;

	int _tcl;//, _pospair;
	Round_t * _pRnd; // optional 

	void para_init();
	Lem_Gdb_t * _pGdb;
	Tex_Man_t * _pTex;

};
void Lem_InitRoute( Lem_Gdb_t * pGdb, Tex_Man_t * pTex );
void Lem_InitRouteRace( Lem_Gdb_t * pGdb, Tex_Man_t * pTex );
#endif