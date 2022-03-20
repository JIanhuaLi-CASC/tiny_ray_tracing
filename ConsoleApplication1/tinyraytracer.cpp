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
	vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {};
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

class Material {
public:
	Material(const vec3 &color, const float &exp, const vec3 &a) :
		color(color), specular_exponent(exp), albedo(a) {}
	Material() : color(0.f, 0.f, 0.f), specular_exponent(0.f), albedo(0.f,0.f,0.f) {}
	vec3 color;
	float specular_exponent;//镜面反射的指数因子，越小角度越集中
	vec3 albedo;//反照的比例，如果为1则只有镜面反射，没有漫反射
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
	sphere(const vec3 &c, const float &r, const Material &m) : center(c), radius(r), mat(m) {}
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
	Material mat;
};

//渲染函数
void render() {

}

bool scene_intersect(const vec3 &orig, const vec3 &dir, const std::vector<sphere> &spheres, vec3 &hit, vec3 &N, Material &mat) {
	float dist_sphere = std::numeric_limits<float>::max();
	float dist_i = std::numeric_limits<float>::max();
	vec3 top_c;
	//	vec3  hit;
	for (size_t i = 0; i < spheres.size(); i++)
	{
		if (spheres[i].ray_intersect(orig, dir, dist_i) && dist_i < dist_sphere)//相交
		{
			dist_sphere = dist_i;
			hit = orig + dir*dist_i;
			top_c = spheres[i].mat.color; // background vec3
			N = (hit - spheres[i].center).normalized();
			mat = spheres[i].mat;
		}
	}
	if (dist_sphere > 1000)//都不相交
		return false;
	else
		return true;
}

vec3 cast_ray(const vec3 &orig, const vec3 &dir, const std::vector<sphere> &spheres, const std::vector<Light> &lights, size_t depth = 0)
{
	vec3 N, hit, light_dir;
	float light_distance;
	vec3 color;
	Material mat;

	if (depth > 2 || !scene_intersect(orig, dir, spheres, hit, N, mat)) //depth表示光线追迹的次数
		return color = vec3(0.2, 0.2, 0.2);//backgroud color

	vec3 reflect_dir = reflect(dir, N).normalized();
	vec3 reflect_orig = reflect_dir*N < 0 ? hit - N*1e-3 : hit + N*1e-3; // offset the original point to avoid occlusion by the object itself
	vec3 reflect_color = cast_ray(reflect_orig, reflect_dir, spheres, lights, depth + 1);//函数嵌套

	float diffuse_light_intensity = 0, specular_light_intensity = 0;
	for (int k = 0; k < lights.size(); k++)
	{
		light_dir = (lights[k].position - hit).normalized();//从交点到光源，法线是从圆心到表面
		light_distance = (lights[k].position - hit).norm();

		vec3 shadow_orig = light_dir * N > 0 ? hit + N * 1e-3 : hit - N * 1e-3;
		vec3 shadow_pt, shadow_N;
		Material tmp_mat;
		//阴影主要处理别的球的遮挡，不是自己遮挡，因为自己在light_dir*N中有体现
		if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmp_mat) && (shadow_pt - shadow_orig).norm() < light_distance)
			continue;
		diffuse_light_intensity += std::max(0.f, (light_dir*N)) * lights[k].intensity;
		specular_light_intensity += powf(std::max(0.f, reflect(light_dir, N)*dir), mat.specular_exponent)*lights[k].intensity;
	}
	color = mat.color * diffuse_light_intensity * mat.albedo[0]
		+ vec3(0.2, 0.2, 0.2) * specular_light_intensity * mat.albedo[1] + reflect_color*mat.albedo[2];

	if (color.x > 1)	color.x = 1;
	if (color.y > 1)	color.y = 1;
	if (color.z > 1)	color.z = 1;
	return color;
}





void main()
{
	const int height = 600;
	const int width = 600;
	const int comp = 3;//每个像素的通道数
	float fov = 60;//相机视角大小，相机为原点，屏幕图像为0 0 1平面，也就是说焦距为1
	float d = 2 * tan(fov / 2 / 180.0 * PI) / (float)width;//每个像素的尺寸

	Material   red(vec3(0.3, 0.0, 0.0), 20, vec3(0.5,0.2,0.3));
	Material green(vec3(0.0, 0.2, 0.0), 5, vec3(0.5, 0.2, 0.3));
	Material blue(vec3(0.2, 0.3, 0.6), 20, vec3(0.5, 0.2, 0.1));
	Material glass(vec3(0.6, 0.7, 0.8), 200, vec3(0.5, 0.2, 0.1));
	Material mirror(vec3(0.1, 0.2, 0.3), 1500, vec3(0.1, 1, 1));

	std::vector<sphere> spheres;

	sphere sph1(vec3(-1, 0, 12), 1.2, red);//第一个球
	sphere sph2(vec3(0, -0.7, 9), 0.8, green);//第二个球
	sphere sph3(vec3(1.5, 0, 12.5), 1.2, blue);//第三个球
	sphere sph4(vec3(4, 4, 15), 2, mirror);//第三个球

	spheres.push_back(sph1);
	spheres.push_back(sph2);
	spheres.push_back(sph3);
	spheres.push_back(sph4);

	Light light1(vec3(0, 10, 0), 2);
	Light light2(vec3(50, 2, 20), 2);
	Light light3(vec3(4, -4, 0), 2);
	std::vector<Light> lights;
	lights.push_back(light1);
	lights.push_back(light2);
	lights.push_back(light3);
	//	char* filename = "step3.jpg";//more spheres
	//	char* filename = "step4.jpg";//light,需要知道交点位置
	unsigned char* framebuffer;
	framebuffer = (unsigned char *)malloc(height*height*comp);
	int index = 0;
	vec3 dir, orig, N, hit, light_dir;
	float light_distance;
	vec3 clr;
	Material mat;
	float diffuse_light_intensity = 0, specular_light_intensity = 0;
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
		{
			dir = vec3((j - width / 2) * d, (-i + height / 2) * d, 1);//坐标系，向右为x，向上为y，正前为z
			dir = dir.normalized();//从相机过来的光线矢量

			clr = cast_ray(orig, dir, spheres, lights);

			framebuffer[index++] = (unsigned char)(clr.x * 255);
			framebuffer[index++] = (unsigned char)(clr.y * 255);
			framebuffer[index++] = (unsigned char)(clr.z * 255);
		}

	stbi_write_bmp("step7-reflection.bmp", width, height, comp, framebuffer);
}
