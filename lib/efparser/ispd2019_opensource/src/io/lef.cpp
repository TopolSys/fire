#include "lef.h"
#include "io.h"
#include <cmath>
#include <cassert>

using namespace ispd19;

bool IO::readLef(const std::string &lef)
{
    FILE *fp = 0;
    if( !( fp = fopen(lef.c_str(), "r") ) )
    {
        std::cerr<<"Unable to open LEF file : "<<lef<<std::endl;
        return false;
    }
    //std::cout<<"reading LEF file: "<<lef<<std::endl;

    lefrSetUnitsCbk(        IO::readLefUnits        );
    lefrSetLayerCbk(        IO::readLefLayer        );
    lefrSetViaCbk(          IO::readLefVia          );
    lefrSetMacroBeginCbk(   IO::readLefMacroBegin   );
    lefrSetMacroCbk(        IO::readLefMacro        );
    lefrSetObstructionCbk(  IO::readLefMacroObs     );
    lefrSetPinCbk(          IO::readLefPin          );
    lefrSetViaRuleCbk(      IO::readLefViaRule      );

    lefrInit();
    lefrReset();

    int result = lefrRead(fp, lef.c_str(), (void*)Database::get());

    lefrReleaseNResetMemory();
    lefrUnsetCallbacks();

    fclose(fp);

    if( result )
    {
        std::cerr<<"Error in parsing LEF file (error: "<<result<<")"<<std::endl;
        return false;
    }
    return true;
}

int IO::readLefUnits( lefrCallbackType_e c, lefiUnits *unit, lefiUserData ud )
{
    Database *db = (Database*) ud;
    if( unit->lefiUnits::hasDatabase() && std::strcmp(unit->lefiUnits::databaseName(), "MICRONS") == 0 )
    {
        db->lefDbuPerMicron(unit->lefiUnits::databaseNumber());
    }
    if( unit->lefiUnits::hasCapacitance() )
    {
        //ignore capacitance
    }
    return 0;
}

int IO::readLefLayer( lefrCallbackType_e c, lefiLayer *lefLayer, lefiUserData ud )
{
    Database *db = (Database*) ud;
    double scale = (double)db->lefDbuPerMicron();
    if( !strcmp( lefLayer->lefiLayer::type(), "ROUTING" ) )
    {
        RouteLayer &layer = db->addRouteLayer( std::string( lefLayer->lefiLayer::name() ) );
        //DIRECTION
        if( lefLayer->lefiLayer::hasDirection() )
        {
            if( !strcmp( lefLayer->lefiLayer::direction(), "HORIZONTAL" ) )
                layer.setH();
            else if( !strcmp( lefLayer->lefiLayer::direction(), "VERTICAL" ) )
                layer.setV();
        }
        //PITCH
        if( lefLayer->lefiLayer::hasXYPitch() )
        {
            layer.pitchX( (int)std::round(lefLayer->lefiLayer::pitchX() * scale) );
            layer.pitchY( (int)std::round(lefLayer->lefiLayer::pitchY() * scale) );
        }
        else if( lefLayer->lefiLayer::hasPitch() )
        {
            layer.pitchX( (int)std::round(lefLayer->lefiLayer::pitch() * scale) );
            layer.pitchY( (int)std::round(lefLayer->lefiLayer::pitch() * scale) );
        }
        //WIDTH
        if( lefLayer->lefiLayer::hasWidth() )
        {
            layer.width( (int)std::round(lefLayer->lefiLayer::width() * scale) );
        }
        //SPACING
        if( lefLayer->lefiLayer::hasSpacingNumber() )
        {
            layer.spacing( (int)std::round(lefLayer->lefiLayer::spacing(0) * scale) );
        }
        //SPACINGTABLE
        if( lefLayer->lefiLayer::numSpacingTable() > 0 )
        {
            for( unsigned tableIdx = 0; tableIdx < (unsigned)lefLayer->lefiLayer::numSpacingTable(); ++tableIdx )
            {
                lefiSpacingTable *spTable = lefLayer->lefiLayer::spacingTable(tableIdx);
                if( spTable->lefiSpacingTable::isParallel() )
                {
                    lefiParallel *lefPRLTable = spTable->lefiSpacingTable::parallel();
                    unsigned numLengths = lefPRLTable->lefiParallel::numLength();
                    unsigned numWidths = lefPRLTable->lefiParallel::numWidth();
                    PRLTable &prlTable = layer.addPRLTable(numWidths, numLengths);
                    
                    for( unsigned lIdx = 0; lIdx < numLengths; ++lIdx ){
                        int length  = scale * lefPRLTable->lefiParallel::length(lIdx);
                        prlTable.setLength(lIdx,length);
                    }
                    for( unsigned wIdx = 0; wIdx < numWidths; ++wIdx )
                    {
                        int width  = scale * lefPRLTable->lefiParallel::width(wIdx);
                        for( unsigned lIdx = 0; lIdx < numLengths; ++lIdx )
                        {
                            int spacing= scale * lefPRLTable->lefiParallel::widthSpacing(wIdx, lIdx);
                            prlTable.setSpacing(wIdx, lIdx, width, spacing );
                        }
                    }
                }
                else if( spTable->lefiSpacingTable::isInfluence() )
                {
                    //INFLUENCE not implemented
                }
                else
                {
                    //TWOWIDTHS not implemented
                }
            }

        }
        //AREA
        if( lefLayer->hasArea() )
        {
            layer.area( (int)std::round(lefLayer->lefiLayer::area() * scale * scale ) );
        }

    }
    else if( !strcmp( lefLayer->lefiLayer::type(), "CUT" ) )
    {
        CutLayer &layer = db->addCutLayer( std::string( lefLayer->lefiLayer::name() ) );
        if( lefLayer->lefiLayer::hasSpacingNumber() )
        {
            for( int i = 0; i < lefLayer->lefiLayer::numSpacing(); ++i ) {
                layer.spacing( (int)std::round(lefLayer->lefiLayer::spacing(i) * scale) );
            }
        }
    }
    return 0;
}

int IO::readLefVia( lefrCallbackType_e c, lefiVia *lefVia, lefiUserData ud )
{
    Database *db = (Database*) ud;
    double scale = (double)db->lefDbuPerMicron();
    Via &via = db->addVia( std::string(lefVia->lefiVia::name()) );
    unsigned char layerId[3] = { UCHAR_MAX, UCHAR_MAX, UCHAR_MAX };
    int layerIdx[3] = { INT_MAX, INT_MAX, INT_MAX };
    if( lefVia->lefiVia::numLayers() != 3 ) {
        std::cerr<<"via pillar not supported"<<std::endl;
        return 0;
    }
    for( int i = 0; i < lefVia->lefiVia::numLayers(); ++i )
    {
        auto layer = Database::layer(std::string(lefVia->lefiVia::layerName(i)));
        if( layer.first == 'c' ) {
            layerIdx[1] = i;
            layerId[1] = layer.second;
        } else if( layer.first == 'r' ) {
            if( layerIdx[0] == INT_MAX ) {
                layerIdx[0] = i;
                layerId[0] = layer.second;
            } else {
                layerIdx[2] = i;
                layerId[2] = layer.second;
            }
        } else {
            std::cerr<<"unknown layer : "<<lefVia->lefiVia::layerName(i)<<std::endl;
        }
    }
    if( layerId[0] > layerId[2] ) {
        std::swap(layerId[0], layerId[2]);
        std::swap(layerIdx[0], layerIdx[2]);
    }
    via.botLayer(layerId[0]);
    via.cutLayer(layerId[1]);
    via.topLayer(layerId[2]);
    for( int i = 0; i < 3; ++i ) {
        for( int rectIdx = 0; rectIdx < lefVia->lefiVia::numRects(i); ++rectIdx )
        {
            int lx = (int)std::round(lefVia->lefiVia::xl(i, rectIdx) * scale);
            int ly = (int)std::round(lefVia->lefiVia::yl(i, rectIdx) * scale);
            int hx = (int)std::round(lefVia->lefiVia::xh(i, rectIdx) * scale);
            int hy = (int)std::round(lefVia->lefiVia::yh(i, rectIdx) * scale);
            if( i == 0 ) {
                via.addBotShape(lx, ly, hx, hy);
            } else if( i == 1 ) {
                via.addCutShape(lx, ly, hx, hy);
            } else if( i == 2 ) {
                via.addTopShape(lx, ly, hx, hy);
            }

        }
    }
    return 0;
}

int IO::readLefMacroBegin( lefrCallbackType_e c, const char *name, lefiUserData ud )
{
    Database *db = (Database*) ud;
    db->addMacro( std::string( name ) );
    return 0;
}

int IO::readLefMacro( lefrCallbackType_e c, lefiMacro *lefMacro, lefiUserData ud )
{
    Database *db = (Database*) ud;
    double scale = (double)db->lefDbuPerMicron();
    Macro &macro = db->getLastMacro();

    if( lefMacro->lefiMacro::hasClass() ) {
        macro.cls( lefMacro->lefiMacro::macroClass() );
    }
    if( lefMacro->lefiMacro::hasSiteName() ) {
        macro.site( lefMacro->lefiMacro::siteName() );
    }
    if( lefMacro->lefiMacro::hasForeign() ) {
        for( int i = 0; i < lefMacro->lefiMacro::numForeigns(); ++i ) {
            //std::cout<<lefMacro->lefiMacro::foreignName(i);
            if( lefMacro->lefiMacro::hasForeignPoint(i) ) {
                //std::cout<<lefMacro->lefiMacro::foreignX(i);
                //std::cout<<lefMacro->lefiMacro::foreignY(i);
            }
        }
    }
    int origX = 0;
    int origY = 0;
    int sizeX = 0;
    int sizeY = 0;
    if( lefMacro->lefiMacro::hasOrigin() ) {
        origX = std::round( lefMacro->lefiMacro::originX() * scale );
        origY = std::round( lefMacro->lefiMacro::originY() * scale );
    }
    if( lefMacro->lefiMacro::hasSize() ) {
        sizeX = std::round( lefMacro->lefiMacro::sizeX() * scale );
        sizeY = std::round( lefMacro->lefiMacro::sizeY() * scale );
    }
    macro.size(origX, origY, sizeX, sizeY);

    return 0;
}

int IO::readLefMacroObs( lefrCallbackType_e c, lefiObstruction *lefObs, lefiUserData ud ){
    Database *db = (Database*) ud;
    char buff[1024];
    int nLayerError = 0;
    char layer_type = 'u'; //undefined
    int layer_id = RouteLayer::NullIndex;
    double scale = (double)db->lefDbuPerMicron();
    lefiGeometries *geom = lefObs->geometries();
    Macro& macro = db->getLastMacro();
    Obs& obs = macro.obs();

    for(int i=0; i<geom->numItems(); i++){
        if( geom->itemType(i) == lefiGeomLayerE ){
            layer_id = db->rLayer( geom->getLayer(i) );
            if( RouteLayer::NullIndex == layer_id ){
                if( 0==strcmp(geom->getLayer(i),"OVERLAP") ){
                    db->warnings.push_back("OBS skip \'OVERLAP\' layer");
                    layer_type = 'o';
                    continue;
                } else
                if( CutLayer::NullIndex != db->cLayer( geom->getLayer(i) ) ){
                    layer_type = 'v';
                    sprintf(buff, "OBS skip cut layer \'%s\' ", geom->getLayer(i) );
                    db->warnings.push_back(buff);
                    continue;
                }
                layer_type = 'u';
                sprintf(buff, "unknown layer \'%s\'", geom->getLayer(i) );
                db->errors.push_back(buff);
            }
            layer_type = 'r';
        } else 
        if( geom->itemType(i) == lefiGeomRectE ){
            if( 'u'==layer_type )
                nLayerError ++;
            
            if( RouteLayer::NullIndex == layer_id ){
                continue;
            }
            lefiGeomRect *rect = geom->lefiGeometries::getRect(i);
            int lx = (int) std::round( rect->xl * scale );
            int ly = (int) std::round( rect->yl * scale );
            int hx = (int) std::round( rect->xh * scale );
            int hy = (int) std::round( rect->yh * scale );
            obs.addShape(lx, ly, hx, hy, layer_id);
        } else {
            sprintf(buff, "Exception geo-type %d", geom->itemType(i) );
            db->errors.push_back(buff);
        }
    }
    if( 0 != nLayerError ){
        sprintf(buff, "%d obstructions cannot find target layer", nLayerError );
        db->errors.push_back(buff);
    }
    return 0;
}

int IO::readLefPin(lefrCallbackType_e c, lefiPin *lefPin, lefiUserData ud)
{
    Database *db = (Database*) ud;
    double scale = (double)db->lefDbuPerMicron();
    Macro &macro = db->getLastMacro();

    Pin &pin = macro.addPin( std::string( lefPin->lefiPin::name() ) );

    if( lefPin->lefiPin::hasUse() ) {
        std::string use( lefPin->lefiPin::use() );
        if( use == "ANALOG" ) pin.setIsUseAnalog();
        else if( use == "CLOCK" ) pin.setIsUseClock();
        else if( use == "GROUND" ) pin.setIsUseGround();
        else if( use == "POWER" ) pin.setIsUsePower();
        else if( use == "SIGNAL" ) pin.setIsUseSignal();
    }
    if( lefPin->lefiPin::hasDirection() ) {
        std::string dir( lefPin->lefiPin::direction() );
        if( dir == "OUTPUT" ) pin.setIsOutput();
        else if( dir == "INPUT" ) pin.setIsInput();
        else if( dir == "INOUT" ) pin.setIsInOut();
    }
    for( int port = 0; port < lefPin->lefiPin::numPorts(); ++port )
    {
        unsigned char layerId = RouteLayer::NullIndex;
        lefiGeometries *geom = lefPin->lefiPin::port(port);
        for( int i = 0; i < geom->lefiGeometries::numItems(); ++i )
        {
            if( geom->lefiGeometries::itemType(i) == lefiGeomLayerE ) {
                auto layerInfo = Database::layer( std::string( geom->lefiGeometries::getLayer(i) ) );
                if( layerInfo.first != 'r' )
                    continue;

                layerId = layerInfo.second;
            } else if( geom->lefiGeometries::itemType(i) == lefiGeomRectE ) {
                lefiGeomRect *rect = geom->lefiGeometries::getRect(i);
                int lx = (int) std::round( rect->xl * scale );
                int ly = (int) std::round( rect->yl * scale );
                int hx = (int) std::round( rect->xh * scale );
                int hy = (int) std::round( rect->yh * scale );
                pin.addShape(lx, ly, hx, hy, layerId);
            }
        }
    }
    return 0;
}

int IO::readLefViaRule( lefrCallbackType_e c, lefiViaRule *lefVR,  lefiUserData ud )
{
    Database *db = (Database*) ud;
    double scale = (double)db->lefDbuPerMicron();
    ViaRule &vr = db->addViaRule( std::string(lefVR->lefiViaRule::name()) );
    vr.isGenerate( lefVR->lefiViaRule::hasGenerate() );
    vr.isDefault( lefVR->lefiViaRule::hasDefault() );
    for( int i = 0; i < lefVR->lefiViaRule::numLayers(); ++i )
    {
        ViaRuleLayer *vrl = 0;
        if( i == 0 ) {
            vrl = &(vr.botLayerRules());
        } else if( i == 1 ) {
            vrl = &(vr.cutLayerRules());
        } else if( i == 2 ) {
            vrl = &(vr.topLayerRules());
        }

        if( !vrl ) {
            continue;
        }

        lefiViaRuleLayer *vrLayer = lefVR->lefiViaRule::layer(i);
        auto layer = Database::layer( std::string( vrLayer->lefiViaRuleLayer::name() ) );

        vrl->layer( layer.second );

        if( vrLayer->lefiViaRuleLayer::hasEnclosure() ) {
            int oh1 = (int) std::round( vrLayer->lefiViaRuleLayer::enclosureOverhang1() * scale );
            int oh2 = (int) std::round( vrLayer->lefiViaRuleLayer::enclosureOverhang2() * scale );
            vrl->enclosureOverhang(oh1, oh2);
        }
        if( vrLayer->lefiViaRuleLayer::hasRect() ) {
            int lx = (int) std::round( vrLayer->lefiViaRuleLayer::xl() * scale );
            int ly = (int) std::round( vrLayer->lefiViaRuleLayer::yl() * scale );
            int hx = (int) std::round( vrLayer->lefiViaRuleLayer::xh() * scale );
            int hy = (int) std::round( vrLayer->lefiViaRuleLayer::yh() * scale );
            vrl->rect(lx, ly, hx, hy);
        }
        if( vrLayer->lefiViaRuleLayer::hasSpacing() ) {
            int sx = (int) std::round( vrLayer->lefiViaRuleLayer::spacingStepX() * scale );
            int sy = (int) std::round( vrLayer->lefiViaRuleLayer::spacingStepY() * scale );
            vrl->spacing(sx, sy);
        }
    }

    return 0;
}
