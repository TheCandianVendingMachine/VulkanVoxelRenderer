// voxel.hpp
// Represents voxel data
#pragma once

struct voxel
    {
        voxel() = default;
        voxel(unsigned char r, unsigned char g, unsigned char b) :
        colour{r, g, b}
        {}
        voxel(float r, float g, float b) :
            colour{ static_cast<unsigned char>(r * 32), static_cast<unsigned char>(g * 64), static_cast<unsigned char>(b * 32) }
        {}

        union {
            struct 
                {
                    unsigned char r : 5; // 32
                    unsigned char g : 6; // 64
                    unsigned char b : 5; // 32
                } colour;
            short entire;
        };
    };

