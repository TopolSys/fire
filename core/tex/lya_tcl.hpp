#ifndef CORE_TEX_LYA_TCL_HPP
#define CORE_TEX_LYA_TCL_HPP

#include <map>
#include <unordered_map>
#include <vector>
#include "core/tex/route.hpp"
#include "utils/sys/sys.hpp"
#include "lem_type.hpp"
#include "teg.hpp"
class Tex_Man_t;
class Lem_Gdb_t;

class Lya_Tcl_t {
public:
	Lya_Tcl_t( Lem_Gdb_t * pGdb, Tex_Man_t * pTex );
	void init ( const Tcl_t& Tcl );
	void erase( const Tcl_t& Tcl );
	void decompose( const Tcl_t& );
	void chunk( const Tcl_t& );
	void update( Tcl_t& );
private:
	typedef std::multimap<int, int, std::less<int>, Lem_Alloc<std::pair<const int,int> > > PosIntMap_t;
	typedef std::pair<PosIntMap_t::iterator,PosIntMap_t::iterator> PosIntRange_t;
	void chunk_seg( const int sid );

	bool has_adj( int pos_index, bool isV );
	void tag_boundary_seg( const int sid );
	void tag_boundary();
	
	void addSwitch2Map( const int sid );
	void build_adj( const Tcl_t& );
	void build_adj_seg( const int sid );

	void build_pin();
	void build_pin_seg( const int sid );

	void addPin2Map( const int sid );
	void PosSearchHV( const int pos_index, bool& hasV, bool& hasH );
	void build_pin_fill( const Tcl_t& ); // add fill if necessary

	void _finalize( const Tcl_t& Tcl );

	void _extend_rec( int teid, Seg_t& Seg );
	void _extend( int teid, Seg_t& Seg );
	void _tagConn( const Tcl_t& );
	int nSeg; 			// edge segment count
	vSeg_t _vSeg; 		// edge segment
	
	WireGuide_t WireGuide;
	WireGuide_t SwitchGuide;
	WireGuide_t PinGuide;

	std::unordered_multimap<int,int> Tex2TegID;

	PosIntMap_t PosSidMap;
	PosIntMap_t PosTegMap;

	TravTag_t _TexTrav;
	TravTag_t _EdgeTrav;
	Lem_Gdb_t * _pGdb;
	Tex_Man_t * _pTex;
};

#endif