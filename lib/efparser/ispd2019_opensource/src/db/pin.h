#ifndef _DB_PIN_H_
#define _DB_PIN_H_

#include "inst.h"

namespace ispd19 {

class Pin
{
public:
    static const unsigned NullIndex = UINT_MAX;
    Pin() { }
    Pin(const std::string &n) : _name( n ), _compId(Instance::NullIndex) { }
    Pin(const std::string &n, const unsigned comp_id) : _name( n ), _compId(comp_id) { } // add for lem2
    const std::string& name() const { return _name; }
    const unsigned compId() const { return _compId; }
    bool isUseAnalog() const { return _use == 'a'; }
    bool isUseClock() const { return _use == 'c'; }
    bool isUseGround() const { return _use == 'g'; }
    bool isUsePower() const { return _use == 'p'; }
    bool isUseSignal() const { return _use == 's'; }
    void setIsUseAnalog() { _use = 'a'; }
    void setIsUseClock() { _use = 'c'; }
    void setIsUseGround() { _use = 'g'; }
    void setIsUsePower() { _use = 'p'; }
    void setIsUseSignal() { _use = 's'; }

    bool isInput() const { return _dir == 'i'; }
    bool isOutput() const { return _dir == 'o'; }
    bool isInOut() const { return _dir == 'b'; }
    void setIsInput() { _dir = 'i'; }
    void setIsOutput() { _dir = 'o'; }
    void setIsInOut() { _dir = 'b'; }
    void addShape(int lx, int ly, int hx, int hy, unsigned char z) {
        _shapes.emplace_back(lx, ly, hx, hy, z);
        if( _box == Box::Null ) {
            _box == _shapes.back().box();
        }
        _box.merge(_shapes.back().box());
    }
    bool inside(const Point &pt);
    void report() const {
        std::cout<<_name<<std::endl;
        std::cout<<"use : "<<_use<<std::endl;
        std::cout<<"dir : "<<_dir<<std::endl;
        std::cout<<"shape : "<<std::endl;
        for( const Shape &shape : _shapes )
        {
            std::cout<<"\t"<<shape.toString()<<std::endl;
        }
        if( hasCoordinate() ){
            std::cout<<"PLACED/COVER/FIXED at "<< _loc.toString()<<std::endl;
            std::cout<<(int)_ori <<std::endl;
            //std::cout<<"flipX: "<<_flipX<<", _flipY: "<<_flipY<<", rotate: "<<_rot<<std::endl;
        }
    }
    int ori() const { return (int) _ori; }

    // lem2: place for non-macro-pin
    void place(int ix, int iy, unsigned char ori ){
        _loc.x(ix);
        _loc.y(iy);
        _ori = ori;
        //_flipX = fx;
        //_flipY = fy;
        //_rot   = rot;
    }
    void unplace() {
        _loc = Point::Null;
        _ori = 0;
        //_flipX = false;
        //_flipY = false;
        //_rot   = false;
    }
    bool hasCoordinate() const { return _loc != Point::Null; }
    int x() const { return _loc.x(); }
    int y() const { return _loc.y(); }

    Box box() const { return _box; }
    const std::vector<Shape>& shapes() const { return _shapes; }
    std::vector<Shape>& shapes() { return _shapes; }
private:
    std::string _name;
    std::vector<Shape> _shapes;
    Box _box;
    Point _loc; // lem2: for non-macro-pin
    //char _direction;
    char _use;
    char _dir;
    unsigned char _ori;
    //bool _rot; // lem2: rotation
    //bool _flipX;
    //bool _flipY;
    unsigned _compId; //component id: the instance of Macro
};

}

#endif

