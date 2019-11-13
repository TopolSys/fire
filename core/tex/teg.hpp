#ifndef TEG_HPP
#define TEG_HPP

#include "lem_type.hpp"
#include "lem_geom.hpp"
#include "alg/rsmt/rsmt.hpp"
#include <climits>
#include <iostream>
#include <vector>
#include <string>
#include <set>

#include "tex.hpp"

class Lem_Gdb_t;
// TErminal Group
// a terminal has several candidate of connecting points in 3D space
// 2D view: bounding box of all candidates points
// 3D view: box on layers
class Teg_t {
public:
	Teg_t():_y(Null()),_x(Null()){}
	int gposX( const int& nx ){ return (_x = nx); }
	int gposY( const int& ny ){ return (_y = ny); }
	const int gposX()const{ return _x; }
	const int gposY()const{ return _y; }
	IntPair_t grangeX( const IntPair_t& nrange ){ return (_grangeX = nrange); }
	IntPair_t grangeY( const IntPair_t& nrange ){ return (_grangeY = nrange); }
	const IntPair_t grangeX()const{ return _grangeX; }
	const IntPair_t grangeY()const{ return _grangeY; }
	//IntPair_t tex( Tex_Man_t * );

	const Rect_t& set_box( const Rect_t& rect ) { _box = rect; return _box; }
	const Rect_t& box() const {return _box;}

	vInt_t& vLayer(){ return _vLayer; }
	const vInt_t& vLayer() const { return _vLayer; }
	const int vLayer( int index ) const { return _vLayer[index]; }
	const bool is_nonstd() const { return _vLayer.empty()? true: (_vLayer.back()>0); }
	const bool is_std() const { return (1==_vLayer.size())? 0==_vLayer.back(): false; }

	static int Null(){ return INT_MAX; }
private:
	Rect_t _box; // 2D bounding box
	int _y, _x;
	vInt_t _vLayer; // layer choices
	IntPair_t _grangeY, _grangeX;
	//int _index; // i-th pin of net
};


class Seg_t: public vInt_t {
public:
	static const int NullLayer(){ return INT_MAX; }
	static const int NullIndex(){ return -1; }
	Seg_t( int layer=NullLayer(), char isV=0, char isPinFill=0 ):_layer(layer),_isV(isV),_isPinFill(isPinFill),_length(0),_tcl(-1),_cost(0),_head( NullIndex() ),_tail( NullIndex() ){}
	void set_layer ( int layer ) { _layer = layer; }
	void set_dir   ( char isV )  { _isV  = isV; }
	void set_head  ( int sid )   { _head = sid; }
	void set_tail  ( int sid )   { _tail = sid; }
	void set_length( int val )   { _length = val; }
	void set_PinFill() { _isPinFill = true; }
	void set_cost  ( int val )   { _cost = val; }
	const int      layer() const { return _layer     ;}
	const bool       isV() const { return _isV       ;}
	const bool isPinFill() const { return _isPinFill ;}
	const int      head () const { return _head      ;}
	const int      tail () const { return _tail      ;}
	const int     length() const { return _length    ;}
	const int       cost() const { return _cost      ;}

	Seg_t * next( Lem_Gdb_t * ) const ;
	Seg_t * prev( Lem_Gdb_t * ) const ;
	
	vInt_t& vTeg(){ return _vTeg; }
	const vInt_t& vTeg() const { return _vTeg; }

	vInt_t& vAdj(){ return _vAdj; }
	const vInt_t& vAdj() const { return _vAdj; }

	vIntPair_t& vAux(){ return _vAux; }
	const vIntPair_t& vAux() const { return _vAux; }

	void resetAdj();
	void addAdj(int index, int sid);
	const vInt_t& Adj(int index) const ;

	void resetTeg();
	void addTeg(int index, int sid);
	const vInt_t& Teg(int index) const ;

	void resetLayer();
	void setLayer(int index, int lid);
	const int Layer(int index) const ;
	IntPair_t layerRange(int index, Lem_Gdb_t * pGdb, bool fCountSelf=true ) const ;
	int layer_of(int tex) const ;
	const bool race_assigned() const ;
	const bool adj_race_assigned( Lem_Gdb_t * pGdb ) const ;

	const int tcl() const { return _tcl; }
	void set_tcl( int val ){ _tcl = val; }
	int  countVia( std::set<int>& SeenTex, Lem_Gdb_t * ) const ;
	int  LayerUsage() const ;
	void print_race_layer(std::ostream& ostr, Tex_Man_t *) const ;
private:
	vInt_t _vLayer;
	vInt_t _vAdj; // adjacent segment
	vInt_t _vTeg;
	vIntPair_t _vAux;
	mInt_t _mAdj;
	mInt_t _mTeg;
	char _isV, _isPinFill;
	int _layer;
	int _tcl;
	int _head, _tail;
	int _length; // original length before making chunk
	int _cost;
};
inline std::ostream& operator<<(std::ostream& ostr, const Seg_t& Seg){
	ostr<<"lid="<<Seg.layer()<<" isV="<<Seg.isV();
	if( Seg.head()!=Seg_t::NullIndex() || Seg.tail()!=Seg_t::NullIndex() ){
		ostr<<" head="<<Seg.head()<<" tail="<<Seg.tail();
	}
	ostr<<" length="<<Seg.length()<<" ";
	ostr<<" adj=";
	print_member(ostr, Seg.vAdj());
	ostr<<"\tTegs: ";
	print_member(ostr, Seg.vTeg());
	ostr<<"\tCells: ";
	print_member(ostr, Seg);
	return ostr;
}
typedef std::vector<Seg_t> vSeg_t;
//Segment cluster
class Scl_t: public vSeg_t {
public:
};
typedef Tex_Man_t::Pos_t TexPos_t;
typedef std::vector<TexPos_t> vTexPos_t;
// Teg CLuster
class Tcl_t: public std::vector<Teg_t> {
public:
	Tcl_t():_ok(false),_id(-1){}
	bool setOk(){ _ok = true; }
	bool notOk(){ _ok = false; }
	bool ok() const { return _ok; }
	vIntPair_t& vTegPair()  { return _vTegPair; }
	Rsmt_t    & Rsmt    ()  { return _Rsmt    ; }
	const vIntPair_t& vTegPair() const { return _vTegPair; }
	const Rsmt_t    & Rsmt    () const { return _Rsmt    ; }

	const mInt_t& mTegPath() const { return _mTegPath; }
	mInt_t& mTegPath(){ return _mTegPath; }
	const vInt_t& TegPath(int index ) const { return _mTegPath[index]; }
	vInt_t& TegPath(int index ){ return _mTegPath[index]; }
	vInt_t& TegBaseLen(){ return _vTegBaseLen; }
	const vInt_t& TegBaseLen() const { return _vTegBaseLen; }
	int  TegBaseLen(int index ) const { return _vTegBaseLen[index]; }

	const mInt_t& mTegTeid() const { return _mTegTeid; }
	mInt_t& mTegTeid(){ return _mTegTeid; }
	vInt_t&        TegTeid(int index )       { return _mTegTeid[index]; }
	const vInt_t&  TegTeid(int index ) const { return _mTegTeid[index]; }

	vInt_t& vPath(){ return _vPath; }
	vInt_t& vTeid(){ return _vTeid; }
	const vInt_t& vPath() const { return _vPath; }
	const vInt_t& vTeid() const { return _vTeid; }

	vSeg_t& vSeg() { return _vSeg; }
	const vSeg_t& vSeg() const { return _vSeg; }
	Seg_t& vSeg( int index ) { return _vSeg[index]; }
	const Seg_t& vSeg( int index ) const { return _vSeg[index]; }

	void set_name( const char * name ){ _name = name; }
	const std::string& name()const{ return _name; }

	const vTexPos_t& vTexPos() const { return _vTexPos; }
	vTexPos_t& vTexPos(){ return _vTexPos; }

	bool isFreezed( int index ) const { return _vFreeze.size()<=index? false: _vFreeze[index]; }
	void setFreeze( int index )       { if( _vFreeze.size()<=index ) _vFreeze.resize(index+1,false); _vFreeze[index]=true; }

	int  id() const { return _id; }
	void set_id( int val ){ _id = val; }
private:
	vIntPair_t _vTegPair;
	vTexPos_t _vTexPos;   //unique tex pos of tegs

	mInt_t _mTegPath;
	mInt_t _mTegTeid;
	vBool_t _vFreeze;
	vInt_t _vTegBaseLen; //base length
	vInt_t _vPath;
	vInt_t _vTeid;
	Rsmt_t _Rsmt;

	vSeg_t _vSeg;     // segments for layer assignment
	std::string _name;
	bool _ok;
	int _id;
};

typedef std::vector<Tcl_t> vTcl_t;


inline std::ostream& operator<<( std::ostream& ostr, const Teg_t& Teg ){
	ostr<<"box: "<< Teg.box()<<", tile ("<<Teg.gposX()<<","<<Teg.gposY()<<")";
	return ostr;
}

inline std::ostream& operator<<( std::ostream& ostr, const Tcl_t& Tcl ){
	for( Tcl_t::const_iterator itr = Tcl.begin(); itr != Tcl.end(); itr ++ )
		ostr << *itr <<std::endl;
	return ostr;
}

#endif