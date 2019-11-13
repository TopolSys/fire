#ifndef LEM_TYPE_HPP
#define LEM_TYPE_HPP

#include <utility>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cstring> //memset
#include <string>
#include "lem_geom.hpp"
#include "utils/sys/sys.hpp" //trav id
typedef std::vector<int>    vInt_t;
typedef std::vector<vInt_t> mInt_t;
typedef std::pair<int,int> IntPair_t;

typedef std::vector<float>  vFlt_t;
typedef std::vector<vFlt_t> mFlt_t;

typedef std::vector< IntPair_t> vIntPair_t;
typedef std::vector<vIntPair_t> mIntPair_t;

typedef std::vector<bool>    vBool_t;
typedef std::vector<std::string> vString_t;

template<typename Type_t>
class Lematrix_t {
public:
	Lematrix_t(){}
	Lematrix_t( const int y, const int x ){
		resize(y,x);
	}
	void resize( const int y, const int x ){
		_data.clear();
		_data.resize(y*x);
		_offset.clear();
		_offset.resize(y);
		for(int i=0, inc = 0; i<y; i++, inc+=x)
			_offset[i] = inc;
	}
	const std::vector<Type_t>& data() const { return _data; }
	      std::vector<Type_t>& data()       { return _data; }
	      Type_t& entry( const int y, const int x )     { return _data[ _offset[y] + x ]; }
	const Type_t& entry( const int y, const int x )const{ return _data[ _offset[y] + x ]; }
	Type_t& operator()( const int y, const int x ){ return entry(y,x); }
private:
	vInt_t _offset;
	std::vector<Type_t> _data;
};

template<typename Type_t>
class Lemcube_t {
public:
	Lemcube_t(){}
	Lemcube_t( const int z, const int y, const int x ){
		resize(z,y,x);
	}
	void resize( const int z, const int y, const int x ){
		_z = z;
		_y = y;
		_x = x;
		_data.clear();
		_data.resize(z*y*x);

		_offsety.clear();
		_offsety.resize(y);
		for(int i=0, inc = 0; i<y; i++, inc+=x)
			_offsety[i] = inc;
		
		_offsetz.clear();
		_offsetz.resize(z);
		for(int i=0, inc = 0; i<z; i++, inc+=y*x)
			_offsetz[i] = inc;
	}
	const std::vector<Type_t>& data() const { return _data; }
	      std::vector<Type_t>& data()       { return _data; }
	      Type_t& entry( const int z, const int y, const int x )     { return _data[ _offsetz[z] + _offsety[y] + x ]; }
	const Type_t& entry( const int z, const int y, const int x )const{ return _data[ _offsetz[z] + _offsety[y] + x ]; }
	Type_t&  operator()( const int z, const int y, const int x )     { return entry(z,y,x); }
	const int xdim() const { return _x; }
	const int ydim() const { return _y; }
	const int zdim() const { return _z; }
private:
	int _z, _y, _x;
	vInt_t _offsetz, _offsety;
	std::vector<Type_t> _data;
};


template<int WIDTH>
class vLembit_t: private std::vector<unsigned int> {
public:
	vLembit_t(){
		unsigned int remain = sizeof(unsigned int)*8 - WIDTH;
		MASK = FULL() >> remain;
	}
	unsigned int unit_volume()          const { return sizeof(unsigned int)*8/WIDTH; }
	const unsigned int size ()          const { return vec().size() * unit_volume(); }
	void clear(){ vec().clear(); }
	void resize( unsigned int nsize ){
		int nunit = _unit(nsize) + ( _rank(nsize) ?1:0);
		vec().resize( nunit );
		memset( data(), 0, sizeof(int)*nunit );
	}
	void set( unsigned int index, unsigned int val ){
		vec()[ _unit(index) ] &= ~_mask(index);
		vec()[ _unit(index) ] |= ( val<< _offset(index) );
	}
	const unsigned int val( unsigned int index ) const {
		unsigned int ret = ( vec()[_unit(index)] & _mask(index) )>> _offset(index);
		return ret;
	}
	void  fill( unsigned int from, unsigned int to, bool fOne = true ) {
		int funit = _unit(from);
		int tunit = _unit(  to);
		if( funit == tunit ){
			unsigned int mask = FULL();
			mask <<= sizeof(int)*8 - _offset(to);
			mask >>= sizeof(int)*8 - _offset(to);
			mask >>= _offset(from);
			mask <<= _offset(from);
			fill_unit(funit, mask, fOne );
			return;
		}

		{ // fill head 
			unsigned int mask = FULL();
			mask >>= _offset(from);
			mask <<= _offset(from);
			fill_unit(funit, mask, fOne );
		}
		
		size_t dist = tunit - funit - 1;
		if( dist > 0 )
			memset( &vec()[funit+1], fOne? FULL(): 0, dist * sizeof(int) );
		{ // fill tail
			if( 0 == _offset(to) )
				return;
			unsigned int mask = FULL();
			mask <<= sizeof(int)*8 - _offset(to);
			mask >>= sizeof(int)*8 - _offset(to);
			fill_unit(tunit, mask, fOne );
		}
	}
	const std::vector<unsigned int>& vec() const { return dynamic_cast<const std::vector<unsigned int>&>(*this);}
	std::vector<unsigned int>& vec(){ return dynamic_cast<std::vector<unsigned int>&>(*this);}
	void print(std::ostream& ostr){
		for(int i=0; i<vec().size(); i++){
			for(int j=0; j<sizeof(int)*8; j++)
				ostr<< (((vec()[i])&(1<<j))? 1: 0);
			ostr<<" ";
		}
	}
private:
	unsigned int MASK;
	unsigned int _unit       (unsigned int index) const { return index/unit_volume(); }
	unsigned int _rank       (unsigned int index) const { return index%unit_volume(); }
	unsigned int _offset     (unsigned int index) const { return _rank(index)*WIDTH; }
	unsigned int _mask       (unsigned int index) const { return MASK<<_offset(index);}
	unsigned int FULL       () const { return ~0;}
	void  fill_unit( unsigned int index_unit, unsigned int mask, bool fOne ){
		if( fOne )
			vec()[index_unit] |= mask;
		else
			vec()[index_unit] = vec()[index_unit] & ~mask;
	}
};


class TravTag_t: private std::vector<lem::Trav_t> {
public:
	TravTag_t( int nsize=0 ){
		init(nsize);
	}
	void init( int nsize ){
		_Trav = 0;
		clear();
		if( nsize ) resize( nsize );
	}
	int size() const { return ((const std::vector<lem::Trav_t>&) *this).size(); }
	void inc(){ ++_Trav;}
	const int  cur() const { return _Trav; }
	const int  get( const int& index ) const { return data()[index].current(); }
	const bool is_cur ( const int& index ) const { return get(index)==cur(); }
	const bool is_prev( const int& index ) const { return get(index)==cur()-1; }
	void       set( const int& index ){ data()[index].set(cur()); }
private:
	int _Trav;
};

inline int PairMidpoint( const IntPair_t& ip ){
	return ( ip.first + ip.second )>>1;
}

template<typename TYPE>
inline void print_member( std::ostream& ostr, TYPE& Containter, const int dump_num=0 ){
	int i=0;
	const int member_num = Containter.size();
	typename TYPE::const_iterator itr = Containter.begin();
	for(i=0; itr!=Containter.end() && (0==dump_num || i < dump_num); itr++, i++)
		ostr<< *itr <<" ";
	if( dump_num && i<member_num - dump_num )
		ostr<<" ... ... ";
	for(; itr!=Containter.end(); itr++, i++)
		if( i >= member_num - dump_num )
			ostr<< * itr <<" ";
	ostr<<"\n";
}


inline IntPair_t Lem_CoveredTex( const Range_t& range, const vInt_t& vGrid ){
	vInt_t::const_iterator first = std::lower_bound( vGrid.begin(), vGrid.end(), range.n1 );
	vInt_t::const_iterator last  = std::upper_bound(         first, vGrid.end(), range.n2 );
	IntPair_t ret( first - vGrid.begin(), last - vGrid.begin() );
	if( vGrid.end() != first ){
		if( *first > range.n1 && first != vGrid.begin() )
			ret.first--;
	}
	if( vGrid.size() == ret.second )
		ret.second --;
	return ret;
}

inline std::ostream& operator<<(std::ostream& ostr, const IntPair_t& range ){
	return ostr<<"("<<range.first<<","<<range.second<<")";
}
template<typename TYPE>
inline void Lem_Union( const TYPE& c1, const TYPE& c2, TYPE& dist ){
	//c1, c2 need to be sorted
	TYPE tmp(c1.size()+c2.size());
	typename TYPE::iterator itr = std::set_union( c1.begin(), c1.end(), c2.begin(), c2.end(), tmp.begin() );
	dist.resize(itr-tmp.begin());
	dist.assign(tmp.begin(),itr);
}
#endif