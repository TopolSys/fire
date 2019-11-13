#ifndef _IO_IO_H_
#define _IO_IO_H_

#include "../global.h"
#include "../db/db.h"
#include "../lefdef/lef/include/lefiUtil.hpp"
#include "../lefdef/lef/include/lefrReader.hpp"
#include "../lefdef/def/include/defiUtil.hpp"
#include "../lefdef/def/include/defrReader.hpp"

namespace ispd19 {

class IO
{
public:
    IO(){ }
    static bool readLef( const std::string &lef );
    static bool readDef( const std::string &def );
    static bool readRg( const std::string &rg );
private:
    //helper
    static bool isFlipX (int orient);
    static bool isFlipY (int orient);
    static bool isRotate(int orient);
    //LEF
    static int readLefUnits          ( lefrCallbackType_e c, lefiUnits *unit,     lefiUserData ud );
    static int readLefLayer          ( lefrCallbackType_e c, lefiLayer *layer,    lefiUserData ud );
    static int readLefVia            ( lefrCallbackType_e c, lefiVia *via,        lefiUserData ud );
    static int readLefMacroBegin     ( lefrCallbackType_e c, const char *name,    lefiUserData ud );
    static int readLefMacro          ( lefrCallbackType_e c, lefiMacro *lefMacro, lefiUserData ud );
    static int readLefPin            ( lefrCallbackType_e c, lefiPin *lefPin,     lefiUserData ud );
    static int readLefMacroObs       ( lefrCallbackType_e c, lefiObstruction *Obs,lefiUserData ud );
    static int readLefViaRule        ( lefrCallbackType_e c, lefiViaRule *lefVR,  lefiUserData ud );

    //DEF
    static int readDefUnits          ( defrCallbackType_e c, double d,             defiUserData ud );
    static int readDefDesign         ( defrCallbackType_e c, const char *name,     defiUserData ud );
    static int readDefProperty       ( defrCallbackType_e c, defiProp *prop,       defiUserData ud );
    static int readDefDieArea        ( defrCallbackType_e c, defiBox *box,         defiUserData ud );
    static int readDefRow            ( defrCallbackType_e c, defiRow *row,         defiUserData ud );
    static int readDefTrack          ( defrCallbackType_e c, defiTrack *track,     defiUserData ud );
    static int readDefGcellGrid      ( defrCallbackType_e c, defiGcellGrid *gcell, defiUserData ud );
    static int readDefComponentStart ( defrCallbackType_e c, int numObj,           defiUserData ud );
    static int readDefComponent      ( defrCallbackType_e c, defiComponent *comp,  defiUserData ud );
    static int readDefPinStart       ( defrCallbackType_e c, int numObj,           defiUserData ud );
    static int readDefPin            ( defrCallbackType_e c, defiPin *pin,         defiUserData ud );
    static int readDefNetStart       ( defrCallbackType_e c, int numObj,           defiUserData ud );
    static int readDefNet            ( defrCallbackType_e c, defiNet *net,         defiUserData ud );
    static int readDefSNet           ( defrCallbackType_e c, defiNet *net,         defiUserData ud );
    static int readDefVia            ( defrCallbackType_e c, defiVia *via,         defiUserData ud );

    static bool readLineTokens(std::istream &is, const std::vector<char> &symbols, std::vector<std::string> &tokens, bool ignoreEmptyLine = true);
};

}

#endif

