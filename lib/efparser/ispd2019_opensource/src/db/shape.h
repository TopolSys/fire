#ifndef _DB_SHAPE_H_
#define _DB_SHAPE_H_

namespace ispd19 {

class Point
{
public:
    static Point Null;
    Point():Point(INT_MAX, INT_MAX, UCHAR_MAX){ }
    Point(int px, int py):Point(px, py, UCHAR_MAX){ }
    Point(int px, int py, unsigned char pz) : _x(px), _y(py), _z(pz) { }
    int x() const { return _x; }
    int y() const { return _y; }
    unsigned char z() const { return _z; }
    void x(int px) { _x = px; }
    void y(int py) { _y = py; }
    void z(unsigned char pz) { _z = pz; }

    bool operator==( const Point &p ) const { return p.x() == _x && p.y() == _y && p.z() == _z; }
    bool operator!=( const Point &p ) const { return p.x() != _x || p.y() != _y || p.z() != _z; }

    static int distance(const Point &a, const Point &b) {
        return std::abs(a.x() - b.x()) + std::abs(a.y() - b.y());
    }
    std::string toString() const {
        if( _z == UCHAR_MAX ) {
            return "(" + std::to_string(_x) + "," + std::to_string(_y) + ")";
        }
        return "(" + std::to_string(_x) + "," + std::to_string(_y) + "," + std::to_string((int)_z) + ")";
    }
private:
    int _x;
    int _y;
    unsigned char _z;
};

class Line
{
public:
    static Line Null;
    Line() : Line(INT_MAX, INT_MAX, INT_MAX, INT_MAX) { }
    Line(Point p1, Point p2) : _p1(p1), _p2(p2) { }
    Line(int x1, int y1, int x2, int y2) {
        _p1 = Point(x1, y1);
        _p2 = Point(x2, y2);
    }
    const Point& p1() const { return _p1; }
    const Point& p2() const { return _p2; }
    int x1() const { return _p1.x(); }
    int y1() const { return _p1.y(); }
    int x2() const { return _p2.x(); }
    int y2() const { return _p2.y(); }

    bool operator==( const Line &line ) const { return line.p1() == _p1 && line.p2() == _p2; }
    bool operator!=( const Line &line ) const { return line.p1() != _p1 || line.p2() != _p2; }

    int length() const {
        return std::abs(_p1.x() - _p2.x()) + std::abs(_p1.y() - _p2.y());
    }
    std::string toString() const {
        return _p1.toString() + ":" + _p2.toString();
    }
private:
    Point _p1;
    Point _p2;
};

class Box
{
public:
    static Box Null;
    Box() : _lx(INT_MAX), _ly(INT_MAX), _hx(INT_MIN), _hy(INT_MIN) { }
    Box(int x1, int y1, int x2, int y2) {
        _lx = std::min(x1, x2);
        _hx = std::max(x1, x2);
        _ly = std::min(y1, y2);
        _hy = std::max(y1, y2);
    }
    int lx() const { return _lx; }
    int ly() const { return _ly; }
    int hx() const { return _hx; }
    int hy() const { return _hy; }
    void lx(int x) { _lx = x; }
    void ly(int y) { _ly = y; }
    void hx(int x) { _hx = x; }
    void hy(int y) { _hy = y; }

    bool isValid() const { return _lx != INT_MAX && _ly != INT_MAX && _hx != INT_MIN && _hy != INT_MIN; }

    bool operator==( const Box &box ) const {
        return box.lx() == _lx && box.ly() == _ly && box.hx() == _hx && box.hy() == _hy;
    }
    bool operator!=( const Box &box ) const {
        return box.lx() != _lx || box.ly() != _ly || box.hx() != _hx || box.hy() != _hy;
    }

    int width() const { return _hx - _lx; }
    int height() const { return _hy - _ly; }
    int area() const { return width() * height(); }
    Point ll() const { return Point(_lx, _ly); }
    Point lr() const { return Point(_hx, _ly); }
    Point ul() const { return Point(_lx, _hy); }
    Point ur() const { return Point(_hx, _hy); }

    void merge( const Box &b ) {
        _lx = std::min(_lx, b.lx());
        _ly = std::min(_ly, b.ly());
        _hx = std::max(_hx, b.hx());
        _hy = std::max(_hy, b.hy());
    }

    void merge( const Point &p ) {
        _lx = std::min(_lx, p.x());
        _ly = std::min(_ly, p.y());
        _hx = std::max(_hx, p.x());
        _hy = std::max(_hy, p.y());
    }

    bool inside(const Point &pt) const {
        return ( pt.x() >= _lx && pt.x() <= _hx &&
                 pt.y() >= _ly && pt.y() <= _hy );
    }
    static Box merge(const Box &b1, const Box &b2) {
        return Box( std::min(b1.lx(), b2.lx()),
                    std::min(b1.ly(), b2.ly()),
                    std::max(b1.hx(), b2.hx()),
                    std::max(b1.hy(), b2.hy()) );
    }
    static int overlap(const Box &b1, const Box &b2) {
        int ox = std::min(b1.hx(), b2.hx()) - std::max(b1.lx(), b2.lx());
        if( ox <= 0 )
            return 0;
        int oy = std::min(b1.hy(), b2.hy()) - std::max(b1.ly(), b2.ly());
        if( oy <= 0 )
            return 0;
        return ox * oy;
    }
    static bool touch(const Box &b1, const Box &b2) {
        int ox = std::min(b1.hx(), b2.hx()) - std::max(b1.lx(), b2.lx());
        if( ox < 0 )
            return 0;
        int oy = std::min(b1.hy(), b2.hy()) - std::max(b1.ly(), b2.ly());
        if( oy < 0 )
            return 0;
        return ox > 0 || oy > 0;
    }
    std::string toString() const {
        return ll().toString() + ":" + ur().toString();
    }
private:
    int _lx;
    int _ly;
    int _hx;
    int _hy;
};

class Shape
{
public:
    static Shape Null;
    Shape() : _box(Box::Null), _z(UCHAR_MAX) { }
    Shape(const Box &box, unsigned char z) : _box(box), _z(z) { }
    Shape(int x1, int y1, int x2, int y2, unsigned char z) {
        _box.lx(std::min(x1, x2));
        _box.ly(std::min(y1, y2));
        _box.hx(std::max(x1, x2));
        _box.hy(std::max(y1, y2));
        _z = z;
    }

    int lx() const { return _box.lx(); }
    int hx() const { return _box.hx(); }
    int ly() const { return _box.ly(); }
    int hy() const { return _box.hy(); }
    unsigned char z() const { return _z; }

    void lx(int x) { _box.lx(x); }
    void ly(int y) { _box.ly(y); }
    void hx(int x) { _box.hx(x); }
    void hy(int y) { _box.hy(y); }
    void z(unsigned char iz) { _z = iz; }

    const Box& box() const { return _box; }
    void box(const Box &b) { _box = b; }

    bool isValid() const { return _z != UCHAR_MAX && _box.isValid(); }

    bool operator==( const Shape &shape ) const {
        return _z == shape.z() && _box == shape.box();
    }
    bool operator!=( const Shape &shape ) const {
        return _z != shape.z() || _box != shape.box();
    }

    std::string toString() const {
        return _box.toString() + " : " + std::to_string((int)_z);
    }
    Shape hSplit(int x) {
        Shape shape(_box, _z);
        hx(x);
        shape.lx(x);
        return shape;
    }
    Shape vSplit(int y) {
        Shape shape(_box, _z);
        hy(y);
        shape.ly(y);
        return shape;
    }
private:
    Box _box;
    unsigned char _z;
};

}

#endif

