#include "aarect.hpp"

bool xy_rect::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    auto t = (k-r.origin().z()) / r.direction().z();
    if (t < t_min || t > t_max)
        return false;
    auto x = r.origin().x() + t*r.direction().x();
    auto y = r.origin().y() + t*r.direction().y();
    if (x < x0 || x > x1 || y < y0 || y > y1)
        return false;
    rec.u = (x-x0)/(x1-x0);
    rec.v = (y-y0)/(y1-y0);
    rec.t = t;
    auto outward_normal = vec3(0, 0, 1);
    rec.set_face_normal(r, outward_normal);
    rec.mat_ptr = mp;
    rec.p = r.at(t);
    return true;
}

void xy_rect::gui_edit(int idx){
    float X0[1]; X0[0] = x0;
    float X1[1]; X1[0] = x1;
    float Y0[1]; Y0[0] = y0;
    float Y1[1]; Y1[0] = y1;
    float K[1]; K[0] = k;
    std::string s = "XY_Rect - " + std::to_string(idx);
    const char *cs = s.c_str();
    if (ImGui::TreeNode(cs)){
        // if(ImGui::InputFloat("x0", X0))
        //     this->x0 = X0[0];
        // if(ImGui::InputFloat("x1", X1))
        //     this->x1 = X1[0];
        // if(ImGui::InputFloat("y0", Y0))
        //     this->y0 = Y0[0];
        // if(ImGui::InputFloat("y1", Y1))
        //     this->y1 = Y1[0];
        // if(ImGui::InputFloat("k", K))
        //     this->k = K[0];
        ImGui::TreePop();
    }
}

bool xz_rect::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    auto t = (k-r.origin().y()) / r.direction().y();
    if (t < t_min || t > t_max)
        return false;
    auto x = r.origin().x() + t*r.direction().x();
    auto z = r.origin().z() + t*r.direction().z();
    if (x < x0 || x > x1 || z < z0 || z > z1)
        return false;
    rec.u = (x-x0)/(x1-x0);
    rec.v = (z-z0)/(z1-z0);
    rec.t = t;
    auto outward_normal = vec3(0, 1, 0);
    rec.set_face_normal(r, outward_normal);
    rec.mat_ptr = mp;
    rec.p = r.at(t);
    return true;
}

void xz_rect::gui_edit(int idx){
    std::string s = "XZ_Rect - " + std::to_string(idx);
    const char *cs = s.c_str();
    if (ImGui::TreeNode(cs)){
        
        ImGui::TreePop();
    }
}

bool yz_rect::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    auto t = (k-r.origin().x()) / r.direction().x();
    if (t < t_min || t > t_max)
        return false;
    auto y = r.origin().y() + t*r.direction().y();
    auto z = r.origin().z() + t*r.direction().z();
    if (y < y0 || y > y1 || z < z0 || z > z1)
        return false;
    rec.u = (y-y0)/(y1-y0);
    rec.v = (z-z0)/(z1-z0);
    rec.t = t;
    auto outward_normal = vec3(1, 0, 0);
    rec.set_face_normal(r, outward_normal);
    rec.mat_ptr = mp;
    rec.p = r.at(t);
    return true;
}

void yz_rect::gui_edit(int idx){
    std::string s = "YZ_Rect - " + std::to_string(idx);
    const char *cs = s.c_str();
    if (ImGui::TreeNode(cs)){
        
        ImGui::TreePop();
    }
}
