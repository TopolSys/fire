#include "db.h"

#include "net.h"

using namespace ispd19;

void Net::addGuide(int lx, int ly, int hx, int hy, unsigned char z) {
    if( _guides.empty() ) {
        _guides.resize(Database::get()->numLayers());
    }
    _guides[(unsigned)z].emplace_back(lx, ly, hx, hy, z);
}

void Net::addGuide(const Shape &guide) {
    if( _guides.empty() ) {
        _guides.resize(Database::get()->numLayers());
    }
    _guides[(unsigned)guide.z()].push_back(guide);
}

