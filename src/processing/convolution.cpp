#include "convolution.hpp"

vector<vector<float>> gaussianKernel(int dimension, float sigma){
	if(!(dimension&1)){
		cout << "Error: dimension in Gaussian Filter must be odd\n";
		return {{}};
	}
	vector<vector<float>> kernel(dimension);
	float s = 2.0*sigma*sigma; dimension >>= 1;
	for(int i = -dimension; i <= dimension; i++)
		for(int j = -dimension; j <= dimension; j++)
			kernel[i+dimension].emplace_back((exp(-((i*i+j*j)/(s))))/(s*pi));
	return kernel;
}

bool in(int i, int j, int width, int height){
    return i >= 0 && i < height && j >= 0 && j < width;
}

vector<float> get_pixel(vector<float> &image, int i, int j, int width, int height){
    int base = (i*width + j)*4;
    return {image[base], image[base+1], image[base+2]};
}

vector<float> convolution_pixel(vector<float> &image, vector<vector<float>> &kernell, int i, int j, int width, int height){
    vector<float> px_answer(3, 0), px; 
    int step = kernell.size()/2;
    int i_min = i-step, i_max = i+step;
    int j_min = j-step, j_max = j+step;
    float weigth = 0;
    for(int x = i_min, a = 0; x <= i_max; x++, a++){
        for(int y = j_min, b = 0; y <= j_max; y++, b++){
            std::cout << x << " " << y << "\n";
            if(in(x, y, width, height)){
                px = get_pixel(image, x, y, width, height);
                for(auto &component:px) component *= kernell[a][b];
                for(int k = 0; k < 3; k++) px_answer[k] += px[k];
                weigth += kernell[a][b];
            }
        }
    }
    for(auto &component:px_answer) component /= weigth;
    for(auto &component:px_answer) component = (component > 1.0 ? 1.0 : component);
    return px;
}

void convolution(vector<float> &image, int width, int height, vector<vector<float>> &kernell){
    vector<float> aux(width*height*4, 0.0);
    swap(image, aux);
    for(int i = 0, c = 0; i < height; i++){
        for(int j = 0; j < width; j++, c += 4){
            // std::cout << i << " " << j << "\n";
            auto px = convolution_pixel(aux, kernell, i, j, width, height);
            image[c] = px[0]; image[c+1] = px[1]; image[c+2] = px[2]; image[c+3] = 1.0;
        }
    }
}

void convolution(vector<float> &image, int width, int height, int dimension, float sigma){
    vector<vector<float>> kernell = gaussianKernel(dimension, sigma);
    convolution(image, width, height, kernell);
}
