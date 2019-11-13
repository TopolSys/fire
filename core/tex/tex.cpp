#include "tex.hpp"

#include "lem_type.hpp" 	 // print_member
#include "utils/plt/plt.hpp" // print view
#include "utils/sys/sys.hpp"
Tex_t::Tex_t(){
	_obsLoadH = 0;
	_obsLoadV = 0;
	_netLoadH = 0;
	_netLoadV = 0;
	_pinPrefill= 0;
	_hprev   = NullEdge();
	_hnext   = NullEdge();
	_vprev   = NullEdge();
	_vnext   = NullEdge();
}

void Tex_Man_t::buildEdge(){
	_vEdge.clear();
	int y_conn_col = ynum()-1;
	int x_conn_row = xnum()-1;
	int y_conn = y_conn_col * xnum();
	int x_conn = x_conn_row * ynum();
	_vEdge.resize( y_conn + x_conn );
	int nEdge = 0;
	for(int j=0; j<ynum(); j++)
		for(int i=0; i<xnum(); i++){
			Pos_t cur(i,j);
			int cid = Pos2Index(cur);

			if(i<xnum()-1){
				int nid = Pos2Index(cur.hnext());
				_vEdge[nEdge++] = Tedge_t( cid, nid );
				data()[cid].set_hnext(nEdge-1);
				data()[nid].set_hprev(nEdge-1);
			}
			if(j<ynum()-1){
				int nid = Pos2Index(cur.vnext());
				_vEdge[nEdge++] = Tedge_t( cid, nid );
				data()[cid].set_vnext(nEdge-1);
				data()[nid].set_vprev(nEdge-1);
			}
		}

	assert( nEdge == _vEdge.size() );
}

void Tex_Man_t::allocate_layerV( int layer_num ){
	_layerV.clear();
	_layerV.resize(layer_num);
}
void Tex_Man_t::set_layerV( bool isV, int layer_id ){
	_layerV[layer_id] = isV;
}
void Tex_Man_t::allocate_layerCapX( int layer_num, int num ){
	_layerCapX.clear();
	_layerCapX.resize(layer_num);
	for(int i=0; i<layer_num; i++)
		_layerCapX[i].resize(num,0);
}
void Tex_Man_t::allocate_layerCapY( int layer_num, int num ){
	_layerCapY.clear();
	_layerCapY.resize(layer_num);
	for(int i=0; i<layer_num; i++)
		_layerCapY[i].resize(num,0);
}


float Tex_Man_t::edge_fillrate(int eid) const {
	int cap = trackCap(eid);
	return (0<cap? (float) getEdgeLoad(eid)/cap: 1.0f);
}
float Tex_Man_t::tex_fillrate (int tid) const {
	const Tex_t& Tex = getTex(tid);
	int teids[4] = {Tex.next_eid(1), Tex.prev_eid(1), Tex.next_eid(0), Tex.prev_eid(0)};
	float maxfill = 0;
	for(int i=0; i<4; i++){
		if( Tex_t::NullEdge()==teids[i] )
			continue;
		maxfill = std::max(maxfill, edge_fillrate(teids[i]));
	}
	return maxfill;
}

bool Tex_Man_t::is_overflow( const int eid, float SoftOverflow ) const {
	int cap = trackCap(eid);
	if( 0!=SoftOverflow )
		cap = (float) cap * SoftOverflow;
	return cap < getEdgeLoad(eid);
}

int Tex_Man_t::resCap( const int eid ) const {
	return trackCap(eid) - getEdgeLoad(eid);
}

int Tex_Man_t::trackCap( const int eid ) const {
	//int min_cap = minCap(eid);
	//return 0==min_cap? maxCap(eid): min_cap;
	return (minCap(eid)+maxCap(eid))>>1;
}

int Tex_Man_t::minCap( const int eid ) const {
	int index1 = Edge(eid).first ;
	int index2 = Edge(eid).second;
	Tex_Man_t::Pos_t pos1 = getPos(index1);
	Tex_Man_t::Pos_t pos2 = getPos(index2);
	bool isV = pos1._x==pos2._x;
	const Tex_t& tex1 = getTex(index1);
	const Tex_t& tex2 = getTex(index2);
	int cap = get_Cap( isV, isV? pos1._x: pos1._y );
	int load= std::max( tex1.obsLoad(isV), tex2.obsLoad(isV) );
	return cap <= load? 0: (cap - load);
}
int Tex_Man_t::maxCap( const int eid ) const {
	int index1 = Edge(eid).first ;
	int index2 = Edge(eid).second;
	Tex_Man_t::Pos_t pos1 = getPos(index1);
	Tex_Man_t::Pos_t pos2 = getPos(index2);
	bool isV = pos1._x==pos2._x;
	const Tex_t& tex1 = getTex(index1);
	const Tex_t& tex2 = getTex(index2);
	int cap = get_Cap( isV, isV? pos1._x: pos1._y );
	int load= std::min( tex1.obsLoad(isV), tex2.obsLoad(isV) );
	return cap <= load? 0: (cap - load);
}

int  Tex_Man_t::get_Cap( int isV, int index ) const {
	return isV? _vCapX[index]: _vCapY[index];
}
void Tex_Man_t::inc_Cap( int isV, int index ){
	( isV? _vCapX[index]: _vCapY[index] ) ++;
}
void Tex_Man_t::set_Cap( int isV, int index, int val ){
	( isV? _vCapX[index]: _vCapY[index] ) = val;
}

int  Tex_Man_t::get_layerCap( int isV, int layer_id, int index ) const {
	return isV? _layerCapX[layer_id][index]: _layerCapY[layer_id][index];
}
void Tex_Man_t::inc_layerCap( int isV, int layer_id, int index ){
	( isV? _layerCapX[layer_id][index]: _layerCapY[layer_id][index] ) ++;
}
void Tex_Man_t::set_layerCap( int isV, int layer_id, int index, int val ){
	( isV? _layerCapX[layer_id][index]: _layerCapY[layer_id][index] ) = val;
}

void Tex_Man_t::dump_Cap( std::ostream& ostr ){
	const int dump_num = 6;
	ostr<<"_vCapX["<< _vCapX.size() <<"]: ";
	print_member( ostr, _vCapX, dump_num );

	ostr<<"_vCapY["<< _vCapY.size() <<"]: ";
	print_member( ostr, _vCapY, dump_num );
}

bool Tex_Man_t::intrinsic_overflow( const Pos_t& pos, float SoftOverflow ) const {
	bool isV = true;
	int cap_v = (float) get_Cap(  isV, pos._x ) * SoftOverflow;
	int cap_h = (float) get_Cap( !isV, pos._y ) * SoftOverflow;
	int load_v = getTex(Pos2Index(pos)).obsLoad( isV);
	int load_h = getTex(Pos2Index(pos)).obsLoad(!isV);
	return cap_v <= load_v || cap_h <= load_h;
}

int  Tex_Man_t::num_overflow( const vInt_t& vTeid, float SoftOverflow ) const {
	int nOverflow = 0;
	for(int i=0; i<vTeid.size(); i++){
		int teid = vTeid[i];
		if( is_overflow(teid, SoftOverflow ) )
			nOverflow ++ ;
	}
	return nOverflow;
}

void Tex_Man_t::dump_layerCap( std::ostream& ostr ){
	const int dump_num = 6;
	ostr<<"Vertical   direction: \n";
	for(int i=0; i<_layerCapX.size(); i++ ){
		ostr<<"\tlayer "<<i<<" ["<< _layerCapX[i].size() <<"]: ";
		print_member( ostr, _layerCapX[i], dump_num );
	}
	ostr<<"Horizontal direction: \n";
	for(int i=0; i<_layerCapY.size(); i++ ){
		ostr<<"\tlayer "<<i<<" ["<< _layerCapY[i].size() <<"]: ";
		print_member( ostr, _layerCapY[i], dump_num );
	}
}

Lem_Rgb_t Load2Heat( int load, int cap ){
	const int cold = 0x00FFFF;
	const int hot0 = 0xFF0000;
	if( 0==cap )
		return Lem_Rgb_t(0,255,255,255);
	if( load<cap ){
		float ratio = (float) load/cap;
		int color = (float) cold * ratio;
//		return Lem_Rgb_t(0, 0, (color&0x00FF00)>>8, color&0x0000FF );
		if( ratio <= 0.5 )
			return Lem_Rgb_t(0, 0, 0, color&0x0000FF );
		color = (float) 0xFF * (1.0f-ratio);
		return Lem_Rgb_t(0, 0, color, 0xFF );
	}
	int overflow = load - cap;
	int fill = std::min(0xFFFF, overflow);
	return Lem_Rgb_t(0, hot0>>16, (fill&0x00FF00)>>8, fill&0x0000FF );
}

void Tex_Man_t::plot( const char * FileName ) const {
	if( data().empty() ){
		lem::warning.add("no congestion map found. skip plotting.");
		return ;
	}
	Lem_Plt_t plt(FileName);
	for(int j=0; j<ynum(); j++){
		int ycap = get_Cap(0,j);
		for(int i=0; i<xnum(); i++){
			int cap = ycap + get_Cap(1,i);
			if( 0==cap ){
				plt.addRect( Rect_t( Point_t(vGridX()[i],vGridY()[j]), Point_t(vGridX()[i+1],vGridY()[j+1]) ), Lem_Rgb_t(0,255,255,255) );
				continue;
			}
			int load_obs = entry(j,i).obsLoad();
			int load_wire_h= std::max(entry(j,i).hnext_eid()>=0? getEdgeLoad(entry(j,i).hnext_eid()): 0, entry(j,i).hprev_eid()>=0? getEdgeLoad(entry(j,i).hprev_eid()): 0 );
			int load_wire_v= std::max(entry(j,i).vnext_eid()>=0? getEdgeLoad(entry(j,i).vnext_eid()): 0, entry(j,i).vprev_eid()>=0? getEdgeLoad(entry(j,i).vprev_eid()): 0 );
			int load = load_obs + load_wire_h + load_wire_v;
			if( 0==load )
				continue;
			plt.addRect( Rect_t( Point_t(vGridX()[i],vGridY()[j]), Point_t(vGridX()[i+1],vGridY()[j+1]) ), Load2Heat(load, cap) );
		}
	}
	plt.tail();
}


int Tex_Man_t::num_edge_overflow( float SoftOverflow ) const {
	int nOverflow = 0, nNoCap = 0;
	const int nEdge = Edge().size();
	for(int i=0; i<nEdge; i++){
		if( is_overflow(i,SoftOverflow) && 0 < getEdgeLoad(i) )
			nOverflow++;
	}
	return nOverflow;
}

void Tex_Man_t::print_edge() const {
	int nOverflow = 0, nNoCap = 0;
	const int nEdge = Edge().size();
	vInt_t vLoadStat(11,0);
	for(int i=0; i<nEdge; i++){
		int cap = trackCap(i);
		int load= getEdgeLoad(i);// - Edge(i).pinPrefill();
		if( 0==cap && 0==load ){
			nNoCap ++ ;
			continue;
		}
		float ratio = (float) load/cap;
		if( 1.0f < ratio ){
			nOverflow++;
			continue;
		}
		int index = ratio*10;
		vLoadStat[index] ++ ;
	}
	printf("\n");
	if( nNoCap )
		printf(" No track:%12d (%4.1f\%)\n", nNoCap, 100*(nEdge? (float)nNoCap/nEdge: 0) );
	if( nOverflow )
		printf("%4s~%3d\%:%12d (%4.1f\%)\n","", 101, nOverflow, 100*(nEdge? (float)nOverflow/nEdge: 0) );
	for(int i=vLoadStat.size()-1; i>=0; i--)
		printf("%3d\%~%3d\%:%12d (%4.1f\%)\n", (i+(i<10?1:0))*10, i*10, vLoadStat[i],  100*(nEdge? (float)vLoadStat[i]/nEdge: 0) );
}

const int Tex_Man_t::getTidPrev( const int index, const bool isV ) const {
	int teid = getTex(index).prev_eid(isV);
	if( Tex_t::NullEdge()==teid )
		return NullTex ();
	assert( index == Edge(teid).first || index == Edge(teid).second );
	return  index == Edge(teid).first? Edge(teid).second: Edge(teid).first;
}

const int Tex_Man_t::getTidNext( const int index, const bool isV ) const {
	int teid = getTex(index).next_eid(isV);
	if( Tex_t::NullEdge()==teid )
		return NullTex ();
	assert( index == Edge(teid).first || index == Edge(teid).second );
	return  index == Edge(teid).first? Edge(teid).second: Edge(teid).first;
}

const int Tex_Man_t::total_teid_cap() const {
	int cap = 0;
	for(int i=0; i<_vEdge.size(); i++)
		cap += trackCap(i);
	return cap;
}

int  Tex_Man_t::getEdgeLoad( int teid ) const {
	bool isV = is_veid(teid);
	return std::max( Edge(teid).load(), std::max( getTexByTeid1(teid).netLoad(isV), getTexByTeid2(teid).netLoad(isV) ));
}