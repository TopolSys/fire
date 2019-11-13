#ifndef _DB_LAYER_H_
#define _DB_LAYER_H_

namespace ispd19 {

class TrackSet
{
public:
    TrackSet() { }
    TrackSet(int srt, int n, int stp) : _start(srt), _num(n), _step(stp) { }

    void set(int srt, int n, int stp) {
        _start = srt;
        _num = n;
        _step = stp;
    }
    int start() const { return _start; }
    int num() const { return _num; }
    int step() const { return _step; }
    int end() const { return _start + _step * (_num - 1); }
    int nearestTrack(int x) const;
private:
    int _start;
    int _num;
    int _step;
};

class PRLTable
{
public:
    PRLTable() { }
    PRLTable(unsigned nWidths, unsigned nLengths) : PRLTable(nWidths, nLengths, 'B') { }
    PRLTable(unsigned nWidths, unsigned nLengths, char dir) {
        _direction = dir;
        _table .resize(nWidths , std::vector<int>(nLengths, 0));
        _width .resize(nWidths , 0);
        _length.resize(nLengths, 0);
    }
    void setHV() { _direction = 'B'; }
    void setH() { _direction = 'H'; }
    void setV() { _direction = 'V'; }
    bool isH() const { return _direction == 'H' || _direction == 'B'; }
    bool isV() const { return _direction == 'V' || _direction == 'B'; }
    void setLength (unsigned lIdx, int ilength){
        _length[lIdx] = ilength;
    }
    void setSpacing(unsigned wIdx, unsigned lIdx, int iwidth, int spacing) {
        _table[wIdx][lIdx] = spacing;
        _width[wIdx] = iwidth;
    }
    int spacing(int width, int runLength) const;
    int spacing(int width1, int width2, int runLength);
    bool isValid(const Shape &s1, const Shape &s2);
    void report() const;

    const std::vector<int>& getWidth () const { return _width ; }
    const std::vector<int>& getLength() const { return _length; }
    const std::vector<std::vector<int>>& getTable() const { return _table; }
    int numWidth () const { return _width.size(); }
    int numLength() const { return _table.empty()? 0: _table.front().size(); }
private:
    std::vector<int> _width;
    std::vector<int> _length;
    std::vector<std::vector<int>> _table;
    char _direction;
};

class RouteLayer
{
public:
    static const unsigned char NullIndex = UCHAR_MAX;
    RouteLayer(const std::string &name) : _name(name) {
        _pitchX = 0;
        _pitchY = 0;
        _direction= 'X';
        _width = 0;
        _spacing = 0;
        _area = 0;
    }

    const std::string& name() const { return _name; }

    int preferTrackPitch() const { return _preferWayTracks.step(); }
    int wrongTrackPitch() const { return _wrongWayTracks.step(); }
    void pitchX(int p) { _pitchX = p; }
    void pitchY(int p) { _pitchY = p; }

    int trackPitchX() const { return ( isV() ? preferTrackPitch() : wrongTrackPitch() ); }
    int trackPitchY() const { return ( isH() ? preferTrackPitch() : wrongTrackPitch() ); }

    void setH() { _direction = 'H'; }
    void setV() { _direction = 'V'; }
    bool isH() const { return _direction == 'H'; }
    bool isV() const { return _direction == 'V'; }

    void width(int w) { _width = w; }
    int width() const { return _width; }
    void spacing(int s) { _spacing = s; }
    int spacing() const { return _spacing; }

    void area(int a) { _area = a; }
    int area() const { return _area; }

    PRLTable& addPRLTable(unsigned numWidths, unsigned numLengths) {
        _prlTables.emplace_back(numWidths, numLengths);
        return _prlTables.back();
    }

    const std::vector<PRLTable>& getPRLTable() const {
        return _prlTables;
    }

    void setTrack(char dir, int start, int num, int step) {
        if( _direction == dir ) {
            _preferWayTracks.set(start, num, step);
        } else {
            _wrongWayTracks.set(start, num, step);
        }
    }
    const TrackSet& tracks(bool wrongway = false) const {
        return wrongway ? _wrongWayTracks : _preferWayTracks;
    }

    Point firstTrack() const {
        Point pt;
        if( isH() ) {
            pt.x(_wrongWayTracks.start());
            pt.y(_preferWayTracks.start());
        } else {
            pt.x(_preferWayTracks.start());
            pt.y(_wrongWayTracks.start());
        }
        return pt;
    }
    Point lastTrack() const {
        Point pt;
        if( isH() ) {
            pt.x(_wrongWayTracks.end());
            pt.y(_preferWayTracks.end());
        } else {
            pt.x(_preferWayTracks.end());
            pt.y(_wrongWayTracks.end());
        }
        return pt;
    }

    void report() const
    {
        std::cout<<_name<<" ("<<_direction<<") p"<<_pitchX<<","<<_pitchY;
        std::cout<<" w"<<_width<<" s"<<_spacing<<" a"<<_area;
        std::cout<<" prefer: srt"<<_preferWayTracks.start()<<" stp"<<_preferWayTracks.step()<<" num"<<_preferWayTracks.num();
        std::cout<<" wrong: srt"<<_wrongWayTracks.start()<<" stp"<<_wrongWayTracks.step()<<" num"<<_wrongWayTracks.num();
        std::cout<<std::endl;
    }

private:
    std::string _name;
    char _direction;
    int _pitchX;
    int _pitchY;
    int _width;
    int _spacing;
    int _area;
    TrackSet _preferWayTracks;
    TrackSet _wrongWayTracks;
    std::vector<PRLTable> _prlTables;
};

class CutLayer
{
public:
    static const unsigned char NullIndex = UCHAR_MAX;
    CutLayer(const std::string &name) : _name(name) {
        _width = 0;
        _spacing = 0;
    }

    void width(int w) { _width = w; }
    int width() const { return _width; }
    void spacing(int s) { _spacing = s; }
    int spacing() const { return _spacing; }
private:
    std::string _name;
    int _width;
    int _spacing;
};

}

#endif

