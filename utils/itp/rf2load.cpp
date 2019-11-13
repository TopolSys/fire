#include "db/db.hpp" 						//db access

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cassert>
#include "lem_mac.hpp" 	// tile vertex 
#include "core/tex/tex.hpp" 	// tile vertex 
#include "utils/sys/sys.hpp" 	// error logging
/**

int Lem_Rf2LoadWriteTclTeidSub( std::ostream& ostr, const Tcl_t& Tcl, int pair_index ){
	ostr<<"subteid "<<pair_index<<" ";
	for(int i=0; i<Tcl.mTegTeid().size(); i++)
		ostr<< Tcl.mTegTeid()[pair_index][i]<<" ";
	ostr<<" ;\n";
}

int Lem_Rf2LoadWriteTclTeid( std::ostream& ostr, const Tcl_t& Tcl ){
	ostr<<"teid \n";

	ostr<<"fullteid ";
	for(int i=0; i<Tcl.vTeid().size(); i++)
		ostr<<Tcl.vTeid()[i]<<" ";
	ostr<<";\n";
	for(int i=0; i<Tcl.mTegTeid().size(); i++){
		ostr<<"\t";
		Lem_Rf2LoadWriteTclTeidSub(ostr, Tcl, i);
	}

	ostr<<";\n";
}

int Lem_Rf2LoadWriteTclPathSub( std::ostream& ostr, const Tcl_t& Tcl, int pair_index ){
	ostr<<"subpath "<<pair_index<<" ";
	for(int i=0; i<Tcl.mTegPath().size(); i++)
		ostr<< Tcl.mTegPath()[pair_index][i]<<" ";
	ostr<<" ;\n";
}

int Lem_Rf2LoadWriteTclPath( std::ostream& ostr, const Tcl_t& Tcl ){
	ostr<<"path \n";

	ostr<<"fullpath ";
	for(int i=0; i<Tcl.vPath().size(); i++)
		ostr<<Tcl.vPath()[i]<<" ";
	ostr<<" ;\n";
	for(int i=0; i<Tcl.mTegPath().size(); i++){
		ostr<<"\t";
		Lem_Rf2LoadWriteTclPathSub(ostr, Tcl, i);
	}
	ostr<<";\n";
}
int Lem_Rf2LoadWriteTcl( std::ostream& ostr, const Tcl_t& Tcl, int nid ){
	ostr<<"net "<<nid<<"\n";
	Lem_Rf2LoadWriteTclTeid(ostr, Tcl);
	Lem_Rf2LoadWriteTclPath(ostr, Tcl);
	ostr<<";\n";
}
int Lem_Rf2LoadWrite( std::ostream& ostr, const Lem_Gdb_t * pGdb ){
	int nNet = 0;
	Lem_IterateCount(pGdb->vTcl, pTcl, nNet){
		Lem_Rf2LoadWriteTcl(ostr, *pTcl, nNet);
	}
}



static bool isLineEnd( const std::string& line ){
	std::istringstream istr(line);
	std::string word;
	return (istr>>word) ? word==";": false ;
}

static bool isLinePath( const std::string& line ){
	std::istringstream istr(line);
	std::string word;
	return (istr>>word) ? word=="path": false ;
}
static bool isLineTeid( const std::string& line ){
	std::istringstream istr(line);
	std::string word;
	return (istr>>word) ? word=="teid": false ;
}

static int isLineNet( const std::string& line ){
	std::istringstream istr(line);
	std::string word;
	int nid = -1;
	if( (istr>>word) ? word!="net": false )
		return -1;
	if( (istr>>nid) )
		return nid;
	return -1;
}



int Lem_Rf2LoadReadTclPath(Tcl_t& Tcl, std::istream& istr ){
	;
	std::string line;
	while( std::getline(istr,line) ){
		if( line.empty() )
			continue;
		;
	}
}

int Lem_Rf2LoadReadTcl(Lem_Gdb_t * pGdb, std::istream& istr, int nid){
	int nPath = 0;
	int nTeid = 0;
	Tcl_t& Tcl = pGdb->vTcl[nid];
	//if( Tcl.mTegPath().size() )
	std::string line;
	while( std::getline(istr,line) ){
		if( line.empty() )
			continue;

		if( isLineEnd (line) )
			break;
		if( isLinePath(line) ){
			if( -1==Lem_Rf2LoadReadTclPath(Tcl, istr) )
				goto ERROR;
			continue;
		}
		if( isLineTeid(line) ){
			continue;
		}
	}
	return 1;
ERROR:
	return 0;
}

int Lem_Rf2LoadRead ( std::istream& istr, Lem_Gdb_t * pGdb ){
	int nid = -1;
	std::string line;
	while( std::getline(istr,line) ){
		if( line.empty() )
			continue;
		if( -1==(nid = isLineNet(line)) )
			continue;
		if( !Lem_Rf2LoadReadTcl(pGdb, istr, nid) ){
			std::cout<<"error in Rf2LoadRead"<<std::endl;
			return 0;
		}
	}
	return 1;
}
/**/