#pragma once

#include <iostream>
#include <vector>
#include <array>
#include "rtweekend.hpp"

using namespace std;

vector<vector<float>> gaussianKernel(int dimension, float sigma);

bool in(int i, int j, int width, int height);

vector<float> get_pixel(vector<float> &image, int i, int j, int width, int height);

vector<float> convolution_pixel(vector<float> &image, vector<vector<float>> &kernell, int i, int j, int width, int height);

void convolution(vector<float> &image, int width, int height, vector<vector<float>> &kernell);

void gaussian_filter(vector<float> &image, int width, int height, int dimension, float sigma);

void median_filter(vector<float> &image, int width, int height, int dimension, float sigma);
