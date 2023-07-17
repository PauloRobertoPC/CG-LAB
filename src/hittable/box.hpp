#pragma once

#include "../util/rtweekend.hpp"

#include "aarect.hpp"
#include "hittable_list.hpp"

class box : public hittable  {
    public:
        box() {}
        box(const point3& p0, const point3& p1, shared_ptr<material> ptr);

        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;

        virtual bool bounding_box(double time0, double time1, aabb& output_box) const override {
            output_box = aabb(box_min, box_max);
            return true;
        }

        virtual int gui_edit(int idx) override;

    public:
        point3 box_min;
        point3 box_max;
        shared_ptr<material> mat;
        hittable_list sides;

    private:
        void construct_box_with_rectangles();
};
