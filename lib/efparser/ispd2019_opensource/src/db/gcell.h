#ifndef _DB_GCELL_H_
#define _DB_GCELL_H_

namespace ispd19 {

class GCell
{
public:
    int gridX(int x) const;
    int gridY(int y) const;

    const std::vector<int>& grid( bool isV ) const { return isV? _x: _y; }
    int gcellLX(int x) const { return _x[x]; }
    int gcellLY(int y) const { return _y[y]; }
    int gcellHX(int x) const { return _x[x + 1]; }
    int gcellHY(int y) const { return _y[y + 1]; }
    int numX() const { return _x.size(); }
    int numY() const { return _y.size(); }
    void addX(int x) { _x.push_back(x); }
    void addY(int y) { _y.push_back(y); }
    void clear() {
        _x.clear();
        _y.clear();
    }
    bool empty() const;
    void init();
    const std::vector<int>& x() const { return _x; }
    const std::vector<int>& y() const { return _y; }
private:
//    unsigned _numX;
//    unsigned _numY;
    std::vector<int> _x;
    std::vector<int> _y;
};

}

#endif

