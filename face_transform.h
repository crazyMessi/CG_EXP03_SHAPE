#pragma once
#define MY_INIT_SORT_IMPLEMENTATION




//// mix前检查两个图片中的像素值
//#define CHECK_PIC_DATA 
//
//// 检查网格生成是否正常
//#define CHECK_MESH

//强行去黑线
#define DATAFIXING 
// 保存图像
#define SAVING 

#include <myIncludes/my_init_sort.h>
#include <myIncludes/camera.h>
#include <myIncludes/shader_s.h>
#include <windows.h>
#include "mesh.h"
#include "img.h"

// 路径插值次数
#define GRID_MESH_SUB_TIME 10

void transforming(unsigned int argc, string* argv);

// 混合两个图像，(根据参数t)
Mat mix(Mat& a, Mat& b, float t);

void test_grid_mesh(vector<vector<points_set>> mid_grid_mesh_set);

glm::vec3 cast_cvvec3b_2_glmvec3(Vec3b v);





