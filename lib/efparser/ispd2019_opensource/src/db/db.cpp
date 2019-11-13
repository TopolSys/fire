#include "db.h"

using namespace ispd19;

Database *Database::_instance = 0;
Point Point::Null = Point(INT_MAX, INT_MAX);
Line Line::Null = Line(INT_MAX, INT_MAX, INT_MAX, INT_MAX);
Box Box::Null = Box(INT_MAX, INT_MAX, INT_MIN, INT_MIN);
Shape Shape::Null = Shape(Box::Null, UCHAR_MAX);

RouteLayer& Database::addRouteLayer( const std::string &name )
{
    _rLayers.emplace_back(name);
    _rLayerDict.emplace(name, _rLayers.size() - 1);
    return _rLayers.back();
}

CutLayer& Database::addCutLayer( const std::string &name )
{
    _cLayers.emplace_back(name);
    _cLayerDict.emplace(name, _cLayers.size() - 1);
    return _cLayers.back();
}

Macro& Database::addMacro( const std::string &name )
{
    _macros.emplace_back(name);
    _macroDict.emplace(name, _macros.size() - 1);
    return _macros.back();
}

Via& Database::addVia( const std::string &name )
{
    _vias.emplace_back(name);
    _viaDict.emplace(name, _vias.size() - 1);
    return _vias.back();
}

SVia& Database::addSVia( const std::string &name )
{
    _svias.emplace_back(name);
    _sviaDict.emplace(name, _svias.size() - 1);
    return _svias.back();
}

ViaRule& Database::addViaRule( const std::string &name )
{
    _viarules.emplace_back(name);
    _viaruleDict.emplace(name, _viarules.size() - 1);
    return _viarules.back();
}

Instance& Database::addInstance( const std::string &name )
{
    _instances[_nInstances++] = Instance(name);
    _instanceDict.emplace(name, _nInstances - 1);
    return _instances[_nInstances-1];
}

Pin& Database::addPin( const std::string &name )
{
    _pins[_nPins++] = Pin(name);
    _pinDict.emplace(name, _nPins - 1);
    return _pins[_nPins-1];
}

Net& Database::addNet( const std::string &name )
{
    _nets[_nNets++] = Net(name);
    _netDict.emplace(name, _nNets - 1);
    return _nets[_nNets-1];
}

SNet& Database::addSNet( const std::string &name )
{
    _snets.emplace_back(name);
    _snetDict.emplace(name, _snets.size() - 1);
    return _snets.back();
}

Row& Database::addRow( const std::string &name )
{
    _rows.emplace_back(name);
    return _rows.back();
}

