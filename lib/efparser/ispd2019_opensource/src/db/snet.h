#ifndef _DB_SNET_H_
#define _DB_SNET_H_

#include "db.h"

namespace ispd19 {

class SNetStrip
{
public:
    SNetStrip( const Point &p1, const Point &p2, unsigned char z, int w ) :
        _fm    ( p1 ),
        _to    ( p2 ),
        _z     ( z  ),
        _width ( w  ) { }
    const Point& from() const { return _fm; }
    const Point& to() const { return _to; }
    int z() const { return _z; }
    int width() const { return _width; }
private:
    Point _fm;
    Point _to;
    unsigned char _z;
    int _width;
};

class SNetRing
{
public:
    SNetRing( const Point &p1, const Point &p2, unsigned char z, int w ) :
        _fm    ( p1 ),
        _to    ( p2 ),
        _z     ( z  ),
        _width ( w  ) { }
    const Point& from() const { return _fm; }
    const Point& to() const { return _to; }
    int z() const { return _z; }
    int width() const { return _width; }
private:
    Point _fm;
    Point _to;
    unsigned char _z;
    int _width;
};

class SVia
{
public:
    SVia() { }
    SVia( const std::string &n ) :
        _name(n),
        _topLayer(RouteLayer::NullIndex),
        _cutLayer(RouteLayer::NullIndex),
        _botLayer(RouteLayer::NullIndex) { }
    unsigned char topLayer() const { return _topLayer; }
    unsigned char cutLayer() const { return _cutLayer; }
    unsigned char botLayer() const { return _botLayer; }
    const std::vector<Box>& topShapes() const { return _topShapes; }
    const std::vector<Box>& cutShapes() const { return _cutShapes; }
    const std::vector<Box>& botShapes() const { return _botShapes; }
    void topLayer( unsigned char layer ) { _topLayer = layer; }
    void botLayer( unsigned char layer ) { _botLayer = layer; }
    void cutLayer( unsigned char layer ) { _cutLayer = layer; }
    void addTopShape(int lx, int ly, int hx, int hy) {
        _topShapes.emplace_back(lx, ly, hx, hy);
    }
    void addCutShape(int lx, int ly, int hx, int hy) {
        _cutShapes.emplace_back(lx, ly, hx, hy);
    }
    void addBotShape(int lx, int ly, int hx, int hy){
        _botShapes.emplace_back(lx, ly, hx, hy);
    }

    const std::string& viaRule() const { return _viaRule; }
    void viaRule( const std::string &rule ) { _viaRule = rule; }

    int cutSizeX() const { return _cutSizeX; }
    int cutSizeY() const { return _cutSizeY; }
    int cutSpacingX() const { return _cutSpacingX; }
    int cutSpacingY() const { return _cutSpacingY; }
    int enclosureLX() const { return _enclosureLX; }
    int enclosureLY() const { return _enclosureLY; }
    int enclosureHX() const { return _enclosureHX; }
    int enclosureHY() const { return _enclosureHY; }
    int numCutRows() const { return _numCutRows; }
    int numCutCols() const { return _numCutCols; }
    int originX() const { return _originX; }
    int originY() const { return _originY; }
    int offsetLX() const { return _offsetLX; }
    int offsetLY() const { return _offsetLY; }
    int offsetHX() const { return _offsetHX; }
    int offsetHY() const { return _offsetHY; }

    void cutSize( int x, int y ) {
        _cutSizeX = x;
        _cutSizeY = y;
    }
    void cutSpacing( int x, int y ) {
        _cutSpacingX = x;
        _cutSpacingY = y;
    }
    void enclosure( int lx, int ly, int hx, int hy ) {
        _enclosureLX = lx;
        _enclosureLY = ly;
        _enclosureHX = hx;
        _enclosureHY = hy;
    }
    void numCutRows( int num ) {
        _numCutRows = num;
    }
    void numCutCols( int num ) {
        _numCutCols = num;
    }
    void origin( int x, int y ) {
        _originX = x;
        _originY = y;
    }
    void offset( int lx, int ly, int hx, int hy ) {
        _offsetLX = lx;
        _offsetLY = ly;
        _offsetHX = hx;
        _offsetHY = hy;
    }

private:
    std::string _name;
    unsigned char _topLayer;
    unsigned char _cutLayer;
    unsigned char _botLayer;
    std::vector<Box> _topShapes;
    std::vector<Box> _cutShapes;
    std::vector<Box> _botShapes;

    std::string _viaRule;
    int _cutSizeX, _cutSizeY;
    int _cutSpacingX, _cutSpacingY;
    int _enclosureLX, _enclosureLY;
    int _enclosureHX, _enclosureHY;
    int _numCutRows, _numCutCols;
    int _originX, _originY;
    int _offsetLX, _offsetLY;
    int _offsetHX, _offsetHY;
};

class SNet
{
public:
    static const unsigned NullIndex = UINT_MAX;
    SNet() { }
    SNet( const std::string &n ) : _name( n ) { }
    const std::string& name() const { return _name; }

    void use(const std::string &u) { _use = u; }
    const std::string& use() const { return _use; }

    void addStripe(const Point &fm, const Point &to, unsigned char z, int width) {
        _stripes.emplace_back(fm, to, z, width);
        //std::cout<<"S : "<<fm.toString()<<", "<<to.toString()<<" ["<<(int)z<<"]"<<width<<std::endl;
    }
    void addRing( const Point &fm, const Point &to, unsigned char z, int width) {
        _rings.emplace_back(fm, to, z, width);
        //std::cout<<"R : "<<fm.toString()<<", "<<to.toString()<<" ["<<(int)z<<"]"<<width<<std::endl;
    }

    void report() const {
        std::cout<<"#strip wires "<<_stripes.size()<<std::endl;
        std::cout<<"#ring wires  "<<_rings.size()<<std::endl;
    }

    const std::vector<SNetStrip>& stripes() const { return _stripes;};
    const std::vector<SNetRing> & rings  () const { return _rings  ;};
    const std::vector<SVia>     & vias   () const { return _vias   ;};
private:
    std::string _name;
    std::string _use;
    std::vector<SNetStrip> _stripes;
    std::vector<SNetRing> _rings;
    std::vector<SVia> _vias;
};

}

#endif
