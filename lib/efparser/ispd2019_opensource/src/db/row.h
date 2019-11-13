#ifndef _DB_ROW_H_
#define _DB_ROW_H_

namespace ispd19 {

class Row
{
public:
    Row() { }
    Row(const std::string &n) : Row(n, INT_MAX, INT_MAX, 0, 0, false) { }
    Row(const std::string &n, int rx, int ry, int ns, int sw, bool flip):
        _name(n), _x(rx), _y(ry), _numSites(ns), _siteWidth(sw), _flip(flip) { }
    int x() const { return _x; }
    int y() const { return _y; }
    int lx() const { return _x; }
    int hx() const { return _x + _numSites * _siteWidth; }
    int numSites() const { return _numSites; }
    int siteWidth() const { return _siteWidth; }
    bool isFlip() const { return _flip; }

    void x(int rx) { _x = rx; }
    void y(int ry) { _y = ry; }
    void numSites(int ns) { _numSites = ns; }
    void siteWidth(int sw) { _siteWidth = sw; }
    void isFlip(bool flip) { _flip = flip; }
private:
    std::string _name;
    int _x;
    int _y;
    int _numSites;
    int _siteWidth;
    bool _flip;
};

}

#endif

