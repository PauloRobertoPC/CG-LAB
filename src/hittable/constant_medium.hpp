#pragma once

#include "../util/rtweekend.hpp"

#include "../hittable/hittable.hpp"
#include "../material/material.hpp"
#include "../texture/texture.hpp"

class constant_medium : public hittable {
    public:
        constant_medium(shared_ptr<hittable> b, double d, shared_ptr<texture> a);

        constant_medium(shared_ptr<hittable> b, double d, color c);

        virtual bool hit(
            const ray& r, double t_min, double t_max, hit_record& rec) const override;

        virtual bool bounding_box(double time0, double time1, aabb& output_box) const override {
            return boundary->bounding_box(time0, time1, output_box);
        }

        virtual void gui_edit(int idx) override;

    public:
        shared_ptr<hittable> boundary;
        shared_ptr<material> phase_function;
        double neg_inv_density;
        double density;
};
