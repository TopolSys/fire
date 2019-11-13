#include "db.hpp"
#include "utils/sys/sys.hpp"
#include <unistd.h>
#include <dirent.h>
#include <cstdlib>
#include <cstring>

#include <vector>
#include <iostream>
#include <iomanip>

int  Lem_Gdb_t::_CheckParams(){
	int nErrors = 0;
	char buff[1024];


	if( ! _EfPath.empty() && (""==_DefFile || ""==_LefFile) ){
		DIR * dir;
		if( NULL == (dir = opendir(_EfPath.c_str())) ){
			sprintf(buff, "Exchange File Path \'%s\' is not accessible", _EfPath.c_str() );
			lem::error.add(buff);
			nErrors ++;
		} else {
			struct dirent * de;
			for( de = readdir(dir); NULL!=de; de = readdir(dir) ){
				int len = strlen( de->d_name );
				if( len < 4 )
					continue;
				if( "" == _DefFile ){
					if( 0==strcmp(de->d_name + len - 4, ".def") ){
						_DefFile = _EfPath + de->d_name;
						_ostr<<"Import DEF from "<<_DefFile<<"\n";
						continue;
					}
				}
				if( "" == _LefFile ){
					if( 0==strcmp(de->d_name + len - 4, ".lef") ){
						_LefFile = _EfPath + de->d_name;
						_ostr<<"Import LEF from "<<_LefFile<<"\n";
						continue;
					}
				}
			}
			closedir(dir);
		}
	}


	if( -1 == access( _DefFile.c_str(), R_OK ) ){
		sprintf(buff, "DefFile \'%s\' is not readable or does not exist", _DefFile.c_str() );
		lem::error.add(buff);
		nErrors ++;
	}
	if( -1 == access( _LefFile.c_str(), R_OK ) ){
		sprintf(buff, "LefFile \'%s\' is not readable or does not exist", _LefFile.c_str() );
		lem::error.add(buff);
		nErrors ++;
	}
	return nErrors==0;
}

void Lem_Gdb_t::ParseOptions( int argc, char * argv[] ){
	int argTop = 1;

	//parse options
	for(; argTop<argc; argTop++){
		if( argv[argTop][0] != '-' )
			break;

		if( !strcmp(argv[argTop]+1, "h") ){
			goto Usage;
		}
		
		if( !strcmp(argv[argTop]+1, "lef") ){
			if( argTop+1 >= argc )
				goto Usage;
			argTop++;
			_LefFile = argv[argTop];
			continue;
		}
		
		if( !strcmp(argv[argTop]+1, "def") ){
			if( argTop+1 >= argc )
				goto Usage;
			argTop++;
			_DefFile = argv[argTop];
			continue;
		}

		if( !strcmp(argv[argTop]+1, "efp") ){
			if( argTop+1 >= argc )
				goto Usage;
			argTop++;
			_EfPath = argv[argTop];
			continue;
		}
		
		if( !strcmp(argv[argTop]+1, "threads") ){
			if( argTop+1 >= argc )
				goto Usage;
			argTop++;
			_nThread = atoi(argv[argTop]);
			if( _nThread < 1){
				char buff[2048];
				snprintf(buff,2047,"Thread number should > 0 (given %d). Using default=1", _nThread);
				lem::warning.add(buff);
				_nThread = 1;
			}
			continue;
		}
		
		if( !strcmp(argv[argTop]+1, "output") ){
			if( argTop+1 >= argc )
				goto Usage;
			argTop++;
			_OutputFile = argv[argTop];
			continue;
		}
	}
	PrintParams( std::cout );
	return ;
Usage:
	printf("./lem -lef <LEF_FILE> -def <DEF_FILE> -efp <EX_FILE_PATH> [-threads <THREAD_NUM>] -output <OUTPUT_FILE>\n");
}

int  Lem_Gdb_t::BuildDB(){
	if( ! _CheckParams() ){
		lem::error.dump();
		return 0;
	}

	if( ! ImportFromExchangeFile( _LefFile.c_str(), _DefFile.c_str() ) )
		return 0;
	
	return 1;
}

void Lem_Gdb_t::PrintParams( std::ostream& ostr ){
	std::ostream * postr = _ostr.redirect(&ostr);
	#define CndPrintArg(ostr,cnd,op,a) if(cnd op a)ostr<<std::setw(15)<< #a" = "<< a <<std::endl;
	_ostr<<"Cmd arguments:\n";
	CndPrintArg(_ostr, "", !=, _LefFile);
	CndPrintArg(_ostr, "", !=, _DefFile);
	CndPrintArg(_ostr, "", !=, _EfPath );
	CndPrintArg(_ostr, "", !=, _OutputFile );
	CndPrintArg(_ostr,  1, !=, _nThread);
	#undef CndPrintArg

	if( lem::dev() ){
		#define printParam(ostr,a) ostr<<std::setw(19)<< #a" = "<< a <<std::endl;
		_ostr <<"Parameters: \n";
		_ostr<<"Route:\n";
		printParam(_ostr, _Prefill2D   );
		printParam(_ostr, _RoundLim    );
		printParam(_ostr, _RoundStep   );
		printParam(_ostr, _DahReset    );

		_ostr<<"Lya:\n";
		printParam(_ostr, _Prefill3D   );		
		printParam(_ostr, _StartLya    );
		printParam(_ostr, _LyaThrough  );
		printParam(_ostr, _DynViaCost  );
		printParam(_ostr, _IntlvSort   );
		printParam(_ostr, _OvflWall    );
		printParam(_ostr, _Lya1Bfs     );
		printParam(_ostr, _Lv2CostWt   );
		printParam(_ostr, _LowCostTh   );
		printParam(_ostr, _LayerFactor );
		printParam(_ostr, _PrlFactor   );

		printParam(_ostr, _AuxGuide    );
		printParam(_ostr, _AuxRatio    );
		printParam(_ostr, _AuxSegSize  );
		printParam(_ostr, _DynPrefill3D);
		#undef printParam
	}

	_ostr.redirect(postr);
}

Lem_Gdb_t::Lem_Gdb_t():_ostr("GDB",""){
	_DefaultParams();
}

void Lem_Gdb_t::_DefaultParams(){
	_nThread = 1;

	_Prefill2D   =  0.70;
	_RoundLim    =    16;
	_RoundStep   =  0.06;
	_DahReset    =  true;
	_ReuseTrial  =  true;
	_TexLoad     = false;

	_Prefill3D   =  0.75;
	_StartLya    =     1;
	_SegChunk    =     0;
	_RaceLya     =  true;
	_LyaThrough  =  true;
	_DynViaCost  = false;
	_Lya1Bfs     = false;
	_Lv2CostWt   =     1;
	_LowCostTh   =     1;
	_LayerFactor =  true;

	_AuxGuide    =  true;
	_AuxRatio    =   0.1;
	_AuxSegSize  =     6;
	_DynPrefill3D= false;

	_PrlFactor   =  true;

	_IntlvSort  = false;
	_OvflWall   =  true;


	char buff[2048];
	//snprintf(buff,2047,"Modify pin prefill counting. Exclusive pin prefill rate. Probably fix another via counting bug. Slightly modified 3D prefill. Pin2Load. Correct lya congest. Using normal prefill rate for pinPreload. %s %s", __DATE__, __TIME__ );
	//snprintf(buff,2047,"Add only upper adj via. Obs pin penalty. No-stdpin cong-aware. No Boundary Cost. Version %s %s", __DATE__, __TIME__ );
	snprintf(buff,2047,"Version %s %s", __DATE__, __TIME__ );
	_Comment   = buff;
}
