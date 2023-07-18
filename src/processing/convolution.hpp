#pragma once

#include <iostream>
#include <vector>
#include <array>
#include "rtweekend.hpp"

using namespace std;

vector<vector<float>> gaussianKernel(int dimension, float sigma);

bool in(int i, int j, int width, int height);

vector<float> get_pixel(vector<float> &image, int i, int j, int width, int height);

vector<vector<vector<float>>> get_pixels_window(vector<float> &image, int i, int j, int dimension_window, int width_image, int height_image);

vector<float> convolution_pixel(vector<float> &image, vector<vector<float>> &kernell, int i, int j, int width, int height);

void convolution(vector<float> &image, int width, int height, vector<vector<float>> &kernell);

void gaussian_filter(vector<float> &image, int width, int height, int dimension, float sigma);

vector<float> median_pixel(vector<vector<vector<float>>> &pxs);

void median_filter(vector<float> &image, int width, int height, int dimension);

void mean_filter(vector<float> &image, int width, int height, int dimension);
