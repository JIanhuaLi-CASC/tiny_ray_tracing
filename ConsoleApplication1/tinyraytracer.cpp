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
	vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {};
	float x = 0, y = 0, z = 0;
	float& operator[](const int i) { return i == 0 ? x : (1 == i ? y : z); }
	const float& operator[](const int i) const { return i == 0 ? x : (1 == i ? y : z); }
	vec3  operator*(const float v) const { return{ x*v, y*v, z*v }; }
	float operator*(const vec3& v) const { return x*v.x + y*v.y + z*v.z; }//inner product
	vec3  operator/(const float& v) const { return{ x / v, y / v, z / v }; }
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
	Material(const vec3 &color, const float &exp, const vec3 &a, const float &indx, const float &fct) :
		color(color), specular_exponent(exp), albedo(a), refra_index(indx), refra_factor(fct) {}
	Material() : color(0.f, 0.f, 0.f), specular_exponent(0.f), albedo(0.f, 0.f, 0.f), refra_index(1.f), refra_factor(0.f) {}
	vec3 color;
	float specular_exponent;//���淴���ָ�����ӣ�ԽС�Ƕ�Խ����
	vec3 albedo;//���յı��������Ϊ1��ֻ�о��淴�䣬û��������
	float refra_index;
	float refra_factor;
};

struct Light {
	Light(const vec3 &p, const float &i) : position(p), intensity(i) {}
	vec3 position;
	float intensity;
};

vec3 reflect(const vec3 &I, const vec3 &N) {
	return I - N*2.f*(I*N);
}

vec3 refract(const vec3 &I, const vec3 &N, const float &ref_index) {
	vec3 tmp;
	if (I*N < 0)//from outside to inside
		tmp = (I - N*(I*N)) / ref_index + N * (I * N);
	else
		tmp = (I - N*(I*N)) * ref_index + N * (I * N);
	return tmp.normalized();
}


class sphere
{
public:
	//sphere();
	//~sphere();
	sphere(const vec3 &c, const float &r, const Material &m) : center(c), radius(r), mat(m) {}
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
	Material mat;
};


class chessboard
{
public:
	//sphere();
	//~sphere();
	chessboard(const vec3 &n, const vec3 &c, const float &l, const int &num, const vec3 c1, const vec3 c2)
		: normal(n), center(c), length(l), num(num), color1(c1), color2(c2) {}
	//	chessboard() { normal = { 0 }; };
	//����ֱ����chessboard�Ľ��㡣dir ����Ϊ��һ��ʸ����һ����orig�㷢��ģ�����Ϊdir��ֱ�ߣ�
	//��chessbord�ཻ�����ֱ�ߵľ���Ϊt0
	//���㷽����ֱ�߷���Ϊx = x0 + dir_x * t;y = y0 + dir_y * t,z = z0 + dir_z * t
	//��ֱ�߷��̴���ƽ�淽�̣��õ�t��n1*(x-c1)+n2*(y-c2)+n3*(z-c3)=0
	//n1 * (x0+dir_x*t-c1)+n2 * (y0+dir_y*t-c2)+n3 * (z0+dir_z*t-c3)=0
	//t = -(n *( orig-c))/(n*dir)
	std::tuple<bool, float, vec3, vec3>  ray_intersect(const vec3 &orig, const vec3 &dir) const
	{
		if (fabs(normal*dir) < 1e-6) return  std::make_tuple(false, 0, vec3(0, 0, 0), vec3(0, 0, 0));
		float t = -(normal * (orig - center)) / (normal*dir);
		vec3 hit = orig + dir*t;
		vec3 c2p = hit - center;
		//�жϽ����Ƿ���length��Χ�ڣ��ж���ɫ
		int x_num = fabsf(c2p[0]) / length;
		int z_num = fabsf(c2p[2]) / length;

		if (t > 0 && x_num < num  && z_num < num)
		{	
			if ((x_num + z_num) % 2 == 0)
				return  std::make_tuple(true, t, hit, color1);
			else
				return  std::make_tuple(true, t, hit, color2);
		}
		else return  std::make_tuple(false, 0, vec3(0, 0, 0), vec3(0, 0, 0));
	}
	vec3 normal;//���߷���
	vec3 center;//���ĵ�
	float length = 0.5f;//����chessboard�Ŀ��
	int num = 2;//how many chess board along one direction
	vec3 color1 = vec3(1, 1, 0);//yellow
	vec3 color2 = vec3(1, 1, 1);//two color
};





//��Ⱦ����
void render() {

}

bool scene_intersect(const vec3 &orig, const vec3 &dir, const std::vector<sphere> &spheres, 
	chessboard &chessbd,vec3 &hit, vec3 &N, Material &mat) {
	float dist_sphere = std::numeric_limits<float>::max();
	float dist_i = std::numeric_limits<float>::max();
	vec3 top_c;
	//	vec3  hit;
	for (size_t i = 0; i < spheres.size(); i++)
	{
		if (spheres[i].ray_intersect(orig, dir, dist_i) && dist_i < dist_sphere)//�ཻ,�����ȡ�����
		{
			dist_sphere = dist_i;
			hit = orig + dir*dist_i;
			top_c = spheres[i].mat.color; // background vec3
			N = (hit - spheres[i].center).normalized();
			mat = spheres[i].mat;
		}
	}
	
	bool hit_flag;
	float dist_chessbd;
	vec3 hit_point, hit_color;
	std::tie(hit_flag, dist_chessbd, hit_point, hit_color) = chessbd.ray_intersect(orig, dir);

	if (dist_sphere < dist_chessbd)
	
	if (dist_sphere > 1000)//�����ཻ
		return false;
	else
		return true;
}

//����chessboard���ཻ�ж�
//bool scene_intersect(const vec3 &orig, const vec3 &dir, vec3 &hit, chessboard &chessbd) {
//	//	float dist_sphere = std::numeric_limits<float>::max();
//	float dist_i = std::numeric_limits<float>::max();
//	vec3 top_c;	//	vec3  hit;
//
//}




vec3 cast_ray(const vec3 &orig, const vec3 &dir, const std::vector<sphere> &spheres, 
			const std::vector<Light> &lights, const chessboard &chessbd, size_t depth = 0)
{
	vec3 N, hit, light_dir;
	float light_distance;
	vec3 color;
	Material mat;

	if (depth > 4 || !scene_intersect(orig, dir, spheres, hit, N, mat)) //depth��ʾ����׷���Ĵ���
		return color = vec3(0.2, 0.2, 0.2);//backgroud color

	vec3 reflect_dir = reflect(dir, N).normalized();
	//vec3 reflect_orig = reflect_dir*N < 0 ? hit - N*1e-3 : hit + N*1e-3; // offset the original point to avoid occlusion by the object itself
	vec3 reflect_orig = hit + reflect_dir*1e-3;
	vec3 reflect_color = cast_ray(reflect_orig, reflect_dir, spheres, lights, depth + 1);//����Ƕ��

	vec3 refract_dir = refract(dir, N, mat.refra_index);
	vec3 refract_orig = hit + refract_dir*1e-3;; // offset the original point to avoid occlusion by the object itself
	vec3 refract_color = cast_ray(refract_orig, refract_dir, spheres, lights, depth + 1);//����Ƕ��


	float diffuse_light_intensity = 0, specular_light_intensity = 0;
	for (int k = 0; k < lights.size(); k++)
	{
		light_dir = (lights[k].position - hit).normalized();//�ӽ��㵽��Դ�������Ǵ�Բ�ĵ�����
		light_distance = (lights[k].position - hit).norm();

		vec3 shadow_orig = light_dir * N > 0 ? hit + N * 1e-3 : hit - N * 1e-3;
		vec3 shadow_pt, shadow_N;
		Material tmp_mat;
		//��Ӱ��Ҫ����������ڵ��������Լ��ڵ�����Ϊ�Լ���light_dir*N��������
		if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmp_mat) && (shadow_pt - shadow_orig).norm() < light_distance)
			continue;
		diffuse_light_intensity += std::max(0.f, (light_dir*N)) * lights[k].intensity;
		specular_light_intensity += powf(std::max(0.f, reflect(light_dir, N)*dir), mat.specular_exponent)*lights[k].intensity;
	}
	color = mat.color * diffuse_light_intensity * mat.albedo[0]
		+ vec3(0.2, 0.2, 0.2) * specular_light_intensity * mat.albedo[1]
		+ reflect_color*mat.albedo[2] + refract_color * (mat.refra_factor);

	if (color.x > 1)	color.x = 1;
	if (color.y > 1)	color.y = 1;
	if (color.z > 1)	color.z = 1;
	return color;
}





void main()
{
	const int height = 600;
	const int width = 600;
	const int comp = 3;//ÿ�����ص�ͨ����
	float fov = 60;//����ӽǴ�С�����Ϊԭ�㣬��Ļͼ��Ϊ0 0 1ƽ�棬Ҳ����˵����Ϊ1
	float d = 2 * tan(fov / 2 / 180.0 * PI) / (float)width;//ÿ�����صĳߴ�

	Material   red(vec3(0.2, 0.6, 0.1), 20, vec3(0.5, 0.2, 0.1), 1.5, 0.0);
	Material green(vec3(0.1, 0.1, 0.1), 500, vec3(0.1, 0.5, 0.5), 1.5, 1);//������ǿ
	Material blue(vec3(0.6, 0.3, 0.2), 20, vec3(0.5, 0.2, 0.1), 1.5, 0.0);
	//	Material glass(vec3(0.6, 0.7, 0.8), 200, vec3(0.1, 0.1, 0.1),1.5,0.1);
	Material mirror(vec3(0.1, 0.2, 0.3), 1500, vec3(0.1, 1, 1), 1.5, 0.3);//������ǿ

	std::vector<sphere> spheres;

	sphere sph1(vec3(-1, 0, 12), 1.2, red);//��һ����
	sphere sph2(vec3(0, -0.7, 9), 0.8, green);//�ڶ�����
	sphere sph3(vec3(1.5, 0, 12.5), 1.2, blue);//��������
	sphere sph4(vec3(4, 4, 15), 2, mirror);//��������

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


	vec3 norm(0, 1, 0), center(0, -1, 9);
	vec3 yellow(1, 1, 0), white(1, 1, 1);
	chessboard chessbd(norm, center, 0.6, 5, yellow, white);



	//	char* filename = "step3.jpg";//more spheres
	//	char* filename = "step4.jpg";//light,��Ҫ֪������λ��
	unsigned char* framebuffer;
	framebuffer = (unsigned char *)malloc(height*height*comp);
	int index = 0;
	vec3 dir, orig, N, hit, light_dir;
	float light_distance;
	vec3 clr;
	Material mat;
	float diffuse_light_intensity = 0, specular_light_intensity = 0;
	for (int i = 0; i < height; i++)//y����
		for (int j = 0; j < width; j++)//x����
		{
			dir = vec3((j - width / 2) * d, (-i + height / 2) * d, 1);//����ϵ������Ϊx������Ϊy����ǰΪz
			dir = dir.normalized();//����������Ĺ���ʸ��

			//clr = cast_ray(orig, dir, spheres, lights);

			bool hit_flag;
			float hit_dis;
			vec3 hit_point, hit_color;
			std::tie(hit_flag, hit_dis, hit_point, hit_color) = chessbd.ray_intersect(orig, dir);

			if (hit_flag)  clr = hit_color;
			else clr = vec3(0.1, 0.2, 0.3);

			framebuffer[index++] = (unsigned char)(clr.x * 255);
			framebuffer[index++] = (unsigned char)(clr.y * 255);
			framebuffer[index++] = (unsigned char)(clr.z * 255);
		}

	stbi_write_bmp("step9_chessborad.bmp", width, height, comp, framebuffer);
}
