// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

Mat Vertical(Mat rock1_, Mat rock2_, int cutRows);
Mat Horizontal(Mat rock1_, Mat rock2_, int cutCols);
/* run this program using the console pauser or add your own getch, system("pause") or input loop */


//void Vertical(Vec3b**, Vec3b** , Mat , Mat , int , int , int );

// TODO: reference additional headers your program requires here
