#ifndef RSMT_HPP
#define RSMT_HPP

#include "lem_geom.hpp"
#include "lem_type.hpp"

class Rsmt_t {
public:
	void addPoint( const Point_t& point ){ _vPoint.push_back(point); }
	void addPoints( const vPoint_t& vPoint ){ _vPoint = vPoint; }
	void addUniquePoints( const vPoint_t& vPoint );
	int  rsmt();
//	void updateGraph();
//	vIntPair_t vPinPair; // result of rmst algorithm
//	const Point_t& point(int idx) const { return _vPoint[idx]; }
	static const int node_limit();
	vIntPair_t& vEdge(){ return _vEdge; }
	const vIntPair_t& vEdge() const { return _vEdge; }
	const vPoint_t& vPoint () const { return _vPoint; }
private:
//	void _addEdge( const int& v1, const int& v2 );
	vIntPair_t _vEdge;
//	vInt_t _vEdgeCost;
	vPoint_t _vPoint; // size and order could change 
};

class rsmt_init_t {
public:
	rsmt_init_t();
	~rsmt_init_t();
	static rsmt_init_t starter;
};

#endif
