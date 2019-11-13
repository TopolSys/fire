#include <iostream>
#include <fstream> 				// write guide file
#include "db/db.hpp" 			// parse options, and read input files
#include "utils/sys/sys.hpp" 	// memory

#include "core/tex/tex.hpp"
#include "utils/itp/ef2tex.hpp"

// algorithm
#include "core/tex/round.hpp"  // Lem_RoundRoute
#include "core/tex/route.hpp"  // Lem_InitRoute
#include "core/tex/lya.hpp"    // Lem_AssignLayer
#include "core/tex/guide.hpp"  // Lem_DeriveGuide


int main( int argc, char * argv[] ){
	//lefrInit();
	//std::cout<< ispd19::Setting::techLef() << std::endl;
	Lem_Gdb_t gdb;
	lem::Clk_t clk;
	clk.start();
	Tex_Man_t * pTex;
	gdb.ParseOptions(argc, argv);

	if( !gdb.BuildDB() ){
		goto SUMMARY;
	}
	// EF dependent 
	pTex = Lem_StartTexFromEf( &gdb );
	Lem_StartNetFromEf( &gdb, pTex );
	Lem_StartObsFromEf( &gdb, pTex );

	// pure algorithm
	//Lem_InitRoute( &gdb, pTex );
	Lem_InitRouteRace( &gdb, pTex );
	Lem_RoundRoute( &gdb, pTex );
	//Lem_RescueRoute( &gdb, pTex );
	if( "" != gdb.OutputFile() ){
		if( gdb.RaceLya() )
			Lem_AssignLayerRace( &gdb, pTex );
		else
			Lem_AssignLayer( &gdb, pTex );
		std::ofstream ostr(gdb.OutputFile());
		Lem_DeriveGuide( &gdb, pTex, ostr );
		ostr.close();
	}

	//pTex->plot("tex.plot");
	delete pTex;

SUMMARY:
	int nError = lem::error.size();
	int nWarning = lem::warning.size();
	if( nError )
		std::cout<<nError<<" errors "<<std::endl;
	if( nWarning )
		std::cout<<nWarning<<" warnings "<<std::endl;
	if( lem::dev() ){
		lem::error.dump();
		lem::warning.dump();
	}
	char munit[] = "M";
	std::cout<<"VmPeak: "<< lem::mem(munit)<<" "<<munit<< std::endl;
	std::cout<<"lem's time: "<<clk.elapsed()<<std::endl;
	std::cout<< gdb.Comment() <<std::endl;
	return 1;
}
