#ifndef _DB_INST_H_
#define _DB_INST_H_

namespace ispd19 {

class Pin;

class Instance
{
public:
    static const unsigned NullIndex = UINT_MAX;
    Instance() { }
    Instance(const std::string &name) :
        _name( name ) { }
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
    int ori() const { return (int) _ori; }
    bool isPlaced() const { return _loc != Point::Null; }
    int x() const { return _loc.x(); }
    int y() const { return _loc.y(); }
    unsigned macro() const { return _macro; }
    void macro(unsigned m) { _macro = m; }
    const std::string& name() const { return _name; }
private:
    std::string _name;
    unsigned _macro;
    Point _loc;
    unsigned char _ori;
    //bool _flipX;
    //bool _flipY;
    //bool _rot;
    //std::vector<Pin*> _pins; // not used in lem2
};

}

#endif

