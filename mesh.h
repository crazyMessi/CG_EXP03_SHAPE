/*



*/
// 在图像中显示网格。
#define SHOW_MESHINPIC
//检查图像的像素采样是否非负
#define PIX_EXAMING 

//// 像素边界坐标查询优化。可能导致黑线！（如果存在异常像素）
//#define PIX_INDEX_OPTIMIZATION

////显示采样权重图
//#define SHOW_WEIGHT  

#pragma once
#include <myIncludes/my_pic.h>
#include "img.h"
#include <string>
using namespace std;
//GridCoordinate细分次数 貌似个位数次就已经足够光滑
#define TIMES 10

typedef vector<points_set> line_mesh;

glm::vec3 convertcvvectoglmvec(Vec3b d);

Vec3b convertglmvctocvvec(glm::vec3);


class GridCoordinate {
	friend class PixelCoordinate;


public:
	//网格坐标是一个由三维向量组成的矩阵。网格将以第一个点作为坐标原点，顶点的坐标将被归一化至([0,1],[0,1])
	line_mesh grid_mesh;
	//网格坐标系的纵向曲线数w、横向曲线数h。注意是曲线数！！
	unsigned int w, h;
	//横向曲线容器
	vector<Catmull_Rom3d> row;
	//纵向曲线容器
	vector<Catmull_Rom3d> col;
	
	//填充曲线容器，被构造函数调用
	void init();
	GridCoordinate(const char* path,unsigned int w,unsigned int h);
	GridCoordinate(line_mesh& grid_mesh);
	GridCoordinate() {};
	
};


class PixelCoordinate {

public:

	Mat pix_mat;
	GridCoordinate mesh;
	PixelCoordinate(const char* pic_path, const char* obj_path, unsigned int w, unsigned int h);
	PixelCoordinate(line_mesh&grid_mesh);
	PixelCoordinate(GridCoordinate&);
	PixelCoordinate(){
		pix_mat = Mat();
		mesh = GridCoordinate();
	}

	Mat switchMeshX(GridCoordinate& aim);
	Mat switchMeshY(GridCoordinate& aim);
	Mat getMixedIntermediate(GridCoordinate& aim);


};
