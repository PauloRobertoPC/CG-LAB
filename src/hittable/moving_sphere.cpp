#include "moving_sphere.hpp"


point3 moving_sphere::center(double time) const {
    return center0 + ((time - time0) / (time1 - time0))*(center1 - center0);
}

bool moving_sphere::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    vec3 oc = r.origin() - center(r.time());
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
    auto outward_normal = (rec.p - center(r.time())) / radius;
    rec.set_face_normal(r, outward_normal);
    rec.mat_ptr = mat_ptr;

    return true;
}

bool moving_sphere::bounding_box(double _time0, double _time1, aabb& output_box) const {
    aabb box0(
        center(_time0) - vec3(radius, radius, radius),
        center(_time0) + vec3(radius, radius, radius));
    aabb box1(
        center(_time1) - vec3(radius, radius, radius),
        center(_time1) + vec3(radius, radius, radius));
    output_box = surrounding_box(box0, box1);
    return true;
}

int moving_sphere::gui_edit(int idx){
    bool remove = false;
    float CENTER0[3]; CENTER0[0] = center0[0]; CENTER0[1] = center0[1]; CENTER0[2] = center0[2];
    float CENTER1[3]; CENTER1[0] = center1[0]; CENTER1[1] = center1[1]; CENTER1[2] = center1[2];
    float t0[1] = {0}, t1[1] = {1}, r[1] = {1};
    std::string s = "Moving Sphere - " + std::to_string(idx);
    const char *cs = s.c_str();
    if (ImGui::TreeNode(cs)){
        if(ImGui::InputFloat3("Center 0", CENTER0)){
            center0[0] = CENTER0[0];
            center0[1] = CENTER0[1];
            center0[2] = CENTER0[2];
        }
        if(ImGui::InputFloat3("Center 1", CENTER1)){
            center1[0] = CENTER1[0];
            center1[1] = CENTER1[1];
            center1[2] = CENTER1[2];
        }
        if(ImGui::InputFloat("Time 0", t0))
            time0 = t0[0];
        if(ImGui::InputFloat("Time 1", t1))
            time1 = t1[0];
        if(ImGui::InputFloat("Radius", r))
            radius = r[0];
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
        if (ImGui::Button("Remove"))
            remove = true;
        ImGui::PopStyleColor(1);
        ImGui::TreePop();
    }
    return remove;
}
