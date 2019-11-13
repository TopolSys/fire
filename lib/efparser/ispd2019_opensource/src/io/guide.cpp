#include "io.h"

using namespace ispd19;

bool IO::readRg(const std::string &rg)
{
    std::ifstream fs(rg.c_str());
    if( !fs.good() ) {
        std::cerr<<"Unable to open guide file: "<<rg<<std::endl;
        return false;
    }
    
    std::vector<char> symbols(256, (char)0);
    
    symbols[(int)' '] = 2;
    symbols[(int)'\t'] = 2;
    symbols[(int)'\n'] = 2;
    symbols[(int)'\r'] = 2;
    symbols[(int)'#'] = 3;

    bool readingGuide = false;

    std::vector<std::string> tokens;
    unsigned netId = Net::NullIndex;
    while(readLineTokens(fs, symbols, tokens, true)) {
        if( tokens[0] == "(" ) {
            readingGuide = true;
        } else if( tokens[0] == ")" ) {
            readingGuide = false;
        } else if( readingGuide && tokens.size() == 5 ) {
            int lx = atoi(tokens[0].c_str());
            int ly = atoi(tokens[1].c_str());
            int hx = atoi(tokens[2].c_str());
            int hy = atoi(tokens[3].c_str());
            unsigned char layerId = Database::rLayer(tokens[4]);
            if( netId != Net::NullIndex ) {
                Net &net = Database::get()->getNet(netId);
                net.addGuide(lx, ly, hx, hy, layerId);
            }
        } else if( !readingGuide ) {
            netId = Database::net(tokens[0]);
        }
    }

    fs.close();
    Database::get()->hasGuides(true);
    return true;
}

