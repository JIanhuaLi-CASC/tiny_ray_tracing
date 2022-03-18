#define STB_IMAGE_WRITE_IMPLEMENTATION
#define _CRT_SECURE_NO_DEPRECATE


#include "stb_image_write.h"

#include <tuple>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cmath>

class vec3//默认float 
{
public:
	vec3(float x, float y, float z) : x(x), y(y), z(z) {};
	float x = 0, y = 0, z = 0;
	float& operator[](const int i) { return i == 0 ? x : (1 == i ? y : z); }
	const float& operator[](const int i) const { return i == 0 ? x : (1 == i ? y : z); }
	vec3  operator*(const float v) const { return{ x*v, y*v, z*v }; }
	float operator*(const vec3& v) const { return x*v.x + y*v.y + z*v.z; }
	vec3  operator+(const vec3& v) const { return{ x + v.x, y + v.y, z + v.z }; }
	vec3  operator-(const vec3& v) const { return{ x - v.x, y - v.y, z - v.z }; }
	vec3  operator-()              const { return{ -x, -y, -z }; }
	float norm() const { return std::sqrt(x*x + y*y + z*z); }
	vec3 normalized() const { return (*this)*(1.f / norm()); }
};

class color//默认float 
{
public:
	color(unsigned char R, unsigned char y, unsigned char z) : x(x), y(y), z(z) {};
	unsigned char x = 0, y = 0, z = 0;
	unsigned char& operator[](const int i) { return i == 0 ? x : (1 == i ? y : z); }
	const unsigned char& operator[](const int i) const { return i == 0 ? x : (1 == i ? y : z); }
};




void main()
{
	const int height = 200;
	const int width = 200;
	const int comp = 3;//每个像素的通道数
	char* filename = "test.jpg";
	unsigned char* framebuffer;
	framebuffer=(unsigned char *)malloc(height*height*comp);
	int index = 0;
	//std::vector<color> framebuffer(height*width);
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
		{			
			framebuffer[index++] = i % 255;
			framebuffer[index++] = j % 255;
			framebuffer[index++] = 0;
		}
			
	stbi_write_jpg("test.jpg", width, height, comp, framebuffer, 100);
}
