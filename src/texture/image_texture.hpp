#pragma once

#include "texture.hpp"

class image_texture : public texture {
    public:
        const static int bytes_per_pixel = 3;

    image_texture();
    image_texture(const char* filename);
    ~image_texture();
    virtual color value(double u, double v, const vec3& p) const override;

    public:
        unsigned char *data;
        int width, height;
        int bytes_per_scanline;
};
