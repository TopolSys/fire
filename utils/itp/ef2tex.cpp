#include "db/db.hpp" 						//db access
#include "ispd2019_opensource/src/db/db.h" 	//db access

#include <iostream>
#include <iomanip>
#include <cassert>
#include "ef2tex.hpp" 			// translator
#include "core/tex/tex.hpp" 	// tile vertex 
#include "utils/sys/sys.hpp" 	// error logging
#include "utils/plt/plt.hpp"

#include "lem_mac.hpp"
#include "lem_geom.hpp"

class _Tex2Ef_t {
public:
	_Tex2Ef_t( ispd19::Database * pDb ){
		_pDb = pDb;
	}
	const ispd19::GCell & gcells(){ return _pDb->gcells(); }
private:
	ispd19::Database * _pDb;
};


Tex_Man_t * Lem_StartTexFromEf( Lem_Gdb_t * pGdb ){
	using namespace std;

	char buff[1024];
	ispd19::Database * pDb = pGdb->efdb();
	const ispd19::GCell & gcells = pDb->gcells();
	assert( NULL != pDb );
	const int nRow = gcells.numY()-1;
	const int nCol = gcells.numX()-1;
	if( nRow < 2 || nCol < 1 ){
		sprintf(buff, "illegal tile number (row,col)=%2d,%2d", nRow,nCol);
		lem::error.add(buff);
		return NULL;
	}
	Tex_Man_t * pTex = new Tex_Man_t;
	Tex_Man_t & Tex = * pTex;
	Tex.vGridX() = gcells.x();
	Tex.vGridY() = gcells.y();
//	Tex.resize( nRow, nCol, 0 );
//	for(int i=0; i<nRow; i++){
//		Tex[i].resize( nCol );
//		for(int j=0; j<nCol; j++){
//			Tex[i][j];
//		}
//	}

	if( lem::dev() )
		std::cout<< nRow * nCol <<" tile vertex constructed\n";

	pTex->allocate_vCapX( nCol );
	pTex->allocate_vCapY( nRow );

	pTex->allocate_layerCapX( pDb->numLayers(), nCol );
	pTex->allocate_layerCapY( pDb->numLayers(), nRow );

	pTex->allocate_layerV( pDb->numLayers() );
	for(int l=0; l<pDb->numLayers(); l++){
		ispd19::RouteLayer& layer = pDb->getRouteLayer(l);
		pTex->set_layerV(layer.isV(), l);
	}

	// build capacity: 
	// [O] compute cap for each layer
	// [X] offest of grid specified on each layer
	pTex->layerName().clear();
	pTex->layerName().resize(pDb->numLayers());
	pTex->initLSpacing(pDb->numLayers());
	for(int l=0; l<pDb->numLayers(); l++){
		ispd19::RouteLayer     &     layer = pDb->getRouteLayer(l);
		const ispd19::TrackSet & TrackInfo = layer.tracks( /* default = prefered-way track*/ );
		bool isV = layer.isV();
		const vInt_t& grid = gcells.grid( isV );
		pTex->layerName()[l] = layer.name();

		int track_pos = TrackInfo.start(); // add offset if specified
		int track_num = 0;
		for(int i=0; i<grid.size()-1; i++){
			int curPos  = grid[i];
			int nextPos = grid[i+1];
			while( track_pos <  curPos && track_num < TrackInfo.num() )
				track_pos += TrackInfo.step(), track_num ++;
			
			while( track_pos < nextPos && track_num < TrackInfo.num() ){
				track_pos += TrackInfo.step(), track_num ++;
				pTex->inc_layerCap( isV, l, i ); 	// layer capacity
				if( pGdb->StartLya()<=l )
					pTex->inc_Cap( isV, i ); 		// total capacity
			}
			//printf("Layer %s lid=%2d track_num=%4d pos=[%6d:%6d]\n", layer.name().c_str(), l, track_num, curPos, nextPos );
		}

		LSpacing_t& LSpacing = pTex->getLSpacing(l);
		LSpacing.resize( layer.getPRLTable().size() );
		for(int i=0; i<layer.getPRLTable().size(); i++){
			const ispd19::PRLTable& prlTable = layer.getPRLTable()[i];
			const mInt_t& table = prlTable.getTable();
			LSpacing[i].reset(prlTable.getWidth(), prlTable.getLength());
			for(int j=0; j<table.size(); j++)
				LSpacing[i].setSpacing(j, prlTable.getWidth()[j], prlTable.getTable()[j] );
		}
//		if(3==l){
//			std::cout<<layer.name()<<std::endl;
//			for(int i=0; i<layer.getPRLTable().size(); i++){
//				const mInt_t& table = layer.getPRLTable()[i].getTable();
//				for(int j=0; j<table.size(); j++){
//					std::cout<<"i="<<i<<", j="<<j<<": "<<layer.getPRLTable()[i].getWidth()[j]<<": ";
//					for(int k=0; k<table[j].size(); k++)
//						std::cout<<table[j][k]<<" ";
//					std::cout<<std::endl;
//				}
//			}
//			LSpacing.report();
//			int target = 0, length = 0;
//			target = 179;
//			std::cout<<" spacing of width "<< target <<": "<<LSpacing.spacing (target) <<"\n";
//			target = 180;
//			std::cout<<" spacing of width "<< target <<": "<<LSpacing.spacing (target) <<"\n";
//			target = 181;
//			std::cout<<" spacing of width "<< target <<": "<<LSpacing.spacing (target) <<"\n";
//			target = 930;
//			std::cout<<" spacing of width "<< target <<": "<<LSpacing.spacing (target) <<"\n";
//			target = 940;
//			std::cout<<" spacing of width "<< target <<": "<<LSpacing.spacing (target) <<"\n";
//			target = 950;
//			std::cout<<" spacing of width "<< target <<": "<<LSpacing.spacing (target) <<"\n";
//			target = 1260;
//			std::cout<<" spacing of width "<< target <<": "<<LSpacing.spacing (target) <<"\n";
//			target = 1261;
//			std::cout<<" spacing of width "<< target <<": "<<LSpacing.spacing (target) <<"\n";
//			target = 1320, length=target;
//			std::cout<<" spacing of width "<< target <<": "<<LSpacing.spacing (target, target) <<"\n";
//			target = 1320, length=length-1;
//			std::cout<<" spacing of width "<< target <<": "<<LSpacing.spacing (target, target-1) <<"\n";
//			target = 1320-1, length=target+1;
//			std::cout<<" spacing of width "<< target <<": "<<LSpacing.spacing (target-1, target) <<"\n";
//			target = 1320-1, length= target;
//			std::cout<<" spacing of width "<< target <<": "<<LSpacing.spacing (3000, 6000) <<"\n";
//		}
	}
//	pDb->reportLayers();\
	pTex->dump_Cap( std::cout );\
	pTex->dump_layerCap( std::cout );\
	exit(0);
	return pTex;
}

int Lem_EfMacroSearchPin( Lem_Gdb_t * pGbd, const std::string& pin_name, int macro_id ){
	ispd19::Database * pDb = pGbd->efdb();
	ispd19::Macro * pMacro = &pDb->getMacro( macro_id );
	Lem_Iterate( pMacro->pins(), pPin )
		if( pin_name == pPin->name() )
			return pPin - pMacro->pins().begin();
	return ispd19::Macro::NullIndex;
}

Rect_t Lem_ApplyEfOrien( const Rect_t& rect, int orien ){
	Rect_t ret = Lem_R90( rect, orien );
	if( orien >= 4 )
		ret = Lem_Flip( ret, 0 );
	return ret;
}

static Rect_t Lem_Shape2Rect( const ispd19::Shape& shape ){
	return Rect_t( shape.lx(), shape.ly(), shape.hx(), shape.hy() );
}
static Rect_t Lem_Box2Rect( const ispd19::Box& box ){
	return Rect_t( box.lx(), box.ly(), box.hx(), box.hy() );
}

void Ef_RectOnTex( Lem_Gdb_t * pGdb, const Rect_t& rect, IntPair_t& IdxRangeY, IntPair_t& IdxRangeX ){
	IdxRangeY = Lem_CoveredTex( rect.rangeY(), pGdb->efdb()->gcells().y() );
	IdxRangeX = Lem_CoveredTex( rect.rangeX(), pGdb->efdb()->gcells().x() );
}

Rect_t Lem_EfInstPlacedRect( Lem_Gdb_t * pGdb, const ispd19::Instance& inst, const Rect_t& rect ){
	using namespace ispd19;
	ispd19::Database * pDb = pGdb->efdb();
	
	Macro&    macro  = pDb->getMacro   (    inst.macro() );
	Rect_t FrameRect = Rect_t( 0,0, macro.width(), macro.height() );
	FrameRect        = Lem_ApplyEfOrien( FrameRect, inst.ori() );
	Point_t PlacedPosition = Point_t( inst.x(), inst.y() );
	Point_t ExternOffset   = Point_t( -macro.origX(), -macro.origY() );

	Rect_t ret = rect;
	ret = Lem_ApplyEfOrien( ret, inst.ori() );
	ret = Lem_Offset( ret, FrameRect.p1, true );
	ret = Lem_Offset( ret, ExternOffset );
	ret = Lem_Offset( ret, PlacedPosition );

	return ret;
}

void Lem_Pin2LayerChoice( Lem_Gdb_t * pGdb, const ispd19::Pin& NetPin, Teg_t& Teg ){
	using namespace ispd19;
	ispd19::Database * pDb = pGdb->efdb();
	
	int nLayerUsed = 0;
	vBool_t vLayerUsed( pDb->numLayers() ,false );
	for(int i=0; i<NetPin.shapes().size() && nLayerUsed < pDb->numLayers(); i++){
		if( vLayerUsed[ NetPin.shapes()[i].z() ] )
			continue;
		nLayerUsed ++ ;
		vLayerUsed[ NetPin.shapes()[i].z() ] = true;
	}
	Teg.vLayer().clear();
	Teg.vLayer().resize(nLayerUsed);
	int i=0, j=0;
	for(i=0, j=0; i<vLayerUsed.size(); i++)
		if( vLayerUsed[i] )
			Teg.vLayer()[j++] = i;
	assert( j==nLayerUsed );
	assert( nLayerUsed>0 );
}

bool Lem_MacroPin2Teg( Lem_Gdb_t * pGdb, Tex_Man_t * pTex, const ispd19::Pin& NetPin, Teg_t& Teg ){
	using namespace ispd19;
	ispd19::Database * pDb = pGdb->efdb();
	
	Instance&  inst  = pDb->getInstance( NetPin.compId() );
	Macro&    macro  = pDb->getMacro   (    inst.macro() );
	
	int index = Lem_EfMacroSearchPin(pGdb, NetPin.name(), inst.macro());
	assert( index < macro.pins().size() );
	const Pin& pin = macro.pins()[index];
	Rect_t PinRect = Lem_Box2Rect( pin.box() );
	PinRect = Lem_EfInstPlacedRect( pGdb, inst, PinRect );
	Teg.set_box( PinRect );

	IntPair_t IdxRangeX, IdxRangeY;
	Ef_RectOnTex( pGdb, PinRect, IdxRangeY, IdxRangeX );
	Teg.gposY( PairMidpoint(IdxRangeY) );
	Teg.gposX( PairMidpoint(IdxRangeX) );
	Teg.grangeY( IdxRangeY );
	Teg.grangeX( IdxRangeX );

	Lem_Pin2LayerChoice(pGdb, pin, Teg);
	return true;
}

bool Lem_Pin2Teg( Lem_Gdb_t * pGdb, Tex_Man_t * pTex, const ispd19::Pin& NetPin, Teg_t& Teg ){
	using namespace ispd19;
	ispd19::Database * pDb = pGdb->efdb();

	const Pin&  pin  = pDb->getPin( NetPin.name() );
	if( !pin.hasCoordinate() ){
		char buff[1024];
		sprintf( buff, "Pin \'%s\' is not one of placed/fixed/cover", pin.name().c_str() );
		lem::warning.add(buff);
		return false;
	}
	Rect_t PinRect = Lem_Box2Rect( pin.box() );
	Point_t PlacedPosition = Point_t( pin.x(), pin.y() );

	PinRect = Lem_ApplyEfOrien( PinRect, pin.ori() );
	PinRect = Lem_Offset( PinRect, PlacedPosition );
	Teg.set_box( PinRect );

	IntPair_t IdxRangeX, IdxRangeY;
	Ef_RectOnTex( pGdb, PinRect, IdxRangeY, IdxRangeX );
	Teg.gposY( PairMidpoint(IdxRangeY) );
	Teg.gposX( PairMidpoint(IdxRangeX) );
	Teg.grangeY( IdxRangeY );
	Teg.grangeX( IdxRangeX );

	Lem_Pin2LayerChoice(pGdb, pin, Teg);
	return true;
}

bool Lem_Net2Tcl( Lem_Gdb_t * pGdb, Tex_Man_t * pTex, const ispd19::Net& net, Tcl_t& Tcl ){
	using namespace ispd19;
	ispd19::Database * pDb = pGdb->efdb();

	int nPins = 0, nProblem = 0;
	Tcl.set_id( &Tcl - pGdb->vTcl.data() );
	Tcl.set_name( net.name().c_str() );
	Tcl.clear();
	Tcl.resize(net.pins().size());

	Lem_Iterate( net.pins(), pPin ){
		if( ispd19::Instance::NullIndex != pPin->compId() ){
			// macro 
			if( ! Lem_MacroPin2Teg(pGdb, pTex, *pPin, Tcl[nPins++]) )
				nProblem ++;
		} else {
			// non-macro-pin
			// lefdefref: p281
			if( ! Lem_Pin2Teg(pGdb, pTex, *pPin, Tcl[nPins++]) )
				nProblem ++;
		}
		//std::cout<< Tcl[nPins-1] <<std::endl; // print
	}
	if( 0 != nProblem ){
		char buff[1024];
		sprintf(buff,"in net \'%s\'", net.name().c_str() );
		lem::warning.add(buff);
		return false;
	} else {
		Tcl.setOk();
	}
	return true;
}

void Lem_StartNetFromEf( Lem_Gdb_t * pGdb, Tex_Man_t * pTex ){
	ispd19::Database * pDb = pGdb->efdb();

	pGdb->vTcl.clear();
	pGdb->vTcl.resize( pDb->nets().size() );
	
	int nNets = 0;
	int nProblem = 0;
	Lem_Iterate( pDb->nets(), pNet ){
		pGdb->vTcl[nNets].set_id(nNets);
		if( ! Lem_Net2Tcl(pGdb, pTex, *pNet, pGdb->vTcl[nNets++]) )
			nProblem ++;
	}
	if( lem::dev() ){
		int nMultiYPins = 0;
		int nMultiXPins = 0;
		int MaxYCross = 0;
		int MaxXCross = 0;
		int nMulti2DPins = 0;
		int nTotalPins = 0;
		for(int i=0; i<pGdb->vTcl.size(); i++){
			const Tcl_t& Tcl = pGdb->vTcl[i];
			nTotalPins += Tcl.size();
			for(int j=0; j<Tcl.size(); j++){
				int nCrossY = Tcl[j].grangeY().second - Tcl[j].grangeY().first ;
				int nCrossX = Tcl[j].grangeX().second - Tcl[j].grangeX().first ;
				MaxYCross = std::max( nCrossY, MaxYCross );
				MaxXCross = std::max( nCrossX, MaxXCross );
				if( 1 < nCrossY )
					nMultiYPins++;
				if( 1 < nCrossX )
					nMultiXPins++;
				if( 1 < nCrossY && 1 < nCrossX )
					nMulti2DPins++;
				//assert( 0<nCrossY );\
				assert( 0<nCrossX );
			}
		}
		std::cout<<std::setw(13)<< "nGlobalPins="<<std::setw(8)<< pGdb->efdb()->pins().size()<<std::endl;
		std::cout<<std::setw(13)<< "nMultiYPins="<<std::setw(8)<< nMultiYPins<<std::endl;
		std::cout<<std::setw(13)<< "nMultiXPins="<<std::setw(8)<< nMultiXPins<<std::endl;
		std::cout<<std::setw(13)<<   "MaxYCross="<<std::setw(8)<<   MaxYCross<<std::endl;
		std::cout<<std::setw(13)<<   "MaxXCross="<<std::setw(8)<<   MaxXCross<<std::endl;
		std::cout<<std::setw(13)<<"nMulti2DPins="<<std::setw(8)<<nMulti2DPins<<std::endl;
		std::cout<<std::setw(13)<<  "nTotalPins="<<std::setw(8)<<  nTotalPins<<std::endl;
	}
}


bool isPRLWarning( Tex_Man_t * pTex, int layer, const Rect_t& rect ){
	bool isV = pTex->layerV()[layer];
	int spacing = isV
		? pTex->getLSpacing(layer).spacing(rect.rangeX().scalar(), rect.rangeY().scalar())   // x-spacing
		: pTex->getLSpacing(layer).spacing(rect.rangeY().scalar(), rect.rangeX().scalar());  // y-spacing
	int min_spacing = isV
		? pTex->getLSpacing(layer).spacing(rect.rangeX().scalar(), 1)   // x-spacing
		: pTex->getLSpacing(layer).spacing(rect.rangeY().scalar(), 1);  // y-spacing
	return (min_spacing<<1) < spacing;
}

Rect_t Rect2SpacingBox( Tex_Man_t * pTex, int layer, const Rect_t& rect ){
	int yspacing = pTex->getLSpacing(layer).spacing(rect.rangeY().scalar(), rect.rangeX().scalar());
	int xspacing = pTex->getLSpacing(layer).spacing(rect.rangeX().scalar(), rect.rangeY().scalar());
	Point_t p1 = Point_t(rect.p1.x - xspacing, rect.p1.y - yspacing);
	Point_t p2 = Point_t(rect.p2.x + xspacing, rect.p2.y + yspacing);
	return Rect_t( p1, p2 );
}

class ObsLoad_t {
public:
	typedef std::vector<LineSeg_t> vLineSeg_t;
	void addLoadMask( int layer_id, const Range_t& range ){
		if( _vLoadMask.size() <= layer_id )
			_vLoadMask.resize( layer_id + 1 );
		_vLoadMask[layer_id].add( range.n1, range.n2 );
	}
	const vLineSeg_t& vLoadMask(){ return _vLoadMask; }
private:
	vLineSeg_t _vLoadMask;
};

typedef std::vector< ObsLoad_t> vObsLoad_t;
typedef std::vector<vObsLoad_t> mObsLoad_t;
class Obs_t: public Rect_t {
public:
	int _lid;
	Obs_t(const int lid, const Rect_t& rect):_lid(lid),Rect_t(rect){}
};
class MacroObs_t {
public:
	typedef std::vector< Obs_t> vObs_t;
	typedef std::vector< Obs_t> vPin_t;
	vObs_t vObs; 			 // obstructions on layers 
	vPin_t vPin; 			 // obstructions on layers 
	void add( int layer_id, const Rect_t& rect ){
		vObs.push_back(Obs_t(layer_id,rect));
	}
	void addPin( int layer_id, const Rect_t& rect ){
		vPin.push_back(Obs_t(layer_id,rect));
	}
	void report( std::ostream& ostr )const{
		int layer = 0;
		Lem_Iterate(vObs, pObs){
			layer ++;
			if( *pObs == Rect_t::Null() )
				continue;
			ostr<<"\t"<<"layer "<<layer<<": "<< *pObs <<std::endl;
		}
	}
};

typedef std::vector<MacroObs_t> vMacroObs_t;

class Obs_Man_t {
public:
	Obs_Man_t(Lem_Gdb_t * npGdb, Tex_Man_t * npTex):pGdb(npGdb),pTex(npTex){
		_vMacroObs.resize( pGdb->efdb()->macros().size() );
		mObsLoad.resize( pTex->ynum() );
		Lem_Iterate( mObsLoad, pRow )
			pRow->resize( pTex->xnum() );
	}
	void setMacroObs( int macro_id, const ispd19::Macro& macro ){
		const ispd19::Obs& obs = macro.obs();
		for(int i=0; i<obs.shapes().size(); i++){
			const ispd19::Shape& shape = obs.shapes()[i];
			_vMacroObs[macro_id].add( shape.z(), Lem_Shape2Rect(shape) );
			assert( shape.z() < pGdb->efdb()->numLayers() );
		}
		Lem_Iterate(macro.pins(), pPin ){\
			Lem_Iterate(pPin->shapes(), pShape)\
				_vMacroObs[macro_id].addPin( pShape->z(), Lem_Shape2Rect(*pShape) );\
		}
		//std::cout<<macro.name()<<":\n";\
		_vMacroObs[macro_id].report(std::cout);
	}
	void addObsLoad( int layer_id, const Rect_t& rectOrg ){
		const ispd19::GCell& gc = pGdb->efdb()->gcells();
		bool isV = pGdb->efdb()->getRouteLayer(layer_id).isV();
		bool fPRL = isPRLWarning(pTex, layer_id, rectOrg);
		Rect_t rect = Rect2SpacingBox( pTex, layer_id, rectOrg);
		// get affected grids
		IntPair_t IdxRangeX = Lem_CoveredTex( rect.rangeX(), pTex->vGridX() );
		IntPair_t IdxRangeY = Lem_CoveredTex( rect.rangeY(), pTex->vGridY() );
		// add load
		for(int j=IdxRangeY.first; j<IdxRangeY.second; j++){
			Range_t GcellRangeY( gc.gcellLY(j), gc.gcellHY(j) );
			for(int i=IdxRangeX.first; i<IdxRangeX.second; i++){
				Range_t GcellRangeX( gc.gcellLX(i), gc.gcellHX(i) );
				//Range_t loadMask = isV\
					? rect.rangeX().overlap(GcellRangeX)\
					: rect.rangeY().overlap(GcellRangeY);
				Range_t loadMask = isV? rect.rangeX(): rect.rangeY();
				if( fPRL )
					pTex->getTex(Tex_Man_t::Pos_t(i,j)).set_layerPRL(layer_id);
				if( Range_t::Null() == loadMask )
					continue;
				mObsLoad[j][i].addLoadMask( layer_id, loadMask );
			}
		}
	}

	// how many tracks are shadowed by load mask
	int loadMask2Track( int layer_id, int idx_y, int idx_x, const LineSeg_t& loadMask ){
		if( loadMask.empty() )
			return 0;
		const ispd19::GCell& gc = pGdb->efdb()->gcells();
		bool isV = pGdb->efdb()->getRouteLayer(layer_id).isV();
		LineSeg_t FreeSpace = loadMask;
		int idx = isV? idx_x: idx_y;
		if( isV )
			FreeSpace.complement( gc.gcellLX(idx), gc.gcellHX(idx) );
		else
			FreeSpace.complement( gc.gcellLY(idx), gc.gcellHY(idx) );
		const int cellBegin = isV? gc.gcellLX(idx): gc.gcellLY(idx);
		const int cellEnd   = isV? gc.gcellHX(idx): gc.gcellHY(idx);
		ispd19::RouteLayer     &     layer = pGdb->efdb()->getRouteLayer( layer_id );
		const ispd19::TrackSet & TrackInfo = layer.tracks( /* default = prefered-way track*/ );
		int FreeTrack = 0;
		int tex = pTex->Pos2Index(Tex_Man_t::Pos_t(idx_x,idx_y));
		Lem_Iterate(FreeSpace, pSeg ){
			int num_step = (pSeg->n1 - TrackInfo.start())/TrackInfo.step();
			int cur_pos  = num_step * TrackInfo.step() + TrackInfo.start();
			//const int begin_spacing = cellBegin==pSeg->n1? 0: (layer.width()/2 + layer.spacing());
			//const int begin_spacing = cellBegin==pSeg->n1? 0: (TrackInfo.step()-layer.width()/2);
			const int begin_spacing = cellBegin==pSeg->n1? 0: (layer.width()/2);

//				if( 140704==tex|| 140714==tex ||140784==tex){
//					std::cout<<"\t"<<tex<<" space range="<<pSeg->n1 <<","<< pSeg->n2<<std::endl;
//					std::cout<<"\t"<<tex<<" org pos="<<cur_pos<<", step="<< TrackInfo.step()<<" begin_spacing="<<begin_spacing<<",layer.spacing="<<layer.spacing()<<", min width="<< layer.width()<<std::endl;
//				}
			while( cur_pos - begin_spacing < pSeg->n1 ){
//				if( 140704==tex|| 140714==tex ||140784==tex)\
				std::cout<<"\t"<<tex<<" cmp " << cur_pos - begin_spacing <<" vs. "<< pSeg->n1<< " dist = "<< pSeg->n1-(cur_pos - begin_spacing)<<std::endl;
				cur_pos += TrackInfo.step();
			}

//				if( 140704==tex|| 140714==tex ||140784==tex){
//					std::cout<<"\t"<<tex<<": begin at = "<<cur_pos<<std::endl;
//				}
			int end_pos = TrackInfo.num() * TrackInfo.step() + TrackInfo.start();
			int num_track = 0;
			//const int pos_inc = std::max( TrackInfo.step(), layer.spacing() + layer.width() );
			const int pos_inc = TrackInfo.step();

//				if( 140704==tex|| 140714==tex ||140784==tex){
//					std::cout<<"\t"<<tex<<": pos_inc= "<<pos_inc<<std::endl;
//				}
			//const int end_spacing = cellEnd==pSeg->n2? 0: (layer.width()/2 + layer.spacing());
			//const int end_spacing = cellEnd==pSeg->n2? 0: (TrackInfo.step()-layer.width()/2);
			const int end_spacing = cellEnd==pSeg->n2? 0: (layer.width()/2);
			for(; cur_pos < cellEnd && cur_pos + end_spacing <= pSeg->n2 ; cur_pos += pos_inc ){

				num_track++;
//				if( 140704==tex|| 140714==tex ||140784==tex){
//					std::cout<<"\t"<<tex<<": "<<cur_pos<<": num_track="<<num_track <<std::endl;
//				}
			}
			FreeTrack += num_track;
		}
		int Capacity = pTex->get_layerCap( isV, layer_id, idx );
//		if( 140704==tex|| 140714==tex ||140784==tex){
//			std::cout<<"\t"; loadMask.print();std::cout<<"\n";
//			std::cout<<"\t"; FreeSpace.print();std::cout<<"\n";
//			std::cout<<tex<<": cap=="<<Capacity<<" FreeTrack= "<<FreeTrack<<std::endl;
//			std::cout<<tex<<": gcell ypos= "<< gc.gcellLY(idx_y)<<","<< gc.gcellHY(idx_y) <<std::endl;
//			std::cout<<tex<<": gcell xpos= "<< gc.gcellLY(idx_x)<<","<< gc.gcellHY(idx_x) <<std::endl;
//		}
		return Capacity - FreeTrack;
	}

	MacroObs_t::vObs_t& vMacroObs( int macro_id ){ return _vMacroObs[macro_id].vObs;}
	MacroObs_t::vPin_t& vMacroPin( int macro_id ){ return _vMacroObs[macro_id].vPin;}
	Lem_Gdb_t * pGdb;
	Tex_Man_t * pTex;
	mObsLoad_t mObsLoad;
private:
	vMacroObs_t _vMacroObs;
};

void Lem_PlotInst( Lem_Gdb_t * pGdb, Lem_Plt_t * pPlt, const ispd19::Instance& inst ){
	ispd19::Database * pDb = pGdb->efdb();
	ispd19::Macro&    macro  = pDb->getMacro   (    inst.macro() );
	Rect_t FrameRect = Rect_t( 0,0, macro.width(), macro.height() );
	Rect_t rect = Lem_EfInstPlacedRect(pGdb, inst, FrameRect);
	pPlt->addRect(rect,Lem_Rgb_t(30,255,255,255));
}
void Lem_PlotInst( Lem_Gdb_t * pGdb ){
	ispd19::Database * pDb = pGdb->efdb();
	Lem_Plt_t plt("cell.plot");
	Lem_Iterate( pDb->instances(), pInst )
		Lem_PlotInst( pGdb, &plt, *pInst );
	plt.tail();
}
void Lem_Inst2Obs( Tex_Man_t * pTex, Obs_Man_t * pMan, const ispd19::Instance& inst ){
	ispd19::Database * pDb = pMan->pGdb->efdb();
	int macro_id = inst.macro();
	Lem_Iterate( pMan->vMacroObs(macro_id), pObs ){
		if( *pObs == Rect_t::Null() )
			continue;
		Rect_t rect = Lem_EfInstPlacedRect(pMan->pGdb, inst, *pObs);
		int layer = pObs->_lid;
//		rect = Rect2SpacingBox( pTex, layer, rect);
		pMan->addObsLoad(layer, rect);
	}
	Lem_Iterate(pMan->vMacroPin(macro_id), pPin){
		if( *pPin == Rect_t::Null() )
			continue;
		Rect_t rect = Lem_EfInstPlacedRect(pMan->pGdb, inst, *pPin);
		int layer = pPin->_lid;
//		rect = Rect2SpacingBox( pTex, layer, rect);
		pMan->addObsLoad(layer, rect);
	}

//	ispd19::Macro&    macro  = pDb->getMacro   (    inst.macro() );
//	Rect_t FrameRect = Rect_t( 0,0, macro.width(), macro.height() );
//	Rect_t rect = Lem_EfInstPlacedRect(pMan->pGdb, inst, FrameRect);
//	pMan->addObsLoad(0, rect);
}

void Lem_Man2TexLoad( Obs_Man_t * pMan, Lem_Gdb_t * pGdb, Tex_Man_t * pTex ){
	Tex_Man_t& TexMan = *pTex;
//	TexMan.resize( TexMan.ynum(), TexMan.xnum() );
//	TexMan.resize( TexMan.ynum() );
//	for(int j=0; j<TexMan.ynum(); j++)
//		TexMan[j].resize( TexMan.xnum() );

	for(int j=0; j<TexMan.ynum(); j++)
		for(int i=0; i<TexMan.xnum(); i++){
			for(int k=0; k<pMan->mObsLoad[j][i].vLoadMask().size(); k++)
				if( pGdb->StartLya() <= k && ! pMan->mObsLoad[j][i].vLoadMask()[k].empty() ){
					int NonFreeTrack = pMan->loadMask2Track(k, j, i, pMan->mObsLoad[j][i].vLoadMask()[k] );
					//TexMan[j][i].add_obsLoad(NonFreeTrack);
					if( pTex->layerV()[k] ){
						TexMan(j,i).add_obsLoadV(NonFreeTrack);
						TexMan(j,i).add_layerObs(NonFreeTrack, k, true );
					} else {
						TexMan(j,i).add_obsLoadH(NonFreeTrack);
						TexMan(j,i).add_layerObs(NonFreeTrack, k, false);
					}
				}
		}
	//for(int j=0; j<TexMan.ynum(); j++){\
		for(int i=0; i<TexMan.xnum(); i++)\
			std::cout<< TexMan(j,i)<<" ";\
		std::cout<<"\n";\
	}
	//for(int j=0; j<TexMan.ynum(); j++)\
		print_member( std::cout, TexMan[j] );

	pTex->buildEdge();
}

template<typename T>
static void Lem_Snet2Obs_AddSnetWire( Tex_Man_t * pTex, Obs_Man_t * pMan, const T& vWIRE ){
	Lem_Iterate(vWIRE, pWire){
		Point_t p1(pWire->from().x(), pWire->from().y());
		Point_t p2(pWire->to  ().x(), pWire->to  ().y());
		int width = pWire->width();
		int span  = width>>1;
		int layer = pWire->z();
		Rect_t rect;
		if( p1.x==p2.x )
			rect = Rect_t( Point_t( p1.x-span, p1.y ), Point_t( p2.x+span, p2.y ) );
		else
			rect = Rect_t( Point_t( p1.x, p1.y-span ), Point_t( p2.x, p2.y+span ) );
//		Rect_t sprect = Rect2SpacingBox( pTex, layer, rect);
		pMan->addObsLoad(layer, rect);
	}
}

void Lem_Snet2Obs( Tex_Man_t * pTex, Obs_Man_t * pMan, const ispd19::SNet& snet ){
	Lem_Snet2Obs_AddSnetWire( pTex, pMan, snet.stripes() );
	Lem_Snet2Obs_AddSnetWire( pTex, pMan, snet.rings  () );
}

void Lem_Pin2Obs( Tex_Man_t * pTex, Obs_Man_t * pMan, const ispd19::Pin& pin ){
	if( ! pin.hasCoordinate() ){
		char buff[1024];
		sprintf( buff, "Pin \'%s\' is not one of placed/fixed/cover. Skip obstruction.", pin.name().c_str() );
		lem::warning.add(buff);
		return ;
	}
	Rect_t FrameRect = Lem_Box2Rect(pin.box() );
	FrameRect        = Lem_ApplyEfOrien( FrameRect, pin.ori() );
	Point_t PlacedPosition = Point_t( pin.x(), pin.y() );

	Lem_Iterate(pin.shapes(), pShape){
		Rect_t rect = Lem_Box2Rect(pShape->box());
		rect = Lem_ApplyEfOrien( rect, pin.ori() );
		rect = Lem_Offset( rect, PlacedPosition );
		int layer = pShape->z();
//		Rect_t rect = Rect2SpacingBox( pTex, layer, rect);
		pMan->addObsLoad(layer, rect);
	}
}

void Lem_PinPrefillTcl( const Tcl_t& Tcl, Tex_Man_t * pTex ){
	for(int i=0; i<Tcl.size(); i++){
		//if( Tcl[i].is_nonstd() )\
			continue;
		if( 1<Tcl[i].vLayer().size() || Tcl[i].vLayer().empty() )
			continue;
		if( 0!=Tcl[i].vLayer().front() )
			continue;
		int index = pTex->Pos2Index( Tex_Man_t::Pos_t(Tcl[i].gposX(), Tcl[i].gposY()) );
		if( !lem::dev() ){
			if( index < 0 || pTex->data().size() <= index )
				continue;
		}
		assert(0<=index && index < pTex->data().size() );
		pTex->getTex(index).add_pinPrefill(1);
	}
}

void Lem_PinPrefill( Lem_Gdb_t * pGdb, Tex_Man_t * pTex ){
	Lem_Iterate(pGdb->vTcl, pTcl)
		Lem_PinPrefillTcl(*pTcl, pTex);
}

void Lem_StartObsFromEf( Lem_Gdb_t * pGdb, Tex_Man_t * pTex ){
	ispd19::Database * pDb = pGdb->efdb();
	Obs_Man_t LefObs( pGdb, pTex );

	Tex_Man_t& TexMan = *pTex;
	TexMan.resize( TexMan.ynum(), TexMan.xnum() );

	// approximate obstruction for macros
	int nMacros = 0;
	Lem_Iterate( pDb->macros(), pMacro )
		LefObs.setMacroObs( nMacros++, * pMacro );

	// obstruction of instances
	Lem_Iterate( pDb->instances(), pInst )
		Lem_Inst2Obs( pTex, &LefObs, *pInst );
	
	// special nets
	Lem_Iterate( pDb->snets(), pSnet )
		Lem_Snet2Obs( pTex, &LefObs, *pSnet );

	// obstruction of pins
	Lem_Iterate( pDb->pins(), pPin )
		Lem_Pin2Obs(pTex, &LefObs, *pPin );

	Lem_Man2TexLoad( &LefObs, pGdb, pTex );
	Lem_PinPrefill(pGdb, pTex);
	//Lem_PlotInst(pGdb);
}
