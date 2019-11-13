#include "io.h"

using namespace ispd19;

// 0:N
// 2:S
// 4:FN
// 6:FS
bool IO::isFlipX(int orient)
{
    return ( orient == 2 || orient == 4 );
}

bool IO::isFlipY(int orient)
{
    return ( orient == 2 || orient == 6 );
}

bool IO::isRotate(int orient){
    return ( orient == 1 || orient == 3 || orient == 5 || orient == 7 );
}

bool IO::readLineTokens(std::istream &is, const std::vector<char> &symbols, std::vector<std::string> &tokens, bool ignoreEmptyLine)
{
    const unsigned TokenLenLimit = 1000;
    tokens.clear();
    std::string line;
    while( is && tokens.empty() ) {
        getline(is, line);
        char token[TokenLenLimit + 1] = {0};
        unsigned lineLen = line.size();
        unsigned tokenLen = 0;
        for( unsigned i = 0; i < lineLen; ++i ) {
            char c = line[i];
            if( c == '#' )
                break;
            bool isSymbol = symbols[(int)c];
            if( !isSymbol )
                token[tokenLen++] = c;

            if( isSymbol || tokenLen >= TokenLenLimit || i == lineLen - 1 ) {
                if( tokenLen > 0 ) {
                    token[tokenLen] = (char)0;
                    tokens.push_back(std::string(token));
                    token[0] = (char)0;
                    tokenLen = 0;
                }
            }
        }
        if( !ignoreEmptyLine )
            return true;
    }
    return !tokens.empty();
}

