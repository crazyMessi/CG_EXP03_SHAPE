/*



*/
// ��ͼ������ʾ����
#define SHOW_MESHINPIC
//���ͼ������ز����Ƿ�Ǹ�
#define PIX_EXAMING 

//// ���ر߽������ѯ�Ż������ܵ��º��ߣ�����������쳣���أ�
//#define PIX_INDEX_OPTIMIZATION

////��ʾ����Ȩ��ͼ
//#define SHOW_WEIGHT  

#pragma once
#include <myIncludes/my_pic.h>
#include "img.h"
#include <string>
using namespace std;
//GridCoordinateϸ�ִ��� ò�Ƹ�λ���ξ��Ѿ��㹻�⻬
#define TIMES 10

typedef vector<points_set> line_mesh;

glm::vec3 convertcvvectoglmvec(Vec3b d);

Vec3b convertglmvctocvvec(glm::vec3);


class GridCoordinate {
	friend class PixelCoordinate;


public:
	//����������һ������ά������ɵľ��������Ե�һ������Ϊ����ԭ�㣬��������꽫����һ����([0,1],[0,1])
	line_mesh grid_mesh;
	//��������ϵ������������w������������h��ע��������������
	unsigned int w, h;
	//������������
	vector<Catmull_Rom3d> row;
	//������������
	vector<Catmull_Rom3d> col;
	
	//������������������캯������
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
