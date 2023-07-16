#include "box.hpp"

box::box(const point3& p0, const point3& p1, shared_ptr<material> ptr) {
    box_min = p0;
    box_max = p1;
    mat = ptr;
    construct_box_with_rectangles();
}

bool box::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    return sides.hit(r, t_min, t_max, rec);
}
void box::construct_box_with_rectangles(){
    sides.objects.clear();

    sides.add(make_shared<xy_rect>(box_min.x(), box_max.x(), box_min.y(), box_max.y(), box_max.z(), mat));
    sides.add(make_shared<xy_rect>(box_min.x(), box_max.x(), box_min.y(), box_max.y(), box_min.z(), mat));

    sides.add(make_shared<xz_rect>(box_min.x(), box_max.x(), box_min.z(), box_max.z(), box_max.y(), mat));
    sides.add(make_shared<xz_rect>(box_min.x(), box_max.x(), box_min.z(), box_max.z(), box_min.y(), mat));

    sides.add(make_shared<yz_rect>(box_min.y(), box_max.y(), box_min.z(), box_max.z(), box_max.x(), mat));
    sides.add(make_shared<yz_rect>(box_min.y(), box_max.y(), box_min.z(), box_max.z(), box_min.x(), mat));
}

void box::gui_edit(int idx){
    std::string s = "Box - " + std::to_string(idx);
    float start[3]; start[0] = box_min[0]; start[1] = box_min[1]; start[2] = box_min[2];
    float end[3]; end[0] = box_max[0]; end[1] = box_max[1]; end[2] = box_max[2];
    const char *cs = s.c_str();
    if (ImGui::TreeNode(cs)){
        if(ImGui::InputFloat3("Start Position", start)){
            box_min[0] = start[0];
            box_min[1] = start[1];
            box_min[2] = start[2];
            construct_box_with_rectangles();
        }
        if(ImGui::InputFloat3("End Position", end)){
            box_max[0] = end[0];
            box_max[1] = end[1];
            box_max[2] = end[2];
            construct_box_with_rectangles();
        }
        ImGui::TreePop();
    }
}
