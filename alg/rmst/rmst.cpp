#include "rmst.hpp"
#include "lem_mem.hpp"

#include <set> // rmst algorithm
#include <algorithm>

struct getPointX {
	int operator()( const Point_t& point ) const {
		return point.x;
	}
};

template<typename vDATA_T, typename GET_T >
struct Sorter {
	vDATA_T * _pvDATA;
	GET_T _GET;
	Sorter( vDATA_T * pvDATA, GET_T GET ):_pvDATA(pvDATA),_GET(GET){}
	bool operator()( const int& p1, const int& p2 )const{
		return _GET( (*_pvDATA)[p1] ) < _GET( (*_pvDATA)[p2] );
	}
};

static int QuadRegion( const Point_t& ref, const Point_t& tar ){
	using namespace std;
	int relative_y = tar.y - ref.y;
	int relative_x = tar.x - ref.x;
	return relative_y >= 0
		? ( abs(relative_y) > abs(relative_x)? 0: 1)  // region 0,1
		: ( abs(relative_y) < abs(relative_x)? 2: 3); // region 2,3
}

void Rmst_t::_addEdge( const int& v1, const int& v2 ){
	_vEdge.push_back( IntPair_t(v1,v2) );
	IntPair_t& Edge = _vEdge.back();
	if( Edge.first > Edge.second )
		std::swap( Edge.first , Edge.second );
}

void Rmst_t::updateGraph(){
	_vEdge.clear();
	vInt_t vOrder( _vPoint.size() );
	for(int i=0; i<vOrder.size(); i++)
		vOrder[i] = i;
	std::sort( vOrder.begin(), vOrder.end(), Sorter< vPoint_t, getPointX >( &_vPoint, getPointX() ) );
	for(int i=0; i<vOrder.size(); i++){
		int nRegion = 0;
		bool hasR[4]={ false, false, false, false };
		IntPair_t Edges[4] = { IntPair_t(-1,-1),IntPair_t(-1,-1),IntPair_t(-1,-1),IntPair_t(-1,-1)};
		int y_i = _vPoint[vOrder[i]].y;
		int x_i = _vPoint[vOrder[i]].x;
		for(int j=i+1; j<vOrder.size() /** && nRegion<4 **/ ; j++){
			int rid = QuadRegion( _vPoint[vOrder[i]], _vPoint[vOrder[j]] );
			if( hasR[rid] ){
				int dist_prev = abs(_vPoint[Edges[rid].second].y - y_i) + abs(_vPoint[Edges[rid].second].x - x_i);
				int dist_cur  = abs(_vPoint[       vOrder[j] ].y - y_i) + abs(_vPoint[       vOrder[j] ].x - x_i);
				if( dist_cur > dist_prev )
					continue;
			}
			hasR[rid] = true;
			Edges[rid] = IntPair_t( vOrder[i], vOrder[j] );
			//_addEdge( vOrder[i], vOrder[j] );
			nRegion++;
		}

		for(int j=0; j<4; j++)
			if( hasR[j] )
				_addEdge( Edges[j].first, Edges[j].second );
	}

	_vEdgeCost.clear();
	_vEdgeCost.resize( _vEdge.size(), 0 );
	for(int i=0; i<_vEdge.size(); i++){
		const Point_t& p1 = _vPoint[ _vEdge[i].first ];
		const Point_t& p2 = _vPoint[ _vEdge[i].second];
		_vEdgeCost[i] = sqdist( p1, p2 );
	}
}

static void SortEdgeByCost( const vInt_t& vCost, vInt_t& vOrder ){
	vOrder.clear();
	vOrder.resize( vCost.size() );
	for(int i=0; i<vCost.size(); i++)
		vOrder[i] = i;
	struct getVec {
		int operator()( const int& val ) const { return val; }
	};
	std::sort( vOrder.begin(), vOrder.end(), Sorter<const vInt_t, getVec >( &vCost, getVec()) );
}

// Kruskal
void Rmst_t::rmst(){
	using namespace std;
	typedef set<int> IntSet_t;
	typedef vector<IntSet_t> vIntSet_t;

	vInt_t    vGroupId( _vPoint.size() );
	vIntSet_t vGroup  ( _vPoint.size() );
	for(int i=0; i<_vPoint.size(); i++){
		vGroupId[i] = i;        // belongs to itself's group
		vGroup  [i].insert(i);
	}

	vInt_t vEdgeOrder;
	SortEdgeByCost( _vEdgeCost, vEdgeOrder );


	int nGroup = vGroup.size();
	vPinPair.clear(); // initialize space for solution
	for(int i=0; i<vEdgeOrder.size() && nGroup>1 ; i++){
		int eid = vEdgeOrder[i];
		int p1  = _vEdge[eid].first ;
		int p2  = _vEdge[eid].second;
		if( vGroupId[p1] == vGroupId[p2] )
			continue;  // already in the same group
		nGroup --;
		IntSet_t& Group1 = vGroup[ vGroupId[p1] ];
		IntSet_t& Group2 = vGroup[ vGroupId[p2] ];
		for(IntSet_t::iterator itr=Group2.begin(); itr!=Group2.end(); itr++){
			vGroupId[*itr] = vGroupId[p1];
			Group1.insert(*itr);
		}
		Group2.clear();
		vGroupId[p2] = vGroupId[p1];
		vPinPair.push_back( _vEdge[eid] );
	}
}

void Lem_TerminalParition( int cluster_size, const vPoint_t& vPoint, const vIntPair_t& vEdge, mInt_t& vCluster ){
	using namespace std;
	typedef set<int> IntSet_t;
	typedef vector<IntSet_t> vIntSet_t;
	
	vInt_t    vGroupId( vPoint.size() );
	vIntSet_t vGroup  ( vPoint.size() );
	for(int i=0; i<vPoint.size(); i++){
		vGroupId[i] = i;        // belongs to itself's group
		vGroup  [i].insert(i);
	}

	vInt_t vCost ( vEdge.size() );
	for(int i=0; i<vCost.size(); i++)
		vCost[i] = sqdist(vPoint[vEdge[i].first ], vPoint[vEdge[i].second]);

	vInt_t vEdgeOrder;
	SortEdgeByCost( vCost, vEdgeOrder );

	int nBridgePoint = 0;
	vBool_t vBridgePoint(vPoint.size(), false);
	vIntPair_t vBridgeEdge;
	for(int i=0; i<vEdgeOrder.size(); i++){
		int eid = vEdgeOrder[i];
		int p1  = vEdge[eid].first ;
		int p2  = vEdge[eid].second;
		if( vGroupId[p1] == vGroupId[p2] )
			continue;  // already in the same group

		IntSet_t& Group1 = vGroup[ vGroupId[p1] ];
		IntSet_t& Group2 = vGroup[ vGroupId[p2] ];
		if( cluster_size <= Group1.size() + Group2.size() ){
			vBridgeEdge.push_back( vEdge[eid] );
			if( ! vBridgePoint[p1] ){
				vBridgePoint[p1] = true;
				nBridgePoint++;
			}
			if( ! vBridgePoint[p2] ){
				vBridgePoint[p2] = true;
				nBridgePoint++;
			}
			continue;
		}

		for(IntSet_t::iterator itr=Group2.begin(); itr!=Group2.end(); itr++){
			vGroupId[*itr] = vGroupId[p1];
			Group1.insert(*itr);
		}
		Group2.clear();
		vGroupId[p2] = vGroupId[p1];
		//vPinPair.push_back( vEdge[eid] );
		//cout<< _vPoint[p1] <<" , "<< _vPoint[p2]<<"\n";
	}
	vBridgePoint.clear();

	int nGroup=0;
	for(int i=0; i<vGroup.size(); i++)
		if( ! vGroup[i].empty() )
			nGroup++;
	
	mInt_t vClusterNext;
	if( cluster_size < nBridgePoint ){
		Lem_TerminalParition(cluster_size, vPoint, vBridgeEdge, vClusterNext );
		nGroup += vClusterNext.size();
	}

	vCluster.clear();
	vCluster.resize(nGroup);
	int GroupTop=0;
	for(int i=0; i<vGroup.size(); i++)
		if( ! vGroup[i].empty() ){
			int nMember = 0;
			vCluster[ GroupTop ].resize( vGroup[i].size() );
			for(IntSet_t::iterator itr=vGroup[i].begin(); itr!=vGroup[i].end(); itr++)
				vCluster[GroupTop][ nMember++ ] = *itr;
			GroupTop++;
		}

	for(int i=0; i<vClusterNext.size(); i++)
		vCluster[GroupTop++] = vClusterNext[i];
}

std::ostream& operator<<( std::ostream& ostr, const Rmst_t& rmst ){
	int length = 0;
	for(int i=0; i<rmst.vPinPair.size(); i++){
		int p1 = rmst.vPinPair[i].first ;
		int p2 = rmst.vPinPair[i].second;
		if( i>0 )
			ostr<<", ";
		ostr<< rmst.point(p1) <<"->"<<rmst.point(p2);
		length += mahn( rmst.point(p1), rmst.point(p2) );
	}
	ostr<<". Rectilinear_length= "<< length<<"\n";
	return ostr;
}

// test code: 
//  #include "alg/rmst/rmst.hpp"
//	Rmst_t rmst;
//	rmst.addPoint( Point_t(0,0) );
//	rmst.addPoint( Point_t(2,5) );
//	rmst.addPoint( Point_t(5,2) );
//	rmst.addPoint( Point_t(7,0) );
//	rmst.updateGraph();
//	rmst.rmst();
//	std::cout<<rmst;
//
//  answer: (5,2)->(7,0), (2,5)->(5,2), (0,0)->(2,5)