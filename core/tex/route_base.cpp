#include <map>       // point to index in terminal parition
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "db/db.hpp" // Lem_Gdb_t
#include "utils/sys/sys.hpp" // error log

#include "alg/rmst/rmst.hpp"
#include "alg/rsmt/rsmt.hpp"
#include "route.hpp"
#include "round.hpp"

#include "lem_mac.hpp"
#include "lem_type.hpp"


void Route_t::para_init(){
	_pGdb = NULL;
	_pTex = NULL;
	_pRnd = NULL;

	accu_teid = 0;
	accu_rsmt_improve = 0;
	peak_queue= 0;
	max_teid  = 0;
	max_push = 0;
	max_rsmt_improve = 0;
	max_manh_area =  0;

	_StepBase   =  0;
	_StepOffset =  0;
	_PredOffset =  0;
	_CostOffset =  0;

	_CapzOffset = 22;
	_OvflOffset = 18;
	_TurnOffset = 3;
	_PinPrefillOffset = 0;

	_tcl = NullIndex();
	//_pospair = NullIndex();
	_fUseStepLimit = true;
	_fUseCost      = true;
}
void Route_t::_data_init(){
	if( _EdgeTrav.size() < _pTex->Edge().size() )
		_EdgeTrav.init( _pTex->Edge().size() );
	if( _TexLeave.size() < _pTex->data().size() )
		_TexLeave.init( _pTex->data().size() );
}
Route_t::Route_t( const argpk_t& argpk ){
	para_init();
	_pGdb = argpk._pGdb;
	_pTex = argpk._pTex;
	_pRnd = argpk._pRnd;
	_StepBase   = argpk._StepBase  ;
	_StepOffset = argpk._StepOffset;
	_PredOffset = argpk._PredOffset;
	_CostOffset = argpk._CostOffset;
	_fUseStepLimit= argpk._fUseStepLimit;
	_fUseCost     = argpk._fUseCost     ;
	_data_init();
}
Route_t::Route_t( Lem_Gdb_t * pGdb, Tex_Man_t * pTex ){
	para_init();
	_pGdb = pGdb;
	_pTex = pTex;
	_data_init();
}

int Lem_NetDecomp( Tcl_t& Tcl, vIntPair_t& vPinPair ){
	bool fUnique = false;
	Rmst_t rmst;
	vPoint_t vPoint;
	Lem_Iterate( Tcl.vTexPos(), pPos )
		vPoint.push_back( Point_t(pPos->_x, pPos->_y ) );
	rmst.addPoints( vPoint );

	rmst.updateGraph();
	rmst.rmst();
	//std::cout<<rmst;

	vPinPair.swap( rmst.vPinPair );
}

const bool Route_t::is_vstep( const Step_t& step ) const {
	if( step.isFirstStep() )
		return false;
	int teid = _pTex->getTeid(step.index(), step.src_index());
	if( lem::dev() )
		assert( teid != Tex_t::NullEdge() );
	return _pTex->is_veid(teid);
}

struct PointCmp_t {
	bool operator()( const Point_t& p1, const Point_t& p2 ) const {
		return p1.x != p2.x? p1.x < p2.x: p1.y < p2.y;
	}
};
typedef std::map<Point_t,int,PointCmp_t> Point2Int_t;

int Lem_NetSteinerPoint( Tcl_t& Tcl, Rsmt_t& rsmt ){
	vPoint_t vPoint;
	Lem_Iterate( Tcl.vTexPos(), pPos )
		vPoint.push_back( Point_t(pPos->_x, pPos->_y ) );
	rsmt.addPoints( vPoint );

	int cluster_size = Rsmt_t::node_limit();
	if( cluster_size < Tcl.vTexPos().size() ){
		
		// point to index map
		Point2Int_t Point2Int;
		for(int i=0; i<vPoint.size(); i++)
			Point2Int[ vPoint[i] ] = i;
		int nPoint = Point2Int.size();

		mInt_t vCluster;
		Lem_TerminalParition( cluster_size, vPoint, Tcl.vTegPair(), vCluster );
		for(int i=0; i<vCluster.size(); i++){
			if( vCluster[i].size() < 2 )
				continue;
			vPoint_t vPointNext( vCluster[i].size() );
			for(int j=0; j<vCluster[i].size(); j++)
				vPointNext[j] = vPoint[ vCluster[i][j] ];
			Rsmt_t rsmtj;
			rsmtj.addPoints(vPointNext);
			if( ! rsmtj.rsmt() ){
				lem::error.add("after terminal parition");
				return 0;
			}
			// interpret edge
			for(int j=0; j<rsmtj.vEdge().size(); j++){
				Point_t p1 = rsmtj.vPoint()[ rsmtj.vEdge()[j].first ];
				Point_t p2 = rsmtj.vPoint()[ rsmtj.vEdge()[j].second];

				if( Point2Int.find(p1) == Point2Int.end() )
					Point2Int[p1] = nPoint++;

				if( Point2Int.find(p2) == Point2Int.end() )
					Point2Int[p2] = nPoint++;

				int index1 = Point2Int[p1];
				int index2 = Point2Int[p2];
				if( index2 < index1 )
					std::swap( index1, index2 );
				rsmt.vEdge().push_back( IntPair_t( index1, index2 ) );
			}
		}

		// update points of rsmt here  *******************************************
		for(Point2Int_t::iterator itr=Point2Int.begin(); itr!=Point2Int.end(); itr++)
			rsmt.addPoint(itr->first);

//		std::cout<<"("<<vCluster.size()<<": ";
//		for(int i=0; i<vCluster.size(); i++)
//			print_member(std::cout,vCluster[i],6);
//		std::cout<<")"<<std::endl;
		return 1;
	}
	
	rsmt.addPoints( vPoint );
	if( ! rsmt.rsmt() )
		return 0;
	return 1;
}


void Route_t::_ResetStepQueue( const Tex_Man_t::Pos_t& pos1, const Tex_Man_t::Pos_t& pos2, int lenLim, bool fFreezed ){
	_pTex->travId.step();
	StepQueue.clear();
	StepRecord.clear();
	if( StepRecord.size() < _pTex->data().size() )
		StepRecord.resize( _pTex->data().size() );

	StepQueueCmptor_t::argpk_t argpk;
	argpk.pTex = _pTex;
	argpk.pos1 = pos1;
	argpk.pos2 = pos2;
	argpk.pRoute = this;
	StepQueue.reset(argpk);
	Step_t step = Step_t::init_step( _pTex->Pos2Index(pos1), 0, 0/* step_cost*/, 0 /* cost_t */ );
	push_step( step );

	_StepLimit = 0<lenLim? lenLim: manh(pos1, pos2);
	if( using_round() && !fFreezed )
		_StepLimit = (float) _StepLimit * _pRnd->lenRatio();
}

void Route_t::push_step( const Step_t& step ){
	//Step_t step( _pTex->Pos2Index(pos) );
	//Tex_t& Tex = _pTex->getTex( step.index() );
	StepQueue.push( step );
	//Tex.travId.set( _pTex->travId.current() );
}

int Route_t::try_push( const Step_t& step_from, const Tex_Man_t::Pos_t& from, const Tex_Man_t::Pos_t& to ){
	if( ! _pTex->valid(to) )
		return 0;
//	if( _pTex->travId.current() == _pTex->getTex(to).travId.current() )\
		return 0;
	int to_index = _pTex->Pos2Index(to);
	if( _TexLeave.is_cur(to_index) )
		return 0;
	if( step_from.src_index() == to_index )
		return 0;
	//std::cout<<step_from.src_index() <<" , "<< to_index <<")) ";\
	std::cout.flush();
	int edge_between = _pTex->getTeid( step_from.index(), to_index );
	if( _EdgeTrav.is_cur(edge_between) )
		return 0;
	//_EdgeTrav.set(edge_between);
	//StepRecord.set( to_index, to.dir_from(from) ); // record the direction of from-to
	Step_t step( step_from.index(), to_index, step_from.nstep()+1, step_cost(step_from, to_index), step_from.cost() + cost(edge_between) /* + cost */);
	step.addCost( turning_cost(step) );
	step.addCost( pinPrefill_cost(step) );
	push_step( step );
	return 1;
}

void Route_t::Stepped( const Step_t& step ){
	if( step.isFirstStep() )
		return;
	int to_index = step.index();
	int src_index= step.src_index();
	const Tex_Man_t::Pos_t& to   = _pTex->getPos( to_index);
	const Tex_Man_t::Pos_t& from = _pTex->getPos(src_index);
	int edge_between = _pTex->getTeid( src_index, to_index );
	StepRecord.set( to_index, to.dir_from(from) ); // record the direction of from-to
	_EdgeTrav.set( edge_between );
	_TexLeave.set( src_index );
}

//bool Route_t::order_ok( const Teg_t& t1, const Teg_t& t2 ) const {
//	Tex_Man_t::Pos_t pos1( t1.gposX(), t1.gposY() );
//	Tex_Man_t::Pos_t pos2( t2.gposX(), t2.gposY() );
bool Route_t::order_ok( const Tex_Man_t::Pos_t& pos1, const Tex_Man_t::Pos_t& pos2 ) const {
	return _pTex->Pos2Index(pos1) < _pTex->Pos2Index(pos2);
}

Step_t Route_t::NextStep(){
	while( ! StepQueue.empty() ){
		Step_t step = StepQueue.pop();
		if( !_TexLeave.is_cur(step.index()) )
			return step;
	}
	if( lem::dev() )
		assert( false );
	return Step_t(-1,-1,0,0,0);
}

//int Route_t::trial_route( const Teg_t& tsrc, const Teg_t& tdst, vInt_t& Path, vInt_t& vTegTeid ){
int Route_t::_trial_route( const Tex_Man_t::Pos_t& pos_src, const Tex_Man_t::Pos_t& pos_dst, vInt_t& Path, vInt_t& vTegTeid, int lenLim, bool fFreezed ){
	Path.clear();
	vTegTeid.clear();
	//const Teg_t& t1 = order_ok(tsrc, tdst)? tsrc: tdst;
	//const Teg_t& t2 = order_ok(tsrc, tdst)? tdst: tsrc;
	Tex_Man_t::Pos_t pos1 = order_ok(pos_src, pos_dst)? pos_src: pos_dst;
	Tex_Man_t::Pos_t pos2 = order_ok(pos_src, pos_dst)? pos_dst: pos_src;
	_ResetStepQueue(pos1,pos2,lenLim,fFreezed);
	if( pos1==pos2 ){
		Path.push_back( _pTex->Pos2Index(pos1) );
		return 1;
	}

	_EdgeTrav.inc();
	_TexLeave.inc();
	int nPush = 0;
	bool fHit = false;
	while( ! StepQueue.empty() ){
		//Step_t step = StepQueue.pop();
		Step_t step = NextStep();
		Stepped( step );
	//	std::cout<<"?"<<std::endl;
		Tex_Man_t::Pos_t pos = _pTex->getPos( step.index() );
		//std::cout<<"("<<step.nstep()<<"+"<<manh(pos,pos2)<<"="<<(step.nstep()+manh(pos,pos2))<<") ";
		if( pos==pos2 ){
			// collect path

		//std::cout<<"go derive"<<std::endl;
			StepRecord.derive(_pTex, _pTex->Pos2Index(pos1), _pTex->Pos2Index(pos2), Path, &vTegTeid);
		//	std::cout<<"leave derive"<<std::endl;
			//print_member(std::cout,Path);\
			_pTex->print_teid(std::cout,vTeid);\
			std::cout<<std::endl;
			//std::cout<< step.nstep()<<","<<step.cost()<<std::endl;
			std::sort(Path.begin(), Path.end());
			std::sort(vTegTeid.begin(), vTegTeid.end());
			fHit = true;
			break;
		}
		nPush += try_push( step, pos, pos.up   () );
		nPush += try_push( step, pos, pos.down () );
		nPush += try_push( step, pos, pos.left () );
		nPush += try_push( step, pos, pos.right() );
	}

	//std::cout<<"?"<<std::endl;
//std::cout<<"\n";\
std::cout<<"\n";
	max_push  = std::max(nPush, max_push);
	return fHit;
}

void Route_t::_guide_init ( const Tcl_t& Tcl ){
	if( RsmtGuide.vec().size() < _pTex->data().size() ){
		RsmtGuide.clear();
		RsmtGuide.resize( _pTex->data().size() );
	}
	if( WireGuide.vec().size() < _pTex->Edge().size() ){
		WireGuide.clear();
		WireGuide.resize( _pTex->Edge().size() );
	}
_GuideClk.start();

	//for(int i=0; i<RsmtGuide.vec().size(); i++)\
		assert(RsmtGuide.vec()[i]==0);
	Lem_Iterate(Tcl.Rsmt().vEdge(), pEdge){\
		Rect_t rect( Tcl.Rsmt().vPoint()[pEdge->first], Tcl.Rsmt().vPoint()[pEdge->second] );\
		Rect_t box = rect;
		box.p2.x++;
		box.p2.y++;
		RsmtGuide.fillRect(_pTex,box,1);
		for(int j=rect.p1.x; j<rect.p2.x; j++){
			int index1 = _pTex->Pos2Index( Tex_Man_t::Pos_t( j, rect.p1.y ) );
			int index2 = _pTex->Pos2Index( Tex_Man_t::Pos_t( j, rect.p2.y ) );
			//std::cout<<"index1="<<index1<<": " << RsmtGuide.val(index1)<<", index2="<<index2<<": "<<RsmtGuide.val(index2)<<"\n";
			if( lem::dev() ){
				assert(RsmtGuide.val(index1) && RsmtGuide.val(index2));
				assert(0 <= index1 && index1 < _pTex->data().size() );\
				assert(0 <= index2 && index2 < _pTex->data().size() );
			}
		}
		for(int j=rect.p1.y; j<rect.p2.y; j++){
			int index1 = _pTex->Pos2Index( Tex_Man_t::Pos_t( rect.p1.x, j ) );
			int index2 = _pTex->Pos2Index( Tex_Man_t::Pos_t( rect.p2.x, j ) );
			if( lem::dev() ){
				assert(RsmtGuide.val(index1));
				assert(RsmtGuide.val(index2));
				assert(0 <= index1 && index1 < _pTex->data().size() );\
				assert(0 <= index2 && index2 < _pTex->data().size() );
			}
		}
	}
_GuideClk.check();

	
	
	for(int i=0; i<WireGuide.vec().size(); i++)\
		assert(WireGuide.vec()[i]==0);
}

void Route_t::_guide_erase( const Tcl_t& Tcl, const vInt_t& vTeid ){
_GuideClk.start();
	Lem_Iterate(Tcl.Rsmt().vEdge(), pEdge){\
		Rect_t rect( Tcl.Rsmt().vPoint()[pEdge->first], Tcl.Rsmt().vPoint()[pEdge->second] );\
		RsmtGuide.fillRect(_pTex,rect,0);\
	}
	WireGuide.fillLine(vTeid, 0);
_GuideClk.check();
	//for(int i=0; i<WireGuide.vec().size(); i++)\
		assert(WireGuide.vec()[i]==0);
}

void Route_t::demand_add( const vInt_t& vTeid ){
	for(int i=0; i<vTeid.size(); i++)
		_pTex->Edge(vTeid[i]).incLoad();
}

bool Route_t::using_round() const {
	bool has_index = _tcl!=NullIndex();// && _pospair!=NullIndex();
	bool has_round = _pRnd != NULL;
//	if( lem::dev() && ret ){
//		assert( _tcl     < _pGdb->vTcl.size() );
//		assert( _pospair < _pGdb->vTcl[_tcl].vTegPair().size() );
//	}
	return has_round && has_index;
}

int Route_t::StepBase  () const { return using_round()? _pRnd->StepBase(): _StepBase  ; }