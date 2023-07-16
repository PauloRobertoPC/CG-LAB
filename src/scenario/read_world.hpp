#pragma once

#include "hittable_list.hpp"
#include "json.hpp"

using json = nlohmann::json;

color toColor(std::vector<float> &vf);
hittable_list read_world(json &j);
