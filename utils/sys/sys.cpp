#include "sys.hpp"
#include <cstdio>
#include <ctype.h>
#include <cstring>

#include <iostream>

namespace lem {

static int getVmPeak(){
	int mem_sz = 0;
	const char * mem_key = "VmPeak:";
	const int len = strlen( mem_key);

	char buff[1024];
	FILE * f = fopen("/proc/self/status", "r");
	if( NULL==f )
		return 0;
	while( fgets(buff, 1024, f) ){
		if( strlen(buff) < len + 1 )
			continue;
		buff[len] = '\0';
		if( strcmp(buff, mem_key) )
			continue;
		if( EOF == sscanf(buff + len + 1, "%d kB\n", &mem_sz) )
			continue;
		break;
	}
	fclose(f);

	return mem_sz;
}

double mem( const char * f ){
	int val = getVmPeak();
	if( 0==val )
		return 0;
	switch( tolower(f[0]) ){
		case 'k': return val;
		case 'm': return (double) val/1024;
		case 'g': return (double) (val/1024)/1024;
	}
	return val;
}

logger::logger( const char * title, const char * colors ){
	_dump_top = 0;
	_title    = title ;
	_colors   = colors;
	_postr    = & std::cout;
}

void logger::add( const char * str ){
	_info.push_back(str);
}

void logger::clear(){
	_dump_top = 0;
	_info.clear();
}
void logger::cast_output( const char * msg, std::ostream * postr ) const {
	std::ostream& ostr = postr? * postr: * _postr;
	_head(&ostr);
	ostr<<" "<< msg <<"\n";
}

void logger::_head( std::ostream * postr ) const {
	std::ostream& ostr = * postr;
	if( !_colors.empty() )
		ostr<<"\033["<< _colors <<"m";
	
	if( !_title.empty() )
		ostr<<"["<<_title<<"]";

	if( !_colors.empty() )
		ostr<<"\033[0m";
}

std::ostream * logger::redirect( std::ostream * postr ){
	std::ostream * ret = _postr;
	_postr = postr;
	return ret;
}

void logger::dump( std::ostream * postr ){
	using namespace std;
	std::ostream& ostr = postr? * postr: * _postr;
	int i=0;
	for(list<string>::iterator itr=_info.begin(); itr!=_info.end(); itr++, i++){
		if( i<_dump_top )
			continue;
		_dump_top ++;
		cast_output( itr->c_str(), &ostr );
	}
}

bool dev(){ return 1==mode; }

logger error("ERROR", "47;31");
logger warning("WARNING", "33");
int mode=0;
} // end of namespace lem