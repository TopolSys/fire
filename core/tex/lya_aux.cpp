#include "lya.hpp"
#include "lya_tcl.hpp"
#include "db/db.hpp"
#include "core/tex/tex.hpp"

int  Lya_t::get_cap( int tex_index, int layer ) const {
	Tex_Man_t::Pos_t pos = _pTex->getPos( tex_index );
	bool isV = _pTex->layerV()[layer];
	return _pTex->get_layerCap(isV, layer, isV? pos._x: pos._y );
}

int  Lya_t::get_netLoad( int tex_index, int layer ) const {
	Tex_Man_t::Pos_t pos = _pTex->getPos( tex_index );
	return entry(layer, pos._y, pos._x).load();
}

int  Lya_t::get_viaLoad( int tex_index, int layer ) const {
	Tex_Man_t::Pos_t pos = _pTex->getPos( tex_index );
	return entry(layer, pos._y, pos._x).load() - entry(layer, pos._y, pos._x).wload();
}

int  Lya_t::get_wireLoad( int tex_index, int layer ) const {
	Tex_Man_t::Pos_t pos = _pTex->getPos( tex_index );
	return entry(layer, pos._y, pos._x).wload();
}

int  Lya_t::get_stdPinLoad( int tex_index, int layer ) const {
	Tex_Man_t::Pos_t pos = _pTex->getPos( tex_index );
	return entry(layer, pos._y, pos._x).PinLoad();
}

int  Lya_t::get_obsLoad( int tex_index, int layer ) const {
	bool isV = _pTex->layerV()[layer];
	return _pTex->getTex(tex_index).layerObs(layer, _pTex->layerV()[layer]);
}

void Lya_t::assign_layer( Seg_t& Seg, int layer ){
	if( Seg.layer()!=Seg_t::NullLayer() )
		detach_layer(Seg);
	
	Seg.set_layer(layer);
	if( Seg.isPinFill() && 0==_PinFillCost )
		return;

	//if( ! Seg.vTeg().empty() )\
		return ;
	for(int i=0; i<Seg.size(); i++)\
		inc_netLoad(Seg[i],layer);

	if( is_head_layer(Seg,layer) )\
		dec_netLoad(Seg.front(),layer);\
	if( is_tail_layer(Seg,layer) )\
		dec_netLoad(Seg.back (),layer);
}

void Lya_t::detach_layer( Seg_t& Seg ){
	if( Seg.layer()==Seg_t::NullLayer() )
		return ;
	int layer = Seg.layer();
	if( Seg.isPinFill() && 0==_PinFillCost )
		;
	else {
		for(int i=0; i<Seg.size(); i++)\
			dec_netLoad(Seg[i],layer);

		if( is_head_layer(Seg,layer) )\
			inc_netLoad(Seg.front(),layer);\
		if( is_tail_layer(Seg,layer) )\
			inc_netLoad(Seg.back (),layer);
	}

	Seg.set_layer(Seg_t::NullLayer());
}

void Lya_t::inc_netLoad( const Seg_t& Seg, int index, int layer, bool isWire ){
	int tex = Seg[index];
	Tex_Man_t::Pos_t pos = _pTex->getPos( tex );
	entry(layer, pos._y, pos._x).incLoad();
	if( isWire )
		entry(layer, pos._y, pos._x).incWireLoad();
	if( 1==layer && !Seg.Teg(index).empty() )
		entry(layer, pos._y, pos._x).incPinLoad( num_std_pin(Seg,index) );
}
void Lya_t::dec_netLoad( const Seg_t& Seg, int index, int layer, bool isWire ){
	int tex = Seg[index];
	Tex_Man_t::Pos_t pos = _pTex->getPos( tex );
	entry(layer, pos._y, pos._x).decLoad();
	if( isWire )
		entry(layer, pos._y, pos._x).decWireLoad();
	if( 1==layer && !Seg.Teg(index).empty() )
		entry(layer, pos._y, pos._x).decPinLoad( num_std_pin(Seg,index) );
}

void Lya_t::inc_netLoad( int tex_index, int layer ){
	Tex_Man_t::Pos_t pos = _pTex->getPos( tex_index );
	entry(layer, pos._y, pos._x).incLoad();
}

void Lya_t::dec_netLoad( int tex_index, int layer ){
	Tex_Man_t::Pos_t pos = _pTex->getPos( tex_index );
	entry(layer, pos._y, pos._x).decLoad();
}

int  Lya_t::get_nVia   ( const Seg_t& Seg ) const {
	if( Seg.layer()==Seg_t::NullLayer() )
		return 0;
	return get_nVia(Seg,Seg.layer());
}
int  Lya_t::get_nVia   ( const int tcl ) const {
	;
}

int  Lya_t::get_nVia   ( const Seg_t& Seg, int layer ) const {
	const Tcl_t& Tcl = _pGdb->vTcl[Seg.tcl()];
	const vSeg_t& vSeg = Tcl.vSeg();
	int nVia = 0;
	for(int i=0; i<Seg.vAdj().size(); i++){
		int adj = Seg.vAdj()[i];
		if( vSeg[adj].layer()==Seg_t::NullLayer() )
			continue;
		nVia = abs(layer - vSeg[adj].layer() );
	}
	for(int i=0; i<Seg.vTeg().size(); i++){
		int teg = Seg.vTeg()[i];
		int minChoice = 0;
		for(int j=0; j<Tcl[teg].vLayer().size(); j++){
			int cur = abs(Tcl[teg].vLayer()[j]-layer);
			if( cur < minChoice || 0==j )
				minChoice = cur;
		}
		nVia += minChoice;
	}
	return nVia;
}

bool Lya_t::is_head_layer( const Seg_t& Seg, int layer ) const {
	return Seg.head()!=Seg_t::NullIndex()? _pGdb->vTcl[Seg.tcl()].vSeg()[Seg.head()].layer()==layer: false;
}
bool Lya_t::is_tail_layer( const Seg_t& Seg, int layer ) const {
	return Seg.tail()!=Seg_t::NullIndex()? _pGdb->vTcl[Seg.tcl()].vSeg()[Seg.tail()].layer()==layer: false;
}
bool Lya_t::is_overflow( int tex_index, int layer, bool fSoftOverflow ) const {
	int obs = get_obsLoad(tex_index, layer);
	int cap = get_cap    (tex_index, layer);
	int net = get_netLoad(tex_index, layer);
	if( fSoftOverflow )
		cap = (float) cap * (_pGdb->DynPrefill3D()? _DPrefill[tex_index]: _pGdb->Prefill3D());
	return cap < obs+net;
}

int  Lya_t::get_nOverflow( const Seg_t& Seg, bool fSoftOverflow ) const {
	return get_nOverflowLayer(Seg, Seg_t::NullLayer(), fSoftOverflow);
}

int  Lya_t::get_nOverflowLayer( const Seg_t& Seg, int layer, bool fSoftOverflow ) const {
	int nOverflow = 0;
	for(int i=0; i<Seg.size(); i++){
		if( Seg.Layer(i)!=Seg_t::NullLayer() && Seg.Teg(i).empty() )
		if( (Seg_t::NullLayer()==layer || layer==Seg.Layer(i))? is_overflow(Seg[i], Seg.Layer(i), fSoftOverflow): false )
			nOverflow ++ ;
	}
	
	if( Seg.Layer(0)!=Seg_t::NullLayer() && Seg.Teg(0).empty() )
	if( is_head_layer(Seg,Seg.Layer(0))? is_overflow(Seg.front(), Seg.Layer(0), fSoftOverflow): false )
		--nOverflow;

	if( Seg.Layer(Seg.size()-1)!=Seg_t::NullLayer() && Seg.Teg(Seg.size()-1).empty() )
	if( is_tail_layer(Seg,Seg.Layer(Seg.size()-1))? is_overflow(Seg.back (), Seg.Layer(Seg.size()-1), fSoftOverflow): false )
		--nOverflow;
	return nOverflow;
}

bool Lya_t::is_wpot(Seg_t& Seg, int index) const {
	return !Seg.Teg(index).empty();
}

int  Lya_t::num_std_pin(const Seg_t& Seg, int index) const {
	if( Seg.Teg(index).empty() )
		return 0;
	const Tcl_t& Tcl = _pGdb->vTcl[Seg.tcl()];
	int nStdPin = 0;
	const vInt_t& vTeg = Seg.Teg(index);
	for(int i=0; i<vTeg.size(); i++)
		if(Tcl[vTeg[i]].is_std())
			nStdPin++;
	return nStdPin;
}

int  Lya_t::num_ovfl_layer( const int layer, const bool fSoftOverflow ) const {
	int nOverflowSeg = 0, nOverflowHardSeg = 0;
	for(int i=0; i<_vSegPtr.size(); i++)
		nOverflowSeg     += 0<get_nOverflowLayer(*_vSegPtr[i], layer, fSoftOverflow );
	return nOverflowSeg;
}

int  Lya_t::num_ovfl( const bool fSoftOverflow ) const {
	return num_ovfl_layer(Seg_t::NullLayer(), fSoftOverflow);
}


int  Lya_t::get_nOverflow( const Seg_t& Seg, int layer, bool fSoftOverflow ) const {
	int nOverflow = 0;
	for(int i=0; i<Seg.size(); i++)
		if( is_overflow(Seg[i], layer, fSoftOverflow) )
			nOverflow ++ ;
	if( is_head_layer(Seg,layer)? is_overflow(Seg.front(), layer, fSoftOverflow): false )
		--nOverflow;
	if( is_tail_layer(Seg,layer)? is_overflow(Seg.back (), layer, fSoftOverflow): false )
		--nOverflow;
	return nOverflow;
}

int  Lya_t::get_segCost( const Seg_t& Seg, int layer ) const {
	if( Seg.isV()!=_pTex->layerV()[layer] )
		return INT_MAX;
	// via cost < soft overflow cost < hard overflow cost
	int nHardOverflow = get_nOverflow(Seg,layer,false);
	int nSoftOverflow = get_nOverflow(Seg,layer,true ) - nHardOverflow;
	return (nHardOverflow<<overflow_offset(false)) + (nSoftOverflow<<overflow_offset(true )) 
	+ (get_nVia(Seg,layer)<<via_offset()) ;//+ Seg.vTeg().size() + Seg.length() ;//+ (Seg.vTeg().size()<<overflow_offset(true));// - (Seg.length()<<overflow_offset(true));
}
