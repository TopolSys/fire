#ifndef SYS_HPP
#define SYS_HPP

#include <list>
#include <string>
#include <iostream>
#include <time.h>

namespace lem {
double mem( const char * f="k"); //f: 'k'/'K', 'm'/'M', 'g'/'G'

class logger {
public:
	logger( const char * title, const char * colors );
	void add( const char * str );
	void dump( std::ostream * postr=NULL );
	void cast_output( const char * msg, std::ostream * postr=NULL ) const ;
	void clear();
	int size() const { return _info.size(); }
	std::ostream * redirect( std::ostream * postr );
	template<typename T>
	std::ostream& operator<<(const T& DATA) const { _head(_postr); (*_postr)<<" "<<DATA; return *_postr;}
private:
	void _head( std::ostream * postr ) const ;

	int _dump_top; // points to the last undumped object
	std::string _title;
	std::string _colors;
	std::list< std::string > _info;
	std::ostream * _postr;
};

extern logger error;
extern logger warning;
extern int mode; 		// 0: operation mode, 1: develop mode

class Trav_t {
public:
	Trav_t():_step(0){}
	void reset    ()       { _step = 0; }
	int  step     ()       { return ++_step; }
	int  current  () const { return _step; }
	int  step_back()       { return --_step; }
	void set(int step)     { _step = step; }
private:
	int _step;
};

template<typename TYPE>
class Progress_t {
public:
	Progress_t( int width, TYPE ub ):_width(width),_ub(ub),_count(0){}
	Progress_t& set( TYPE cur ){ _count = cur; return *this; }
	void print( std::ostream& ostr ) const {
		float ratio = (float)_count/_ub;
		int pos   = (float) _width * ratio;
		ostr<<"[";
		for(int i=0; i<_width; i++){
			if( i<pos )
				ostr<<"=";
			else
			if( i==pos )
				ostr<<">";
			else
				ostr<<" ";
		}
		ostr<<"] "<< int(ratio*100) <<"%, "<<_count<<"/"<<_ub<<"\r";
		ostr.flush();
	}
private:
	int _width;
	TYPE _ub, _count;
};
inline int operator==( const Trav_t& t1, const Trav_t& t2 ){
	return t1.current() == t2.current();
}

class Clk_t {
public:
	//clock_t clk, accu_clk;
	struct timespec clk, accu_clk;
	bool fCount;
	std::string name;
	int nAccess;
	Clk_t( const char * Name=NULL ):fCount(false){
		if( NULL!=Name ) name = Name;
		nAccess = 0;
		//clk = 0;\
		accu_clk = 0;
		clk.tv_nsec = 0;
		accu_clk.tv_nsec = 0;
		clk.tv_sec = 0;
		accu_clk.tv_sec = 0;
	}
	void start();
	double check();
	double elapsed();
	static double nsec(){ return 1000000000.0f; }
};

inline double Clk_t::elapsed(){
	bool fRestart = false;
	if( (fRestart = fCount) )
		check();
	if( fRestart )
		start();
	//return (double)accu_clk/CLOCKS_PER_SEC;
	return (double) accu_clk.tv_sec + accu_clk.tv_nsec / nsec();
}

inline void Clk_t::start(){
	if( true==fCount ){
		std::cout<<"Warning: start a already started timer"<<std::endl;
	}
	fCount = true;
	//clk = clock();
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &clk);
}

inline double Clk_t::check(){
	if( false==fCount ){
		std::cout<<"Warning: check timer before it starts"<<std::endl;
		return 0;
	}
	fCount = false;
	//clock_t clk_cur = clock();
	//accu_clk += clk_cur-clk;\
	return (double)(clk_cur-clk)/CLOCKS_PER_SEC;
	

	struct timespec clk_cur, clk_inc;
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &clk_cur);
	clk_inc. tv_sec = clk_cur. tv_sec- clk. tv_sec;
	clk_inc.tv_nsec = clk_cur.tv_nsec- clk.tv_nsec;
	if( clk_inc.tv_nsec<0 ){
		clk_inc. tv_sec -= 1;
		clk_inc.tv_nsec += nsec();
	}
	accu_clk.tv_sec += clk_inc. tv_sec;
	accu_clk.tv_nsec+= clk_inc.tv_nsec;
	return (double)clk_inc. tv_sec + clk_inc.tv_nsec/nsec();
}

bool dev();

};

#endif