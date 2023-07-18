#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <cstring>
#include <iostream>
#include <memory>
#include <thread>
#include <pthread.h>
#include <chrono>
#include <vector>
#include <iomanip>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>

#include "read_world.hpp"
#include "aabb.hpp"
#include "rtweekend.hpp"
#include "color.hpp"
#include "hittable.hpp"
#include "hittable_list.hpp"
#include "sphere.hpp"
#include "camera.hpp"
#include "material.hpp"
#include "moving_sphere.hpp"
#include "aarect.hpp"
#include "box.hpp"
#include "constant_medium.hpp"
#include "bvh.hpp"
#include "texture.hpp"
#include "solid_color.hpp"
#include "checker_texture.hpp"
#include "image_texture.hpp"
#include "noise_texture.hpp"
#include "canvas.h"
#include "convolution.hpp"
#include "stb_image_write.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "vec3.hpp"

#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
    #include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
    #pragma comment(lib, "legacy_stdio_definitions")
#endif

#ifdef __EMSCRIPTEN__
    #include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

// %% INICIO

bool rendering, stop;
std::vector<float> framebuffer;

int image_width;
int image_height;
int samples_per_pixel;
int max_depth;
color background;
hittable_list world;
camera cam;

//Render

color ray_color(const ray& r, const color& background, const hittable& world, int depth) {
    hit_record rec;

    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
        return color(0,0,0);

    // If the ray hits nothing, return the background color.
    if (!world.hit(r, 0.001, infinity, rec))
        return background;

    ray scattered;
    color attenuation;
    color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);

    if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
        return emitted;

    return emitted + attenuation * ray_color(scattered, background, world, depth-1);
}

double duration = 0.0;
void render(){
    auto startTime = std::chrono::high_resolution_clock::now();
    for (int j = image_height-1, px = 0; j >= 0 && !stop; --j) {
        std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < image_width && !stop; ++i, px += 4) {
            color pixel_color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel && !stop; ++s) {
                auto u = (i + random_double()) / (image_width-1);
                auto v = (j + random_double()) / (image_height-1);
                ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, background, world, max_depth);
            }
            color c = write_color(pixel_color, samples_per_pixel);
            framebuffer[px] = c[0]; framebuffer[px+1] = c[1]; framebuffer[px+2] = c[2]; framebuffer[px+3] = 1;
        }
    }

    stop = false;
    rendering = false;
    std::cerr << "\nDone.\n";

    auto endTime = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()/1000000.0;
}


void save_image(const char* image_name){
    int CHANNEL_NUM = 3;
	int width = image_width, height = image_height;
  	int8_t *imageW = new int8_t[width * height * CHANNEL_NUM];
	for(int i = 0, c1 = 0, c2 = 0; i < height; i++)
		for(int j = 0; j < width; j++, c1 += 3, c2 += 4)
			imageW[c1] = framebuffer[c2]*255, imageW[c1+1] = framebuffer[c2+1]*255, imageW[c1+2] = framebuffer[c2+2]*255;
	stbi_write_png(image_name, width, height, CHANNEL_NUM, imageW, width * CHANNEL_NUM);
}

vec3 toVec3(float *v4){
    return vec3(v4[0], v4[1], v4[2]);
}

vec3 toVec3(ImVec4 v4){
    return vec3(v4.x, v4.y, v4.z);
}

//HITTABLE
const char *used_hittable = "Box";
static float position0[4] = {0., 0., 0., 0.};
static float position1[4] = {0., 0., 0., 0.};
static float radius[1] = {0};
static float time0_hit[1] = {0.0};
static float time1_hit[1] = {1.0};
static float density[1] = {1.0};
static float X0[1] = {0}, X1[1] = {1}, Y0[1] = {0}, Y1[1] = {1}, Z0[1] = {0}, Z1[1] = {1}, K[1] = {0};
//MATERIAL
const char *used_material = "Lambertian";
static float fuzz[1] = {0};
static float index_refraction[1] = {0};
//TEXTURE
static ImVec4 checker1 = ImVec4(1., 0., 0., 1.);
static ImVec4 checker2 = ImVec4(1., 1., 0., 1.);
static char filename[128] = "./texture_image/";
static float noise_scale[1] = {0};
static ImVec4 color_texture = ImVec4(0., 1., 0., 1.);
const char *used_texture = "Checker";
//OPTIONS
static char filename_scene[128] = "./scenarios/";
static char filename_image[128] = "./images/";
int dimension[1] = {5};
float sigma[1] = {1.5};

shared_ptr<texture> get_texture(){
    shared_ptr<texture> t;
    if(strcmp(used_texture, "Checker") == 0)
        t = make_shared<checker_texture>(toVec3(checker1), toVec3(checker2));
    else if(strcmp(used_texture, "Image") == 0)
        t = make_shared<image_texture>(filename);
    else if(strcmp(used_texture, "Noise") == 0)
        t = make_shared<noise_texture>(noise_scale[0]);
    else if(strcmp(used_texture, "Color") == 0)
        t = make_shared<solid_color>(toVec3(color_texture));
    return t;
}

shared_ptr<material> get_material(){
    shared_ptr<material> m;
    if(strcmp(used_material, "Lambertian") == 0)
        m = make_shared<lambertian>(get_texture());
    else if(strcmp(used_material, "Metal") == 0)
        m = make_shared<metal>(toVec3(color_texture), fuzz[0]);
    else if(strcmp(used_material, "Dieletric") == 0)
        m = make_shared<dielectric>(index_refraction[0]);
    else if(strcmp(used_material, "Light") == 0)
        m = make_shared<diffuse_light>(get_texture());
    else if(strcmp(used_material, "Isotropic") == 0)
        m = make_shared<isotropic>(get_texture());
    return m;
}

shared_ptr<hittable> get_hittable(){
    shared_ptr<hittable> h;
    if(strcmp(used_hittable, "Sphere") == 0)
        h = make_shared<sphere>(toVec3(position0), radius[0], get_material());
    else if(strcmp(used_hittable, "MSphere") == 0)
        h = make_shared<moving_sphere>(toVec3(position0), toVec3(position1), time0_hit[0], time1_hit[0], radius[0], get_material());
    else if(strcmp(used_hittable, "Box") == 0)
        h = make_shared<box>(toVec3(position0), toVec3(position1), get_material());
    else if(strcmp(used_hittable, "XY Rect") == 0)
        h = make_shared<xy_rect>(X0[0], X1[0], Y0[0], Y1[0], K[0], get_material());
    else if(strcmp(used_hittable, "XZ Rect") == 0)
        h = make_shared<xz_rect>(X0[0], X1[0], Z0[0], Z1[0], K[0], get_material());
    else if(strcmp(used_hittable, "YZ Rect") == 0)
        h = make_shared<yz_rect>(Y0[0], Y1[0], Z0[0], Z1[0], K[0], get_material());
    return h;
}

//IMAGE
static ImVec4 background_input = ImVec4(1.0, 1.0, 1.0, 1.0);
static int image_width_input[1] = {500};
static int aspect_ratio_fraction[2] = {1, 1};
//CAMERA
static float lookfrom[4] = { 0., 0., 0., 0. };
static float lookat[4] = { 0., 0., -1.0, 0. };
static float vup[4] = { 0., 1., .0, 0. };
static float vfov[1] = {40.0};
static float aperture[1] = {0.0};
static float dist_to_focus[1] = {10.0};
static float time0[1] = {0.0};
static float time1[1] = {1.0};
//Render Option
static int samples_per_pixel_input[1] = {100};
static int max_depth_input[1] = {50};

hittable_list read_scene(const char* filename){
    std::ifstream file(filename);
    json j; file >> j;

    std::vector<float> bg = j["background"]; background_input.x = bg[0]/255.0; background_input.y = bg[1]/255.0; background_input.z = bg[2]/255.0;
    float wd = j["image_width"]; image_width_input[0] = wd;
    std::vector<float> ap = j["aspect_ratio"]; aspect_ratio_fraction[0] = ap[0]; aspect_ratio_fraction[1] = ap[1];

    std::vector<float> lf = j["lookfrom"]; lookfrom[0] = lf[0]; lookfrom[1] = lf[1]; lookfrom[2] = lf[2];
    std::vector<float> la = j["lookat"]; lookat[0] = la[0]; lookat[1] = la[1]; lookat[2] = la[2];
    std::vector<float> vu = j["vup"]; vup[0] = vu[0]; vup[1] = vu[1]; vup[2] = vu[2];
    float vf = j["vfov"]; vfov[0] = vf;
    float apr = j["aperture"]; aperture[0] = apr;
    float df = j["dist_to_focus"]; dist_to_focus[0] = df;
    float t0 = j["time0"]; time0[0] = t0;
    float t1 = j["time1"]; time1[0] = t1;

    float sp = j["samples"]; samples_per_pixel_input[0] = sp;
    float dp = j["depth"]; max_depth_input[0] = dp;

    hittable_list hl = read_world(j);

    file.close();
    return hl;

}

// %% FIM

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Main code
int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    #if defined(IMGUI_IMPL_OPENGL_ES2)
        const char* glsl_version = "#version 100";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    #elif defined(__APPLE__)
        const char* glsl_version = "#version 150";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
    #else
    const char* glsl_version = "#version 130";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
    #endif

    GLFWwindow* window = glfwCreateWindow(1200, 800, "Computer Graphics Laboratory", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // ImVec4 clear_color = ImVec4(1.f, 1.f, 0.60f, 1.00f);
    ImVec4 clear_color = ImVec4(0.f, 0.f, 0.f, 1.00f);

    //Image Texture
    GLuint framebufferTexture;
    glGenTextures(1, &framebufferTexture);
    glBindTexture(GL_TEXTURE_2D, framebufferTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // %%

    //Instanciation
    auto aspect_ratio = (1.0*aspect_ratio_fraction[0]) / (1.0*aspect_ratio_fraction[1]);
    background = toVec3(background_input);
    samples_per_pixel = samples_per_pixel_input[0];
    max_depth = max_depth_input[0];
    image_width = 500;
    image_height = static_cast<int>(image_width / aspect_ratio);
    framebuffer.assign(image_width*image_height*4, 0.0);
    cam = camera(toVec3(lookfrom), toVec3(lookat), toVec3(vup), vfov[0], aspect_ratio, aperture[0], dist_to_focus[0], time0[0], time1[0]);

#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window))
#endif
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();
//
        // SETTINGS
        {
            ImGui::SetNextWindowPos(ImVec2(image_width, 0));
            // ImGui::SetNextWindowSize(ImVec2(400, image_height+16));
            ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoTitleBar);
            ImGui::SeparatorText("Settings");
            if (ImGui::Button("Render")){
                if(rendering){
                    std::cout << "rendering yet\n";
                }else{
                    std::cout << "start rendering\n";
                    // Setting things 
                    background = toVec3(background_input);
                    samples_per_pixel = samples_per_pixel_input[0];
                    max_depth = max_depth_input[0];
                    image_width = image_width_input[0];
                    aspect_ratio = (1.0*aspect_ratio_fraction[0]) / (1.0*aspect_ratio_fraction[1]);
                    image_height = static_cast<int>(image_width / aspect_ratio);
                    framebuffer.assign(image_width*image_height*4, 0.0);
                    cam = camera(toVec3(lookfrom), toVec3(lookat), toVec3(vup), vfov[0], aspect_ratio, aperture[0], dist_to_focus[0], time0[0], time1[0]);

                    rendering = true;
                    std::thread render_thread(render);

                    render_thread.detach();
                }
                std::cout << "render button clicked\n";
            }
            if(rendering){
                ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
                if (ImGui::Button("Stop Rendering")){
                    stop = true;
                }
                ImGui::PopStyleColor(1);
            }
            ImGui::Text("Rendering Time: %0.4fs", duration);



            ImGui::SeparatorText("Edit Scene");
            if (ImGui::CollapsingHeader("Image")){
                ImGui::ColorEdit3("Background##1", (float*)&background_input, 0);
                ImGui::InputInt("Image Width", image_width_input);
                ImGui::InputInt2("Aspect Ratio", aspect_ratio_fraction);
            }
            if (ImGui::CollapsingHeader("Camera")){
                ImGui::InputFloat3("Look From", lookfrom);
                ImGui::InputFloat3("Look At", lookat);
                ImGui::InputFloat3("View Up", vup);
                ImGui::InputFloat("VFOV", vfov);
                ImGui::InputFloat("Aperture", aperture);
                ImGui::InputFloat("Dist to Focus", dist_to_focus);
                ImGui::InputFloat("Time 0", time0);
                ImGui::InputFloat("Time 1", time1);
            }
            if (ImGui::CollapsingHeader("Tracing")){
                ImGui::InputInt("Samples Per Pixel", samples_per_pixel_input);
                ImGui::InputInt("Max Recursion Depth", max_depth_input);
            }
            if (ImGui::CollapsingHeader("Edit Hittables")){
                for(int i = 0; i < world.objects.size(); i++){
                    auto h = world.objects[i];
                    if(h->gui_edit(i)){
                        // Doing a trick to remove in O(1)
                        swap(world.objects[i], world.objects.back());
                        world.objects.pop_back();
                        --i;
                    }
                }
            }
            if (ImGui::CollapsingHeader("Load Scene")){
                ImGui::InputText("File", filename_scene, IM_ARRAYSIZE(filename_scene));
                if(ImGui::Button("Load")){
                    if(rendering)
                        std::cout << "Cannot load scene, render in progress\n";
                    else
                        world = read_scene(filename_scene);
                }
            }
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
            if (ImGui::Button("Clean Scene"))
                world.objects.clear();
            ImGui::PopStyleColor(1);



            ImGui::SeparatorText("Add Scene");
            if (ImGui::CollapsingHeader("Hittable")){
                if (ImGui::TreeNode("Sphere")){
                    ImGui::Button("Sphere", ImVec2(60, 60));
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)){
                        const char* hittable = "Sphere";
                        ImGui::SetDragDropPayload("HITTABLE", &hittable, strlen(hittable)*8);
                        ImGui::Text("Copy %s", hittable);
                        ImGui::EndDragDropSource();
                    }
                    ImGui::SeparatorText("Drag the Hittable Above");
                    ImGui::InputFloat3("Position", position0);
                    ImGui::InputFloat("Radius", radius);
                    ImGui::Button(used_material, ImVec2(60, 60));
                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Material"))
                            used_material = *(const char**)payload->Data;
                        ImGui::EndDragDropTarget();
                    }
                    ImGui::SameLine(); ImGui::Text("Material");
                    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(2 / 7.0f, 0.6f, 0.6f));
                    if(ImGui::Button("Add Sphere")){
                        world.add(make_shared<sphere>(toVec3(position0), radius[0], get_material()));
                        std::cout << "SPHERE ADDED\n";
                    }
                    ImGui::PopStyleColor(1);
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Moving Sphere")){
                    ImGui::Button("MSphere", ImVec2(60, 60));
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)){
                        const char* hittable = "MSphere";
                        ImGui::SetDragDropPayload("HITTABLE", &hittable, strlen(hittable)*8);
                        ImGui::Text("Copy %s", hittable);
                        ImGui::EndDragDropSource();
                    }
                    ImGui::SeparatorText("Drag the Hittable Above");
                    ImGui::InputFloat3("Position 0", position0);
                    ImGui::InputFloat3("Position 1", position1);
                    ImGui::InputFloat("Time 0", time0_hit);
                    ImGui::InputFloat("Time 1", time1_hit);
                    ImGui::InputFloat("Radius", radius);
                    ImGui::Button(used_material, ImVec2(60, 60));
                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Material"))
                            used_material = *(const char**)payload->Data;
                        ImGui::EndDragDropTarget();
                    }
                    ImGui::SameLine(); ImGui::Text("Material");
                    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(2 / 7.0f, 0.6f, 0.6f));
                    if(ImGui::Button("Add Moving Sphere"))
                        world.add(make_shared<moving_sphere>(toVec3(position0), toVec3(position1), time0_hit[0], time1_hit[0], radius[0], get_material()));
                    ImGui::PopStyleColor(1);
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Rect")){
                    if (ImGui::TreeNode("XY")){
                        ImGui::Button("XY Rect", ImVec2(60, 60));
                        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)){
                            const char* hittable = "Box";
                            ImGui::SetDragDropPayload("HITTABLE", &hittable, strlen(hittable)*8);
                            ImGui::Text("Copy %s", hittable);
                            ImGui::EndDragDropSource();
                        }
                        ImGui::SeparatorText("Drag the Hittable Above");
                        ImGui::InputFloat("x0", X0);
                        ImGui::InputFloat("x1", X1);
                        ImGui::InputFloat("y0", Y0);
                        ImGui::InputFloat("y1", Y1);
                        ImGui::InputFloat("k", K);
                        ImGui::Button(used_material, ImVec2(60, 60));
                        if (ImGui::BeginDragDropTarget())
                        {
                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Material"))
                                used_material = *(const char**)payload->Data;
                            ImGui::EndDragDropTarget();
                        }
                        ImGui::SameLine(); ImGui::Text("Material");
                        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(2 / 7.0f, 0.6f, 0.6f));
                        if(ImGui::Button("Add XY_Rect"))
                            world.add(make_shared<xy_rect>(X0[0], X1[0], Y0[0], Y1[0], K[0], get_material()));
                        ImGui::PopStyleColor(1);
                        ImGui::TreePop();
                    }
                    if (ImGui::TreeNode("XZ")){
                        ImGui::Button("XZ Rect", ImVec2(60, 60));
                        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)){
                            const char* hittable = "Box";
                            ImGui::SetDragDropPayload("HITTABLE", &hittable, strlen(hittable)*8);
                            ImGui::Text("Copy %s", hittable);
                            ImGui::EndDragDropSource();
                        }
                        ImGui::SeparatorText("Drag the Hittable Above");
                        ImGui::InputFloat("x0", X0);
                        ImGui::InputFloat("x1", X1);
                        ImGui::InputFloat("z0", Z0);
                        ImGui::InputFloat("z1", Z1);
                        ImGui::InputFloat("k", K);
                        ImGui::Button(used_material, ImVec2(60, 60));
                        if (ImGui::BeginDragDropTarget())
                        {
                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Material"))
                                used_material = *(const char**)payload->Data;
                            ImGui::EndDragDropTarget();
                        }
                        ImGui::SameLine(); ImGui::Text("Material");
                        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(2 / 7.0f, 0.6f, 0.6f));
                        if(ImGui::Button("Add XZ_Rect"))
                            world.add(make_shared<xz_rect>(X0[0], X1[0], Z0[0], Z1[0], K[0], get_material()));
                        ImGui::PopStyleColor(1);
                        ImGui::TreePop();
                    }
                    if (ImGui::TreeNode("YZ")){
                        ImGui::Button("YZ Rect", ImVec2(60, 60));
                        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)){
                            const char* hittable = "Box";
                            ImGui::SetDragDropPayload("HITTABLE", &hittable, strlen(hittable)*8);
                            ImGui::Text("Copy %s", hittable);
                            ImGui::EndDragDropSource();
                        }
                        ImGui::SeparatorText("Drag the Hittable Above");
                        ImGui::InputFloat("y0", Y0);
                        ImGui::InputFloat("y1", Y1);
                        ImGui::InputFloat("z0", Z0);
                        ImGui::InputFloat("z1", Z1);
                        ImGui::InputFloat("k", K);
                        ImGui::Button(used_material, ImVec2(60, 60));
                        if (ImGui::BeginDragDropTarget())
                        {
                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Material"))
                                used_material = *(const char**)payload->Data;
                            ImGui::EndDragDropTarget();
                        }
                        ImGui::SameLine(); ImGui::Text("Material");
                        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(2 / 7.0f, 0.6f, 0.6f));
                        if(ImGui::Button("Add YZ_Rect"))
                            world.add(make_shared<yz_rect>(Y0[0], Y1[0], Z0[0], Z1[0], K[0], get_material()));
                        ImGui::PopStyleColor(1);
                        ImGui::TreePop();
                    }

                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Box")){
                    ImGui::Button("Box", ImVec2(60, 60));
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)){
                        const char* hittable = "Box";
                        ImGui::SetDragDropPayload("HITTABLE", &hittable, strlen(hittable)*8);
                        ImGui::Text("Copy %s", hittable);
                        ImGui::EndDragDropSource();
                    }
                    ImGui::SeparatorText("Drag the Hittable Above");
                    ImGui::InputFloat3("Start Position", position0);
                    ImGui::InputFloat3("End Position", position1);
                    ImGui::Button(used_material, ImVec2(60, 60));
                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Material"))
                            used_material = *(const char**)payload->Data;
                        ImGui::EndDragDropTarget();
                    }
                    ImGui::SameLine(); ImGui::Text("Material");
                    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(2 / 7.0f, 0.6f, 0.6f));
                    if(ImGui::Button("Add Box"))
                        world.add(make_shared<box>(toVec3(position0), toVec3(position1), get_material()));
                    ImGui::PopStyleColor(1);
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Constant Medium")){
                    ImGui::Button(used_hittable, ImVec2(60, 60));
                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HITTABLE"))
                            used_hittable = *(const char**)payload->Data;
                        ImGui::EndDragDropTarget();
                    }
                    ImGui::SameLine(); ImGui::Text("Hittable");
                    ImGui::InputFloat("Density", density);
                    ImGui::Button(used_texture, ImVec2(60, 60));
                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE"))
                            used_texture = *(const char**)payload->Data;
                        ImGui::EndDragDropTarget();
                    }
                    ImGui::SameLine(); ImGui::Text("Texture");
                    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(2 / 7.0f, 0.6f, 0.6f));
                    if(ImGui::Button("Add Constant Medium"))
                        world.add(make_shared<constant_medium>(get_hittable(), density[0], get_texture()));
                    ImGui::PopStyleColor(1);
                    ImGui::TreePop();
                }
            }
            if (ImGui::CollapsingHeader("Material")){
                ImGui::SeparatorText("Used Material");
                ImGui::Button(used_material, ImVec2(60, 60));
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MATERIAL"))
                        used_material = *(const char**)payload->Data;
                    ImGui::EndDragDropTarget();
                }
                ImGui::SeparatorText("Drag and Drop Above");
                //Lambertian
                ImGui::Button("Lambertian", ImVec2(60, 60));
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)){
                    const char* material = "Lambertian";
                    ImGui::SetDragDropPayload("MATERIAL", &material, strlen(material)*8);
                    ImGui::Text("Copy %s", material);
                    ImGui::EndDragDropSource();
                }
                ImGui::SameLine();
                ImGui::Button(used_texture, ImVec2(60, 60));
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE"))
                        used_texture = *(const char**)payload->Data;
                    ImGui::EndDragDropTarget();
                }
                ImGui::SameLine(); ImGui::Text("Texture");
                //Metal
                ImGui::Button("Metal", ImVec2(60, 60));
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)){
                    const char* material = "Metal";
                    ImGui::SetDragDropPayload("MATERIAL", &material, strlen(material)*8);
                    ImGui::Text("Copy %s", material);
                    ImGui::EndDragDropSource();
                }
                ImGui::SameLine();
                ImGui::ColorEdit3("Color Texture##3", (float*)&color_texture, ImGuiColorEditFlags_NoInputs);
                ImGui::SameLine(); ImGui::PushItemWidth(100);
                ImGui::InputFloat("Fuzz", fuzz);
                //Dieletric 
                ImGui::Button("Dieletric", ImVec2(60, 60));
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)){
                    const char* material = "Dieletric";
                    ImGui::SetDragDropPayload("MATERIAL", &material, strlen(material)*8);
                    ImGui::Text("Copy %s", material);
                    ImGui::EndDragDropSource();
                }
                ImGui::SameLine(); ImGui::PushItemWidth(100);
                ImGui::InputFloat("Index of Refraction", index_refraction);
                //Light
                ImGui::Button("Light", ImVec2(60, 60));
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)){
                    const char* material = "Light";
                    ImGui::SetDragDropPayload("MATERIAL", &material, strlen(material)*8);
                    ImGui::Text("Copy %s", material);
                    ImGui::EndDragDropSource();
                }
                ImGui::SameLine();
                ImGui::Button(used_texture, ImVec2(60, 60));
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE"))
                        used_texture = *(const char**)payload->Data;
                    ImGui::EndDragDropTarget();
                }
                ImGui::SameLine(); ImGui::Text("Texture");
                //Isotropic
                ImGui::Button("Isotropic", ImVec2(60, 60));
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)){
                    const char* material = "Isotropic";
                    ImGui::SetDragDropPayload("MATERIAL", &material, strlen(material)*8);
                    ImGui::Text("Copy %s", material);
                    ImGui::EndDragDropSource();
                }
                ImGui::SameLine();
                ImGui::Button(used_texture, ImVec2(60, 60));
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE"))
                        used_texture = *(const char**)payload->Data;
                    ImGui::EndDragDropTarget();
                }
                ImGui::SameLine(); ImGui::Text("Texture");
            }
            if (ImGui::CollapsingHeader("Texture")){
                //Used Texture
                ImGui::SeparatorText("Used Texture");
                ImGui::Button(used_texture, ImVec2(60, 60));
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE"))
                        used_texture = *(const char**)payload->Data;
                    ImGui::EndDragDropTarget();
                }
                ImGui::SeparatorText("Drag and Drop Above");
                //Checker Texture 
                ImGui::Button("Checker", ImVec2(60, 60));
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)){
                    const char* texture = "Checker";
                    ImGui::SetDragDropPayload("TEXTURE", &texture, strlen(texture)*8);
                    ImGui::Text("Copy %s", texture);
                    ImGui::EndDragDropSource();
                }
                ImGui::SameLine();
                ImGui::ColorEdit3("Color 1##2", (float*)&checker1, ImGuiColorEditFlags_NoInputs);
                ImGui::SameLine();
                ImGui::ColorEdit3("Color 2##3", (float*)&checker2, ImGuiColorEditFlags_NoInputs);
                //Image Texture 
                ImGui::Button("Image", ImVec2(60, 60));
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)){
                    const char* texture = "Image";
                    ImGui::SetDragDropPayload("TEXTURE", &texture, strlen(texture)*8);
                    ImGui::Text("Copy %s", texture);
                    ImGui::EndDragDropSource();
                }
                ImGui::SameLine();
                ImGui::InputText("File", filename, IM_ARRAYSIZE(filename));
                //Noise Texture 
                ImGui::Button("Noise", ImVec2(60, 60));
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)){
                    const char* texture = "Noise";
                    ImGui::SetDragDropPayload("TEXTURE", &texture, strlen(texture)*8);
                    ImGui::Text("Copy %s", texture);
                    ImGui::EndDragDropSource();
                }
                ImGui::SameLine();
                ImGui::InputFloat("Scale", noise_scale);
                //Color Texture 
                ImGui::Button("Color", ImVec2(60, 60));
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)){
                    const char* texture = "Color";
                    ImGui::SetDragDropPayload("TEXTURE", &texture, strlen(texture)*8);
                    ImGui::Text("Copy %s", texture);
                    ImGui::EndDragDropSource();
                }
                ImGui::SameLine();
                ImGui::ColorEdit3("Color Texture##3", (float*)&color_texture, ImGuiColorEditFlags_NoInputs);
            }



            ImGui::SeparatorText("Image");
            if (ImGui::CollapsingHeader("Gauss Filter")){
                ImGui::InputInt("Kernell Dimension", dimension);
                ImGui::InputFloat("Sigma", sigma);
                if(ImGui::Button("Apply Filter")){
                    if(rendering){
                        std::cout << "Cannot apply filter, render in progress\n";
                    }else if(dimension[0] % 2 == 0){
                        std::cout << "Kernell Dimension must be odd\n";
                    }else{
                        convolution(framebuffer, image_width, image_height, dimension[0], sigma[0]);
                        // std::thread my_thread(convolution, framebuffer, image_width, image_height, dimension[0], sigma[0]);
                        // std::thread filter_thread(convolution, framebuffer, image_width, image_height, dimension[0], sigma[0]);
                        std::cout << "Gaussian Filter Applied\n";
                    }
                }
            }
            if (ImGui::CollapsingHeader("Save Image")){
                ImGui::InputText("File", filename_image, IM_ARRAYSIZE(filename_image));
                if(ImGui::Button("Save")){
                    if(rendering)
                        std::cout << "Cannot save image, render in progress\n";
                    else
                        save_image(filename_image);
                }
            }

            ImGui::End();
        }
        //IMAGE
        {
            // Bind the framebuffer texture
            glBindTexture(GL_TEXTURE_2D, framebufferTexture);

            // Update the texture with the latest framebuffer content
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_FLOAT, framebuffer.data());

            // Show the ImGui window with the OpenGL texture
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(image_width, image_height+16));
            ImGui::Begin("Frame Buffer", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(framebufferTexture)), ImVec2(image_width, image_height), ImVec2(0, 0), ImVec2(1, 1));
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
    #ifdef __EMSCRIPTEN__
        EMSCRIPTEN_MAINLOOP_END;
    #endif

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
