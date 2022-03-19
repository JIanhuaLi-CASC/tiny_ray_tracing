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
	color  operator*(const float v) const { return{ R*v, G*v, B*v }; }
	float& operator[](const int i) { return i == 0 ? R : (1 == i ? G : B); }
	const float& operator[](const int i) const { return i == 0 ? R : (1 == i ? G :B); }
	color &operator =(const color &a)
	{
		R = a.R; G = a.G; B = a.B;
		return *this;
	}
	color  operator+(const color& v) const { return{ R + v.R, G + v.G, B + v.B }; }
};

class material {
public:
	material(const color &color, const float &exp, const float &a) : 
		    diffuse_color(color), specular_exponent(exp), albedo(a){}
	material() : diffuse_color(0.f,0.f,0.f), specular_exponent(0.f), albedo(0.f){}
	color diffuse_color;
	float specular_exponent;//���淴���ָ�����ӣ�ԽС�Ƕ�Խ����
	float albedo;//���յı��������Ϊ1��ֻ�о��淴�䣬û��������
};

struct Light {
	Light(const vec3 &p, const float &i) : position(p), intensity(i) {}
	vec3 position;
	float intensity;
};

vec3 reflect(const vec3 &I, const vec3 &N) {
	return I - N*2.f*(I*N);
}



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

bool cast_ray(const vec3 &orig, const vec3 &dir, const std::vector<sphere> &spheres, vec3 &hit, vec3 &N, material &mat) {
	float dist_sphere = std::numeric_limits<float>::max();
	float dist_i = std::numeric_limits<float>::max();
	color top_c;
//	vec3  hit;
	for (size_t i = 0; i < spheres.size(); i++) 
	{
		if (spheres[i].ray_intersect(orig, dir, dist_i) && dist_i < dist_sphere)//�ཻ
		{
			dist_sphere = dist_i;
			hit = orig + dir*dist_i;
			top_c = spheres[i].mat.diffuse_color; // background color
			N = (hit - spheres[i].center).normalized();
			mat = spheres[i].mat;
		}
	}
	if (dist_sphere >  1000)//�����ཻ
		return false;
	else
		return true;
}



void main()
{
	const int height = 600;
	const int width = 600;
	const int comp = 3;//ÿ�����ص�ͨ����
	float fov = 60;//����ӽǴ�С�����Ϊԭ�㣬��Ļͼ��Ϊ0 0 1ƽ�棬Ҳ����˵����Ϊ1
	float d = 2 * tan(fov / 2 / 180.0 * PI) / (float)width;//ÿ�����صĳߴ�

	material      ivory(color(0.3, 0.0, 0.0),20,0.5);
	material red_rubber(color(0.0, 0.2, 0.0),5,1);
	material red(color(0.0, 0.0, 0.2),20,0.1);
	material glass(color(0.6, 0.7, 0.8), 200, 0.2);

	std::vector<sphere> spheres;
	
	sphere sph1(vec3(-1,0,10),1, ivory);//��һ����
	sphere sph2(vec3(0,1.2,9), 0.8, red_rubber);//�ڶ�����
	sphere sph3(vec3(0, -1, 8), 0.5, red);//��������
	sphere sph4(vec3(-4, -4, 15), 2, glass);//��������

	spheres.push_back(sph1);
	spheres.push_back(sph2);
	spheres.push_back(sph3);
	spheres.push_back(sph4);

	Light light1(vec3(0, 10, 0), 2);
	Light light2(vec3(-50, 2, 2), 1);
	std::vector<Light> lights;
	lights.push_back(light1);
	lights.push_back(light2);

//	char* filename = "step3.jpg";//more spheres
//	char* filename = "step4.jpg";//light,��Ҫ֪������λ��
	unsigned char* framebuffer;
	framebuffer=(unsigned char *)malloc(height*height*comp);
	int index = 0;
	vec3 dir, orig,N,hit, light_dir;
	color clr;
	material mat;
	float diffuse_light_intensity = 0, specular_light_intensity = 0;
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
		{			
			dir = vec3((i - height / 2) * d, (j - width / 2) * d, 1);
			dir = dir.normalized();//����������Ĺ���ʸ��
			if (!cast_ray(orig, dir, spheres,  hit, N, mat)) clr= color(0.2, 0.2, 0.2);
			else
			{
				diffuse_light_intensity = 0;
				specular_light_intensity = 0;
				for (int k = 0; k < lights.size(); k++)
				{
					light_dir = (lights[k].position - hit).normalized();
					diffuse_light_intensity += std::max(0.f, (light_dir*N)) * lights[k].intensity;
					specular_light_intensity += powf(std::max(0.f, reflect(light_dir, N)*dir), mat.specular_exponent)*lights[k].intensity;
				}
				clr = mat.diffuse_color * diffuse_light_intensity * mat.albedo 
					+ color(0.2, 0.2, 0.2) * specular_light_intensity * (1 - mat.albedo);
				
			}

			if (clr.R > 1)	clr.R = 1;
			if (clr.G > 1)	clr.G = 1;
			if (clr.B > 1)	clr.B = 1;

			framebuffer[index++] = (unsigned char)(clr.R * 255);
			framebuffer[index++] = (unsigned char)(clr.G * 255);
			framebuffer[index++] = (unsigned char)(clr.B * 255);
		}
			
	stbi_write_bmp("step5.bmp", width, height, comp, framebuffer);
}
