// textureFunctions.hpp
// A set of functions to modify textures
#pragma once

class vulkanImage;
class renderer;

struct textureData
    {
        int width = 0;
        int height = 0;
        int channels = 0;
    };

textureData loadTexture(const char *filepath, vulkanImage &image, renderer &renderer);
