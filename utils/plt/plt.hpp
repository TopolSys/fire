#ifndef UTILS_PLT_HPP
#define UTILS_PLT_HPP

#include <iostream>
#include <fstream>
#include "lem_geom.hpp"
class Lem_Rgb_t {
public:
	Lem_Rgb_t( unsigned char na, unsigned char nr, unsigned char ng, unsigned char nb ):a(na),r(nr),g(ng),b(nb){}
	unsigned char a,r,g,b;
};

class Lem_Plt_t {
public:
	Lem_Plt_t( const char * FileName ):_FileName(FileName){
		ostr.open(_FileName);
		_init();
	}
	~Lem_Plt_t(){ if(_FileName) ostr.close(); }

	void addLine ( const Line_t& line ){
		_addArrow(line.p1, _Null(), line.p2, _Null(), " nohead");
	}
	void addArrow( const Point_t& p1, const Point_t& p2 ){
		_addArrow(     p1, _Null(),      p2, _Null(),        "");
	}
	void addArrow( const Point_t& p1, const int& z1, const Point_t& p2, const int& z2 ){
		_addArrow(     p1,      z1,      p2,      z2,        "");
	}

	void addRect( const Rect_t& rect, const Lem_Rgb_t& rgb ){
		_addRect(rect,rgb,"fc","");
	}

	void tail(){
		if( _Null()!=_min_x && _Null()!=_max_x )
			ostr<<"set xrange ["<< _min_x<<":"<<_max_x<<"]\n";
		if( _Null()!=_min_y && _Null()!=_max_y )
			ostr<<"set yrange ["<< _min_y<<":"<<_max_y<<"]\n";
		if( _Null()!=_min_z && _Null()!=_max_z )
			ostr<<"set zrange ["<< _min_z<<":"<<_max_z<<"]\n";

		if( _Null()!=_min_x && _Null()!=_max_x && _Null()!=_min_y && _Null()!=_max_y ){
			_addRect( Rect_t(Point_t(_min_x,_min_y), Point_t(_max_x,_max_y)), Lem_Rgb_t(0,0,0,0),"fc", "behind" );
		}
		ostr<<"set lmargin 0\n";
		ostr<<"set rmargin 0\n";
		ostr<<"set bmargin 0\n";
		ostr<<"set tmargin 0\n";

		ostr<<"set terminal png size "<< _width<<","<< _height<<" truecolor\n";
		ostr<<"set output \""<< _FileName <<".png\"\n";
		if( _Null()!=_min_z && _Null()!=_max_z )
			ostr<<"splot 0";
		else
			ostr<<"plot 0";
	}
private:
	void _addRect( const Rect_t& rect, const Lem_Rgb_t& rgb, const char * color, const char * style ){
		_update_range(rect.p1);
		_update_range(rect.p2);
		ostr<<"set obj "<< ++ _nObj <<" rect from ";
		ostr<< rect.p1.x <<","<< rect.p1.y;
		ostr<<" to ";
		ostr<< rect.p2.x <<","<< rect.p2.y;
		char buff[100];
		sprintf(buff,"%02x%02x%02x%02x",rgb.a,rgb.r,rgb.g,rgb.b);
		ostr<<" "<<color<<" rgb \"#"<< buff<<"\"";
		ostr<<" "<< style<<"\n";
	}
	void _addArrow( const Point_t& p1, const int& z1, const Point_t& p2, const int& z2, const char * attr ){
		_update_range(p1);
		_update_range(p2);
		ostr<<"set arrow "<< ++_nArrow <<" from ";
		ostr<< p1.x<<","<<p1.y;
		if( _Null()!=z1 && _Null()!=z2 )
			ostr<<","<<z1;
		ostr<<" to ";
		ostr<< p2.x<<","<<p2.y;
		if( _Null()!=z1 && _Null()!=z2 )
			ostr<<","<<z2;
		ostr<<" "<<attr<<"\n";
	}
	int _Null(){ return INT_MAX; }
	void _init(){
		_nArrow = 0;
		_nObj = 0;
		_width = 900;
		_height= 900;
		_min_x = _Null();
		_min_y = _Null();
		_min_z = _Null();
		_max_x = _Null();
		_max_y = _Null();
		_max_z = _Null();
	}
	void _update_range( const Point_t& point ){
		_min_x = _min_x!=_Null()? std::min(_min_x, point.x): point.x;
		_min_y = _min_y!=_Null()? std::min(_min_y, point.y): point.y;
		_max_x = _max_x!=_Null()? std::max(_max_x, point.x): point.x;
		_max_y = _max_y!=_Null()? std::max(_max_y, point.y): point.y;
	}
	void _update_range( const Point_t& point, const int& z ){
		_update_range(point);
		_min_z = std::min(_min_z, z);
		_max_z = std::max(_max_z, z);
	}
	const char * _FileName;
	std::ofstream ostr;
	int _width, _height;
	int _nArrow, _nObj;
	int _min_x, _min_y, _min_z;
	int _max_x, _max_y, _max_z;
};

inline Lem_Plt_t& operator<<( Lem_Plt_t& plt, const Line_t& line ){
	plt.addLine(line);
	return plt;
}

#endif