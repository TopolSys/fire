#ifndef _SETTING_H_
#define _SETTING_H_

namespace ispd19
{

class Setting
{
public:
    static Setting* get(){
        if( !_instance ) {
            _instance = new Setting;
        }
        return _instance;
    }

    static const std::string& techLef() { return get()->_techLef; }
    static const std::string& designDef() { return get()->_designDef; }
    static const std::string& guideRg() { return get()->_guideRg; }

    static void techLef( const std::string &lef ) { get()->_techLef = lef; }
    static void designDef( const std::string &def ) { get()->_designDef = def; }
    static void guideRg( const std::string &rg ) { get()->_guideRg = rg; }

    static bool readArgs(int argc, char **argv)
    {
        for( int i = 1; i < argc; ) {
            std::string arg(argv[i++]);
            if( arg == "-lef" && i < argc ) {
                Setting::techLef(std::string(argv[i++]));
            } else if( arg == "-def" && i < argc ) {
                Setting::designDef(std::string(argv[i++]));
            } else if( arg == "-guide" && i < argc ) {
                Setting::guideRg(std::string(argv[i++]));
            } else {
                std::cerr<<"Unknown option: "<<arg<<std::endl;
                return false;
            }
        }
        return true;
    }
private:
    static Setting *_instance;
    Setting() {
    }
    ~Setting(){}
    std::string _techLef;
    std::string _designDef;
    std::string _guideRg;
};

Setting *Setting::_instance = 0;

}

#endif

