#pragma once
#define MY_INIT_SORT_IMPLEMENTATION




//// mixǰ�������ͼƬ�е�����ֵ
//#define CHECK_PIC_DATA 
//
//// ������������Ƿ�����
//#define CHECK_MESH

//ǿ��ȥ����
#define DATAFIXING 
// ����ͼ��
#define SAVING 

#include <myIncludes/my_init_sort.h>
#include <myIncludes/camera.h>
#include <myIncludes/shader_s.h>
#include <windows.h>
#include "mesh.h"
#include "img.h"

// ·����ֵ����
#define GRID_MESH_SUB_TIME 10

void transforming(unsigned int argc, string* argv);

// �������ͼ��(���ݲ���t)
Mat mix(Mat& a, Mat& b, float t);

void test_grid_mesh(vector<vector<points_set>> mid_grid_mesh_set);

glm::vec3 cast_cvvec3b_2_glmvec3(Vec3b v);





