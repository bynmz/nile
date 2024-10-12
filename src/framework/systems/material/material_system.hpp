#pragma once

#include "framework/core/nile_texture.hpp"
#include "framework/core/nile_device.hpp"

// std
#include <string>
#include <iostream>

namespace nile
{
class Material
{
private:
    std::string path;
    NileDevice &device;
    std::shared_ptr<NileTexture> texture;
    float width, height;
    int textureID;
    int normalMap;

    float shineDamper = 1;
    float reflectivity = 0;

    bool hasTransparency{false};
    bool useFakeLighting{false};

    int numberOfRows = 1;


public:
    // Material();
    // ~Material();

    int getNumberOfRows(){ return numberOfRows; }
    void setNumberOfRows(int numberOfRows){ this->numberOfRows = numberOfRows; }
    int getNormalMap(){ return normalMap; }
    void setNormalMap(int normalMap){ this->normalMap = normalMap; }
    bool isUseFakeLighting(){ return useFakeLighting; }
    void setUseFakeLighting(bool useFakeLighting){ this->useFakeLighting = useFakeLighting; }
    bool isHasTransparency(){return hasTransparency;}
    void setHasTransparency(bool hasTransparency){ this->hasTransparency = hasTransparency; }
    // void create();
    void create(int textureID){ this->textureID = textureID; }
    void destroy(){ texture->destroy(); }
    float getShineDamper(){ return shineDamper; }
    void setShineDamper(float shineDamper){ this->shineDamper = shineDamper; }
    float getReflectivity(){ return reflectivity; }
    void setReflectivity(float reflectivity){ this->reflectivity = reflectivity; }
    float getWidth(){ return width; }
    float getHeight(){ return height; }
    void setTextureID(int textureID){ this->textureID = textureID; }
    int getTextureID(){ return textureID; }


    void create()  
    {
        try
        {
            texture = NileTexture::createTextureFromFile(device, path);
            width = texture->getTextureWidth();
            height = texture->getTextureHeight();
        }
        catch(const std::exception& e)
        {
            std::cerr << "Exception: " << e.what() << '\n';
        }
        
    }

    Material(NileDevice &device, const std::string& path) 
    : path(path),
    device(device) {}

    ~Material()
    { 
        destroy(); 
    }
    
};


} // namespace nile
