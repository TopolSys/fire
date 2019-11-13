#ifndef _DB_NET_H_
#define _DB_NET_H_

#include "../global.h"
#include "shape.h"
#include "pin.h"

namespace ispd19 {

using Guides = std::vector<std::vector<Shape>>;

class WireRule
{
public:
    WireRule(int w, int s) : _width(w), _spacing(s) { }
    int width() const { return _width; }
    int spacing() const { return _spacing; }
private:
    int _width;
    int _spacing;
};

class Wire
{
public:
    Wire() { }
    Wire(const Point &p1, const Point &p2, unsigned char z, unsigned c) :
        _fm( p1 ),
        _to( p2 ),
        _fz( z ), _tz( z ),
        _code( c ) { }
    Wire(int fx, int fy, int tx, int ty, unsigned char z, unsigned c) :
        _fm( Point(fx, fy) ),
        _to( Point(tx, ty) ),
        _fz( z ), _tz( z ),
        _code( c ) { }
    Wire(const Point &p, unsigned char z1, unsigned char z2, unsigned c) :
        _fm( p ), _to( p ),
        _fz( z1 ),
        _tz( z2 ),
        _code( c ) { }
    Wire(int x, int y, unsigned char z1, unsigned char z2, unsigned c) :
        _fm( Point(x, y) ), _to( Point(x, y) ),
        _fz( z1 ),
        _tz( z2 ),
        _code( c ) { }

    bool isValid() const { return ( (_fm.x() != _to.x()) + (_fm.y() != _to.y()) + (_fz != _tz) == 1 ); }

    const Point& from() const { return _fm; }
    const Point& to() const { return _to; }

    const int fromZ() const { return _fz; }
    const int toZ() const { return _tz; }

    bool isH() const { return _fm.y() == _to.y() && _fz == _tz; }
    bool isV() const { return _fm.x() == _to.x() && _fz == _tz; }
    bool isWire() const { return _fz == _tz; }
    bool isVia() const { return _fz != _tz; }
    bool isWrongWay() const;
    bool isOffTrack() const;
    bool isOutOfGuide(const Guides &guides) const;
    int outOfGuideLength(const Guides &guides) const;
    unsigned code() const { return _code; }

    int length() const;
    int area() const;
    void report() const {
        if( isWire() ) {
            std::cout<<"wire : "<<_fm.toString()<<","<<_to.toString()<<" : "<<(int)_fz<<std::endl;
        }
        if( isVia() ) {
            std::cout<<"via : "<<_fm.toString()<<" : "<<(int)_fz<<" : "<<(int)_tz<<std::endl;
        }
    }
private:
    Point _fm;
    Point _to;
    unsigned char _fz, _tz;
    unsigned _code;
};

class Net
{
public:
    static const unsigned NullIndex = UINT_MAX;
    Net() : _connected(false) { }
    Net(const std::string &n) : _name( n ) ,_connected(false) { }
    const std::string& name() const { return _name; }

    bool isConnected() const { return _connected; }
    void isConnected(bool c) { _connected = c; }

    const std::vector<Pin>& pins() const { return _pins; }
    unsigned numPins() const {
        return _pins.size();
    }
    void addPin(const std::string &name) {
        _pins.emplace_back(name);
    }
    void addPin(const std::string &name, const unsigned compId) {
        _pins.emplace_back(name,compId);
    }
    void addWire(const Point &p1, const Point &p2, unsigned char z, unsigned code = 0) {
        _wires.emplace_back(p1, p2, z, code);
    }
    void addWire(int fx, int fy, int tx, int ty, unsigned char z, unsigned code = 0) {
        _wires.emplace_back(fx, fy, tx, ty, z, code);
    }
    void addVia(const Point &p, unsigned char fz, unsigned char tz, unsigned code = 0) {
        _wires.emplace_back(p, fz, tz, code);
    }
    void addVia(int x, int y, unsigned char fz, unsigned char tz, unsigned code = 0) {
        _wires.emplace_back(x, y, fz, tz, code);
    }
    void addGuide(int lx, int ly, int hx, int hy, unsigned char z);
    void addGuide(const Shape &guide);

    std::vector<Wire>& wires() { return _wires; }
    const std::vector<Wire>& wires() const { return _wires; }

    Guides guides() { return _guides; }
    const Guides guides() const { return _guides; }
    void isConnected();
    void mergeLayerGuidesH(Guides &guides, unsigned char layer);
    void mergeLayerGuidesV(Guides &guides, unsigned char layer);
    void mergeGuides(Guides &guides, char dir);
    void cleanGuides();
    void createGuides();

    unsigned outOfGuideViaCount() const;
    int outOfGuideWireLength() const;
    unsigned totalViaCount() const;
    int totalWireLength() const;
    unsigned offTrackViaCount() const;
    int offTrackWireLength() const;
    int wrongWayWireLength() const;
    void report() const {
        std::cout<<_name<<std::endl;
        unsigned numWires = _wires.size();
        for( unsigned wireIdx = 0; wireIdx < numWires; ++wireIdx )
        {
            const Wire &wire = _wires[wireIdx];
            wire.report();
        }
    }
private:
    std::string _name;
    std::vector<Wire> _wires;
    //std::vector<Pin*> _pins;
    std::vector<Pin> _pins;
    std::vector<std::vector<Shape>> _guides;
    bool _connected;
};

}

#endif

