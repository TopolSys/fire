#ifndef CORE_TEX_ROUND_HPP
#define CORE_TEX_ROUND_HPP

#include <unordered_set>


#include "db/db.hpp"
#include "lem_type.hpp"
#include "core/tex/route.hpp"
#include "utils/sys/sys.hpp"
#include "utils/sys/thread.hpp"

class PathGuide_t: private std::unordered_multiset<int> {
public:
	void add   ( const vInt_t& );
	void add   ( const int& );
	void remove( const vInt_t& );
	void remove( const int& );
	bool has   ( int ) const ;
	int size_() const { return size(); }
	int count( int val ) const {
		std::pair<const_iterator,const_iterator> range;
		range = equal_range(val);
		int num=0;
		for(; range.first!=range.second; range.first++)
			num++;
		return num;
	}
	bool unique( int val ) const {
		std::pair<const_iterator,const_iterator> range;
		range = equal_range(val);
		int num=0;
		for(; range.first!=range.second && num < 2; range.first++)
			num++;
		return 1==num;
	}
	void member2univec( vInt_t& vec ) const {
		vec.clear();
		if( empty() )
			return;
		const_iterator itr =begin();
		const_iterator prev=begin();
		vec.push_back(*itr);
		for(itr++; itr!=end(); prev=itr++)
			if( *itr != *prev )
				vec.push_back(*itr);
	}
};

class Skeleton_t: private std::unordered_set<int> {
public:
	void add   ( const int& );
	void remove( const int& );
	bool has   ( int ) const ;
	int size_() const { return size(); }
};
class Rnd_Tcl_t {
public:
	Skeleton_t  RsmtGuide;
	PathGuide_t StepGuide;
	PathGuide_t EdgeGuide;
	void try_incStepLoad( int tex, Tex_Man_t * );
	void try_decStepLoad( int tex, Tex_Man_t * );
	bool has_dirLoad(int tex, bool isV, Tex_Man_t * pTex) const ;
};


struct PosPair_t {
	int _tcl, _pair_index, _score, _overflow, _nstep, _hard_overflow;
	Rect_t _lenBox;
	PosPair_t(int tcl=-1, int pair_index=-1, int score=0):_tcl(tcl),_pair_index(pair_index),_score(score),_overflow(0), _nstep(0),_hard_overflow(0){}
	struct Comptor {
		bool operator()( const PosPair_t& p1, const PosPair_t& p2 ) const {
			return p1._score > p2._score;
		}
	};
	Rect_t lenBox() const {return _lenBox; }
	Rect_t box( const Lem_Gdb_t * pGdb ) const {
		int p1 = pGdb->vTcl[_tcl].vTegPair()[_pair_index].first ;
		int p2 = pGdb->vTcl[_tcl].vTegPair()[_pair_index].second;
		return Rect_t( pGdb->vTcl[_tcl].vTexPos()[p1].point(), pGdb->vTcl[_tcl].vTexPos()[p2].point() );
	}
};

typedef std::vector<PosPair_t> vPosPair_t;

struct vPosPairComptor_t {
	const vPosPair_t * _pvPosPair;
	vPosPairComptor_t( const vPosPair_t& vPosPair ):_pvPosPair(&vPosPair){}
	bool operator()( const int& index1, const int& index2 ) const {
		return PosPair_t::Comptor()( (*_pvPosPair)[index1], (*_pvPosPair)[index2] );
	}
};
class Round_t;
struct NetDeptor_t: public lem::Deptor_t {
	const Round_t * _pRnd;
	const Lem_Gdb_t * _pGdb;
	const vPosPair_t * _pvPosPair;
	NetDeptor_t( const Lem_Gdb_t * pGdb=NULL, const Round_t* = NULL, const vPosPair_t * pvPosPair=NULL );
	bool operator()(const int& n1, const int& n2) const ;
};
typedef std::vector<Rnd_Tcl_t> vRndTcl_t;
typedef std::vector<Route_t*> vRoutePtr_t;
class Lem_Gdb_t;
class Tex_Man_t;
class Round_t {
public:
	Round_t( Lem_Gdb_t * pGdb, Tex_Man_t * pTex );
	~Round_t();
	int  compress();
	int  route();
	int  post_route();
	void report_overflow( float SoftOverflow ) const ;
	int  StepBase() const { return _StepBase; }
	const Rnd_Tcl_t& RndTcl( int tcl ) const { return _vRndTcl[tcl]; }
	void ripup( const PosPair_t& pospair );
	void demand_add( const PosPair_t& pospair );
	const int round() const { return _k; }
	float dah( int teid ) const ;
	float lenRatio() const ;
	//Rect_t lenBox( const Rect_t& ) const ;
	Rect_t lenBox( const PosPair_t& pospair ) const ;
	int  num_net_overflow( float SoftOverflow ) const ;
	int  num_tex_overflow( int tcl, int pair_index, float SoftOverflow ) const ;
	void set_as_base_length( const bool fOnlyFreezed );
	void round_reset(){ _k = 0; }
	//bool overflow( int teid ) const ;

	//int rescue();
	bool using_rescue() const { return _fRescue; }
	void set_rescue() { _fRescue = true ; }
	void del_rescue() { _fRescue = false; }
	bool using_post_route() const { return _fPostRoute; }
	void set_post_route() { _fPostRoute = true ; }
	void del_post_route() { _fPostRoute = false; }

	float kPrefill() const { return _kPrefill; }
	float kPrefillGoal() const { return _kPrefillGoal; }
	float kPrefillStep() const { return _kPrefillStep; }
	void  setCompress (float start, float goal, float step_size) ;
	bool  CompressEnd () const ;
	int   CompressFail() const ;
	bool  isFreezeWireLength() const { return _FreezeWireLength; }
	double minimal_prefill_rate() const ;
	int   total_teid() const ;
private:
	int  route_on(Route_t& route, const PosPair_t&);
	int  race_router(int id);
	void RoundRouteOrder (vPosPair_t& vPosPair, vInt_t& vOrder, bool fInitRoute );
	void RoundRouteTarget(vPosPair_t& vPosPair, vInt_t& vOrder );
	int  add_initial_demand();
	void para_init();
	const vInt_t& TegPath( const PosPair_t& ) const ;
	const vInt_t& TegTeid( const PosPair_t& ) const ;
	vInt_t& TegPath( const PosPair_t& );
	vInt_t& TegTeid( const PosPair_t& );
	PathGuide_t& StepGuide(const PosPair_t& );
	PathGuide_t& EdgeGuide(const PosPair_t& );
	int _StepBaseBegin, _StepBase;

	void resetDah();
	bool isInitRoute() const { return 0==_j && 0==_k; }
	int  tagFreezed () ;
	int _j; // prefill rate compress iteration
	int _k; // routing round
	int _nCompressFail, _CompressFailLimit;
	bool _FreezeWireLength;
	float _kPrefill, _kPrefillGoal, _kPrefillStep;

	int _nThread;
	int nChildThread() const { return _nThread - 1; }
	vRndTcl_t _vRndTcl;
	// update on each iteration
	vInt_t _vEdgeOvfl, _vDah;


	void initJob();
	void joinJob();
	int  getJob (int);
	std::vector<std::thread> _vChildThread;
	std::atomic<bool> _JobStart;
	int _jobTop;
	vPosPair_t _vPosPair;
	vInt_t _vOrder;
	vInt_t _vJid;
	vInt_t _vJobNum;
	lem::Jidep_t<NetDeptor_t> Jidep;

	Lem_Gdb_t * _pGdb;
	Tex_Man_t * _pTex;
	vRoutePtr_t vRoutePtr;
	vInt_t _vGoing;
	bool _fRescue, _fPostRoute;
	bool _fStopOnReflex;
};

void Lem_RoundRoute ( Lem_Gdb_t * pGdb, Tex_Man_t * pTex );
void Lem_RescueRoute( Lem_Gdb_t * pGdb, Tex_Man_t * pTex );

void Lem_RoundRouteReport( Lem_Gdb_t * pGdb, Tex_Man_t * pTex, vPosPair_t& vPosPair, vInt_t& vOrder );
#endif