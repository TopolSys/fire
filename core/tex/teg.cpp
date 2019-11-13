#include "teg.hpp"
#include "tex.hpp"
#include "db/db.hpp"
void Seg_t::addAdj(int index, int sid){
	_mAdj[index].push_back(sid);
}
void Seg_t::resetAdj(){
	_mAdj.clear();
	_mAdj.resize(size());
}
const vInt_t& Seg_t::Adj(int index) const {
	return _mAdj[index];
}

void Seg_t::addTeg(int index, int sid){
	_mTeg[index].push_back(sid);
}
void Seg_t::resetTeg(){
	_mTeg.clear();
	_mTeg.resize(size());
}
const vInt_t& Seg_t::Teg(int index) const {
	return _mTeg[index];
}

Seg_t * Seg_t::next( Lem_Gdb_t * pGdb ) const {
	if( tail()==NullIndex() )
		return NULL;
	return &pGdb->vTcl[tcl()].vSeg()[tail()];
}
Seg_t * Seg_t::prev( Lem_Gdb_t * pGdb ) const {
	if( head()==NullIndex() )
		return NULL;
	return &pGdb->vTcl[tcl()].vSeg()[head()];
}

void Seg_t::resetLayer(){
	_vLayer.clear();
	_vLayer.resize( size(), NullLayer() );
}
void Seg_t::setLayer(int index, int lid){
	_vLayer[index] = lid;
}
const int Seg_t::Layer(int index) const {
	return _vLayer[index];
}

int Seg_t::layer_of(int tex) const {
	const_iterator pos = std::lower_bound(begin(), end(), tex);
	return end()==pos? NullLayer(): Layer(pos-begin());
}

IntPair_t Seg_t::layerRange(int index, Lem_Gdb_t * pGdb, bool fCountSelf ) const {
	int tex = (*this)[index];
	const Tcl_t& Tcl = pGdb->vTcl[tcl()];
	int InitLayer = fCountSelf? Layer(index): NullLayer();
	IntPair_t ret( InitLayer, InitLayer );

	for(int i=0; i<Adj(index).size(); i++){
		const Seg_t& seg_i = Tcl.vSeg()[Adj(index)[i]];
		int layer_i = seg_i.layer_of(tex);
		if( NullLayer()==layer_i )
			continue;
		ret.first = NullLayer()==ret.first ? layer_i: std::min(ret.first , layer_i);
		ret.second= NullLayer()==ret.second? layer_i: std::max(ret.second, layer_i);
	}
	if( index < size()-1 && fCountSelf ){
		if( NullLayer()!=Layer(index) && NullLayer()!=Layer(index+1) ){
			if( Layer(index) != Layer(index+1) ){
				if( ret.second < Layer(index+1) )
					ret.second = Layer(index+1);
				if( ret.first  > Layer(index+1) )
					ret.first  = Layer(index+1);
			}
		}
	}
	for(int i=0; i<Teg(index).size(); i++){
		const Teg_t& Teg_i = Tcl[Teg(index)[i]];
		int min_layer = Teg_i.vLayer().front();
		int max_layer = Teg_i.vLayer().back ();
		if( max_layer < min_layer )
			std::swap( min_layer, max_layer );
		
		if( NullLayer()==ret.first )
			ret.first  = min_layer;
		else
		if( max_layer < ret.first  )
			ret.first  = max_layer;
		
		if( NullLayer()==ret.second )
			ret.second = max_layer;
		else
		if( min_layer > ret.second )
			ret.second = min_layer;
	}
	return ret;
}

int  Seg_t::countVia( std::set<int>& SeenTex, Lem_Gdb_t * pGdb ) const {
	int nVia = 0;
	for(int i=0; i<size(); i++){
		if( SeenTex.end() != SeenTex.find(data()[i]) )
			continue;
		IntPair_t Range= layerRange(i, pGdb);
		SeenTex.insert(data()[i]);
		if( NullLayer()!=Range.second && NullLayer()!=Range.first )
			nVia += Range.second - Range.first;
	}
	return nVia;
}

const bool Seg_t::race_assigned() const {
	for(int i=0; i<size(); i++)
		if( Layer(i)==NullLayer() )
			return false;
	return true;
}

const bool Seg_t::adj_race_assigned( Lem_Gdb_t * pGdb ) const {
	if( !vTeg().empty() )
		return true;
	for(int i=0; i<size(); i++)
		for(int j=0; j<Adj(i).size(); j++)
			if( pGdb->vTcl[tcl()].vSeg()[Adj(i)[j]].race_assigned() )
				return true;
	return false;
}

int  Seg_t::LayerUsage() const {
	vBool_t LayerUsed;
	for(int i=0; i<size(); i++){
		if( NullLayer() == Layer(i) )
			continue;
		if( LayerUsed.size()<=Layer(i) )
			LayerUsed.resize(Layer(i)+1);
		LayerUsed[Layer(i)] = true;
	}
	int num = 0;
	for(int i=0; i<LayerUsed.size(); i++)
		if(LayerUsed[i])
			++num;
	return num;
}

void Seg_t::print_race_layer(std::ostream& ostr, Tex_Man_t * pTex) const {
	if( !race_assigned() )
		return;

	Rect_t rect;
	for(int i=0; i<size(); i++){
		if( 0==i ){
			rect = rect.merge( pTex->getTexRect(data()[i]) );
			continue;
		}
		if( Layer(i-1)!=Layer(i) ){
			ostr<<rect<<" on "<<Layer(i-1)<<std::endl;
			rect = pTex->getTexRect(data()[i-1]);
		}
		rect = rect.merge( pTex->getTexRect(data()[i]) );
	}
	// the last rect
	if( !empty() )
		ostr<<rect<<" on "<<Layer(size()-1)<<std::endl;
}
