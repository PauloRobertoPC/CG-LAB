#include "sphere.hpp"
#include "imgui.h"

sphere::sphere() {}
sphere::sphere(point3 cen, double r, shared_ptr<material> m)
        : center(cen), radius(r), mat_ptr(m) {}

bool sphere::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    vec3 oc = r.origin() - center;
    auto a = r.direction().length_squared();
    auto half_b = dot(oc, r.direction());
    auto c = oc.length_squared() - radius*radius;

    auto discriminant = half_b*half_b - a*c;
    if (discriminant < 0) return false;
    auto sqrtd = sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range.
    auto root = (-half_b - sqrtd) / a;
    if (root < t_min || t_max < root) {
        root = (-half_b + sqrtd) / a;
        if (root < t_min || t_max < root)
            return false;
    }

    rec.t = root;
    rec.p = r.at(rec.t);
    vec3 outward_normal = (rec.p - center) / radius;
    rec.set_face_normal(r, outward_normal);
    get_sphere_uv(outward_normal, rec.u, rec.v);
    rec.mat_ptr = mat_ptr;

    return true;
}

bool sphere::bounding_box(double time0, double time1, aabb& output_box) const {
    output_box = aabb(
        center - vec3(radius, radius, radius),
        center + vec3(radius, radius, radius));
    return true;
}

int sphere::gui_edit(int idx){
    bool remove = false;
    float center_in[3]; center_in[0] = center[0]; center_in[1] = center[1]; center_in[2] = center[2];
    float radius_in[1]; radius_in[0] = radius;
    std::string s = "Sphere - " + std::to_string(idx);
    const char *cs = s.c_str();
    if (ImGui::TreeNode(cs)){
        if(ImGui::InputFloat3("Position", center_in)){
            center[0] = center_in[0];
            center[1] = center_in[1];
            center[2] = center_in[2];
        }
        if(ImGui::InputFloat("Radius", radius_in))
            this->radius = radius_in[0];
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
        if (ImGui::Button("Remove"))
            remove = true;
        ImGui::PopStyleColor(1);
        ImGui::TreePop();
    }
    return remove;
}
