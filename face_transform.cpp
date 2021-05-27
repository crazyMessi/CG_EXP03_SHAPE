#include "face_transform.h"
// 网格坐标的横、纵向曲线条数。注意曲线条数是格子个数+1
#define W 7
#define H 7

#include <iostream>
using namespace std;

void transforming(unsigned int argc, string* argv);

int main() {
	unsigned int argc;
	cin >> argc;
	string* argv = new string[argc];
	for (size_t i = 0; i < argc; i++){
		cin >> argv[i];
	}
	transforming(argc, argv);
	
}


void transforming(unsigned int argc, string* argv){
	
	assert(argc % 2 == 0);
	int counts = argc / 2;
	vector<Mat> pix_mesh_res_set;
	vector<line_mesh> mid_grid_mesh_set;
	vector<PixelCoordinate> resource_pic;
	


	
	
	mid_grid_mesh_set.resize(GRID_MESH_SUB_TIME*(counts-1));
	resource_pic.resize(counts);
	for (size_t i = 0; i < counts;i++){
		resource_pic[i] = PixelCoordinate(argv[2*i].c_str(), argv[2*i+1].c_str(), W, H);
	}
	
	// 获得所有的中间图像网格坐标系
	for (size_t rcount = 0; rcount < H; rcount++){
		for (size_t colcount = 0; colcount < W; colcount++){
			points_set setting_points;
			setting_points.resize(counts);
			for (size_t i = 0; i < counts; i++){
				setting_points[i] = resource_pic[i].mesh.grid_mesh[rcount][colcount];
			}
			Catmull_Rom3d line(setting_points,GRID_MESH_SUB_TIME);
			setting_points.clear();
			for (size_t index = 0; index < GRID_MESH_SUB_TIME*(counts-1); index++){
				mid_grid_mesh_set[index].resize(H);
				mid_grid_mesh_set[index][rcount].resize(W);
				mid_grid_mesh_set[index][rcount][colcount] = line.getPointByIndex(index);
			}
		}
	}
#ifdef CHECK_MESH
	test_grid_mesh(mid_grid_mesh_set);
#endif //  CHECK_MESH

	pix_mesh_res_set.resize((counts - 1) * GRID_MESH_SUB_TIME);
	// 分别让源图像、目标图像向中间网格坐标系变化，获得两个中间图像并按照t插值
	for (size_t i = 0; i < counts-1; i++){

		for (size_t times = 0; times < GRID_MESH_SUB_TIME; times++){
			float t = float(times) / float(GRID_MESH_SUB_TIME);
			GridCoordinate intermediate_grid(mid_grid_mesh_set[times]);
      		Mat intermediate_pix_from_source = resource_pic[i].getMixedIntermediate(intermediate_grid);
			Mat intermediate_pix_from_aim = resource_pic[i + 1].getMixedIntermediate(intermediate_grid);
			pix_mesh_res_set[i * GRID_MESH_SUB_TIME + times] = mix(intermediate_pix_from_source, intermediate_pix_from_aim,t);

#ifdef DATAFIXING
			Mat temp = pix_mesh_res_set[i * GRID_MESH_SUB_TIME + times];
			int range = 4;
			for (int rowc = range; rowc < temp.rows-range; rowc++)
			{
				for (size_t ccount = range; ccount < temp.cols-range; ccount++)
				{
					if (temp.at<Vec3b>(rowc,ccount) == Vec3b(0,0,0))
					{
						for (int roff = -range; roff <= range; roff++)
						{
							for (int coff = -range; coff <= range; coff++)
							{
								temp.at<Vec3b>(rowc, ccount) += temp.at<Vec3b>(rowc + roff, ccount + coff)* (0.04);
							}
						}
						 
					}
				}
			}
			//不需要额外存一张。上述对mat的操作修复的就是data，申明一个变量temp只是为了更好表示
			//imwrite("D:/Work/c++/OpenGL/myProject/CG_EXP03_SHAPE/pic/output/man/" + to_string(i) + to_string(times) + "fix.png",temp);
			temp.release();
#endif // DATAFIXING


//#ifdef PIX_EXAMING
//			imshow("i", pix_mesh_res_set[i * GRID_MESH_SUB_TIME + times]);
//			waitKey(0);
//#endif // PIX_EXAMING


#ifdef SAVING
			//imshow("i", pix_mesh_res_set[i * GRID_MESH_SUB_TIME + times]);
			imwrite("D:/Work/c++/OpenGL/myProject/CG_EXP03_SHAPE/pic/output/man/" + to_string(i) + to_string(times) + "fromsource.png", intermediate_pix_from_source);
			imwrite("D:/Work/c++/OpenGL/myProject/CG_EXP03_SHAPE/pic/output/man/" + to_string(i) + to_string(times) + "fromaim.png", intermediate_pix_from_aim);
			imwrite("D:/Work/c++/OpenGL/myProject/CG_EXP03_SHAPE/pic/output/man/"+to_string(i)+to_string(times)+".png", pix_mesh_res_set[i * GRID_MESH_SUB_TIME + times]);
			//释放内存
			pix_mesh_res_set[i * GRID_MESH_SUB_TIME + times].release();
			intermediate_pix_from_aim.release();
			intermediate_pix_from_source.release();
			#endif // SAVING
		}



	}


//不一个个释放根本运行不了。。。内存溢出了
//#ifndef PIX_EXAMING
//	for (size_t i = 0; i < counts - 1; i++)
//	{
//		for (size_t j = 0; j < GRID_MESH_SUB_TIME; j++)
//		{
//			imshow("i", pix_mesh_res_set[i * GRID_MESH_SUB_TIME + j]);
//			waitKey(0);
//		}
//	}
//#endif // !PIX_EXAMING


}

Mat mix(Mat& a, Mat& b, float t)
{
	//错误记录：图片A、B的像素个数可能不一致！！！
	//这里简单地按照a的来
	Mat res = a.clone();
#ifdef CHECK_PIC_DATA
	points_set pa,pb;
	pa.resize(res.rows*res.cols);
	pb.resize(b.rows*b.cols);
	for (size_t i = 0; i < res.rows; i++) {
		for (size_t j = 0; j < res.cols; j++) {
			pa[i * res.rows + j] = cast_cvvec3b_2_glmvec3(a.at<Vec3b>(i, j));
			pb[i * res.rows + j] = cast_cvvec3b_2_glmvec3(b.at<Vec3b>(i*b.cols/a.cols, j*b.cols/a.cols));
		}
	}
#endif //  CHECK_PIC_DATA

	for (size_t i = 0; i < res.rows; i++){
		for (size_t j = 0; j < res.cols; j++){
			res.at<Vec3b>(i, j) = a.at<Vec3b>(i, j) * (1 - t) + b.at<Vec3b>(i*b.cols/a.cols, j*b.cols/a.cols) * t;
		}
	}
	return res;
}

void test_grid_mesh(vector<vector<points_set>> mid_grid_mesh_set)
{
	for (size_t i = 0; i < mid_grid_mesh_set.size(); i++){
		
		GridCoordinate b(mid_grid_mesh_set[i]);


		// 调整一下坐标，否则有部分看不见
		for (size_t i = 0; i < b.row.size(); i++){
			for (size_t j = 0; j < b.row[i].res_set.size(); j++){
				b.row[i].res_set[j] -= 0.5;
			}
		}

		for (size_t i = 0; i < b.col.size(); i++) {
			for (size_t j = 0; j < b.col[i].res_set.size(); j++) {
				b.col[i].res_set[j] -= 0.5;
			}
		}
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "ZHUODONG LI", NULL, NULL);
		my_testwindow(window);

		unsigned int rowVAO[W];
		glGenVertexArrays(W, rowVAO);


		for (size_t i = 0; i < H; i++)
		{
			glBindVertexArray(rowVAO[i]);
			VBO_binder(b.row[i].res_set);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
		}

		unsigned int colVAO[W];
		glGenVertexArrays(W, colVAO);


		for (size_t i = 0; i < W; i++)
		{
			glBindVertexArray(colVAO[i]);
			VBO_binder(b.col[i].res_set);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
		}

		while (!glfwWindowShouldClose(window)) {
			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			for (size_t i = 0; i < H; i++)
			{

				glBindVertexArray(rowVAO[i]);
				glDrawArrays(GL_LINE_STRIP, 0, TIMES * (W-1)+1);
			}

			for (size_t i = 0; i < W; i++)
			{
				glBindVertexArray(colVAO[i]);
				glDrawArrays(GL_LINE_STRIP, 0, TIMES * (H-1)+1);
			}
			Sleep(1);
			glfwSwapBuffers(window);
			glfwPollEvents();
		}
	}	
}

glm::vec3 cast_cvvec3b_2_glmvec3(Vec3b v)
{
	return glm::vec3(v[0],v[1],v[2]);
}



// 网格生成测试

/*
	const char* path = "pic/face01.obj";
	GridCoordinate b(path, 7, 6);

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "ZHUODONG LI", NULL, NULL);
	my_testwindow(window);

	unsigned int rowVAO[6];
	glGenVertexArrays(6, rowVAO);


	for (size_t i = 0; i < 6; i++)
	{
		glBindVertexArray(rowVAO[i]);
		VBO_binder(b.row[i].res_set);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}

	unsigned int colVAO[7];
	glGenVertexArrays(7, colVAO);


	for (size_t i = 0; i <7; i++)
	{
		glBindVertexArray(colVAO[i]);
		VBO_binder(b.col[i].res_set);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}

	while (!glfwWindowShouldClose(window)){
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		for (size_t i = 0; i < 6; i++)
		{

			glBindVertexArray(rowVAO[i]);
			glDrawArrays(GL_LINE_STRIP, 0, TIMES * 6);
		}

		for (size_t i = 0; i < 7; i++)
		{
			glBindVertexArray(colVAO[i]);
			glDrawArrays(GL_LINE_STRIP, 0, TIMES * 5);
		}


		glfwSwapBuffers(window);
		glfwPollEvents();
	}




	return 0;
*/

// void test(const char* path);
// 
// float test_points[]{
// 	-0.5,0.5,
// 	0.5,0.5,
// 	-0.5,-0.5,
// 	0.5,-0.5
// };
// 
// unsigned int test_indices[]{
// 	0,1,2,
// 	1,2,3
// };
// 
// 
// 
// int main()
// {
// 	test("pic/01.png");
// 
// 
// 	glfwInit();
// 	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
// 	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
// 	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
// 	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "ZHUODONG LI", NULL, NULL);
// 	my_testwindow(window);
// 
// 
// 	unsigned int test_VAO;
// 	glGenVertexArrays(1, &test_VAO);
// 	glBindVertexArray(test_VAO);
// 
// 	VBO_binder(test_points, 8);
// 	EBO_binder(test_indices, 6);
// 
// 	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
// 	glEnableVertexAttribArray(0);
// 	
// 
// 	Shader test_Shader("t_shader.vs","t_shader.fs");
// 	test_Shader.use();
// 	loadTexture("pic/02.png");
// 
// 
// 
// 
// 
// 	glEnable(GL_DEPTH_TEST);
// 	while (!glfwWindowShouldClose(window)) {
// 
// 
// 		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
// 		glDepthMask(GL_FALSE);
// 		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
// 		test_Shader.use();
// 		glBindVertexArray(test_VAO);
// 		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
// 
// 		glfwSwapBuffers(window);
// 		glfwPollEvents();
// 
// 	}
// 
// }