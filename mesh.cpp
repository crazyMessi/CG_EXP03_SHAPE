#include "mesh.h"
#include <fstream>
#include <sstream>



#define MAX_CAHR_PER_LINE 200

GridCoordinate::GridCoordinate(const char* path,unsigned int w,unsigned int h){
    
    if (!(w>0&&h>0)){
        std::cout << "ERROR! W<0 OR H<O";
    }

    this->w = w;
    this->h = h;
    std::ifstream istr(path);
    if (!istr) {
        std::cout << "ERROR! CANNOT OPEN: " << path << std::endl;
        return;
    }
    std::string token, token2;
    
    // s_x、s_y为起始点。以该点作为坐标原点
    float x, y, z,hei,wid,s_x,s_y;

    char line[MAX_CAHR_PER_LINE];

    unsigned int n_w=0;
    unsigned int n_h=0;


    grid_mesh.resize(h);
    for (size_t i = 0; i < h; i++)
    {
        grid_mesh[i].resize(w);
    }


    while (istr.getline(line, MAX_CAHR_PER_LINE)&&n_h<h&&n_w<w){
        std::stringstream ss;
        ss << line;
        token = "";
        ss >> token;
        if (token == "vt" || token == "vn")continue;
        if (token == (string)"v"){
            ss >> x >> y >> z;
           
            grid_mesh[n_h][n_w]=(glm::vec3(x,y,z));
            n_w++;
            if (n_w==w){
                n_w = 0;
                n_h++;
            }
        }
    }

    assert(n_w == 0);
    assert(n_h == h);

    s_x = grid_mesh[0][0].x;
    s_y = grid_mesh[0][0].y;
    wid = abs(grid_mesh[h-1][w-1].x-s_x);
    hei = abs(grid_mesh[h-1][w-1].y - s_y);

    // 坐标归一化
    for (size_t i = 0; i < h; i++){
        for (size_t j = 0; j < w; j++)
        {
            grid_mesh[i][j].x = (grid_mesh[i][j].x - s_x) / (wid);
            grid_mesh[i][j].y = (grid_mesh[i][j].y - s_y) / (hei);
            grid_mesh[i][j].z = 0.0f;
        }
    }

    init();
}

GridCoordinate::GridCoordinate(line_mesh& grid_mesh){
    this->grid_mesh = grid_mesh;
    init();
}

void GridCoordinate::init(){

    h = grid_mesh.size();
    w = grid_mesh[0].size();
    row.resize(h);
    col.resize(w);
    for (size_t i = 0; i < this->h; i++){
        row[i]=(Catmull_Rom3d(grid_mesh[i],TIMES));
    }

    for (size_t i = 0; i < w; i++){
        points_set temp;
        temp.resize(this->h);
        for (size_t j = 0; j < h; j++) {
            temp[j] = grid_mesh[j][i];
        }
        col[i]=(Catmull_Rom3d(temp,TIMES));
    }


}


Mat PixelCoordinate::switchMeshX(GridCoordinate& aim){
    // 长度为负数的像素数
    // 像素长度为负数以及像素大小为零产生的后果比想象中要大，影响的像素可能不仅是其自身
    // 因此一个都不可忽视！！
    long wrongflag = 0;
    // 长度为0的像素数
    long warningflag = 0;

    Mat result = this->pix_mat.clone();

    float py, px, gy, gx, gxn;
    // 图片行数
    int size_r = (float)pix_mat.rows;
    // 图片列数
    int size_c = (float)pix_mat.cols;
    int grid_len = (float)aim.w;

    for (size_t rcount = 0; rcount < size_r; rcount++){

        //用扫描行上的像素的中点的图像坐标作为扫描行的纵坐标。
        py = float(0.5 + rcount)/size_r;
        
        // 每行（在镜像方法中为列，下同）生成像素-网格坐标曲线,
        // 其中，像素坐标为横坐标（x，x∈[0,n]，n是扫描线上像素总数），网格坐标（y，y∈[0,grid_len-1]）
        // 源图像的曲线为this_col_i（在镜像方法中为辅助图像）
        // 辅助图像曲线为 auxiliary_col_i
        // 这些曲线都是即用即弃
        points_set this_setting_points,auxiliary_setting_points;
        this_setting_points.resize(grid_len);
        auxiliary_setting_points.resize(grid_len);
        for (size_t i = 0; i < grid_len; i++){
            //从网格坐标系统中得到的坐标都是图像坐标。图像坐标*扫描行像素总数即像素坐标
            this_setting_points[i] = glm::vec3(this->mesh.col[i].getXbyY(py)*size_c, float(i), 0.0f);
            auxiliary_setting_points[i] = glm::vec3(aim.col[i].getXbyY(py)*size_c, float(i), 0.0f);
        }
        Catmull_Rom3d this_col_i(this_setting_points);
        Catmull_Rom3d auxiliary_col_i(auxiliary_setting_points);

        // index是待采样图像的像素的所有边界的网格坐标值（y）。
        // 显然，边界的个数是每行的像素个数+1
        vector<float> index;
        index.resize(size_c+1);
        for (size_t i = 0; i < size_c+1; i++){
            index[i] = this_col_i.getYbyX(float(i));
        }

        // *i表示像素边界的网格坐标；*i_map表示像素边界在采样图像中对应像素边界的像素坐标
        // *i_map不一定是整数，因此需要混合插值。这个过程异常麻烦且极易出错。下次可以考虑暴力向外取整
        float formeri, latteri,formeri_map,latteri_map;
        latteri = 0.0f;
        latteri_map = 0.0f;
        
        // j是每个像素格边界在像素-网格坐标曲线中索引到的位置。
        // j只在循环外这算是一次优化，因为并不需要每次行扫描的时候都从头开始
        // 前提是像素边界的坐标值是逐行递增的。如果不是可能会导致黑线！
        unsigned int j = 0;
        for (size_t colcount = 0; colcount < size_c; colcount++){ 
#ifndef PIX_INDEX_OPTIMIZATION
            j = 0;
#endif //  !PIX_INDEX_OPTIMIZATION 如果启用优化则j不用置0

            // 像素格左侧即上一次迭代中的右侧       
            formeri = latteri;
            formeri_map = latteri_map;
            latteri = auxiliary_col_i.getYbyX(float(colcount+1));
            while(latteri > index[j+1]&&j<size_c-1)j++;
            latteri_map = (float)j + (latteri - index[j]) / (index[j + 1] - index[j]);
            if (latteri_map > float(size_c))latteri_map = float(size_c);

            // 当像素左侧网格坐标大于等于像素右侧网格坐标时反转
            // 但实际上，出现这种情况意味着错误。
            if (formeri_map > latteri_map) {
                float temp = formeri_map;
                formeri_map = latteri_map;
                latteri_map = temp;
                wrongflag++;
#ifdef PIX_EXAMING
                cout << to_string(rcount);
                cout << '\t';
                cout << to_string(colcount);
                cout << " wrong";
                cout<< "\n";
#endif //  PIX_EXAMING    
            }
            else if (formeri_map == latteri_map) {
                formeri_map -= 0.01f;
                warningflag++;
#ifdef PIX_EXAMING
                cout << to_string(rcount);
                cout<< '\t';
                cout << to_string(colcount);
                cout << " zero";
                cout << "\n\n";
#endif //  PIX_EXAMING
            }
            assert(formeri_map < latteri_map);
            int former_index = floor(double(formeri_map));
            int latter_index = ceil(double(latteri_map));
            int sample_pix = latter_index - former_index;
            float sampale_range = (latteri_map - formeri_map);

            assert(sample_pix > 0);
            glm::vec3 res = glm::vec3(0);
            if (sample_pix>=2)
            {
                glm::vec3* sample = new glm::vec3[sample_pix];
                float* weight = new float[sample_pix];
                for (size_t i = 0; i < sample_pix; i++) {
                    sample[i] = convertcvvectoglmvec(this->pix_mat.at<Vec3b>(rcount, i + former_index));
                    weight[i] = 1.0f;
                }
                if (former_index < formeri_map)weight[0] = float(former_index+1) - formeri_map;
                if (latter_index > latteri_map)weight[sample_pix - 1] = latteri_map - float(latter_index-1);
               
                for (size_t i = 0; i < sample_pix; i++) {
                    assert(weight[i] >= 0);
                    res += (weight[i] * sample[i]) / sampale_range;
                }
                delete[]sample;
                delete[]weight;
            }
            // 采样点在同一个像素上
            else {
                res = convertcvvectoglmvec(this->pix_mat.at<Vec3b>(rcount, former_index)) * sampale_range;
            }

#ifdef SHOW_WEIGHT
            if (sampale_range >= 2.0f)sampale_range = 2.0f;
            res = (sampale_range)*glm::vec3(120, 120, 120);
#endif // SHOW_WEIGHT
#ifdef SHOW_MESHINPIC
            if (latteri - long(latteri) < 0.01) {
                res = convertcvvectoglmvec(Vec3b(0, 0, 255));
            }
#endif // SHOW_MESHINPIC
            //res = glm::vec3(256, 257, 259);铭记！！！ char溢出会取余！！这行画出来是黑色。有的半黑不黑，是因为取余后的值不尽相同！！！
            for (size_t i = 0; i < 3; i++)assert(0 <= res[i] <= 255);
            Vec3b bgr = convertglmvctocvvec(res);
            result.at<Vec3b>(rcount, colcount) = bgr;
            
        }
    }
#ifdef PIX_EXAMING
            cout<<wrongflag<<"pix wrong!!\n";
            cout<<warningflag<<"pix has no length!!\n\n";
#endif
    return result;
}

Mat PixelCoordinate::switchMeshY(GridCoordinate& aim)
{
    long wrongflag = 0;
    long warningflag = 0;
    Mat result = this->pix_mat.clone();
    float px, gy, gx, gxn;
    int size_r = (float)pix_mat.rows;
    int size_c = (float)pix_mat.cols;
    int glen = (float)aim.h;
   
    for (size_t colcount = 0; colcount < size_c; colcount++) {

        px = float(0.5 + colcount) / size_c;

        points_set auxiliary_setting_points, mid_setting_points;
        auxiliary_setting_points.resize(glen);
        mid_setting_points.resize(glen);
        for (size_t i = 0; i < glen; i++) {
            auxiliary_setting_points[i] = glm::vec3(this->mesh.row[i].getYbyX(px)*size_r, float(i), 0.0f);
            mid_setting_points[i] = glm::vec3(aim.row[i].getYbyX(px)*size_r, float(i), 0.0f);
        }

        Catmull_Rom3d auxiliary_row_i(auxiliary_setting_points);
        Catmull_Rom3d mid_row_i(mid_setting_points);

        vector<float> index;
        index.resize(size_r + 1);
        for (size_t i = 0; i < size_r + 1; i++) {
            index[i] = auxiliary_row_i.getYbyX(float(i));
        }

        float formeri, latteri, formeri_map, latteri_map;
        latteri = 0.0f;
        latteri_map = 0.0f;

        unsigned int j = 0;
        for (size_t rowcount = 0; rowcount < size_r; rowcount++) {
#ifndef PIX_INDEX_OPTIMIZATION
            j = 0;
#endif //  !PIX_INDEX_OPTIMIZATION 如果启用优化则j不用置0        
            formeri = latteri;
            formeri_map = latteri_map;
            latteri = mid_row_i.getYbyX(float(rowcount + 1));
            while (latteri > index[j + 1]&&j<size_r-1)j++;
            latteri_map = (float)j + (latteri-index[j])/(index[j + 1] - index[j]);            
            if (latteri_map>size_r)latteri_map = size_r;
            if (formeri_map>latteri_map){
                float temp = formeri_map;
                formeri_map = latteri_map;
                latteri_map = temp;   
                wrongflag++;
#ifdef PIX_EXAMING
                cout << to_string(rowcount);
                cout << '\t';
                cout << to_string(colcount);
                cout << " wrong";
                cout << "\n";
#endif //  PIX_EXAMING
            }
            else if(formeri_map == latteri_map){
                formeri_map -= 0.01f;
                warningflag++;
#ifdef PIX_EXAMING
                cout << to_string(rowcount);
                cout << '\t';
                cout << to_string(colcount);
                cout << " zero";
                cout << "\n\n";
#endif //  PIX_EXAMING
            }         
            assert(formeri_map < latteri_map);
            int former_index = floor(formeri_map);
            int latter_index = ceil(latteri_map);
            int sample_pix = latter_index - former_index;
            float sampale_range = (latteri_map - formeri_map);
            assert(sample_pix > 0);
            glm::vec3 res = glm::vec3(0);
            if (sample_pix>=2)
            {
                glm::vec3* sample = new glm::vec3[sample_pix];
                float* weight = new float[int(sample_pix)];
                for (size_t i = 0; i < sample_pix; i++) {
                    sample[i] = convertcvvectoglmvec(this->pix_mat.at<Vec3b>(i + former_index, colcount));
                    weight[i] = 1.0f;
                }
                if (former_index < formeri_map)weight[0] = (former_index + 1) - formeri_map;
                if (latter_index > latteri_map)weight[sample_pix - 1] = latteri_map - (latter_index - 1);
                for (size_t i = 0; i < sample_pix; i++) {
                    assert(weight[i] > 0);
                    res += (weight[i] * sample[i]) / sampale_range;
                }
                delete[]sample;
                delete[]weight;
            }
            else
            {
                res = convertcvvectoglmvec(this->pix_mat.at<Vec3b>(former_index,colcount)) * sampale_range;
            }
#ifdef SHOW_WEIGHT
            if (sampale_range >= 2.0f)sampale_range = 2.0f;
            res = (sampale_range)*glm::vec3(120, 120, 120);
#endif // SHOW_WEIGHT
#ifdef SHOW_MESHINPIC
            if (latteri - long(latteri) < 0.01) {
                res = convertcvvectoglmvec(Vec3b(0, 0, 255));
            }
#endif // SHOW_MESHINPIC
            //res = glm::vec3(-1, -2, -3); 铭记！！！ opencv不会检查char值错误！！这行画出来是黑色！！！ 另外，考虑溢出
            //转化前检查，不然转化时的char（）会把溢出错误抹去！！
            for (size_t i = 0; i < 3; i++)assert(0 <= res[i] <= 255);
            Vec3b bgr = convertglmvctocvvec(res);
            result.at<Vec3b>(rowcount, colcount) = convertglmvctocvvec(res);
        
        }
    }
#ifdef PIX_EXAMING
            cout<<wrongflag<<"pix wrong!!\n";
            cout<<warningflag<<"pix has no length!!\n\n";
#endif
    return result;

}

PixelCoordinate::PixelCoordinate(const char* pic_path, const char* obj_path, unsigned int w, unsigned int h): mesh(obj_path, w, h){
    this->mesh = GridCoordinate(obj_path, w, h);
    this->pix_mat = imgReader(pic_path);
    return;
}

PixelCoordinate::PixelCoordinate(line_mesh& grid_mesh):mesh(grid_mesh){

}

PixelCoordinate::PixelCoordinate(GridCoordinate&d)
{
    this->mesh = d;
}




Mat PixelCoordinate::getMixedIntermediate(GridCoordinate& aim){
    line_mesh auxiliary_grid_mesh;
    int r, h;
    r = this->mesh.h;
    h = this->mesh.w;
    auxiliary_grid_mesh.resize(r);
    for (size_t i = 0; i < r; i++){
        auxiliary_grid_mesh[i].resize(h);
        for (size_t j = 0; j < h; j++){
            glm::vec3 pos=glm::vec3();
            pos.x = aim.grid_mesh[i][j].x;
            pos.y = this->mesh.grid_mesh[i][j].y;
            pos.z = 0.0f;
            auxiliary_grid_mesh[i][j] = pos;
        }
    }    
    GridCoordinate d(auxiliary_grid_mesh);
    PixelCoordinate auxiliary(d);
    auxiliary.pix_mat = switchMeshX(auxiliary.mesh);
    return auxiliary.switchMeshY(aim);
}

glm::vec3 convertcvvectoglmvec(Vec3b d)
{ 
    return glm::vec3(d[0],d[1],d[2]);
}

Vec3b convertglmvctocvvec(glm::vec3 d)
{
    //凡是出现负值、溢出，都是错误
    for (size_t i = 0; i < 3; i++)assert(0 <= d[i] <= 255);
    return Vec3b(char(int(d[0])),char(d[1]),char(d[2]));
}
