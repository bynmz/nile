#pragma once

#include "material_system.hpp"

namespace nile
{
class MaterialPack
{
private:
    Material bgTexture;
    Material rTexture;
    Material gTexture;
    Material bTexture;
    Material blendMap;

public:
    MaterialPack();
    MaterialPack(
        Material bgTexture, 
        Material rTexture, 
        Material gTexture, 
        Material bTexture, 
        Material blendMap)  : bgTexture(bgTexture),
            rTexture(rTexture),
            gTexture(gTexture),
            bTexture(bTexture),
            blendMap(blendMap)
            {}
    ~MaterialPack(){
     bgTexture.destroy();
     rTexture.destroy();
     gTexture.destroy();
     bTexture.destroy();
     blendMap.destroy();
     };

    Material getBgTexture() { return bgTexture; }
    Material getrTexture() { return rTexture; }
    Material getgTexture() { return gTexture; }
    Material getbTexture() { return bTexture; }
    Material getBlendMap() { return blendMap; }

    void create() 
    {
        bgTexture.create();
        rTexture.create();
        gTexture.create();
        bTexture.create();
        blendMap.create();
    }
};


}