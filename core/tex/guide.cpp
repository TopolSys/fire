#include "tex.hpp"
#include "teg.hpp"
#include "db/db.hpp"
#include "guide.hpp"
#include "lem_geom.hpp"
#include <time.h>

Guide_t::Guide_t( Lem_Gdb_t * pGdb, Tex_Man_t * pTex ){
	_pGdb = pGdb;
	_pTex = pTex;
	_width= 0;
	_nAux = 0;
}


Rect_t Guide_t::derive_tex_rect( const int tex, int width ) const {
	Rect_t rect;
	rect = _pTex->getTexRect(tex);
	int tex_vprev = tex;
	int tex_vnext = tex;
	int tex_hprev = tex;
	int tex_hnext = tex;
	for(int i=0; i<width; i++){
		#define TexMove(var,func,isV) \
		{\
			if( Tex_Man_t::NullTex() != var ){\
				var = _pTex->func(var, isV );\
				if( Tex_Man_t::NullTex() != var )\
					rect = rect.merge( _pTex->getTexRect(var) );\
			}\
		}
		TexMove(tex_vprev, getTidPrev, true );
		TexMove(tex_vnext, getTidNext, true );
		TexMove(tex_hprev, getTidPrev, false);
		TexMove(tex_hnext, getTidNext, false);
	//	std::cout<<"("<<tex_hprev<<","<<tex_hnext<<") ";
		#undef TexMove
	}
	//std::cout<<"  center@ "<<tex<<std::endl;
	return rect;
}

Rect_t Guide_t::derive_seg_rect( const Seg_t& Seg ) const {
	if( lem::dev() )
		assert( ! Seg.empty() );
	Rect_t rect;
	bool isV = Seg.isV();
	int min_tex =_pTex->data().size()-1;
	int max_tex = 0;

	for(int i=0; i<Seg.size(); i++){
		int tex = Seg[i];
		min_tex = std::min(tex, min_tex);
		max_tex = std::max(tex, max_tex);
	}
	rect = rect.merge(derive_tex_rect(min_tex, _width) );
	rect = rect.merge(derive_tex_rect(max_tex, _width) );
//	assert( 0<=min_tex && min_tex<_pTex->data().size());
//	assert( 0<=max_tex && max_tex<_pTex->data().size());
//	rect = rect.merge( _pTex->getTexRect(min_tex) );
//	rect = rect.merge( _pTex->getTexRect(max_tex) );
//	if( 0 < width ){
//		int tex = Seg[0];
//		//std::cout<<tex<<": ";
//		for(int i=1; i<=width; i++){
//			int tex_prev = _pTex->getTidPrev(tex,!isV);
//			int tex_next = _pTex->getTidNext(tex,!isV);
//		//	std::cout<<"<"<< tex_prev <<":"<< tex_next <<">";
//			if( Tex_Man_t::NullTex() != tex_prev )
//				rect.merge( _pTex->getTexRect(tex_prev) );
//			if( Tex_Man_t::NullTex() != tex_next )
//				rect.merge( _pTex->getTexRect(tex_next) );
//		}
//		for(int i=1; i<=width; i++){
//			int tex_prev = _pTex->getTidPrev(min_tex,isV);
//			int tex_next = _pTex->getTidNext(max_tex,isV);
//		//	std::cout<<"<"<< tex_prev <<":"<< tex_next <<">";
//			if( Tex_Man_t::NullTex() != tex_prev )
//				rect.merge( _pTex->getTexRect(tex_prev) );
//			if( Tex_Man_t::NullTex() != tex_next )
//				rect.merge( _pTex->getTexRect(tex_next) );
//		}
//		//std::cout<<std::endl;
//	}
//	for(int i=0; i<Seg.size(); i++){
//		int tex = Seg[i];
//		rect = rect.merge( _pTex->getTexRect(tex) );
//	}
	return rect;
}

void Guide_t::derive_seg( const Seg_t& Seg, std::ostream& ostr ){
	Rect_t rect;
	if( Seg.isPinFill() ){
		assert( Seg.vTeg().size() && Seg.tcl()!=-1 );
		int tcl = Seg.tcl();
		for(int i=0; i<Seg.vTeg().size(); i++)
			rect = rect.merge( pin_guide_rect( _pGdb->vTcl[tcl][Seg.vTeg()[i]] ) );
		vGuideCell.push_back( GuideCell_t(Seg.layer(), rect, Seg.isV(), GuideCell_t::Normal ) );
	} else {

		if( _pGdb->RaceLya() ){
			Rect_t rect;
			for(int i=0; i<Seg.size(); i++){
				if( 0==i ){
					rect = rect.merge( derive_tex_rect(Seg[i],_width) );
					continue;
				}
				if( Seg.Layer(i-1)!=Seg.Layer(i) ){
					vGuideCell.push_back( GuideCell_t(Seg.Layer(i-1), rect, Seg.isV(), GuideCell_t::Normal ) );
					/** V3 Guide **
					int lid1 = std::max(Seg.Layer(i-1)-1, 1);
					int lid2 = std::min(Seg.Layer(i-1)+1, (int) _pTex->layerName().size()-1);
					vGuideCell.push_back( GuideCell_t(lid1, rect, !Seg.isV(), GuideCell_t::Normal ) );
					vGuideCell.push_back( GuideCell_t(lid2, rect, !Seg.isV(), GuideCell_t::Normal ) );
					/** **/
					rect = derive_tex_rect(Seg[i-1],_width);
				}
				rect = rect.merge( derive_tex_rect(Seg[i], _width) );
			}
			// the last rect
			if( !Seg.empty() ){
				vGuideCell.push_back( GuideCell_t(Seg.Layer(Seg.size()-1), rect, Seg.isV(), GuideCell_t::Normal ) );
				/** V3 Guide **
				int lid1 = std::max(Seg.Layer(Seg.size()-1)-1, 1);
				int lid2 = std::min(Seg.Layer(Seg.size()-1)+1, (int) _pTex->layerName().size()-1);
				vGuideCell.push_back( GuideCell_t(lid1, rect, !Seg.isV(), GuideCell_t::Normal ) );
				vGuideCell.push_back( GuideCell_t(lid2, rect, !Seg.isV(), GuideCell_t::Normal ) );
				/** **/
			}
			if( _pGdb->AuxGuide() ){
				for(int i=0; i<Seg.vAux().size(); i++){
					int tex   = Seg.vAux()[i].second;
					int layer = Seg.vAux()[i].first ;
					Rect_t rect = derive_tex_rect(tex, _width);
					vGuideCell.push_back( GuideCell_t(layer, rect, !Seg.isV(), GuideCell_t::Aux ) );
					_nAux ++ ;
				}
			}
		} else {
			rect = derive_seg_rect(Seg);
			vGuideCell.push_back( GuideCell_t(Seg.layer(), rect, Seg.isV(), GuideCell_t::Normal ) );
		}
	}
	
	//if( Seg.layer()-1>_pGdb->StartLya() )\
		vGuideCell.push_back( GuideCell_t(Seg.layer()-1, rect, !Seg.isV(), GuideCell_t::Normal ) );\
	if( Seg.layer()+1<_pTex->layerV().size() )\
		vGuideCell.push_back( GuideCell_t(Seg.layer()+1, rect, !Seg.isV(), GuideCell_t::Normal ) );
	
	//ostr<<rect<<" "<<_pTex->layerName(Seg.layer())<<" \n";
}

bool Guide_t::has_adj( int pos_index, bool isV ){
	Tex_Man_t::Pos_t pos = _pTex->getPos(pos_index);
	Tex_Man_t::Pos_t adj[2] = { isV? pos.left(): pos.up(), isV? pos.right(): pos.down() };
	for(int k=0; k<2; k++)
		if( _pTex->valid(adj[k])? WireGuide.val( _pTex->Pos2Index(adj[k]) ): false )
			return true;
	
	return false;
}

void Guide_t::tag_boundary_seg( const Seg_t& Seg ){
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
		bool LineSwitch = (i<Seg.size()-1 && _pGdb->RaceLya())? Seg.Layer(i)!=Seg.Layer(i+1): false;
		// tag cross points
		if( has_adj(pos_index, true) && has_adj(pos_index, false) || LineSwitch )
			SwitchGuide.set( pos_index, 1 );
	}
	SwitchGuide.set(_pTex->Pos2Index(posmin),1);
	SwitchGuide.set(_pTex->Pos2Index(posmax),1);
}
void Guide_t::layer_reg_seg( const Seg_t& Seg ){
	if( _pGdb->RaceLya() ){
		for(int i=0; i<Seg.size(); i++){
			Tex_Man_t::Pos_t pos = _pTex->getPos(Seg[i]);
			int pos_index = _pTex->Pos2Index(pos);

			if( ! SwitchGuide.val( pos_index ) )
				continue;
			PosLayerMap.insert( PosLayerMap_t::value_type(Seg[i], Seg.Layer(i)) );
			if( (i<Seg.size()-1)? Seg.Layer(i)!=Seg.Layer(i+1): false )
				PosLayerMap.insert( PosLayerMap_t::value_type(Seg[i], Seg.Layer(i+1)) );
		}
		return ;
	}
	for(int i=0; i<Seg.size(); i++){
		Tex_Man_t::Pos_t pos = _pTex->getPos(Seg[i]);
		int pos_index = _pTex->Pos2Index(pos);

		if( ! SwitchGuide.val( pos_index ) )
			continue;
		PosLayerMap.insert( PosLayerMap_t::value_type(Seg[i], Seg.layer()) );
	}
}
void Guide_t::layer_reg( const Tcl_t& Tcl ){
	tag_boundary(Tcl);
	tag_pin(Tcl);
	Lem_Iterate(Tcl.vSeg(), pSeg)
		layer_reg_seg(*pSeg);
}

void Guide_t::init ( const Tcl_t& Tcl ){
	PosLayerMap.clear();
	if( WireGuide.vec().size() < _pTex->data().size() ){
		WireGuide.clear();
		WireGuide.resize( _pTex->data().size() );
	}
	if( SwitchGuide.vec().size() < _pTex->data().size() ){
		SwitchGuide.clear();
		SwitchGuide.resize( _pTex->data().size() );
	}
	vGuideCell.clear();
	WireGuide.fillLine(Tcl.vPath());
}

void Guide_t::erase( const Tcl_t& Tcl ){
	WireGuide  .fillLine(Tcl.vPath(),0);
	SwitchGuide.fillLine(Tcl.vPath(),0);
}
void Guide_t::tag_pin( const Tcl_t& Tcl ){
	for(int i=0; i<Tcl.size(); i++){
		Tex_Man_t::Pos_t pos( Tcl[i].gposX(), Tcl[i].gposY() );
		SwitchGuide.set(_pTex->Pos2Index(pos),1);
	}
}
void Guide_t::tag_boundary( const Tcl_t& Tcl ){
	Lem_Iterate(Tcl.vSeg(), pSeg)
		tag_boundary_seg(*pSeg);
}

void Guide_t::via_fill_layer(){
	PosLayerMap_t::iterator prev = PosLayerMap.end();
	Lem_Iterate(PosLayerMap, pLayer){
		PosLayerRange_t range= PosLayerMap.equal_range( pLayer->first );
		for( PosLayerMap_t::iterator first = range.first; first!=range.second; first++){
			PosLayerMap_t::iterator next = first; next++;
			if( next == range.second )
				break;
			int pos_index = first->first;
			Rect_t rect = derive_tex_rect(pos_index, _width);//_pTex->getTexRect(pos_index);
			int layer_src = std::min(first->second, next->second)+1;
			int layer_dst = std::max(first->second, next->second);
			for(int j=layer_src; j<layer_dst; j++)
				vGuideCell.push_back( GuideCell_t(j, rect, _pTex->layerV()[j], GuideCell_t::Switch ) );
		}
		pLayer = range.second;
		--pLayer;
	}
}

Rect_t Guide_t::pin_guide_rect( const Teg_t& teg ) const {
	Rect_t rectY, rectX;
	int xstart = teg.grangeX().first ;
	int ystart = teg.grangeY().first ;
	int xend   = teg.grangeX().second;
	int yend   = teg.grangeY().second;
	for(int i=ystart; i<yend; i++){
		const Tex_Man_t::Pos_t pos = Tex_Man_t::Pos_t( xstart, i );
		const int tex = _pTex->Pos2Index( pos );
		rectY = rectY.merge(derive_tex_rect(tex,_width));
		//rectY = rectY.merge(_pTex->getTexRect(tex));
	}
	for(int i=xstart; i<xend; i++){
		const Tex_Man_t::Pos_t pos = Tex_Man_t::Pos_t( i, ystart );
		const int tex = _pTex->Pos2Index( pos );
		rectX = rectX.merge(derive_tex_rect(tex,_width));
		//rectX = rectX.merge(_pTex->getTexRect(tex));
	}
	return Rect_t(rectY).merge(rectX);

	const Tex_Man_t::Pos_t pos = Tex_Man_t::Pos_t( teg.gposX(), teg.gposY() );
	const int tex = _pTex->Pos2Index( pos );
	Rect_t rect = _pTex->getTexRect(tex);
	return rect;
}

void Guide_t::via_fill_pin_extra( const Rect_t& rect, const int layer, bool isPerp){
	bool isV = _pTex->layerV()[layer];
	// upper perpendicular fill
	int fill2 = -1;
	for(int j=layer+1; j<_pTex->layerV().size(); j++)
		if( isPerp^isV == _pTex->layerV()[j] ){
			fill2 = j;
			break;
		}
	if( -1!=fill2 ){
		vGuideCell.push_back( GuideCell_t(fill2, rect, isPerp^isV, GuideCell_t::Pin ) );
		return;
	}
	
	int fill1 = -1;
	for(int j=layer-1; j>=0; j--)
		if( isPerp^isV == _pTex->layerV()[j] ){
			fill1 = j;
			break;
		}
	if( -1!=fill1 ){
		vGuideCell.push_back( GuideCell_t(fill1, rect, isPerp^isV, GuideCell_t::Pin ) );
		return;
	}
}

void Guide_t::via_fill_pin  ( const Tcl_t& Tcl ){
	for(int i=0; i<Tcl.size(); i++){
		const Teg_t& teg = Tcl[i];
		const Tex_Man_t::Pos_t pos = Tex_Man_t::Pos_t( teg.gposX(), teg.gposY() );
		const int tex = _pTex->Pos2Index( pos );
		PosLayerRange_t range = PosLayerMap.equal_range( tex );
		Rect_t rect = pin_guide_rect(teg);
		
		if( range.first == range.second ){ // connection within the same tex
			int layer_cur = teg.vLayer().front();
			assert( Tcl.vTeid().empty() );
		//	PosLayerMap.insert( PosLayerMap_t::value_type(tex, layer_cur) );
			bool isV = _pTex->layerV()[layer_cur];

			// lower perpendicular fill
			via_fill_pin_extra(rect, layer_cur, true );
			via_fill_pin_extra(rect, layer_cur, false);
			vGuideCell.push_back( GuideCell_t(layer_cur, rect, isV, GuideCell_t::Pin ) );
			continue;
		}
		//assert( range.first != range.second );
		int layer_src = range.first ->second;
		int layer_dst = range.first ->second;
		for(PosLayerMap_t::iterator itr= range.first; itr!=range.second; itr++){
			layer_src = std::min(itr->second, layer_src);
			layer_dst = std::max(itr->second, layer_dst);
		}
		int best_dist = -1;
		int layer_cur = -1;
		for(int j=0; j<teg.vLayer().size(); j++){
			int layer_j = teg.vLayer()[j];
			int min_dist= (layer_src <= layer_j && layer_j <= layer_dst)? 0: std::min( abs(layer_src- layer_j ), abs(layer_j- layer_dst) );
			if( 0==j || min_dist < best_dist ){
				best_dist = min_dist;
				layer_cur = layer_j;
			}
		}
		assert( layer_cur >=0 );
		via_fill_pin_extra(rect, layer_cur, true );
		via_fill_pin_extra(rect, layer_cur, false);
		assert( best_dist >=0 );
		if( best_dist==0 )
			continue;

		//PosLayerMap.insert( PosLayerMap_t::value_type(tex, layer_cur) );
		if( layer_cur > layer_dst ){
			for(int j=layer_cur; j>layer_dst; j--)
				vGuideCell.push_back( GuideCell_t(j, rect, _pTex->layerV()[j], GuideCell_t::Pin ) );

		} else {
			// layer_cur < layer_src
			for(int j=layer_cur; j<layer_src; j++)
				vGuideCell.push_back( GuideCell_t(j, rect, _pTex->layerV()[j], GuideCell_t::Pin ) );

		}
	}
}

void Guide_t::derive( const Tcl_t& Tcl, std::ostream& ostr ){
	Lem_Iterate(Tcl.vSeg(), pSeg)
		derive_seg(*pSeg, ostr);

	layer_reg(Tcl);
	via_fill_layer();
	via_fill_pin  (Tcl);
}

void Lem_DeriveGuide( Lem_Gdb_t * pGdb, Tex_Man_t * pTex, std::ostream& ostr ){
	std::cout<<"DeriveGuide"<<std::endl;
	Guide_t guide(pGdb, pTex);
	lem::Progress_t<int> prog( 50, pGdb->vTcl.size());
	int nNet = 0;
	
	//time_t raw_time; time(&raw_time);\
	ostr<<"Lem2: "<< ctime(&raw_time)<<"\n";

	for(vTcl_t::iterator pTcl = pGdb->vTcl.begin(); pTcl!=pGdb->vTcl.end(); pTcl++, nNet++){
		if( !pTcl->ok() )
			continue;
		//if( 0==(nNet%10) || pTcl==--pGdb->vTcl.end())\
			prog.set(nNet+1).print(std::cout);
		guide.init (*pTcl);
		guide.derive(*pTcl, ostr);
		guide.erase(*pTcl);
		guide.write_guide(*pTcl, ostr);
		//guide._write_guide(*pTcl, ostr);
	}
	if( pGdb->AuxGuide() )
		std::cout<< guide.nAux() <<" aux guides added\n";
}

void Guide_t::write_guide(const Tcl_t& Tcl, std::ostream& ostr){
	ostr << Tcl.name()<<"\n(\n";
	for(int i=0; i<vGuideCell.size(); i++){
		Rect_t rect = vGuideCell[i].rect();
		int layer   = vGuideCell[i].layer();
//		assert(layer>=0);
//		assert(layer<_pTex->layerV().size());
//		assert(rect.p1.x >= _pTex->vGridX().front());
//		assert(rect.p1.x <= _pTex->vGridX().back ());
//		assert(rect.p2.x >= _pTex->vGridX().front());
//		assert(rect.p2.x <= _pTex->vGridX().back ());
//		assert(rect.p1.x <  rect.p2.x );
//		assert(rect.p1.y >= _pTex->vGridY().front());
//		assert(rect.p1.y <= _pTex->vGridY().back ());
//		assert(rect.p2.y >= _pTex->vGridY().front());
//		assert(rect.p2.y <= _pTex->vGridY().back ());
//		assert(rect.p1.y <  rect.p2.y );

		ostr<< rect.p1.x <<" "<< rect.p1.y <<" "<< rect.p2.x <<" "<< rect.p2.y <<" " << _pTex->layerName(layer) <<"\n";
	}
	ostr<<")\n";
}

void Guide_t::_write_guide(const Tcl_t& Tcl, std::ostream& ostr){
	ostr << Tcl.name()<<"\n(\n";
	for(int i=0; i<vGuideCell.size(); i++){
		ostr<< vGuideCell[i].rect()<<" "<< _pTex->layerName(vGuideCell[i].layer())<<" \\\\"<< vGuideCell[i].type_str() <<std::endl;
	}
	ostr<<")\n";
}