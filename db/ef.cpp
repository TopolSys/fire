#include <iostream>
#include <algorithm>

#include "db.hpp"
#include "lem_mac.hpp"
#include "lem_type.hpp"
#include "utils/sys/sys.hpp"

#include "ispd2019_opensource/src/global.h"
#include "ispd2019_opensource/src/db/db.h"
#include "ispd2019_opensource/src/io/io.h"
#include "ispd2019_opensource/src/setting.h"

ispd19::Database * Lem_Gdb_t::efdb(){
	return ispd19::Database::get();
}

void Lem_Gdb_t::_FinalizeGCellGrid(){
	using namespace ispd19;
	vInt_t& gridX = (vInt_t&) Database::get()->gcells().grid(1);
	vInt_t& gridY = (vInt_t&) Database::get()->gcells().grid(0);
	std::sort( gridX.begin(), gridX.end() );
	std::sort( gridY.begin(), gridY.end() );

	vInt_t vTmp;
	for(int i=0; i<gridX.size(); i++ ){
		if( i>0? gridX[i]==gridX[i-1]: 0 )
			continue;
		vTmp.push_back(gridX[i]);
	}
	if( vTmp.size() != gridX.size() )
		gridX = vTmp;

	vTmp.clear();
	for(int i=0; i<gridY.size(); i++ ){
		if( i>0? gridY[i]==gridY[i-1]: 0 )
			continue;
		vTmp.push_back(gridY[i]);
	}
	if( vTmp.size() != gridY.size() )
		gridY = vTmp;
}

int Lem_Gdb_t::ImportFromExchangeFile( const char * LefFile, const char * DefFile  ){
	char buff[1024];
	using namespace ispd19;
	Setting::techLef( LefFile );
	Setting::designDef( DefFile );
	if( ! IO::readLef( Setting::techLef() ) ){
		std::cout<< "load lef file fail"<<std::endl;
		return 0;
	}
	if( ! IO::readDef( Setting::designDef() ) ){
		std::cout<< "load def file fail"<<std::endl;
		return 0;
	}

	if( ! efdb()->warnings.empty() ){
		sprintf(buff,"LEF/DEF parser %d warnings", efdb()->warnings.size() );
		if( lem::dev() )
			Lem_Iterate(efdb()->warnings, itr)
				lem::warning.cast_output(itr->c_str());
		lem::warning.add(buff);
		//if( lem::dev() )\
			return 0;
	}
	if( ! efdb()->errors.empty() ){
		sprintf(buff,"LEF/DEF parser %d errors", efdb()->errors.size() );
		if( lem::dev() )
			Lem_Iterate(efdb()->errors, itr)
				lem::error.cast_output(itr->c_str());
		lem::error.add(buff);
		if( lem::dev() )
			return 0;
	}

	_FinalizeGCellGrid();

	if( lem::dev() ){
		std::cout<<"Net# "<< Database::get()->nets().size()<<std::endl;
		std::cout<<"Layer# "<< Database::get()->numLayers()<<std::endl;
	}
	return 1;
}
