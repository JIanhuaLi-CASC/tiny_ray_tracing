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
	vec3 &operator =(const vec3 &a) 
	{
		x = a.x; y = a.y; z = a.z;
		return *this;
	}
};

class color//默认float 
{
public:
	color(float R=0, float G=0, float B=0) : R(R), G(G), B(B) {};
	//color(unsigned char R = 0, unsigned char G = 0, unsigned char B = 0) : R(R), G(G), B(B) {};
	float R = 0, G = 0, B = 0;
	color  operator*(const float v) const { return{ R*v, G*v, B*v }; }
	float& operator[](const int i) { return i == 0 ? R : (1 == i ? G : B); }
	const float& operator[](const int i) const { return i == 0 ? R : (1 == i ? G :B); }
	color &operator =(const color &a)
	{
		R = a.R; G = a.G; B = a.B;
		return *this;
	}
};

class material {
public:
	material(const color &color) : diffuse_color(color) {}
	material() : diffuse_color(0.0,0.0,0.0) {}
	color diffuse_color;
};

struct Light {
	Light(const vec3 &p, const float &i) : position(p), intensity(i) {}
	vec3 position;
	float intensity;
};


class sphere
{
public:
	//sphere();
	//~sphere();
	sphere(const vec3 &c, const float &r,const material &m) : center(c), radius(r), mat(m){}
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
	vec3 center;
	float radius;
	material mat;
};

//渲染函数
void render() {

}

bool cast_ray(const vec3 &orig, const vec3 &dir, const std::vector<sphere> &spheres, vec3 &hit, vec3 &N, material &mat) {
	float dist_sphere = std::numeric_limits<float>::max();
	float dist_i = std::numeric_limits<float>::max();
	color top_c;
//	vec3  hit;
	for (size_t i = 0; i < spheres.size(); i++) 
	{
		if (spheres[i].ray_intersect(orig, dir, dist_i) && dist_i < dist_sphere)//相交
		{
			dist_sphere = dist_i;
			hit = orig + dir*dist_i;
			top_c = spheres[i].mat.diffuse_color; // background color
			N = (hit - spheres[i].center).normalized();
			mat = spheres[i].mat;
		}
	}
	if (dist_sphere >  1000)//都不相交
		return false;
	else
		return true;
}



void main()
{
	const int height = 400;
	const int width = 400;
	const int comp = 3;//每个像素的通道数
	float fov = 30;//相机视角大小，相机为原点，屏幕图像为0 0 1平面，也就是说焦距为1
	float d = 2 * tan(fov / 2 / 180.0 * PI) / (float)width;//每个像素的尺寸

	material      ivory(color(0.3, 0.5, 0.9));
	material red_rubber(color(0.6, 0.2, 0.3));
	material red(color(0.7, 0.0, 0.0));

	std::vector<sphere> spheres;
	
	sphere sph1(vec3(-1,0,10),1, ivory);//第一个球
	sphere sph2(vec3(0,0,9), 1, red_rubber);//第一个球
	sphere sph3(vec3(0, -1, 8), 0.5, red_rubber);//第一个球

	spheres.push_back(sph1);
	spheres.push_back(sph2);
	spheres.push_back(sph3);

	Light light(vec3(0, 10, 0), 3);


//	char* filename = "step3.jpg";//more spheres
//	char* filename = "step4.jpg";//light,需要知道交点位置
	unsigned char* framebuffer;
	framebuffer=(unsigned char *)malloc(height*height*comp);
	int index = 0;
	vec3 dir, orig,N,hit, light_dir;
	color clr;
	material mat;
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
		{			
			dir = vec3((i - height / 2) * d, (j - width / 2) * d, 1);
			dir = dir.normalized();
			if (!cast_ray(orig, dir, spheres,  hit, N, mat)) clr= color(0.2, 0.7, 0.8);
			else
			{
				light_dir = (light.position - hit).normalized();
				clr =  mat.diffuse_color * std::max(0.f, (N*light_dir));
			}

			if (clr.R > 1)
				clr.R = 1;

			framebuffer[index++] = (unsigned char)(clr.R * 255);
			framebuffer[index++] = (unsigned char)(clr.G * 255);
			framebuffer[index++] = (unsigned char)(clr.B * 255);
		}
			
	stbi_write_bmp("step4.bmp", width, height, comp, framebuffer);
}
