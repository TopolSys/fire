#ifndef _DB_OBS_H_
#define _DB_OBS_H_

namespace ispd19 {

class Obs {
public:
    void addShape(int lx, int ly, int hx, int hy, unsigned char z) {
        _shapes.emplace_back(lx, ly, hx, hy, z);
        if( _box == Box::Null ) {
            _box == _shapes.back().box();
        }
        _box.merge(_shapes.back().box());
    }
    Box box() const { return _box; }
    const std::vector<Shape>& shapes() const { return _shapes; }
    std::vector<Shape>& shapes() { return _shapes; }
private:
    std::vector<Shape> _shapes;
    Box _box;
};

}

#endif