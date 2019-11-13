#include <thread>
#include <chrono>

#include "global.h"
#include "db/db.h"
#include "io/io.h"
#include "setting.h"

using namespace ispd19;

int main(int argc, char **argv)
{
    if( !Setting::readArgs(argc, argv) )
        return 1;

    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    
    //check required files
    if( Setting::techLef().empty() )
        errors.push_back("no LEF file is specified.");
    if( Setting::designDef().empty() )
        errors.push_back("no DEF file is specified.");
    if( Setting::guideRg().empty() )
        warnings.push_back("no route guide is specified.");

    //loading
    if( errors.empty() && !IO::readLef(Setting::techLef()) )
        errors.push_back("load LEF file fail.");
    if( errors.empty() && !IO::readDef(Setting::designDef()) )
        errors.push_back("load DEF file fail.");
    if( errors.empty() && !Setting::guideRg().empty() && !IO::readRg(Setting::guideRg()) )
        warnings.push_back("load guide file fail.");

    //Database::get()->reportNets();
    //Database::get()->reportMacros();
    //Database::get()->reportLayers();

    for( const std::string &error : errors ) {
        std::cout<<"[ERROR] "<<error<<std::endl;
    }
    for( const std::string &warning : warnings ) {
        std::cout<<"[WARNING] "<<warning<<std::endl;
    }
    for( const std::string &error : Database::get()->errors ) {
        std::cout<<"[ERROR] "<<error<<std::endl;
    }
    for( const std::string &warning : Database::get()->warnings ) {
        std::cout<<"[WARNING] "<<warning<<std::endl;
    }

    return 0;
}


