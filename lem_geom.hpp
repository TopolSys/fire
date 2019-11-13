#ifndef LEM_GEOM_HPP
#define LEM_GEOM_HPP

#include <climits>
#include <cassert>

#include <iostream>
#include <ostream>
#include <vector>
#include <list>


#include "lem_mac.hpp"


class Point_t;
class Rect_t;
typedef std::vector< Point_t> vPoint_t;
typedef std::vector<vPoint_t> mPoint_t;

std::ostream& operator<<( std::ostream& ostr, const Point_t& point );
std::ostream& operator<<( std::ostream& ostr, const Rect_t& rect );
bool operator==( const Rect_t& rect1, const Rect_t& rect2 );
int mahn  ( const Point_t& p1, const Point_t& p2 );
int sqdist( const Point_t& p1, const Point_t& p2 );

// basic function
inline int Lem_Flip( const int tar, const int axis ){
	return axis * 2 - tar; // axis - (tar - axis);
}

// Lem data type
class Point_t {
public:
	Point_t(){ _init(); }
	Point_t( int nx, int ny ):x(nx),y(ny){}
	int x, y;

	static const Point_t Null(){ return Point_t(INT_MAX,INT_MAX); }
	struct Cmptor {
		bool operator()( const Point_t& p1, const Point_t& p2 ) const {
			return (p1.x != p2.x)? p1.x < p2.x: p1.y < p2.y;
		}
	};
private:
	void _init(){ *this = Null(); }
};

inline bool operator==( const Point_t& p1, const Point_t& p2 ){
	return p1.x==p2.x && p1.y==p2.y;
}

inline int mahn( const Point_t& p1, const Point_t& p2 ){
	return std::abs( p1.x - p2.x ) + std::abs( p1.y - p2.y );
}

inline int sqdist( const Point_t& p1, const Point_t& p2 ){
	int dx = p1.x - p2.x;
	int dy = p1.y - p2.y;
	return dx * dx + dy * dy;
}

class Range_t{
public:
	int n1, n2;
	Range_t( int nn1=0, int nn2=0 ):n1(nn1),n2(nn2){
		if( n1>n2 ) std::swap(n1,n2);
	}
	//std::ostream& print(std::ostream& ostr)const{ }
	int scalar() const { return n2-n1;}
	bool has_overlap(const Range_t& range ) const {
		return std::max( n1, range.n1 ) <= std::min( n2, range.n2 );
	}
	Range_t overlap( const Range_t& range ) const {
		return has_overlap(range)? Range_t( std::max(n1,range.n1), std::min(n2,range.n2) ): Null();
	}
	static const Range_t Null(){ return Range_t(INT_MAX,INT_MAX); }
//	Rect_t expand( bool isV, int pos, int half_width )const;

	struct Cmptor{
		bool operator()( const Range_t& r1, const Range_t& r2 )
		const {
			if( r1.n1==r2.n1 ) return r1.n2 < r2.n2;
			return r1.n1 < r2.n1;
		}
	};
	struct ScalarCmp{
		bool operator()( const Range_t& r1, const Range_t& r2 )
		const {
			return r1.scalar() < r2.scalar();
		}
	};
};
inline bool operator==( const Range_t& range1, const Range_t& range2 ){
	return range1.n1 == range2.n1 && range1.n2 == range2.n2;
}

inline std::ostream& operator<<(std::ostream& ostr, const Range_t& range ){
	return ostr<<"["<<range.n1<<","<<range.n2<<"]";
}

typedef std::vector<Range_t> vRange_t;

class Line_t {
public:
	Point_t p1, p2;
	Line_t(){ _init(); }
	Line_t( const Point_t& np1, const Point_t& np2 ):p1(np1),p2(np2){}

	static const Line_t Null(){ return Line_t(Point_t::Null(),Point_t::Null()); }
private:
	void _init(){ *this = Null(); }
};

class Rect_t {
public:
	Rect_t(){ _init(); }
	Rect_t( const Point_t& np1, const Point_t& np2 ){
		p1.x = std::min(np1.x,np2.x); 
		p1.y = std::min(np1.y,np2.y);
		p2.x = std::max(np1.x,np2.x); 
		p2.y = std::max(np1.y,np2.y);
	}
	Rect_t( int x1, int y1, int x2, int y2 ):p1(x1,y1),p2(x2,y2){
		//_legalize();
	}
	Point_t p1, p2;
	Rect_t merge( const Rect_t& rect ) const {
		if( Null() == *this )
			return rect;
		if( Null() == rect )
			return *this;
		return Rect_t
			( std::min( p1.x, rect.p1.x ), std::min( p1.y, rect.p1.y )
			, std::max( p2.x, rect.p2.x ), std::max( p2.y, rect.p2.y ) );
	}
	Range_t rangeX() const { return Range_t( p1.x, p2.x ); }
	Range_t rangeY() const { return Range_t( p1.y, p2.y ); }
	int centerX() const { return ( p1.x + p2.x )>>1; }
	int centerY() const { return ( p1.y + p2.y )>>1; }
	bool isValid() const { return p1.x <= p2.x && p1.y <= p2.y; }
	static const Rect_t Null(){ return Rect_t(INT_MAX,INT_MAX,INT_MAX,INT_MAX); }
private:
	void _legalize(){*this = Rect_t(p1,p2);}
	void _init(){ *this = Null(); }
};

inline bool overlap( const Rect_t& rect1, const Rect_t& rect2 ){
	return rect1.rangeX().has_overlap( rect2.rangeX() )
	    && rect1.rangeY().has_overlap( rect2.rangeY() );
}

inline bool operator==( const Rect_t& rect1, const Rect_t& rect2 ){
	return rect1.p1 == rect2.p1 && rect1.p2 == rect2.p2;
}


class LineSeg_t: public std::list<Range_t>{
public:
	friend void LineSegCross( LineSeg_t& s1, LineSeg_t& s2 );
	typedef std::vector<iterator> vItr_t;
	typedef std::pair<iterator,iterator> ItrPair_t;
	int pos;
	void add( int from, int to );
	ItrPair_t overlap( int from, int to );
	void remove( ItrPair_t );
	void complement( int from, int to );
//	void break2try( int pos );
//	void mask( LineSeg_t& );
//	void merge_touch();
//	iterator break2( int pos, iterator itr );
	void print()const;
	bool valid() const {
		const_iterator itr = begin();
		for(const_iterator prev=itr++; itr!=end(); prev=itr++){
			if( prev->n2 > itr->n1 || prev->n1 > itr->n1 )
				return false; // range valid
		}
		return true;
	}
};


// Function of LingSeg_t
inline void LineSeg_t::remove( ItrPair_t range ){
	iterator to_remove = end();
	for(iterator itr = range.first; itr != range.second; itr++){
		if( end() != to_remove )
			erase( to_remove );
		to_remove = itr;
	}
	if( end() != to_remove )
		erase( to_remove );
}

inline LineSeg_t::ItrPair_t LineSeg_t::overlap( int from, int to ){
	assert(from<=to);
	iterator itr;
	ItrPair_t range( end(), end() );
	for( itr=begin(); itr!=end(); itr++ ){
		if( to <= itr->n1 ) break; 			//already too large
		if( from>= itr->n2 ) continue; 		//get larger one
		if( end() == range.first ) // if starting point is unset yet
			range.first = itr;
	}
	range.second = end()==range.first? end(): itr; // itr is end() or the last itr leads to break
	return range;
}
inline void LineSeg_t::add( int from, int to ){
	assert(from<=to);
	//declaration
	int NewFrom, NewTo;
	iterator itr;
	vItr_t vItr;

	//implementation
	ItrPair_t range = overlap( from, to );

	/** debug **
		LineSeg_t backup = *this;
		ItrPair_t brange = backup.overlap( from, to );
		/**/

	NewFrom = ( range.first != range.second )
		? std::min( range.first->n1, from ): from;
	NewTo 	= ( range.first != range.second )
		? std::max( Lem_ItrPrev(range.second)->n2, to   ): to;

	remove(range);
	for( itr=begin(); itr!=end(); itr++ ){
		if( NewTo <= itr->n1 ){
			break;
		}
		if( itr->n2 < NewFrom ) // keep searching
			continue;
	}
	if( NewTo == itr->n1 ){
		insert(itr,Range_t(NewFrom,itr->n2));
		erase(itr);
		itr = end();
	} else
		insert(itr,Range_t(NewFrom,NewTo)); //insert at the end
	
	/** debug **/
		assert(valid());
		/**/
	return;
}

inline void LineSeg_t::complement( int from, int to ){
	assert(from<=to);
	//declaration
	int first;
	iterator itr;
	LineSeg_t res;

	//initialization
	first = from;

	//implementation
	ItrPair_t range = overlap( from, to );

	itr = range.first;
	if( itr!=range.second ){
		if( itr->n1 <= first ){
			first = itr->n2;
			itr++;
		}
	}
	for( ; itr!=range.second; itr++ ){
		res.add( first, itr->n1 );
		first = itr->n2;
	}
	if( first < to )
		res.add( first, to );
	
	clear();
	splice( end(), res );

	/** debug **/
		assert(valid());
		/**/
	return;
}

// lem type function

inline Point_t Lem_R90Unit( const Point_t& point ){
	return Point_t( -point.y, point.x );
}
inline Point_t Lem_R90( const Point_t& point, int op ){
	// counter-clockwise
	Point_t ret( point.x, point.y );
	op = op%4;
	for(int i=0; i<op; i++)
		ret = Lem_R90Unit(ret);
	return ret;
}

inline Point_t Lem_Flip( const Point_t& point, const int axis, const bool flipAlongY=true ){
	return flipAlongY 
		? Point_t( Lem_Flip(point.x , axis), point.y )
		: Point_t( point.x, Lem_Flip(point.y , axis) );
}

inline Point_t Lem_Offset( const Point_t& point, const Point_t& origin, bool fMinus=false ){
	return fMinus? Point_t( point.x - origin.x, point.y - origin.y ): Point_t( point.x + origin.x, point.y + origin.y );
}

inline Rect_t Lem_R90( const Rect_t& rect, int op ){
	Point_t p1 = Lem_R90( rect.p1, op );
	Point_t p2 = Lem_R90( rect.p2, op );
	return Rect_t( p1, p2 );  // implicitly legalize
}

inline Rect_t Lem_Flip( const Rect_t& rect, const int axis, const bool flipAlongY=true ){
	Point_t p1 = Lem_Flip( rect.p1, axis, flipAlongY );
	Point_t p2 = Lem_Flip( rect.p2, axis, flipAlongY );
	return Rect_t( p1, p2 );  // implicitly legalize
}


inline Rect_t Lem_Offset( const Rect_t& rect, const Point_t& origin, bool fMinus=false ){
	Point_t p1 = Lem_Offset( rect.p1, origin, fMinus );
	Point_t p2 = Lem_Offset( rect.p2, origin, fMinus );
	return Rect_t( p1, p2 );  // implicitly legalize
}

inline std::ostream& operator<<( std::ostream& ostr, const Point_t& point ){
	ostr<<"("<< point.x <<","<<point.y<<")";
	return ostr;
}

inline std::ostream& operator<<( std::ostream& ostr, const Rect_t& rect ){
	ostr<< rect.p1;
	ostr<< rect.p2;
	return ostr;
}

#endif