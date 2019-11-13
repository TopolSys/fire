#ifndef _DB_VIA_H_
#define _DB_VIA_H_

namespace ispd19 {

class Via
{
public:
    static const unsigned NullIndex = UINT_MAX;
    Via() { }
    Via(const std::string &n) :
        _name(n),
        _topLayer(RouteLayer::NullIndex),
        _cutLayer(CutLayer::NullIndex),
        _botLayer(RouteLayer::NullIndex) { }
    unsigned char topLayer() const { return _topLayer; }
    unsigned char cutLayer() const { return _cutLayer; }
    unsigned char botLayer() const { return _botLayer; }
    const std::vector<Box>& topShapes() const { return _topShapes; }
    const std::vector<Box>& cutShapes() const { return _cutShapes; }
    const std::vector<Box>& botShapes() const { return _botShapes; }
    void topLayer( unsigned char layer ) { _topLayer = layer; }
    void botLayer( unsigned char layer ) { _botLayer = layer; }
    void cutLayer( unsigned char layer ) { _cutLayer = layer; }
    void addTopShape(int lx, int ly, int hx, int hy) {
        _topShapes.emplace_back(lx, ly, hx, hy);
    }
    void addCutShape(int lx, int ly, int hx, int hy) {
        _cutShapes.emplace_back(lx, ly, hx, hy);
    }
    void addBotShape(int lx, int ly, int hx, int hy){
        _botShapes.emplace_back(lx, ly, hx, hy);
    }
    bool isMultiCut() const {
        return _cutShapes.size() > 1;
    }
private:
    std::string _name;
    unsigned char _topLayer;
    unsigned char _cutLayer;
    unsigned char _botLayer;
    std::vector<Box> _topShapes;
    std::vector<Box> _cutShapes;
    std::vector<Box> _botShapes;
};

class ViaInstance
{
public:
    ViaInstance() { }
    ViaInstance(Via *v, int vx, int vy) :
        _via(v), _x(vx), _y(vy) { }
    const Via* via() const { return _via; }
    int x() const { return _x; }
    int y() const { return _y; }
private:
    Via *_via;
    int _x;
    int _y;
};

//for via generation
class ViaRuleLayer
{
public:
    ViaRuleLayer() :
        _layer( RouteLayer::NullIndex ),
        _encOverhang1( INT_MAX ),
        _encOverhang2( INT_MAX ),
        _spacingX( INT_MAX ),
        _spacingY( INT_MAX ),
        _rect( Box::Null )
    { }

    unsigned char layer() const { return _layer; }
    void layer( unsigned char i ) { _layer = i; }
    int enclosureOverhang1() const { return _encOverhang1; }
    int enclosureOverhang2() const { return _encOverhang2; }
    int spacingX() const { return _spacingX; }
    int spacingY() const { return _spacingY; }
    const Box& rect() const { return _rect; }

    void enclosureOverhang(int oh1, int oh2 ) {
        _encOverhang1 = oh1;
        _encOverhang2 = oh2;
    }
    void spacing(int x, int y) {
        _spacingX = x;
        _spacingY = y;
    }
    void rect(int lx, int ly, int hx, int hy) {
        _rect.lx(lx);
        _rect.ly(ly);
        _rect.hx(hx);
        _rect.hy(hy);
    }
    void report() const {
        std::cout<<"layer "<<(int)_layer;
        if( _encOverhang1 != INT_MAX ) {
            std::cout<<"enc "<<_encOverhang1<<" "<<_encOverhang2;
        }
        if( _spacingX != INT_MAX ) {
            std::cout<<"spc "<<_spacingX<<" "<<_spacingY;
        }
        if( _rect != Box::Null ) {
            std::cout<<"rect "<<_rect.toString();
        }
        std::cout<<std::endl;
    }
private:
    unsigned char _layer;
    int _encOverhang1;
    int _encOverhang2;
    int _spacingX;
    int _spacingY;
    Box _rect;
};

class ViaRule
{
public:
    ViaRule() { }
    ViaRule(const std::string &n) :
        _name(n),
        _isGenerate(false),
        _isDefault(false)
    { }
    bool isGenerate() const { return _isGenerate; }
    void isGenerate( bool b ) { _isGenerate = b; }
    bool isDefault() const { return _isDefault; }
    void isDefault( bool b ) { _isDefault = b; }
    ViaRuleLayer& botLayerRules() { return _botLayerRules; }
    ViaRuleLayer& cutLayerRules() { return _cutLayerRules; }
    ViaRuleLayer& topLayerRules() { return _topLayerRules; }
    void report() const {
        std::cout<<_name<<" "<<(_isGenerate?"generate ":" ")<<(_isDefault?"default":"")<<std::endl;
        _botLayerRules.report();
        _cutLayerRules.report();
        _topLayerRules.report();
    }
private:
    std::string _name;
    bool _isGenerate;
    bool _isDefault;
    ViaRuleLayer _botLayerRules;
    ViaRuleLayer _cutLayerRules;
    ViaRuleLayer _topLayerRules;
};

}

#endif

