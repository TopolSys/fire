#ifndef _DB_MACRO_H_
#define _DB_MACRO_H_

#include "obs.h"
namespace ispd19 {

class Pin;
class Obs;

class Macro
{
public:
    static const unsigned NullIndex = UINT_MAX;
    Macro() { }
    Macro(const std::string &n) : _name(n) { }
    const std::string& name() const { return _name; }
    void name( const std::string &n ) { _name = n; }
    const std::string& cls() const { return _class; }
    void cls( const std::string &c ) { _class = c; }
    const std::string& site() const { return _site; }
    void site( const std::string &s ) { _site = s; }

    int width() const { return _size.width(); }
    int height() const { return _size.height(); }
    int origX() const {return _size.lx(); }
    int origY() const {return _size.ly(); }
    void size(int origX, int origY, int width, int height) {
        _size.lx(origX);
        _size.ly(origY);
        _size.hx(origX + width);
        _size.hy(origY + height);
    }

    Pin& addPin(const std::string &n) {
        _pins.emplace_back(n);
        return _pins.back();
    }
    const Obs& obs() const { return _obs; }
    Obs& obs(){ return _obs; }

    void report() const {
        std::cout<<_name<<" size: "<< _size.toString()<<std::endl;
        std::cout<<"--pins("<<_pins.size()<<")"<<std::endl;
        for( const Pin &pin : _pins )
        {
            pin.report();
        }
    }

    const std::vector<Pin>& pins() const { return _pins; }

private:
    std::string _name;
    std::string _class;
    std::string _site;
    Box _size;
    std::vector<Pin> _pins;
    Obs _obs;
};

}

#endif

