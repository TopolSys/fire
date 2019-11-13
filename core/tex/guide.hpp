#ifndef CORE_TEX_GUIDE_HPP
#define CORE_TEX_GUIDE_HPP

#include <map>
#include <vector>
#include "core/tex/route.hpp"
#include "lem_mem.hpp"

class Tex_Man_t;
class Lem_Gdb_t;

class GuideCell_t {
public:
//	struct Cmptor {
//		bool operator()( const GuideCell_t& g1, const GuideCell_t& g2 ){
//			return g1.layer()!=g2.layer()? g1.layer()<g2.layer(): g1.index()<g2.index();
//		}
//	};
	typedef enum {
		Normal, Switch, Pin, Aux
	}Type_t;
	GuideCell_t( int layer, const Rect_t& rect, char isV, Type_t type ):_layer(layer),_rect(rect),_isV(isV),_type(type){}
	const int layer() const { return _layer; }
	const Rect_t& rect() const { return _rect; }
	const int isV() const { return _isV; }
	const Type_t type() const { return _type; }
	const char * type_str() const {
		if( _type==GuideCell_t::Switch )
			return "+switch";
		else
		if( _type==GuideCell_t::Pin )
			return "+pin";
		return "";
	}
private:
	Type_t _type;
	char _isV;
	int _layer;
	Rect_t _rect;
};
typedef std::vector<GuideCell_t> vGuideCell_t;

class Guide_t {
public:
	Guide_t( Lem_Gdb_t * , Tex_Man_t * );

	void init ( const Tcl_t& Tcl );
	void erase( const Tcl_t& Tcl );

	void derive( const Tcl_t& Tcl, std::ostream& );
	void derive_seg( const Seg_t& Seg, std::ostream& );
	Rect_t derive_seg_rect( const Seg_t& Seg ) const ;
	Rect_t derive_tex_rect( const int tex, int width ) const ;

	void write_guide(const Tcl_t& Tcl, std::ostream& );
	void _write_guide(const Tcl_t& Tcl, std::ostream& );
	int nAux() const { return _nAux; }
private:
	typedef std::multimap<int, int, std::less<int>, Lem_Alloc<std::pair<const int,int> > > PosLayerMap_t;
	typedef std::pair<PosLayerMap_t::iterator,PosLayerMap_t::iterator> PosLayerRange_t;
	PosLayerMap_t PosLayerMap;
	void pin_reg( const Tcl_t& );
	void layer_reg_seg( const Seg_t& Seg );
	void layer_reg( const Tcl_t& Tcl );

	void via_fill_layer();
	void via_fill_pin  ( const Tcl_t& Tcl );
	void via_fill_pin_extra( const Rect_t&, const int layer, bool isPerp);
	Rect_t pin_guide_rect( const Teg_t& ) const ;

	WireGuide_t WireGuide;
	WireGuide_t SwitchGuide;
	void tag_boundary( const Tcl_t& Tcl );
	void tag_pin( const Tcl_t& Tcl );
	void tag_boundary_seg( const Seg_t& Seg );
	bool has_adj(int pos_index , bool isV);

	vGuideCell_t vGuideCell;
	Lem_Gdb_t * _pGdb;
	Tex_Man_t * _pTex;
	int _width;
	int _nAux;
};

void Lem_DeriveGuide( Lem_Gdb_t * pGdb, Tex_Man_t * pTex, std::ostream& ostr );
#endif