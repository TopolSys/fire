#include "lem_geom.hpp"

void LineSeg_t::print()const{
	for(const_iterator itr=begin(); itr!=end(); itr++)
		std::cout<< *itr<<" ";
	//std::cout<<"\n";
}