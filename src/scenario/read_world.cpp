#include "read_world.hpp"

#include "hittable.hpp"
#include "aarect.hpp"
#include "box.hpp"
#include "constant_medium.hpp"
#include "hittable.hpp"
#include "moving_sphere.hpp"
#include "sphere.hpp"
#include "texture.hpp"
#include "checker_texture.hpp"
#include "image_texture.hpp"
#include "noise_texture.hpp"
#include "solid_color.hpp"
#include "material.hpp"

using namespace std;

color toColor(vector<float> &vf){
    return color(vf[0]/255.0, vf[1]/255.0, vf[2]/255.0);
}

color toVec3(vector<float> &vf){
    return vec3(vf[0], vf[1], vf[2]);
}

vector<shared_ptr<texture>> read_texture(json &j){
    vector<shared_ptr<texture>> v;
    for(auto t:j["texture"]){
        if(t["type"] == "checker"){
            vector<float> color1 = t["color1"];
            vector<float> color2 = t["color2"];
            v.push_back(make_shared<checker_texture>(toColor(color1), toColor(color2)));
        }else if(t["type"] == "image"){
            string fs = t["filename"];
            const char *filename = fs.c_str();
            v.push_back(make_shared<image_texture>(filename));
        }else if(t["type"] == "noise"){
            v.push_back(make_shared<noise_texture>(t["scale"]));
        }else if(t["type"] == "color"){
            vector<float> color = t["color"];
            v.push_back(make_shared<solid_color>(toColor(color)));
        }
    }
    return v;
}

vector<shared_ptr<material>> read_material(json &j, vector<shared_ptr<texture>> &textures){
    vector<shared_ptr<material>> v;
    for(auto m:j["material"]){
        if(m["type"] == "lambertian"){
            v.push_back(make_shared<lambertian>(textures[m["texture"]]));
        }else if(m["type"] ==  "metal"){
            vector<float> color = m["color"];
            v.push_back(make_shared<metal>(toColor(color), m["fuzz"]));
        }else if(m["type"] == "dieletric"){
            v.push_back(make_shared<dielectric>(m["index_refraction"]));
        }else if(m["type"] == "light"){
            v.push_back(make_shared<diffuse_light>(textures[m["texture"]]));
        }else if(m["type"] == "isotropic"){
            v.push_back(make_shared<isotropic>(textures[m["texture"]]));
        }
    }
    return v;
}

hittable_list read_hittable(json &j, vector<shared_ptr<texture>> &textures, vector<shared_ptr<material>> &materials){
    hittable_list w;
    vector<shared_ptr<hittable>> aux;
    for(auto h:j["hittable"]){
        if(h["type"] == "sphere"){
            vector<float> center = h["center"];
            float radius = h["radius"];
            aux.push_back(make_shared<sphere>(toVec3(center), radius, materials[h["material"]]));
        }else if(h["type"] ==  "moving_sphere"){
            vector<float> center0 = h["center0"];
            vector<float> center1 = h["center1"];
            float time0 = h["time0"];
            float time1 = h["time1"];
            float radius = h["radius"];
            aux.push_back(make_shared<moving_sphere>(toVec3(center0), toVec3(center1), time0, time1, radius, materials[h["material"]]));
        }else if(h["type"] == "box"){
            vector<float> start = h["start"];
            vector<float> end = h["end"];
            aux.push_back(make_shared<box>(toVec3(start), toVec3(end), materials[h["material"]]));
        }else if(h["type"] == "constant_medium"){
            float density = h["density"];
            aux.push_back(make_shared<constant_medium>(aux[h["hittable"]], density, textures[h["texture"]]));
        }else if(h["type"] == "xy_rect"){
            aux.push_back(make_shared<xy_rect>(h["x0"], h["x1"], h["y0"], h["y1"], h["k"], materials[h["material"]]));
        }else if(h["type"] == "xz_rect"){
            aux.push_back(make_shared<xz_rect>(h["x0"], h["x1"], h["z0"], h["z1"], h["k"], materials[h["material"]]));
        }else if(h["type"] == "yz_rect"){
            aux.push_back(make_shared<yz_rect>(h["y0"], h["y1"], h["z0"], h["z1"], h["k"], materials[h["material"]]));
        }else if(h["type"] == "translate"){
            vector<float> dsp = h["displacement"];
            aux.push_back(make_shared<translate>(aux[h["hittable"]], toVec3(dsp)));
        }else if(h["type"] == "rotate_y"){
            aux.push_back(make_shared<rotate_y>(aux[h["hittable"]], h["angle"]));
        }
    }
    int counter = 0;
    for(auto h:j["hittable"]){
        bool show = h["show"];
        if(show)
            w.add(aux[counter]);
        ++counter;
    }
    return w;
}

hittable_list read_world(json &j){
    vector<shared_ptr<texture>> textures = read_texture(j);
    vector<shared_ptr<material>> materials = read_material(j, textures);
    return read_hittable(j, textures, materials);
}
