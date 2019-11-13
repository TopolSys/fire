#ifndef DB_HPP
#define DB_HPP

#include <iostream>
#include <string>
#include <list>

#include "core/tex/teg.hpp"

namespace ispd19{
class Database;
}

// Lem Global-routing DataBase
class Lem_Gdb_t {
public:
	Lem_Gdb_t();
	void ParseOptions( int argc, char * argv[] );
	int  BuildDB();
	void PrintParams( std::ostream& );
	int  nThread() const { return _nThread;}

	vTcl_t vTcl; // Teg CLuster

	// formate dependent API
	ispd19::Database * efdb();
	const std::string& OutputFile()const{ return _OutputFile; }
	const std::string& Comment   ()const{ return _Comment; }
	int   RoundLim   () const { return _RoundLim; }
	int   StartLya   () const { return _StartLya; }
	int   SegChunk   () const { return _SegChunk; }
	float Prefill2D  () const { return _Prefill2D; }
	float Prefill3D  () const { return _Prefill3D; }
	float RoundStep  () const { return _RoundStep; }
	bool  RaceLya    () const { return _RaceLya  ; }
	bool  DahReset   () const { return _DahReset ; }
	bool  ReuseTrial () const { return _ReuseTrial; }
	bool  LyaThrough () const { return _LyaThrough; }
	bool  DynViaCost () const { return _DynViaCost; }
	bool  TexLoad    () const { return _TexLoad  ; }
	bool  IntlvSort  () const { return _IntlvSort; }
	bool  OvflWall   () const { return _OvflWall ; }
	bool  Lya1Bfs    () const { return _Lya1Bfs  ; }
	float Lv2CostWt  () const { return _Lv2CostWt; }
	float LowCostTh  () const { return _LowCostTh; }

	bool  LayerFactor() const { return _LayerFactor; }
	bool  PrlFactor  () const { return _PrlFactor  ; }

	bool  AuxGuide   () const { return _AuxGuide; }
	float AuxRatio   () const { return _AuxRatio ; }
	float AuxSegSize () const { return _AuxSegSize; }

	bool DynPrefill3D() const { return _DynPrefill3D; }
private:
	int _nThread;
	std::string _LefFile, _DefFile, _EfPath, _OutputFile;
	std::string _Comment;
	int ImportFromExchangeFile( const char * LefFile, const char * DefFile );
	void _FinalizeGCellGrid();
	void _DefaultParams();
	int  _CheckParams();

	int  _RoundLim, _StartLya, _SegChunk;
	bool _DahReset, _RaceLya, _ReuseTrial, _LyaThrough, _DynViaCost, _TexLoad, _Lya1Bfs;
	bool _IntlvSort, _OvflWall, _LayerFactor, _PrlFactor, _AuxGuide, _DynPrefill3D;
	int   _AuxSegSize;
	float _AuxRatio;
	float _Lv2CostWt, _LowCostTh;
	float _Prefill2D, _Prefill3D, _RoundStep;
	lem::logger _ostr;
};

#endif