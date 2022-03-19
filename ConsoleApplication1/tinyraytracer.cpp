#define STB_IMAGE_WRITE_IMPLEMENTATION
#define _CRT_SECURE_NO_DEPRECATE
#define PI 3.1415926

#include "stb_image_write.h"

#include <tuple>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cmath>

class vec3//Ĭ��float 
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

class color//Ĭ��float 
{
public:
	color(float R=0, float G=0, float B=0) : R(R), G(G), B(B) {};
	//color(unsigned char R = 0, unsigned char G = 0, unsigned char B = 0) : R(R), G(G), B(B) {};
	float R = 0, G = 0, B = 0;
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

class sphere
{
public:
	//sphere();
	//~sphere();
	sphere(const vec3 &c, const float &r,const material &m) : center(c), radius(r), mat(m){}
	bool ray_intersect(const vec3 &orig, const vec3 &dir, float &t0) const 
	//����ֱ������Ľ��㡣dir ����Ϊ��һ��ʸ����һ����orig�㷢��ģ�����Ϊdir��ֱ�ߣ�
	//�����ཻ�����ֱ�ߵľ���Ϊt0�����˵������ཻ��
	{
		vec3 L = center - orig;
		float tca = L*dir;
		float d2 = L*L - tca*tca;//ԭ�����ֱ�ߵľ���
		if (d2 > radius*radius) 
			return false;//ֱ������һ�����ཻ
		else
		{
			float thc = sqrtf(radius*radius - d2);//d2:ֱ�������ľ��룬thc�����㣨����У����������ľ���
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

//��Ⱦ����
void render() {

}

color cast_ray(const vec3 &orig, const vec3 &dir, const std::vector<sphere> &spheres) {
	float sphere_dist = std::numeric_limits<float>::max();
	float i_dist = std::numeric_limits<float>::max();
	color top_c;
	int flag=0;//�ཻ�ı�־
	for (size_t i = 0; i < spheres.size(); i++) 
	{
		if (spheres[i].ray_intersect(orig, dir, i_dist))//�ཻ
		{
			flag = 1;
			if (i_dist < sphere_dist)//�����������ʾ����
			{
				sphere_dist = i_dist;
				top_c = spheres[i].mat.diffuse_color; // background color
			}
			
		}
	}
	if (flag == 0)//�����ཻ
		return color(0.2, 0.7, 0.8);
	else
		return top_c;
}



void main()
{
	const int height = 400;
	const int width = 400;
	const int comp = 3;//ÿ�����ص�ͨ����
	float fov = 30;//����ӽǴ�С�����Ϊԭ�㣬��Ļͼ��Ϊ0 0 1ƽ�棬Ҳ����˵����Ϊ1
	float d = 2 * tan(fov / 2 / 180.0 * PI) / (float)width;//ÿ�����صĳߴ�

	material      ivory(color(0.4, 0.4, 0.3));
	material red_rubber(color(0.3, 0.1, 0.1));
	
	std::vector<sphere> spheres;
	
	sphere sph1(vec3(-1,0,10),1, ivory);//��һ����
	sphere sph2(vec3(0,2,15), 4, red_rubber);//��һ����

	spheres.push_back(sph1);
	spheres.push_back(sph2);

	char* filename = "step3.jpg";//more spheres
	unsigned char* framebuffer;
	framebuffer=(unsigned char *)malloc(height*height*comp);
	int index = 0;
	vec3 dir, orig;
	color color;
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
		{			
			dir = vec3((i - height / 2) * d, (j - width / 2) * d, 1);
			dir = dir.normalized();
			color = cast_ray(orig, dir, spheres);
			framebuffer[index++] = (unsigned char)(color.R * 255);
			framebuffer[index++] = (unsigned char)(color.G * 255);
			framebuffer[index++] = (unsigned char)(color.B * 255);
		}
			
	stbi_write_jpg("step3.jpg", width, height, comp, framebuffer, 100);
}
