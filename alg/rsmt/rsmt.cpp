#include <cstdio>
#include <map>
#include "rsmt.hpp"
#include "utils/sys/sys.hpp"
#include <algorithm>

extern "C" {
#include "flute.h"
}

struct PointCmp_t {
	bool operator()( const Point_t& p1, const Point_t& p2 ) const {
		return p1.x != p2.x? p1.x < p2.x: p1.y < p2.y;
	}
};
typedef std::map<Point_t,int,PointCmp_t> Point2Int_t;

static void BuildPointMap( const Tree& flutetree, const vPoint_t& vPins, Point2Int_t& Point2Int ){
	int node_num = 0;
	while( node_num<vPins.size() ){
		if( lem::dev() ){
			if( node_num > 0 )
				assert( !(vPins[node_num] == vPins[node_num-1]) );
		}
		Point2Int[vPins[node_num]] = node_num++;
	}
	
	const int branch_num = flutetree.deg*2 -2;
	Branch * BranchArray = flutetree.branch;
	for(int i=0; i<branch_num; i++){
		Point_t point(BranchArray[i].x, BranchArray[i].y);
		if( Point2Int.end() != Point2Int.find( point ) )
			continue;
		Point2Int[point] = node_num++;
	}
}

static void UpdatePointVector( const Point2Int_t& Point2Int, vPoint_t& vPins ){
	vPins.clear();
	Lem_Iterate( Point2Int, pPoint )
		vPins.push_back(pPoint->first);
}

const int Rsmt_t::node_limit() {
	return MAXD;
}

int Rsmt_t::rsmt(){
	if( _vPoint.size() < 2 ){
		return 1;
	}
	if( _vPoint.size() > node_limit() ){
		char buff[1024];
		sprintf(buff, "FLUTE: pin num= %d larger limit MAXD= %d", _vPoint.size(), node_limit() );
		lem::error.add(buff);
		//lem::error.dump();
		return 0;
	}
	int d=0, x[ node_limit() ], y[ node_limit() ];
	for(d=0; d<_vPoint.size(); d++)
		x[d] = _vPoint[d].x, y[d] = _vPoint[d].y;
	Tree flutetree = flute(d,x,y,ACCURACY);
	
	Point2Int_t Point2Int;
	BuildPointMap( flutetree, _vPoint, Point2Int );
	UpdatePointVector( Point2Int, _vPoint );

	const int branch_num = flutetree.deg*2 -2;
	Branch * BranchArray = flutetree.branch;
	for(int i=0; i<branch_num; i++){
		int dist = BranchArray[i].n;
		if( i==dist )
			continue;
		Point_t p1(BranchArray[i].x, BranchArray[i].y);
		Point_t p2(BranchArray[dist].x, BranchArray[dist].y);
		if( p1==p2 )
			continue;
		IntPair_t edge( Point2Int[p1], Point2Int[p2] );
		assert( edge.first != edge.second );
		if( edge.first > edge.second )
			std::swap( edge.first, edge.second );
		_vEdge.push_back(edge);
	}

	{ // make unique edge set 
		vIntPair_t vEdgeTmp;
		vEdgeTmp = _vEdge;
		std::sort( vEdgeTmp.begin(), vEdgeTmp.end() );
		_vEdge.clear();
		for(int i=0; i<vEdgeTmp.size(); i++)
			_vEdge.push_back(vEdgeTmp[i]);
	}
	
	if( flutetree.branch )
		free(flutetree.branch);

	return 1;
}

void Rsmt_t::addUniquePoints( const vPoint_t& vPoint ){
	vPoint_t vTemp;
	vTemp = vPoint;
	std::sort( vTemp.begin(), vTemp.end(), PointCmp_t() );
	_vPoint.clear();
	for(int i=0; i<vTemp.size(); i++){
		if( i>0? vTemp[i-1]==vTemp[i]: false )
			continue;
		_vPoint.push_back(vTemp[i]);
	}
}

rsmt_init_t rsmt_init_t::starter;

rsmt_init_t::rsmt_init_t(){
	if( lem::dev() )
		printf("RSMT init\n");
	
	readStringLUT();
}

rsmt_init_t::~rsmt_init_t(){
	if( lem::dev() )
		printf("RSMT end\n");

	endLUT();
}
