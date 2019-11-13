#ifndef ALG_RMST_HPP
#define ALG_RMST_HPP

#include "lem_geom.hpp"
#include "lem_type.hpp"

class Rmst_t {
public:
	void addPoint( const Point_t& point ){ _vPoint.push_back(point); }
	void addPoints( const vPoint_t& vPoint ){ _vPoint = vPoint; }
	void updateGraph();
	void rmst();
	vIntPair_t vPinPair; // result of rmst algorithm
	const Point_t& point(int idx) const { return _vPoint[idx]; }
	const vPoint_t& vPoint() const { return _vPoint; }
private:
	void _addEdge( const int& v1, const int& v2 );
	vIntPair_t _vEdge;
	vInt_t _vEdgeCost;
	vPoint_t _vPoint; // the order of this array will not change, and is stable with added order
};

extern std::ostream& operator<<( std::ostream& ostr, const Rmst_t& );
void Lem_TerminalParition( int cluster_size, const vPoint_t& vPoint, const vIntPair_t& vEdge, mInt_t& vCluster );
#endif