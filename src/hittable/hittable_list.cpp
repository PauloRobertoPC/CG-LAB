#include "hittable_list.hpp"

bool hittable_list::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    hit_record temp_rec;
    bool hit_anything = false;
    auto closest_so_far = t_max;

    for (const auto& object : objects) {
        if (object->hit(r, t_min, closest_so_far, temp_rec)) {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
    }

    return hit_anything;
}

bool hittable_list::bounding_box(double time0, double time1, aabb& output_box) const {
    if (objects.empty()) return false;

    aabb temp_box;
    bool first_box = true;

    for (const auto& object : objects) {
        if (!object->bounding_box(time0, time1, temp_box)) return false;
        output_box = first_box ? temp_box : surrounding_box(output_box, temp_box);
        first_box = false;
    }

    return true;
}

int hittable_list::gui_edit(int idx){
    std::cout << "GUI HITTABLE_LIST\n";
    // std::string s = "Hittable List - " + std::to_string(idx);
    // const char *cs = s.c_str();
    // if (ImGui::TreeNode(cs)){
    //     
    //     ImGui::TreePop();
    // }
    return 0;
}
