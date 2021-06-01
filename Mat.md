[TOC]

### Opencv中的矩阵类（cv::Mat）

Mat是Opencv的核心数据结构，并且是使用c++设计的数据结构。如果能稍微了解一下这个数据结构，不仅有利于对opencv库的使用，也能对c++开发有所启发



#### 数据结构

先看看官方给定描述

> *Mat* is basically a class with two data parts: **the matrix header** (containing information such as the size of the matrix, the method used for storing, at which address is the matrix stored, and so on) and **a pointer to the matrix** containing the pixel values (taking any dimensionality depending on the method chosen for storing) . The **matrix header size is constant**, however the size of the matrix itself may vary from image to image and usually is larger by *orders of magnitude*（数量级）. 

数据结构很正常，分大小固定的 “头部” 以及指向真正存放数据的内存块的指针。注意数据的存储模式是可变的，模式记录在一个属性变量里。



常用的属性大概有这些

* 数据大小
* 数据存储方式



#### 复制、析构

​	此前笔者设计类的时候也经常采用这种数据结构，然后因为害怕复制的时候出错，选择规避复制或是用一些很麻烦且低效的方式，比如真的把数据区复制一遍。至于析构，你会指望一个菜鸡去考虑析构吗？



> *5月27日早7:00 补充：*
>
> 还真得考虑析构。。。
>
> 不然很有可能，
>
> 你把一大堆的渲染任务下方后去睡觉
>
> 期待着醒来后是一大串处理好的图片
>
> 但如果不随用随放
>
> 那么最终得到的是内存不足的报错。。。
>
> 使用 成员函数 release()即可调用一个Mat()的析构函数，但注意这个函数只能析构一“层”Mat，不能直接保证清理    data。







​	mat在设计之初就考虑到，一个实例在程序中可能会生成多个副本。因此好好定义复制是有必要的。

​	例如有时候，我们对副本的操作只是访问。这个时候就没有必要连着数据区一块复制。

​	mat的复制构造函数和operator=都是只复制头部和指针。所有复制出来的实例的数据指针，都指向同一个数据区。更改头部时，各个副本是独立的，而更改数据时，各个副本是同步的。大多时候，我们只需要更改头部中的一些属性，或是更改**访问数据的方式**，因此没有必要复制数据区





​	但多个这样的副本涉及到一个问题：最后让谁来清理这个数据区。

> The short answer is: the last object that used it. This is handled by using a reference counting mechanism. Whenever somebody copies a header of a *Mat* object, a counter is increased for the matrix. Whenever a header is cleaned, this counter is decreased. When the counter reaches zero the matrix is freed.  

​	妙哇。这种方法简单而高效。类似Linux的文件系统：一个文件可以拥有多个副本，但数据只有一块，删除一个副本后引用数减一，减为零时才完全删除。

​	这样一来，mat的实例可以放心地作为函数的参数使用，而不必担心没用完就被析构的问题了。

​	但如果有更改副本中的数据的需要，就必须老老实实地把数据区复制一份了。opencv同样实现了这种需求。

>  Sometimes you will want to copy the matrix itself too, so OpenCV provides [cv::Mat::clone()](https://docs.opencv.org/master/d3/d63/classcv_1_1Mat.html#adff2ea98da45eae0833e73582dd4a660) and [cv::Mat::copyTo()](https://docs.opencv.org/master/d3/d63/classcv_1_1Mat.html#a33fd5d125b4c302b0c9aa86980791a77) functions. 

​	*以后设计类的时候，也应该贯彻这种思想，把复制的需求分开处理，只复制可能要修改的部分即可*



#### 数据存储方式

##### 颜色空间和颜色系统

​	opencv作为一个视觉库，主要处理的就是像素颜色。

​	如果没理解错的话，颜色系统用于描述颜色，而颜色空间指的是这个颜色系统的取值范围。程序员会根据颜色空间对像素的颜色进行编码存储。

​	常见的颜色空间如下

* RGB。在opencv中存的是BGR，也就是红和蓝的位置倒过来放

* HSV、HLS, 将颜色分解为它们的色调、饱和度、强度/亮度分量，三者分开存储，因此便于调节

*   YCRCB,被JPEG图像格式所使用 

  针对不同的颜色空间，可以采用不同的数据存储方式，怎么选择取决于对效率与效果的取舍。

##### 定义Mat结构

​	mat的构造函数

* [cv::Mat::Mat](https://docs.opencv.org/master/d3/d63/classcv_1_1Mat.html#a2c4229732da267f1fe385458af3896d8) Constructor 

  ```c++
  Mat M(2,2, CV_8UC3, Scalar(0,0,255));
  ```

*  For two dimensional and multichannel images we first define their size: row and column count wise. 

* Then we need to specify the data type to use for storing the elements and the number of channels per matrix point. To do this we have multiple definitions constructed according to the following convention: 

  `CV_`

  `[The number of bits per item（每一个元素的字节数）]`
  
  `[Signed or Unsigned（是否非负）]`
  
  `[Type Prefix]`
  
  `C`
  
  `[The channel number(最多为4)]`
  
  还有大量的构造方法，需要的时候直接查就好
  
  

##### 图像在mat中的存储

数据的存储方式取决于其颜色空间

* 例如灰度图的存储方式如下：

 ![tutorial_how_matrix_stored_1.png](Mat.assets/tutorial_how_matrix_stored_1.png) 

* RGB的存储如下

 ![tutorial_how_matrix_stored_2.png](Mat.assets/tutorial_how_matrix_stored_2.png) 

*记得顺序其实是BGR*

如果没有猜错的话，每一行都是连续存储的。

而行与行之间是否是连续的呢？这得取决于内存是否足够连续存储这些数据。

（连续存储的数据在与gpu缓冲区交互的时候方便很多了，不知道opencv能不能强制连续存储）

如果需要确认数据是否连续，可以使用mat的成员函数bool isContinuous();

##### 使用operator[]高效地访问像素

但不论是否连续存储的，mat都提供了随机访问的方式。如果追求高效并确保index的正确性，可以直接用指针+index的方式来访问数据。

`T* ptr<T>(int r)` ,返回第r行的首地址，然后就可以用`T operator[int c]`访问第c列的数据了。

##### 使用迭代器安全地访问像素

可以使用MatIterator_\<T\>类来访问像素。

Mat类中有一个begin指针和一个end指针，迭代器可以在这两个指针直接安全地便利数据

##### 使用at函数

at函数适用于访问特定元素，但不适用于遍历。at函数其实是先根据给出的元素的坐标（行、列）以及元素的类型，计算出待访问的对象的地址，然后再根据这个地址去访问元素。at()会检查输入的坐标，如果有错误会报错，但不会终止。所以这个函数适用于debug模式

##### 核心函数（原文就是这么写的。。）

我猜核心函数的意思应该就是mat针对一些常用的函数做的优化方案。

比如原文中用于举例的逐像素修改。例子中的修改函数是简单的值到值的映射，比如减去一个固定的的值，那么这种映射可以放在一张一张表里（lookuptable），然后使用核心函数LUT（Mat& old, Mat& lookuptable,Mat& res）来获得结果集res。至于LUT是怎么优化或是代替遍历，我们就不得而知了。



再多说一嘴，这个lookuptable挺有意思的：用查询的方式来代替计算，让人眼前一亮。不得不说，读优质的文档真的是一种享受，忽略自己英文比较烂的客观事实，顶级程序员的代码思想给我这样的菜鸡带来的冲击是巨大的，一个上午读来颇有醍醐灌顶之感。





附：比较不同遍历方式的源码

```c++
#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include "opencv2/imgcodecs.hpp"
#include <opencv2/highgui.hpp>
#include <iostream>
#include <sstream>

using namespace std;
using namespace cv;

static void help()
{
    cout
        << "\n--------------------------------------------------------------------------" << endl
        << "This program shows how to scan image objects in OpenCV (cv::Mat). As use case"
        << " we take an input image and divide the native color palette (255) with the "  << endl
        << "input. Shows C operator[] method, iterators and at function for on-the-fly item address calculation."<< endl
        << "Usage:"                                                                       << endl
        << "./how_to_scan_images <imageNameToUse> <divideWith> [G]"                       << endl
        << "if you add a G parameter the image is processed in gray scale"                << endl
        << "--------------------------------------------------------------------------"   << endl
        << endl;
}

Mat& ScanImageAndReduceC(Mat& I, const uchar* table);
Mat& ScanImageAndReduceIterator(Mat& I, const uchar* table);
Mat& ScanImageAndReduceRandomAccess(Mat& I, const uchar * table);

int main( int argc, char* argv[])
{
    help();
    if (argc < 3)
    {
        cout << "Not enough parameters" << endl;
        return -1;
    }

    Mat I, J;
    if( argc == 4 && !strcmp(argv[3],"G") )
        I = imread(argv[1], IMREAD_GRAYSCALE);
    else
        I = imread(argv[1], IMREAD_COLOR);

    if (I.empty())
    {
        cout << "The image" << argv[1] << " could not be loaded." << endl;
        return -1;
    }

    //! [dividewith]
    int divideWith = 0; // convert our input string to number - C++ style
    stringstream s;
    s << argv[2];
    s >> divideWith;
    if (!s || !divideWith)
    {
        cout << "Invalid number entered for dividing. " << endl;
        return -1;
    }

    uchar table[256];
    for (int i = 0; i < 256; ++i)
       table[i] = (uchar)(divideWith * (i/divideWith));
    //! [dividewith]

    const int times = 100;
    double t;

    t = (double)getTickCount();

    for (int i = 0; i < times; ++i)
    {
        cv::Mat clone_i = I.clone();
        J = ScanImageAndReduceC(clone_i, table);
    }

    t = 1000*((double)getTickCount() - t)/getTickFrequency();
    t /= times;

    cout << "Time of reducing with the C operator [] (averaged for "
         << times << " runs): " << t << " milliseconds."<< endl;

    t = (double)getTickCount();

    for (int i = 0; i < times; ++i)
    {
        cv::Mat clone_i = I.clone();
        J = ScanImageAndReduceIterator(clone_i, table);
    }

    t = 1000*((double)getTickCount() - t)/getTickFrequency();
    t /= times;

    cout << "Time of reducing with the iterator (averaged for "
        << times << " runs): " << t << " milliseconds."<< endl;

    t = (double)getTickCount();

    for (int i = 0; i < times; ++i)
    {
        cv::Mat clone_i = I.clone();
        ScanImageAndReduceRandomAccess(clone_i, table);
    }

    t = 1000*((double)getTickCount() - t)/getTickFrequency();
    t /= times;

    cout << "Time of reducing with the on-the-fly address generation - at function (averaged for "
        << times << " runs): " << t << " milliseconds."<< endl;

    //! [table-init]
    Mat lookUpTable(1, 256, CV_8U);
    uchar* p = lookUpTable.ptr();
    for( int i = 0; i < 256; ++i)
        p[i] = table[i];
    //! [table-init]

    t = (double)getTickCount();

    for (int i = 0; i < times; ++i)
        //! [table-use]
        LUT(I, lookUpTable, J);
        //! [table-use]

    t = 1000*((double)getTickCount() - t)/getTickFrequency();
    t /= times;

    cout << "Time of reducing with the LUT function (averaged for "
        << times << " runs): " << t << " milliseconds."<< endl;
    return 0;
}

//! [scan-c]
Mat& ScanImageAndReduceC(Mat& I, const uchar* const table)
{
    // accept only char type matrices
    CV_Assert(I.depth() == CV_8U);

    int channels = I.channels();

    int nRows = I.rows;
    int nCols = I.cols * channels;

    if (I.isContinuous())
    {
        nCols *= nRows;
        nRows = 1;
    }

    int i,j;
    uchar* p;
    for( i = 0; i < nRows; ++i)
    {
        p = I.ptr<uchar>(i);
        for ( j = 0; j < nCols; ++j)
        {
            p[j] = table[p[j]];
        }
    }
    return I;
}
//! [scan-c]

//! [scan-iterator]
Mat& ScanImageAndReduceIterator(Mat& I, const uchar* const table)
{
    // accept only char type matrices
    CV_Assert(I.depth() == CV_8U);

    const int channels = I.channels();
    switch(channels)
    {
    case 1:
        {
            MatIterator_<uchar> it, end;
            for( it = I.begin<uchar>(), end = I.end<uchar>(); it != end; ++it)
                *it = table[*it];
            break;
        }
    case 3:
        {
            MatIterator_<Vec3b> it, end;
            for( it = I.begin<Vec3b>(), end = I.end<Vec3b>(); it != end; ++it)
            {
                (*it)[0] = table[(*it)[0]];
                (*it)[1] = table[(*it)[1]];
                (*it)[2] = table[(*it)[2]];
            }
        }
    }

    return I;
}
//! [scan-iterator]

//! [scan-random]
Mat& ScanImageAndReduceRandomAccess(Mat& I, const uchar* const table)
{
    // accept only char type matrices
    CV_Assert(I.depth() == CV_8U);

    const int channels = I.channels();
    switch(channels)
    {
    case 1:
        {
            for( int i = 0; i < I.rows; ++i)
                for( int j = 0; j < I.cols; ++j )
                    I.at<uchar>(i,j) = table[I.at<uchar>(i,j)];
            break;
        }
    case 3:
        {
         Mat_<Vec3b> _I = I;

         for( int i = 0; i < I.rows; ++i)
            for( int j = 0; j < I.cols; ++j )
               {
                   _I(i,j)[0] = table[_I(i,j)[0]];
                   _I(i,j)[1] = table[_I(i,j)[1]];
                   _I(i,j)[2] = table[_I(i,j)[2]];
            }
         I = _I;
         break;
        }
    }

    return I;
}
//! [scan-random]
```















 



