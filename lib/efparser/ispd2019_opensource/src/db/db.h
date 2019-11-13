#ifndef _DB_DB_H_
#define _DB_DB_H_

#include "../global.h"
#include <list>
#include <string>

namespace ispd19 {
    class Database;
}

#include "shape.h"
#include "layer.h"
#include "row.h"
#include "pin.h"
#include "macro.h"
#include "inst.h"
#include "via.h"
#include "net.h"
#include "snet.h"
#include "vio.h"
#include "gcell.h"

namespace ispd19 {

class Database
{
public:
    static Database* get(){
        if ( !_instance ) {
            _instance = new Database;
        }
        return _instance;
    }
    static const WireRule& wireRule(unsigned char code) {
        return get()->wireRule(code);
    }

    //layers
    const RouteLayer& getRouteLayer(unsigned char layerId) const {
        return _rLayers[(unsigned)layerId];
    }
    RouteLayer& getRouteLayer(unsigned char layerId) {
        return _rLayers[(unsigned)layerId];
    }
    CutLayer& getCutLayer(unsigned char layerId) {
        return _cLayers[(unsigned)layerId];
    }
    static std::pair<char,unsigned char> layer(const std::string name) {
        unsigned char layerId = Database::rLayer(name);
        if( layerId != RouteLayer::NullIndex ) {
            return std::pair<char,unsigned char>('r', layerId);
        }
        layerId = Database::cLayer(name);
        if( layerId != CutLayer::NullIndex ) {
            return std::pair<char,unsigned char>('c', layerId);
        }
        return std::pair<char,unsigned char>('x', layerId);
    }
    static unsigned char rLayer(const std::string &name) {
        auto layerIter = get()->_rLayerDict.find(name);
        if( layerIter != get()->_rLayerDict.end() ) {
            return layerIter->second;
        }
        return RouteLayer::NullIndex;
    }
    static unsigned char cLayer(const std::string &name) {
        auto layerIter = get()->_cLayerDict.find(name);
        if( layerIter != get()->_cLayerDict.end() ) {
            return layerIter->second;
        }
        return RouteLayer::NullIndex;
    }
    //vias
    static unsigned via( const std::string &name) {
        auto viaIter = get()->_viaDict.find(name);
        if( viaIter != get()->_viaDict.end() )
            return viaIter->second;
        return Via::NullIndex;
    }
    Via& getVia(unsigned viaId) {
        return _vias[viaId];
    }
    //macros
    static unsigned macro( const std::string &name) {
        auto macroIter = get()->_macroDict.find(name);
        if( macroIter != get()->_macroDict.end() )
            return macroIter->second;
        return Macro::NullIndex;
    }
    Macro& getMacro(unsigned macroId) {
        return _macros[macroId];
    }
    //for reading macro with lef parser
    Macro& getLastMacro() {
        return _macros.back();
    }
    //pins
    void allocatePin( int numObj ){
        _pins.resize(numObj);
    }
    static unsigned pin( const std::string &name ) {
        auto pinIter = get()->_pinDict.find(name);
        if( pinIter != get()->_pinDict.end() )
            return pinIter->second;
        return Pin::NullIndex;
    }
    Pin& getPin(unsigned pinId) {
        return _pins[pinId];
    }
    Pin& getPin(const std::string &name) {
        return _pins[Database::pin(name)];
    }

    //instances
    void allocateInstance( int numObj ){
        _instances.resize(numObj);
    }
    static unsigned instance( const std::string &name ) {
        auto instIter = get()->_instanceDict.find(name);
        if( instIter != get()->_instanceDict.end() )
            return instIter->second;
        return Instance::NullIndex;
    }
    Instance& getInstance(unsigned instId) {
        return _instances[instId];
    }
    Instance& getInstance(const std::string &name) {
        return _instances[Database::instance(name)];
    }
    //nets
    void allocateNet( int numObj ){
        _nets.resize(numObj);
    }
    static unsigned net( const std::string &name) {
        auto netIter = get()->_netDict.find(name);
        if( netIter != get()->_netDict.end() )
            return netIter->second;
        return Net::NullIndex;
    }
    Net& getNet(unsigned netId) {
        return _nets[netId];
    }
    //special nets
    static unsigned snet( const std::string &name ) {
        auto snetIter = get()->_snetDict.find(name);
        if( snetIter != get()->_snetDict.end() )
            return snetIter->second;
        return SNet::NullIndex;
    }
    SNet& getSNet( unsigned snetId ) {
        return _snets[snetId];
    }
    //GCell
    static Point getGCellCoor(int x, int y);
    static Point getGCellCoor(const Point &pt);
    static Box getRealCoor(int x, int y);
    static Box getRealCoor(const Point &pt);

    const std::string& designName() const { return _designName; }
    void designName(const std::string &name) { _designName = name; }

    int lefDbuPerMicron() const { return _lefDbuPerMicron; }
    void lefDbuPerMicron(int dbu) { _lefDbuPerMicron = dbu; }

    int defDbuPerMicron() const { return _defDbuPerMicron; }
    void defDbuPerMicron(int dbu) { _defDbuPerMicron = dbu; }

    bool hasGuides() const { return _hasGuides; }
    void hasGuides(bool h) { _hasGuides = h; }

    bool hasWeights() const { return _weights.size() > 0; }
    void weight(const std::string &key, double w);
    double weight( const std::string &key ) const;

    RouteLayer& addRouteLayer(  const std::string &name );
    CutLayer&   addCutLayer(    const std::string &name );
    Macro&      addMacro(       const std::string &name );
    Via&        addVia(         const std::string &name );
    SVia&       addSVia(        const std::string &name );
    ViaRule&    addViaRule(     const std::string &name );
    Instance&   addInstance(    const std::string &name );
    Pin&        addPin(         const std::string &name );
    Net&        addNet(         const std::string &name );
    SNet&       addSNet(        const std::string &name );
    Row&        addRow(         const std::string &name );

    const std::vector<Pin>& pins() const { return _pins; }
    const std::vector<Net>& nets() const { return _nets; }
    const std::vector<Instance> &instances() const { return _instances; }
    const std::vector<Macro> &macros() const { return _macros; }
    const std::vector<SNet>& snets() const { return _snets; }
    const GCell& gcells() const { return _gcells; }
    
    void addGridLineX(int x) { _gcells.addX(x); }
    void addGridLineY(int y) { _gcells.addY(y); }

    unsigned numLayers() const {
        return _rLayers.size();
    }

    void reportNets() {
        for( const auto &net : _nets ) {
            net.report();
        }
    }
    void reportMacros() {
        for( const auto &macro : _macros ) {
            macro.report();
        }
    }
    void reportLayers() {
        for( const auto &layer : _rLayers ) {
            layer.report();
        }
    }

    std::list< std::string > errors, warnings;

private:
    static Database *_instance;
    Database(){
        _lefDbuPerMicron = 2000;
        _defDbuPerMicron = 2000;
        _hasGuides = false;
        _nInstances= 0;
        _nNets = 0;
	_nPins = 0;
    }
    ~Database(){}

    std::string _designName;
    int _lefDbuPerMicron;
    int _defDbuPerMicron;

    std::vector<RouteLayer> _rLayers;
    std::unordered_map<std::string,unsigned char> _rLayerDict;
    std::vector<CutLayer> _cLayers;
    std::unordered_map<std::string,unsigned char> _cLayerDict;

    std::vector<Macro> _macros;
    std::unordered_map<std::string, unsigned> _macroDict;
    std::vector<Via> _vias;
    std::unordered_map<std::string, unsigned> _viaDict;
    std::vector<SVia> _svias;
    std::unordered_map<std::string, unsigned> _sviaDict;

    std::vector<ViaRule> _viarules;
    std::unordered_map<std::string, unsigned> _viaruleDict;

    int _nPins;
    std::vector<Pin> _pins;
    std::unordered_map<std::string,unsigned> _pinDict;

    int _nInstances;
    std::vector<Instance> _instances;
    std::unordered_map<std::string,unsigned> _instanceDict;
    std::vector<ViaInstance> _viaInstances;

    int _nNets;
    std::vector<Net> _nets;
    std::unordered_map<std::string,unsigned> _netDict;
    std::vector<WireRule> _wireRules;

    std::vector<SNet> _snets;
    std::unordered_map<std::string,unsigned> _snetDict;

    std::vector<Row> _rows;

    GCell _gcells;

    bool _hasGuides;

    std::unordered_map<std::string,double> _weights;
};

}

#endif

