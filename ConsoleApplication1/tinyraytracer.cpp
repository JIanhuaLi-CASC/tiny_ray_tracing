#define STB_IMAGE_WRITE_IMPLEMENTATION
#define _CRT_SECURE_NO_DEPRECATE
#define PI 3.1415926

#include "stb_image_write.h"

#include <tuple>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cmath>

class vec3//默认float 
{
public:
	vec3(float x=0, float y=0, float z=0) : x(x), y(y), z(z) {};
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
	color(unsigned char R, unsigned char G, unsigned char B) : R(R), G(G), B(B) {};
	unsigned char R = 0, G = 0, B = 0;
	unsigned char& operator[](const int i) { return i == 0 ? R : (1 == i ? G : B); }
	const unsigned char& operator[](const int i) const { return i == 0 ? R : (1 == i ? G :B); }
};

class sphere
{
public:
	//sphere();
	//~sphere();
	sphere(const vec3 &c, const float &r) : center(c), radius(r) {}
	bool ray_intersect(const vec3 &orig, const vec3 &dir, float &t0) const 
	//计算直线与球的交点。dir 必须为归一化矢量，一个从orig点发射的，方向为dir的直线，
	//与球相交点距离直线的距离为t0，这个说最近的相交点
	{
		vec3 L = center - orig;
		float tca = L*dir;
		float d2 = L*L - tca*tca;//原点距离直线的距离
		if (d2 > radius*radius) 
			return false;//直线与球一定不相交
		else
		{
			float thc = sqrtf(radius*radius - d2);//d2:直线与球心距离，thc，交点（如果有）距离最近点的距离
			t0 = tca - thc;
			float t1 = tca + thc;
			if (t0 < 0) t0 = t1;
			if (t0 < 0) return false;
			else
			{
				return true;
			}
			
		}
	
	}
private:
	vec3 center;
	float radius;
};

//渲染函数
void render() {

}

vec3 cast_ray(const vec3 &orig, const vec3 &dir, const sphere &sphere) {
	float sphere_dist = std::numeric_limits<float>::max();
	if (!sphere.ray_intersect(orig, dir, sphere_dist)) {
		return vec3(0.2, 0.7, 0.8); // background color
	}
	return vec3(0.4, 0.4, 0.3);
}



void main()
{
	const int height = 400;
	const int width = 400;
	const int comp = 3;//每个像素的通道数
	float fov = 30;//相机视角大小，相机为原点，屏幕图像为0 0 1平面，也就是说焦距为1
	float d = 2 * tan(fov / 2 / 180.0 * PI) / (float)width;//每个像素的尺寸



	sphere sph1(vec3(0,0,10),1);//第一个球

	char* filename = "step2.jpg";
	unsigned char* framebuffer;
	framebuffer=(unsigned char *)malloc(height*height*comp);
	int index = 0;
	vec3 dir,orig,color;
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
		{			
			dir = vec3((i - height / 2) * d, (j - width / 2) * d, 1);
			dir = dir.normalized();
			color = cast_ray(orig, dir, sph1);
			framebuffer[index++] = (unsigned char)(color.x * 255);
			framebuffer[index++] = (unsigned char)(color.y * 255);;
			framebuffer[index++] = (unsigned char)(color.z * 255);;
		}
			
	stbi_write_jpg("step2.jpg", width, height, comp, framebuffer, 100);
}
