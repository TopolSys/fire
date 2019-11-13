#include "lya.hpp"
#include "lya_tcl.hpp"
#include "db/db.hpp"
#include "core/tex/tex.hpp"

Lya_Tcl_t::Lya_Tcl_t( Lem_Gdb_t * pGdb, Tex_Man_t * pTex ){
	_pGdb = pGdb;
	_pTex = pTex;
	_TexTrav .init( _pTex->data().size() );
	_EdgeTrav.init( _pTex->Edge().size() );
}

void Lya_Tcl_t::addSwitch2Map( const int sid ){
	for( int i=0; i<_vSeg[sid].size(); i++)
		if( SwitchGuide.val(_vSeg[sid][i]) )
			PosSidMap.insert( PosIntMap_t::value_type( _vSeg[sid][i], sid) );
}
void Lya_Tcl_t::addPin2Map( const int sid ){
	for( int i=0; i<_vSeg[sid].size(); i++)
		if( PinGuide.val(_vSeg[sid][i]) )
			PosSidMap.insert( PosIntMap_t::value_type( _vSeg[sid][i], sid) );
}

void Lya_Tcl_t::build_adj_seg( const int sid ){
	Seg_t& Seg = _vSeg[sid];
	vBool_t vSelected(nSeg, false);
	for(int i=0; i<Seg.size(); i++){
		PosIntRange_t range = PosSidMap.equal_range(Seg[i]);
		for(PosIntMap_t::iterator itr = range.first; itr!=range.second; itr++){
			int sid = itr->second;
			Seg.addAdj(i,sid);
			if( sid==itr->second || vSelected[ itr->second ] )
				continue;
			vSelected[ itr->second ] = true;
			Seg.vAdj().push_back(itr->second);
		}
	}
}

void Lya_Tcl_t::PosSearchHV( const int pos_index, bool& hasV, bool& hasH ){
	PosIntRange_t range = PosSidMap.equal_range( pos_index );
	for(PosIntMap_t::iterator layer=range.first; layer!=range.second; layer++){
		const Seg_t& Seg = _vSeg[layer->second];
		if( Seg.isV() )
			hasV = true;
		else
			hasH = true;
		if( hasV && hasH )
			break;
	}
}

void Lya_Tcl_t::build_pin_fill( const Tcl_t& Tcl ){
	// handle in-tile, cross-tile false
	for(int i=0; i<nSeg; i++)
		addPin2Map(i);

	for(PosIntMap_t::iterator itr=PosTegMap.begin(); itr!=PosTegMap.end(); itr++){
		int pos_index = itr->first;
		bool hasV = false;
		bool hasH = false;
		PosSearchHV(pos_index, hasV, hasH);
		PosIntRange_t range = PosTegMap.equal_range(pos_index);
		for(PosIntMap_t::iterator pin=range.first; pin!=range.second; pin++){
			const Teg_t& teg = Tcl[pin->second];
			bool fV = false;
			bool fH = false;
			for(int i=0; i<teg.vLayer().size(); i++){
				if( _pTex->layerV()[ teg.vLayer()[i] ] )
					fV = true;
				else
					fH = true;
			}
			if( fV && fH ) // flexible, non-determined pin access position
				continue;

			if( fV )
				hasV = true;
			else
				hasH = true;

			if( hasV && hasH ) // at least two pins have different access direction, safe
				break;
		}
		if( hasV && !hasH ){ // add H segment
			_vSeg[nSeg].push_back(pos_index);
			_vSeg[nSeg].set_PinFill();
			_vSeg[nSeg++].set_dir(false);
		} else
		if( hasH && !hasV ){ // add V segment
			_vSeg[nSeg].push_back(pos_index);
			_vSeg[nSeg].set_PinFill();
			_vSeg[nSeg++].set_dir(true);
		}
		assert(nSeg<_vSeg.size());
		itr = --range.second;
	}
//	TravTag_t TravTag( Tcl.size() );
//	for(PosIntMap_t::iterator itr=PosSidMap.begin(); itr!=PosSidMap.end(); itr++){
//		PosIntRange_t range = PosSidMap.equal_range(itr->first);
//		TravTag.inc();
//		for(PosIntMap_t::iterator first=range.first; first!=range.second; first++){
//			const Seg_t& Seg = _vSeg[first->second];
//			for(int i=0; i<Seg.vTeg().size(); i++){
//				int pin = Seg.vTeg()[i];
//				if( TravTag.is_cur(pin) )
//					continue;
//				//if( Tcl[pin] )
//			}
//		}
//	}
}

void Lya_Tcl_t::build_adj( const Tcl_t& Tcl ){
	SwitchGuide.fillLine(Tcl.vPath(),0);
	tag_boundary();
	
	PosSidMap.clear();
	for(int i=0; i<nSeg; i++)
		addSwitch2Map(i);

	for(int i=0; i<nSeg; i++){
		_vSeg[i].vAdj().clear();
		_vSeg[i].resetAdj();
	}

	for(int i=0; i<nSeg; i++)
		build_adj_seg(i);

//	for(int i=0; i<nSeg; i++)\
		std::cout<<i<<"("<<_vSeg[i].size()<<"): " <<_vSeg[i];\
	std::cout<<"\n data size="<<_pTex->data().size() <<"\n\n";
}

bool Lya_Tcl_t::has_adj( int pos_index, bool isV ){
	Tex_Man_t::Pos_t pos = _pTex->getPos(pos_index);
	Tex_Man_t::Pos_t adj[2] = { isV? pos.left(): pos.up(), isV? pos.right(): pos.down() };
	for(int k=0; k<2; k++)
		if( _pTex->valid(adj[k])? WireGuide.val( _pTex->Pos2Index(adj[k]) ): false )
			return true;
	
	return false;
}

void Lya_Tcl_t::tag_boundary_seg( const int sid ){
	const Seg_t& Seg = _vSeg[sid];
	const Tex_Man_t::Pos_t posNull= Tex_Man_t::Pos_t::Null();
	Tex_Man_t::Pos_t posmin = posNull;
	Tex_Man_t::Pos_t posmax = posNull;
	for(int i=0; i<Seg.size(); i++){
		Tex_Man_t::Pos_t pos = _pTex->getPos(Seg[i]);
		int pos_index = _pTex->Pos2Index(pos);
		posmin = posNull == posmin? pos: auto_min(pos, posmin);
		posmax = posNull == posmax? pos: auto_max(pos, posmax);

		if( SwitchGuide.val( pos_index ) )
			continue;
		// tag cross points
		if( has_adj(pos_index, true) && has_adj(pos_index, false) )
			SwitchGuide.set( pos_index, 1 );
	}
	SwitchGuide.set(_pTex->Pos2Index(posmin),1);
	SwitchGuide.set(_pTex->Pos2Index(posmax),1);
}
void Lya_Tcl_t::tag_boundary(){
	for(int i=0; i<nSeg; i++)
		tag_boundary_seg(i);
}

void Lya_Tcl_t::init( const Tcl_t& Tcl ){
	if( WireGuide.vec().size() < _pTex->data().size() ){
		WireGuide.clear();
		WireGuide.resize( _pTex->data().size() );
	}

	if( SwitchGuide.vec().size() < _pTex->data().size() ){
		SwitchGuide.clear();
		SwitchGuide.resize( _pTex->data().size() );
	}

	if( PinGuide.vec().size() < _pTex->data().size() ){
		PinGuide.clear();
		PinGuide.resize( _pTex->data().size() );
	}

	_vSeg.clear();
	PosSidMap.clear();
	PosTegMap.clear();

	_EdgeTrav.inc();
	for(int i=0; i<Tcl.vTeid().size(); i++){
		int teid = Tcl.vTeid()[i];
		_EdgeTrav.set(teid);
	}


	// tag pin
	//for(int i=0; i<PinGuide.vec().size(); i++)\
		assert(PinGuide.vec()[i]==0);

	Tex2TegID.clear();
	for(int i=0; i<Tcl.size(); i++){
		int tex = _pTex->Pos2Index( Tex_Man_t::Pos_t( Tcl[i].gposX(), Tcl[i].gposY() ) );
		PinGuide.set(tex, 1);
		Tex2TegID.insert( std::pair<int,int>(tex,i) );
		PosTegMap.insert( PosIntMap_t::value_type(tex, i) );
	}

	WireGuide.fillLine(Tcl.vPath());
}

void Lya_Tcl_t::build_pin(){
	for(int i=0; i<nSeg; i++)
		build_pin_seg(i);
}
void Lya_Tcl_t::build_pin_seg( const int sid ){
	typedef std::unordered_multimap<int,int>::iterator iterator;
	_vSeg[sid].vTeg().clear();
	_vSeg[sid].resetTeg();
	for(int i=0; i<_vSeg[sid].size(); i++){
		std::pair<iterator,iterator> range = Tex2TegID.equal_range( _vSeg[sid][i] );
		for(iterator itr=range.first; itr!=range.second; itr++){
			_vSeg[sid].vTeg().push_back(itr->second);
			_vSeg[sid].addTeg(i, itr->second);
		}
	}
	std::sort( _vSeg[sid].vTeg().begin(), _vSeg[sid].vTeg().end() );
}

void Lya_Tcl_t::erase( const Tcl_t& Tcl ){
	WireGuide  .fillLine(Tcl.vPath(),0);
	SwitchGuide.fillLine(Tcl.vPath(),0);
	for(int i=0; i<Tcl.size(); i++){
		int tex = _pTex->Pos2Index( Tex_Man_t::Pos_t( Tcl[i].gposX(), Tcl[i].gposY() ) );
		PinGuide.set(tex, 0);
	}
	//for(int i=0; i<PinGuide.vec().size(); i++)\
		assert(PinGuide.vec()[i]==0);
}

void Lya_Tcl_t::_extend_rec( int teid, Seg_t& Seg ){
	_EdgeTrav.set(teid);
	int texs[2] = {  _pTex->Edge(teid).first ,  _pTex->Edge(teid).second };
	for(int i=0; i<2; i++){
		if( ! _TexTrav.is_cur(texs[i]) ){
			_TexTrav.set(texs[i]);
			Seg.push_back(texs[i]);
		}
		const Tex_t& tex = _pTex->data()[ texs[i] ];
		int other_eid = tex.other_eid(teid);
		if( Tex_t::NullEdge() != other_eid ? _EdgeTrav.is_prev(other_eid): false )
			_extend_rec(other_eid, Seg);
	}
}

void Lya_Tcl_t::_extend( int teid, Seg_t& Seg ){
	_TexTrav .inc();
	_extend_rec(teid, Seg);
	std::sort(Seg.begin(), Seg.end());
}

void Lya_Tcl_t::decompose( const Tcl_t& Tcl ){
	_EdgeTrav.inc();
	
	// edge segment
	nSeg = 0;
	_vSeg.clear();
	_vSeg.resize( 2*(Tcl.vPath().size()+1) );

	for(int i=0; i<Tcl.vTeid().size(); i++){
		int teid = Tcl.vTeid()[i];
		if( _EdgeTrav.is_cur(teid) )
			continue;
		_vSeg[nSeg].set_dir( _pTex->is_veid(teid) );
		_extend(teid, _vSeg[nSeg++]);
	}
}

void Lya_Tcl_t::chunk( const Tcl_t& Tcl ){
	const int nBase = nSeg;
	for(int i=0; i<nBase; i++)
		chunk_seg(i);
}

void Lya_Tcl_t::_finalize( const Tcl_t& Tcl ){
	build_adj(Tcl);
	//build_pin_fill(Tcl);
	build_pin();
}

void Lya_Tcl_t::chunk_seg( const int sid ){
	const int ChunkSz = _pGdb->SegChunk();
	int nStep    = 0;
	int prev_sid = Seg_t::NullIndex();
	int length   = _vSeg[sid].size();
	for(int i=0; i<_vSeg[sid].size(); i++, nStep++){
		//if( nStep<ChunkSz || _vSeg[sid].size()-(i+1) <= (ChunkSz>>1) )\
			continue;
		if( nStep<ChunkSz )
			continue;
		for(int j=0; j<nStep; j++ )
			_vSeg[nSeg].push_back( _vSeg[sid][i-nStep+j] );
		_vSeg[nSeg].set_dir   ( _vSeg[sid].isV() );
		_vSeg[nSeg].set_length( length );
		if( Seg_t::NullIndex() != prev_sid ){
			_vSeg[nSeg]    .set_head(prev_sid);
			_vSeg[prev_sid].set_tail(nSeg);
		}
		prev_sid = nSeg;
		//std::cout<<nSeg<<": "<<_vSeg[nSeg];
		nSeg++;
		i--;
		nStep = 0;
	}
	//no change
	if( nStep==_vSeg[sid].size() ){
		//std::cout<<"\n";
		return;
	}
	vInt_t vTmp;
	for(int i=0; i<nStep; i++ )
		vTmp.push_back( _vSeg[sid][_vSeg[sid].size()-nStep+i] );
	_vSeg[sid].clear();
	for(int i=0; i<vTmp.size(); i++)
		_vSeg[sid].push_back(vTmp[i]);

	_vSeg[sid].set_length(length);
	if( Seg_t::NullIndex() != prev_sid ){
		_vSeg[sid]     .set_head(prev_sid);
		_vSeg[prev_sid].set_tail(sid);
	}
	//std::cout<<sid<<": "<<_vSeg[sid];\
	std::cout<<" \n";
}

void Lya_Tcl_t::update( Tcl_t& Tcl ){
	// todo:
	// remove old load
	// add new load
	_finalize(Tcl);
	Tcl.vSeg().clear();
	Tcl.vSeg().resize(nSeg);
	for(int i=0; i<nSeg; i++)
		_vSeg[i].set_tcl( Tcl.id() )
		, _vSeg[i].resetLayer()
		, Tcl.vSeg()[i] = _vSeg[i];

	//std::cout<<Tcl.name()<<"\n";\
	for(int i=0; i<nSeg; i++)\
		std::cout<<i<<": "<<_vSeg[i];\
	std::cout<<"\n\n";
}
