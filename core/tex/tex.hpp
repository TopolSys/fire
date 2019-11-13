#ifndef TEX_HPP
#define TEX_HPP

#include <cstdlib>
#include <iostream>
#include "lem_type.hpp"
#include "lem_geom.hpp"
#include "utils/sys/sys.hpp"

class Tex_t;
typedef std::vector<Tex_t> vTex_t;
typedef std::vector<vTex_t> mTex_t;

// tile vertex
class Tex_t {
public:
	Tex_t();
	int add_obsLoadH( const int& val ){ return (_obsLoadH += val); }
	int     obsLoadH()const           { return  _obsLoadH; }
	int add_obsLoadV( const int& val ){ return (_obsLoadV += val); }
	int     obsLoadV()const           { return  _obsLoadV; }
	int     obsLoad ()const           { return  _obsLoadH+_obsLoadV; }
	int     obsLoad ( bool isV) const { return  isV? _obsLoadV: _obsLoadH; }

	void inc_netLoad(bool isV)       { (isV? _netLoadV:_netLoadH) += 1; }
	void dec_netLoad(bool isV)       { (isV? _netLoadV:_netLoadH) -= 1; }
	int      netLoad(bool isV) const { return  isV? _netLoadV: _netLoadH; }

	int add_pinPrefill ( const int& val ){ return (_pinPrefill += val); }
	int     pinPrefill ()const           { return  _pinPrefill; }
	void set_hnext( int index ){ _hnext = index; }
	void set_vnext( int index ){ _vnext = index; }
	void set_hprev( int index ){ _hprev = index; }
	void set_vprev( int index ){ _vprev = index; }
	const int next_eid(bool isV) const { return isV? vnext_eid(): hnext_eid(); } // next
	const int prev_eid(bool isV) const { return isV? vprev_eid(): hprev_eid(); } // prev
	const int hnext_eid() const { return _hnext; } // right
	const int vnext_eid() const { return _vnext; } // up
	const int hprev_eid() const { return _hprev; } // left
	const int vprev_eid() const { return _vprev; } // down
	const bool is_veid( int index ) const { return vnext_eid()==index || vprev_eid()==index;}
	const bool is_heid( int index ) const { return hnext_eid()==index || hprev_eid()==index;}
	const int other_eid( int index ) const { return is_veid(index)? other_veid(index): other_heid(index);}
	lem::Trav_t travId;
	const static int NullEdge(){ return -1; }

	const int layerObs( int layer, bool isV ) const { return isV? layerObsV(layer): layerObsH(layer);}
	void  add_layerObs( int val, int layer, bool isV ){
		vInt_t& vTar = isV? _layerObsV: _layerObsH;
		if( vTar.size() <= layer )
			vTar.resize( layer+1 );
		vTar[layer] += val;
	}
	void  set_layerPRL( int layer ){
		if( _layerPRL.size()<=layer )
			_layerPRL.resize(layer+1, false);
		_layerPRL[layer] = true;
	}
	const bool layerPRL( int layer ) const { return layer < _layerPRL.size()? _layerPRL[layer]: false; }
	bool isPRL2D() const { return !_layerPRL.empty(); }
private:
	const int other_veid( int index ) const { assert(is_veid(index)); return vnext_eid()==index? vprev_eid(): vnext_eid();}
	const int other_heid( int index ) const { assert(is_heid(index)); return hnext_eid()==index? hprev_eid(): hnext_eid();}
	int _hprev, _hnext, _vprev, _vnext;
	int _obsLoadH,_obsLoadV; // 2D obstruction load in prefered direction
	int _netLoadH,_netLoadV;
	int _pinPrefill;

	vInt_t _layerObsV, _layerObsH;
	vBool_t _layerPRL;
	const int layerObsV( int layer ) const { return layer<_layerObsV.size()? _layerObsV[layer]: 0;}
	const int layerObsH( int layer ) const { return layer<_layerObsH.size()? _layerObsH[layer]: 0;}
};

inline std::ostream& operator<<(std::ostream& ostr, const Tex_t& Tex){
	return ostr<< Tex.obsLoad();
}

class Tedge_t: public std::pair<int,int> {
public:
	Tedge_t( int from_index=0, int to_index=0 ):_load(0),std::pair<int,int>(from_index,to_index){}
	int addload( int val ){ return (_load+=val);}
	int incLoad(){ return ++_load; }
	int decLoad(){ return --_load; }
	const int load()const{ return _load;}
private:
	int _load;
};
typedef std::vector<Tedge_t> vTedge_t;
inline std::ostream& operator<<(std::ostream& ostr, const Tedge_t& teid){
	ostr<<"("<<teid.first<<","<<teid.second<<")";
}

class WmSpacing_t {
public:
	WmSpacing_t(int minSpacing=0){
		_minSpacing = minSpacing;
	}
	void reset(const vInt_t& vWidth, const vInt_t& vLength){
		_width.clear();
		_spacing.clear();
		_width = vWidth ;
		_length= vLength;
		_spacing.resize(_width.size());
		for(int i=0; i<_spacing.size(); i++)
			_spacing[i].resize(_length.size(), 0);
	}
	int spacing(int width, int runLength) const {
		if( _length.empty() || _width.empty() )
			return _minSpacing;
		int i = 0;
		for(; i<_width.size(); i++)
			if( _width[i]>width )
				break;
		int row = i>0? i-1: i;
		int j=0;
		for(; j<_spacing[row].size(); j++)
			if( _spacing[row][j]>runLength )
				break;
		int col = j>0? j-1: j;
		return _spacing[row][col];
	}
	void setSpacing(int idx, int width, const vInt_t& vspacing ){
		_width[idx] = width;
		_spacing[idx] = vspacing;
	}
	void report() const {
		std::cout<<"LEN: ";
		for(int i=0; i<_length.size(); i++){
			std::cout<<_length[i]<<" ";
		}
		std::cout<<"\n";
		for(int i=0; i<_width.size(); i++){
			std::cout<<_width[i]<<": ";
			for(int j=0; j<_spacing[i].size(); j++)
				std::cout<<_spacing[i][j]<<" ";
			std::cout<<"\n";
		}
	}
private:
	int _minSpacing;
	vInt_t _width, _length;
	mInt_t _spacing;
};

typedef std::vector<WmSpacing_t> vWmSpacing_t;
class LSpacing_t: public vWmSpacing_t {
public:
	int spacing(int width, int runLength=0)const{
		int maxSpacing = 0;
		for(int i=0; i<size(); i++)
			maxSpacing = std::max(maxSpacing, data()[i].spacing(width,runLength));
		return maxSpacing;
	}
	void report() const {
		for(int i=0; i<size(); i++){
			std::cout<<"Table "<<i<<": \n";
			data()[i].report();
		}
	}
};
typedef std::vector<LSpacing_t> vLSpacing_t;

class Tex_Man_t: public Lematrix_t<Tex_t> {
public:
	// general
	void print_edge() const ;
	struct Pos_t{
		typedef enum {
			UP, DOWN, LEFT, RIGHT 
		} DIR_t;
		int _x, _y;
		Pos_t( int x=-1, int y=-1 ):_x(x), _y(y){}
		const Pos_t up   () const { return vnext(); }
		const Pos_t down () const { return vprev(); }
		const Pos_t left () const { return hprev(); }
		const Pos_t right() const { return hnext(); }
		const Pos_t vprev() const { return Pos_t(_x,_y-1); }
		const Pos_t vnext() const { return Pos_t(_x,_y+1); }
		const Pos_t hprev() const { return Pos_t(_x-1,_y); }
		const Pos_t hnext() const { return Pos_t(_x+1,_y); }
		bool equal( const Pos_t& pos ) const {
			return pos._x == _x && pos._y == _y;
		}
		unsigned int dir_from( const Pos_t& src ) const {
			if( src.equal( up   () ) ) return    UP;
			if( src.equal( down () ) ) return  DOWN;
			if( src.equal( left () ) ) return  LEFT;
			if( src.equal( right() ) ) return RIGHT;
			return 0; // false condition
		}
		const static Pos_t Null(){ return Pos_t(-1,-1);}
		Point_t point() const { return Point_t(_x,_y); }
	};
	int  xnum() const { return _vCapX.size(); }
	int  ynum() const { return _vCapY.size(); }
	const vInt_t& vGridX() const { return _vGridX; }
	const vInt_t& vGridY() const { return _vGridY; }
	      vInt_t& vGridX()       { return _vGridX; }
	      vInt_t& vGridY()       { return _vGridY; }
	const Tex_t& getTex( int index ) const { return data()[index];}
	      Tex_t& getTex( int index )       { return data()[index];}
	const Tex_t& getTex( const Pos_t& pos ) const { return data()[Pos2Index(pos)];}
	      Tex_t& getTex( const Pos_t& pos )       { return data()[Pos2Index(pos)];}
	const Tex_t& getTexByTeid1( const int teid ) const { return getTex1(Edge(teid));}
	const Tex_t& getTexByTeid2( const int teid ) const { return getTex2(Edge(teid));}
	const Tex_t& getTex1( const Tedge_t& tedge ) const { return data()[tedge.first ];}
	const Tex_t& getTex2( const Tedge_t& tedge ) const { return data()[tedge.second];}
	const int getTidPrev( const int index, const bool isV ) const ;
	const int getTidNext( const int index, const bool isV ) const ;
	const int getTidByTeid1( const int teid ) const { return Edge(teid).first ; }
	const int getTidByTeid2( const int teid ) const { return Edge(teid).second; }
	const int getTeid( const int tid1, const int tid2 ) const {
		const int& tidg = tid1 < tid2? tid2: tid1;
		const int& tids = tid1 < tid2? tid1: tid2;
		if( data()[tids].vnext_eid() == data()[tidg].vprev_eid() )
			return data()[tids].vnext_eid();
		if( data()[tids].hnext_eid() == data()[tidg].hprev_eid() )
			return data()[tids].hnext_eid();
		return Tex_t::NullEdge();
	}
	Pos_t  getPos( const int& index ) const { div_t res = div(index,xnum()); return Pos_t( res.rem , res.quot );}
	int  getCoord( const int& index, bool isV ) const { return isV? getPos(index)._y: getPos(index)._x;}
	int   Pos2Index( const Pos_t& pos ) const { return pos._y*xnum()+pos._x;}
	bool valid( const Pos_t& pos ) const { return pos._x >= 0 && pos._x < xnum() && pos._y >= 0 && pos._y < ynum(); }
	const bool is_veid( int teid ) const { return data()[Edge(teid).first].is_veid(teid); }
	lem::Trav_t travId;
	const static int NullTex (){ return -1; }
	const int total_teid_cap() const ;

	void initLSpacing(int numLayer ){ _vLSpacing.resize(numLayer); }
	LSpacing_t& getLSpacing(int lid){ return _vLSpacing[lid]; }
	const LSpacing_t& getLSpacing(int lid) const { return _vLSpacing[lid]; }

	// 2D-layer capacity 
	float edge_fillrate(int eid) const ;
	float tex_fillrate (int tid) const ;
	int  num_edge_overflow( float SoftOverflow=0 ) const ;
	bool is_overflow( const int eid, float SoftOverflow ) const ;
	int  num_overflow( const vInt_t& vTeid, float SoftOverflow ) const ;
	bool intrinsic_overflow( const Pos_t& pos, float SoftOverflow ) const ;
	int   minCap( const int eid ) const ;
	int   maxCap( const int eid ) const ;
	int trackCap( const int eid ) const ;
	int   resCap( const int eid ) const ; // residual capacity 
	int  get_Cap( int isV, int index ) const ;
	void inc_Cap( int isV, int index );
	void set_Cap( int isV, int index, int val );
	void allocate_vCapX( int num ){ _vCapX.clear(); _vCapX.resize(num,0); }
	void allocate_vCapY( int num ){ _vCapY.clear(); _vCapY.resize(num,0); }
	void dump_Cap( std::ostream& );
	void plot( const char * FileName ) const ;
	Rect_t getTexRect( int index ) const {
		Pos_t pos=getPos(index);
		return Rect_t( Point_t( _vGridX[pos._x], _vGridY[pos._y] ), Point_t( _vGridX[pos._x+1], _vGridY[pos._y+1] ) );
	}
	int  getEdgeLoad( int teid ) const ;

	// 3D-layer capacity 
	int  get_layerCap( int isV, int layer_id, int index ) const ;
	void inc_layerCap( int isV, int layer_id, int index );
	void set_layerCap( int isV, int layer_id, int index, int val );
	void set_layerV( bool isV, int layer_id );
	void allocate_layerCapX( int layer_num, int num );
	void allocate_layerCapY( int layer_num, int num );
	void allocate_layerV( int layer_num );
	void dump_layerCap( std::ostream& );
	void buildEdge();
	const vBool_t& layerV() const { return _layerV; }
	const Tedge_t& Edge( int index) const { return _vEdge[index]; }
	      Tedge_t& Edge( int index)       { return _vEdge[index]; }
	const vTedge_t& Edge() const { return _vEdge; }
	const bool isTedgeV( int index ) const { return data()[Edge(index).first].is_veid(index); }
	void print_teid(std::ostream& ostr, const vInt_t& vTeid ) const {
		for(int i=0; i<vTeid.size(); i++)
			ostr<<_vEdge[vTeid[i]];
	}
	const std::string& layerName( int index ) const { return _layerName[index]; }
	vString_t& layerName(){ return _layerName; }
private:
	vTedge_t _vEdge;
	vInt_t _vGridX, _vGridY;
	vInt_t _vCapX, _vCapY;
	mInt_t _layerCapX, _layerCapY;
	vBool_t _layerV; // prefered direction is V
	vString_t _layerName;
	vLSpacing_t _vLSpacing;
};

inline Tex_Man_t::Pos_t minx( const Tex_Man_t::Pos_t& pos1, const Tex_Man_t::Pos_t& pos2 ){
	return pos1._x < pos2._x? pos1: pos2;
}
inline Tex_Man_t::Pos_t maxx( const Tex_Man_t::Pos_t& pos1, const Tex_Man_t::Pos_t& pos2 ){
	return pos1._x > pos2._x? pos1: pos2;
}
inline Tex_Man_t::Pos_t miny( const Tex_Man_t::Pos_t& pos1, const Tex_Man_t::Pos_t& pos2 ){
	return pos1._y < pos2._y? pos1: pos2;
}
inline Tex_Man_t::Pos_t maxy( const Tex_Man_t::Pos_t& pos1, const Tex_Man_t::Pos_t& pos2 ){
	return pos1._y > pos2._y? pos1: pos2;
}
inline Tex_Man_t::Pos_t auto_min( const Tex_Man_t::Pos_t& pos1, const Tex_Man_t::Pos_t& pos2 ){
	return pos1._x==pos2._x? miny(pos1,pos2): minx(pos1,pos2);
}
inline Tex_Man_t::Pos_t auto_max( const Tex_Man_t::Pos_t& pos1, const Tex_Man_t::Pos_t& pos2 ){
	return pos1._x==pos2._x? maxy(pos1,pos2): maxx(pos1,pos2);
}

inline bool operator==( const Tex_Man_t::Pos_t& pos1, const Tex_Man_t::Pos_t& pos2 ){
	return pos1._x == pos2._x && pos1._y == pos2._y;
}

inline int manh( const Tex_Man_t::Pos_t& p1, const Tex_Man_t::Pos_t& p2 ){
	return abs(p1._x-p2._x) + abs(p1._y-p2._y);
}
inline std::ostream& operator<<( std::ostream& ostr, const Tex_Man_t::Pos_t& pos ){
	return ostr<<pos._x<<","<<pos._y;
}
inline Tex_Man_t::Pos_t operator+( const Tex_Man_t::Pos_t& p1, const Tex_Man_t::Pos_t& p2 ){
	return Tex_Man_t::Pos_t( p1._x + p2._x, p1._y + p2._y );
}
#endif