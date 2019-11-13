#ifndef CORE_TEX_LYA_HPP
#define CORE_TEX_LYA_HPP

#include <vector>
#include "core/tex/teg.hpp"
#include "core/tex/tex.hpp"
#include "lem_type.hpp"
#include "utils/sys/thread.hpp"
class Tex_Man_t;
class Lem_Gdb_t;

// Lya tex
class Lex_t {
public:
	Lex_t():_load(0),_pload(0),_wload(0){}
	void incLoad(){ ++_load; }
	void decLoad(){ --_load; }
	void incWireLoad(){ ++_wload; }
	void decWireLoad(){ --_wload; }
	const int load() const { return _load; }
	const int wload() const { return _wload; }
	void incPinLoad(int val){ _pload += val; }
	void decPinLoad(int val){ _pload -= val; }
	const int PinLoad() const { return _pload; }
private:
	int _load, _pload, _wload;
};

class Lya_t: public Lemcube_t<Lex_t> {
public:
	void print_cube() const ;
public:
	typedef std::vector<Seg_t*> vSegPtr_t;
	typedef std::vector<vSegPtr_t> mSegPtr_t;
	Lya_t( Lem_Gdb_t * pGdb, Tex_Man_t * pTex );
	void  init_assign();
	void round_assign();
	void head_statistic() const ;
	const int overflow_offset(bool fSoftOverflow) const { return fSoftOverflow? _SOFT_OVERFLOW_OFFSET: _OVERFLOW_OFFSET; }
	const int via_offset() const { return _VIA_OFFSET; }
private:
	int  get_segCost( const Seg_t& Seg, int layer ) const ;
	int  get_nOverflow( const Seg_t& Seg, int layer, bool fSoftOverflow ) const ;
	int  get_nVia   ( const Seg_t& Seg, int layer ) const ;
	bool is_head_layer( const Seg_t& Seg, int layer ) const ;
	bool is_tail_layer( const Seg_t& Seg, int layer ) const ;
	bool is_overflow( int tex_index, int layer, bool fSoftOverflow ) const ;
	int  get_obsLoad( int tex_index, int layer ) const ;
	int  get_netLoad( int tex_index, int layer ) const ;
	int  get_viaLoad( int tex_index, int layer ) const ;
	int  get_wireLoad( int tex_index, int layer ) const ;
	int  get_stdPinLoad( int tex_index, int layer ) const ;
	int  get_cap    ( int tex_index, int layer ) const ;

	int  get_nVia   ( const Seg_t& Seg ) const ;
	int  get_nVia   ( const int tcl ) const ;

	void assign_layer( Seg_t& Seg, int layer );
	void detach_layer( Seg_t& Seg );

	void inc_netLoad( const Seg_t& Seg, int index, int layer, bool isWire=true );
	void dec_netLoad( const Seg_t& Seg, int index, int layer, bool isWire=true );
	void inc_netLoad( int tex_index, int layer );
	void dec_netLoad( int tex_index, int layer );

	vSegPtr_t _vSegPtr;
	Lem_Gdb_t * _pGdb;
	Tex_Man_t * _pTex;
	int _OVERFLOW_OFFSET, _SOFT_OVERFLOW_OFFSET;
	int _VIA_OFFSET;
	int _start_layer;
	int _PinFillCost;

public: // for race assignment
	void init_race();
	int  round();
	int  auxguide();
	int  round_stats(bool fPrint);
	int countVia(const Tcl_t&) const ;
	int  get_nOverflow( const Seg_t& Seg, bool fSoftOverflow ) const ;
	int  get_nOverflowLayer( const Seg_t& Seg, int layer, bool fSoftOverflow ) const ;
	int  relax();
	bool using_relax() const { return _fRelax; }
	bool using_aux_guide() const { return _fAuxGuide; }
	int  num_via () const ;
	int  num_ovfl( const bool fSoftOverflow ) const ;
	int  num_ovfl_layer( const int layer, const bool fSoftOverflow ) const ;
	bool check_load_equal() ;
	int _nRelaxRound;
private:
	lem::Spin_t _progSpin;
	//int  prlCost(Seg_t&, int index, int layer);
	void set_relax(){ _fRelax = true ; }
	void del_relax(){ _fRelax = false; }
	void set_aux_guide(){ _fAuxGuide = true ; }
	void del_aux_guide(){ _fAuxGuide = false; }
	void DirectionalAssign(const bool isV);
	void assign_tex_array(const bool * pisV, lem::Jidep_t<>* pJidep, lem::Progress_t<int>* pProg);
	void assign_seg_array(vSegPtr_t&);
	void assign_seg(Seg_t&);

	void auxguide_tex_assign(int layer, const vInt_t& vTex, vIntPair_t& vPos );
	void auxguide_tex_array(const bool * pisV, lem::Jidep_t<>* pJidep, lem::Progress_t<int>* pProg);
	void auxguide_seg_array(vSegPtr_t& vSegPtr, bool isV);

	int  congestCost(Seg_t& Seg, int index, int layer);
	IntPair_t throughRange(const IntPair_t& layerRange, const int& dist_layer );
	int  throughCost (const IntPair_t& thRange, Seg_t& Seg, int index);
	bool is_wpot(Seg_t& Seg, int index) const ;
	int  num_std_pin(const Seg_t& Seg, int index) const ;
	void race_attachLayer(Seg_t& Seg, mInt_t& mCost, mInt_t& mSel );
	void race_detachLayer(Seg_t& Seg);
	mSegPtr_t SegTexV, SegTexH;
	int _HardLimit;
	int _StepCost, _ViaCost, _WrongWayCost;
	bool _fRelax, _fAuxGuide;
	vFlt_t _DPrefill;
};

void Lem_AssignLayer( Lem_Gdb_t * pGdb, Tex_Man_t * pTex );
void Lem_AssignLayerRace( Lem_Gdb_t * pGdb, Tex_Man_t * pTex );
#endif