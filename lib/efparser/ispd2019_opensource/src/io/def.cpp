#include "def.h"

#include "io.h"

using namespace ispd19;

bool IO::readDef(const std::string &def)
{
    FILE *fp = 0;
    if ( !(fp = fopen(def.c_str(), "r") ) )
    {
        std::cerr<<"Unable to open DEF file: "<<def<<std::endl;
        return false;
    }

    defrSetAddPathToNet();

    defrSetDesignCbk         ( IO::readDefDesign );
    defrSetUnitsCbk          ( IO::readDefUnits  );
    defrSetPropCbk           ( IO::readDefProperty );
    defrSetDieAreaCbk        ( IO::readDefDieArea );
    defrSetRowCbk            ( IO::readDefRow );
    defrSetTrackCbk          ( IO::readDefTrack );
    defrSetGcellGridCbk      ( IO::readDefGcellGrid );
    defrSetComponentStartCbk ( IO::readDefComponentStart );
    defrSetComponentCbk      ( IO::readDefComponent );
    defrSetStartPinsCbk      ( IO::readDefPinStart );
    defrSetPinCbk            ( IO::readDefPin );
    defrSetNetStartCbk       ( IO::readDefNetStart );
    defrSetNetCbk            ( IO::readDefNet );
    defrSetSNetCbk           ( IO::readDefSNet );
    defrSetViaCbk            ( IO::readDefVia );

    defrInit();
    defrReset();

    int result = defrRead(fp, def.c_str(), (void*)Database::get(), 1);

    defrReleaseNResetMemory();
    defrUnsetCallbacks();

    fclose(fp);

    if ( result )
    {
        std::cerr<<"Error in parsing DEF file (error: "<<result<<")"<<std::endl;
        return false;
    }
    return true;
}

/***** DEF call backs *****/

//----- Design -----
int IO::readDefDesign(defrCallbackType_e c, const char *name, defiUserData ud)
{
    Database *db = (Database*) ud;
    db->designName(std::string(name));
    return 0;
}

//----- Unit -----
int IO::readDefUnits(defrCallbackType_e c, double d, defiUserData ud)
{
    Database *db = (Database*) ud;
    db->defDbuPerMicron(d);
    return 0;
}

//----- Property -----
int IO::readDefProperty(defrCallbackType_e c, defiProp *prop, defiUserData ud)
{
    /*
    std::string propType(prop->propType());
    char dataType = prop->dataType();
    std::string propName(prop->propName());

    if( propType == "design" )
    {
    }
    if( propType == "componentpin" )
    {
    }
    std::cout<<propType<<std::endl;
    std::cout<<dataType<<std::endl;
    std::cout<<propName<<std::endl;
    if( prop->hasNumber() )
    {
        std::cout<<prop->number()<<std::endl;
    }
    */
    return 0;
}

//----- Die Area -----
int IO::readDefDieArea(defrCallbackType_e c, defiBox *box, defiUserData ud)
{
    //Database *db = (Database*) ud;
    //std::cout<<"Die Area "<<box->xl()<<","<<box->yl()<<":"<<box->xh()<<","<<box->yh()<<std::endl;
    return 0;
}

//----- Row -----
int IO::readDefRow(defrCallbackType_e c, defiRow *defRow, defiUserData ud)
{
    Database *db = (Database*) ud;
    Row &row = db->addRow( std::string(defRow->name()) );
    row.x( defRow->x() );
    row.y( defRow->y() );
    row.numSites( defRow->xNum() );
    row.siteWidth( defRow->xStep() );
    row.isFlip( IO::isFlipY(defRow->orient()) );
    return 0;
}

//----- Track -----
int IO::readDefTrack(defrCallbackType_e c, defiTrack *defTrack, defiUserData ud)
{
    Database *db = (Database*) ud;
    char dir = 'X';
    if( !strcmp( defTrack->macro(), "Y" ) )
        dir = 'H';
    else if( !strcmp( defTrack->macro(), "X" ) )
        dir = 'V';
    int start = defTrack->x();
    int num = defTrack->xNum();
    int step = defTrack->xStep();
    for( int i = 0; i < defTrack->numLayers(); ++i )
    {
        unsigned char layerId = Database::rLayer(std::string(defTrack->layer(i)));
        db->getRouteLayer(layerId).setTrack(dir, start, num, step);
    }
    return 0;
}

//----- GCell -----
int IO::readDefGcellGrid(defrCallbackType_e c, defiGcellGrid *gcell, defiUserData ud)
{
    Database *db = (Database*) ud;
    int gridLoc = gcell->x();
    int gridDo = gcell->xNum();
    int gridStep = gcell->xStep();
    std::string dir(gcell->macro());
    if( dir == "X" ) {
        for( int loc = gridLoc, i = 0; i < gridDo; ++i, loc += gridStep ) {
            db->addGridLineX(loc);
        }
    }
    if( dir == "Y" ) {
        for( int loc = gridLoc, i = 0; i < gridDo; ++i, loc += gridStep ) {
            db->addGridLineY(loc);
        }
    }
    return 0;
}

//----- Pin -----

int IO::readDefPinStart(defrCallbackType_e c, int numObj, defiUserData ud)
{
    Database *db = (Database*) ud;
    db->allocatePin(numObj);
    return 0;
}
int IO::readDefPin(defrCallbackType_e c, defiPin *defPin, defiUserData ud)
{
    Database *db = (Database*) ud;
    Pin &pin = db->addPin( std::string( defPin->pinName() ) );
    if( defPin->defiPin::hasUse() ) {
        std::string use( defPin->defiPin::use() );
        if( use == "ANALOG" ) pin.setIsUseAnalog();
        else if( use == "CLOCK" ) pin.setIsUseClock();
        else if( use == "GROUND" ) pin.setIsUseGround();
        else if( use == "POWER" ) pin.setIsUsePower();
        else if( use == "SIGNAL" ) pin.setIsUseSignal();
    }
    if( defPin->defiPin::hasDirection() ){
        std::string dir( defPin->defiPin::direction() );
        if( dir == "OUTPUT" ) pin.setIsOutput();
        else if( dir == "INPUT" ) pin.setIsInput();
        else if( dir == "INOUT" ) pin.setIsInOut();
    }

    if( defPin->isPlaced() || defPin->isFixed() || defPin->isCover() ){
        int x = defPin->defiPin::placementX();
        int y = defPin->defiPin::placementY();
        pin.place(x, y, defPin->defiPin::orient());
        //fx = IO::isFlipX ( defPin->defiPin::orient() );
        //fy = IO::isFlipY ( defPin->defiPin::orient() );
        //rot= IO::isRotate( defPin->defiPin::orient() );
        //pin.place(x,y,fx,fy,rot);
    } else {
        pin.unplace();
    }

    for(int i=0; i<defPin->defiPin::numLayer(); i++){
        int xl, yl, xh, yh;
        int layer_i = db->rLayer(defPin->defiPin::layer(i));
        defPin->defiPin::bounds( i, &xl, &yl, &xh, &yh );
        pin.addShape(xl,yl,xh,yh,layer_i);
    }
    //pin.report();
    return 0;
}


//----- Component -----

int IO::readDefComponentStart(defrCallbackType_e c, int numObj, defiUserData ud)
{
    Database *db = (Database*) ud;
    db->allocateInstance(numObj);
    return 0;
}
int IO::readDefComponent(defrCallbackType_e c, defiComponent *comp, defiUserData ud)
{
    Database *db = (Database*) ud;
    Instance& inst = db->addInstance( std::string(comp->defiComponent::id()) );
    inst.macro(Database::macro(std::string(comp->defiComponent::name())));
    if( comp->defiComponent::isUnplaced() )
    {
        //unplaced
    }
    else if( comp->defiComponent::isPlaced() || comp->defiComponent::isFixed() || comp->defiComponent::isCover() )
    {
        int x = comp->defiComponent::placementX();
        int y = comp->defiComponent::placementY();
        inst.place(x, y, comp->defiComponent::placementOrient());
        //bool flipX = IO::isFlipX (comp->defiComponent::placementOrient());
        //bool flipY = IO::isFlipY (comp->defiComponent::placementOrient());
        //bool rotate= IO::isRotate(comp->defiComponent::placementOrient());
        //inst.place(x, y, flipX, flipY, rotate);
    }
    return 0;
}


//----- Net -----
int IO::readDefNetStart(defrCallbackType_e c, int numObj, defiUserData ud){
    Database *db = (Database*) ud;
    db->allocateNet(numObj);
    return 0;
}

int IO::readDefNet(defrCallbackType_e c, defiNet *defNet, defiUserData ud)
{
    Database *db = (Database*) ud;
    Net &dNet = db->addNet( std::string( defNet->defiNet::name() ) );

    for( int i = 0; i < defNet->defiNet::numConnections(); ++i )
    {
        std::string instName( defNet->defiNet::instance(i) );
        std::string pinName( defNet->defiNet::pin(i) );
        if( instName == "PIN" )
        {
            //this is a pin
            //std::cout<<"pin : "<<pinName<<std::endl;
            dNet.addPin(pinName);
        }
        else
        {
            //this is a cell
            //Instance &inst = db->getInstance(instName);
	        unsigned compId = Database::instance(instName);
            //std::cout<<"inst: "<<instName<<", pin : "<<pinName<<": check instance: "<< ( compId != Instance::NullIndex? db->getMacro(db->getInstance(compId).macro()).name(): "" ) <<std::endl;
            dNet.addPin(pinName,compId);
        }
    }
    for( int wireIdx = 0; wireIdx < defNet->defiNet::numWires(); ++wireIdx )
    {
        defiWire *wire = defNet->defiNet::wire(wireIdx);
        if( std::string(wire->wireType()) != "ROUTED" )
            continue;
        for( int pathIdx = 0; pathIdx < wire->numPaths(); ++pathIdx )
        {
            defiPath *path = wire->path(pathIdx);
            path->initTraverse();
            
            std::pair<char,unsigned char> layer;
            unsigned viaId = Via::NullIndex;
            Point p1 = Point::Null;
            Point p2 = Point::Null;
            Box rect;
            for( int pathObj = (int) path->next(); pathObj != DEFIPATH_DONE; pathObj = (int) path->next() )
            {
                int lx, ly, hx, hy;
                int x, y, z;
                switch( pathObj )
                {
                    case DEFIPATH_LAYER:
                        layer = Database::layer(std::string(path->getLayer()));
                        break;
                    case DEFIPATH_VIA:
                        viaId = Database::via(std::string(path->getVia()));
                        break;
                    case DEFIPATH_RECT:
                        path->getViaRect(&lx, &ly, &hx, &hy);
                        rect = Box(lx, ly, hx, hy);
                        break;
                    case DEFIPATH_POINT:
                        path->getPoint(&x, &y);
                        if( p1 == Point::Null ) {
                            p1.x(x);
                            p1.y(y);
                        } else {
                            p2.x(x);
                            p2.y(y);
                        }
                        break;
                    case DEFIPATH_FLUSHPOINT:
                        path->getFlushPoint(&x, &y, &z);
                        if( p1 == Point::Null ) {
                            p1.x(x);
                            p1.y(y);
                        } else {
                            p2.x(x);
                            p2.y(y);
                        }
                        break;
                    default:
                        break;
                }
            }
            if( layer.first == 'r' && layer.second != RouteLayer::NullIndex && p1 != Point::Null && p2 != Point::Null )
            {
                dNet.addWire(p1, p2, layer.second);
            }
            if( viaId != Via::NullIndex ) {
                auto& via = Database::get()->getVia(viaId);
                if( p2 != Point::Null ) {
                    dNet.addVia(p2, via.botLayer(), via.topLayer(), viaId);
                } else if( p1 != Point::Null ) {
                    dNet.addVia(p1, via.botLayer(), via.topLayer(), viaId);
                }
            }

        }
    }
    return 0;
}

//----- Special Net -----

int IO::readDefSNet(defrCallbackType_e c, defiNet *defNet, defiUserData ud)
{
    Database *db = (Database*) ud;
    SNet &dSNet = db->addSNet( std::string( defNet->defiNet::name() ) );
    dSNet.use( std::string(defNet->defiNet::use()) );
    for( int wireIdx = 0; wireIdx < defNet->defiNet::numWires(); ++wireIdx )
    {
        defiWire *wire = defNet->defiNet::wire(wireIdx);
        if( std::string(wire->wireType()) != "ROUTED" )
            continue;

        for( int pathIdx = 0; pathIdx < wire->numPaths(); ++pathIdx )
        {
            defiPath *path = wire->path(pathIdx);
            path->initTraverse();

            std::pair<char,unsigned char> layer;
            char shape = 'x';
            //unsigned viaId = Via::NullIndex;
            Point p1 = Point::Null;
            Point p2 = Point::Null;
            Box rect;
            int width = 0;
            for( int pathObj = (int) path->next(); pathObj != DEFIPATH_DONE; pathObj = (int) path->next() )
            {
                int x, y, z;
                switch( pathObj )
                {
                    case DEFIPATH_LAYER:
                        layer = Database::layer(std::string(path->getLayer()));
                        break;
                    case DEFIPATH_VIA:
                        break;
                    case DEFIPATH_RECT:
                        //std::cout<<"box"<<std::endl;
                        break;
                    case DEFIPATH_WIDTH:
                        width = path->getWidth();
                        break;
                    case DEFIPATH_POINT:
                        path->getPoint(&x, &y);
                        if( p1 == Point::Null ) {
                            p1.x(x);
                            p1.y(y);
                        } else {
                            p2.x(x);
                            p2.y(y);
                        }
                        break;
                    case DEFIPATH_FLUSHPOINT:
                        path->getFlushPoint(&x, &y, &z);
                        if( p1 == Point::Null ) {
                            p1.x(x);
                            p1.y(y);
                        } else {
                            p2.x(x);
                            p2.y(y);
                        }
                        break;
                    case DEFIPATH_SHAPE:
                        if( strcmp( path->getShape(), "STRIPE" ) == 0 ) {
                            shape = 's';
                        } else if( strcmp( path->getShape(), "RING" ) == 0 ) {
                            shape = 'r';
                        }
                        break;
                    default:
                        break;
                }
            }
            /*
               std::cout<<"------"<<std::endl;
               std::cout<<"layer type : "<<layer.first<<std::endl;
               std::cout<<"layer      : "<<(int)layer.second<<std::endl;
               std::cout<<"shape      : "<<shape<<std::endl;
               std::cout<<"p1         : "<<p1.toString()<<std::endl;
               std::cout<<"p2         : "<<p2.toString()<<std::endl;
               std::cout<<"via        : "<<viaId<<std::endl;
               std::cout<<"width      : "<<width<<std::endl;
            */
            if( layer.first == 'r' && layer.second != RouteLayer::NullIndex && p1 != Point::Null && p2 != Point::Null ) {
                if( shape == 's' ) {
                    dSNet.addStripe( p1, p2, layer.second, width );
                } else if( shape == 'r' ) {
                    dSNet.addRing( p1, p2, layer.second, width );
                }
            }
        }
    }
    //dSNet.report();
    return 0;
}

//----- Via -----

int IO::readDefVia(defrCallbackType_e c, defiVia *defVia, defiUserData ud )
{
    Database *db = (Database*) ud;
    //double scale = (double) db->defDbuPerMicron();

    SVia &svia = db->addSVia( std::string( defVia->defiVia::name() ) );
    int numRects = defVia->defiVia::numLayers();
    if( numRects > 0 ) {
        unsigned char botLayer = RouteLayer::NullIndex;
        unsigned char cutLayer = CutLayer::NullIndex;
        unsigned char topLayer = RouteLayer::NullIndex;
        for( int i = 0; i < numRects; ++i ) {
            char *name;
            int lx, ly, hx, hy;
            defVia->defiVia::layer(i, &name, &lx, &ly, &hx, &hy);
            auto layer = Database::layer( std::string(name) );
            if( layer.first == 'c' ) {
                if( cutLayer == CutLayer::NullIndex ) {
                    cutLayer = layer.second;
                }
            } else if( layer.first == 'r' ) {
                if( botLayer == RouteLayer::NullIndex ) {
                    botLayer = layer.second;
                } else if( topLayer == RouteLayer::NullIndex ) {
                    topLayer = layer.second;
                }
            }
        }
        if( botLayer > topLayer ) {
            std::swap( botLayer, topLayer );
        }
        svia.botLayer( botLayer );
        svia.cutLayer( cutLayer );
        svia.topLayer( topLayer );
        for( int i = 0; i < defVia->defiVia::numLayers(); ++i )
        {
            char *name;
            int lx, ly, hx, hy;
            defVia->defiVia::layer(i, &name, &lx, &ly, &hx, &hy);
            auto layer = Database::layer( std::string(name) );
            if( layer.first == 'c' && layer.second == cutLayer ) {
                svia.addCutShape( lx, ly, hx, hy );
            }
            if( layer.first == 'r' && layer.second == botLayer ) {
                svia.addBotShape( lx, ly, hx, hy );
            }
            if( layer.first == 'r' && layer.second == topLayer ) {
                svia.addTopShape( lx, ly, hx, hy );
            }
        }
    }
    if( defVia->defiVia::hasViaRule() )
    {
        char *ruleName;
        char *layerBot;
        char *layerCut;
        char *layerTop;
        int cutSizeX;
        int cutSizeY;
        int cutSpacingX;
        int cutSpacingY;
        int encLX;
        int encLY;
        int encHX;
        int encHY;
        int numCutRows;
        int numCutCols;
        int originX;
        int originY;
        int offsetLX;
        int offsetLY;
        int offsetHX;
        int offsetHY;
        defVia->defiVia::viaRule(
            &ruleName,                        // VIARULE
            &cutSizeX, &cutSizeY,             // CUTSIZE
            &layerBot, &layerCut, &layerTop,  // LAYERS
            &cutSpacingX, &cutSpacingY,       // CUTSPACING
            &encLX, &encLY, &encHX, &encHY ); // ENCLOSURE
        svia.viaRule( std::string(ruleName) );
        svia.cutSize( cutSizeX, cutSizeY );
        svia.botLayer( Database::layer( std::string( layerBot ) ).second );
        svia.cutLayer( Database::layer( std::string( layerCut ) ).second );
        svia.topLayer( Database::layer( std::string( layerTop ) ).second );
        svia.cutSpacing( cutSpacingX, cutSpacingY );
        svia.enclosure( encLX, encLY, encHX, encHY );

        if( defVia->defiVia::hasRowCol() ) {
            defVia->defiVia::rowCol( &numCutRows, &numCutCols );
            svia.numCutRows( numCutRows );
            svia.numCutCols( numCutCols );
        }
        if( defVia->defiVia::hasOrigin() ) {
            defVia->defiVia::origin( &originX, &originY );
            svia.origin( originX, originY );
        }
        if( defVia->defiVia::hasOffset() ) {
            defVia->defiVia::offset( &offsetLX, &offsetLY, &offsetHX, &offsetHY );
            svia.offset( offsetLX, offsetLY, offsetHX, offsetHY );
        }
    }

    return 0;
}


