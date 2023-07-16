#pragma once

#include "../util/rtweekend.hpp"
#include "../util/color.hpp"
#include "perlin.hpp"

class texture {
    public:
        virtual color value(double u, double v, const point3& p) const = 0;
};
