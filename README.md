
程式執行方式允許參數設定，

例如： .exe –i infile –o outfile –r 50 –s 1024 -i  
* 輸入影像名稱 -o  
* 輸出影像名稱 -r  
* 重疊區域像素個數 -s  
* 紋理合成影像大小 (1024 x 1024) 

#### 如果 Github 上顯示不友好，請移步 http://codingstory.net/shi-yong-opencv-shi-xian-wen-li-he-cheng-texture-synthesis/

***

# 前言
> A big THANKS to my "Analysis of Algorithms" course teacher, 張元翔

這是“演算法分析”課程第二次機測，具體要求如下
![](http://codingstory.net/content/images/2017/07/QQ--20170712101434.png)
![](http://codingstory.net/content/images/2017/07/QQ--20170712101500.png)
![](http://codingstory.net/content/images/2017/07/QQ--20170712101546.png)

***

**三個參考鏈接**
##### [使用 OpenCV 操作圖像](http://codingstory.net/opencv-shi-xian-tu-xiang-de-du-qu-xian-shi-cun-chu-jian-dan-pin-jie/)
##### [使用 Dynamic Programming 解決生產線排程問題](http://codingstory.net/assembly-line-scheduling-zhuang-pei-xian-diao-du/)
##### [源程式碼已上傳 Github](https://github.com/ShanQincheng/Texture-Synthesis)

***
# 環境
```
OS                                    win10, x64
IDE                                   Visual Studio 2017
```

***

# 思路
簡單的圖像拼接，是將兩張圖像的邊緣靠攏，生成一張新的圖像。**兩張圖像簡單拼接，邊界是一條直線，並且生成新圖像不需要重疊。新生成的圖像長度或寬度是原來兩張圖像的加總。**
![](http://codingstory.net/content/images/2017/05/QQ--20170528190228.png)

圖像無縫拼接，是將兩張圖像的邊緣重疊一部分後，生成一張新的圖像。**圖像的無縫拼接，邊界是一條歪歪扭扭的折線，因為有重疊，所以新生成的圖像長度或寬度要小於原來兩張圖像的加總**
![](http://codingstory.net/content/images/2017/07/view3-3.png)

######紋理合成的關鍵就是，兩張圖像無縫拼接成一張圖像，重疊部分的邊界如何確定。

***

# 開始
* 1、垂直拼貼
 * ( 1 ) 提取重疊區域像素。
 * ( 2 ) 用重疊区域像素權重構造 Assembly-line。
 * ( 3 ) 找出重疊區域的最短路徑，即無縫拼接中兩張圖像重疊部分的邊界路徑。
 * ( 4 ) 合成新圖像。
* 2、水平拼貼
* 3、同時套用垂直拼貼與水平拼貼，合成更大的影像

***
#### 1、垂直拼貼
##### ( 1 ) 提取重疊區域像素
![](http://codingstory.net/content/images/2017/07/QQ--20170712144319-1.png)

從上圖中易知，**重疊區域**由上方圖像的下方區域和下方圖像的上方區域重疊而成。

現在我們需要把上方圖像下方區域的像素提取到 `Mat **upPicture` 資料型別的二維陣列中。同樣需要把下方圖像上方區域的像素提取到 `Mat **downPicture`資料型別的二維陣列中。

```language-python line-numbers
Mat rock1 = rock1_;
Mat rock2 = rock2_;
int i, j;
int cutRows = 50; // number of overlapping rows
int rows = rock1.rows;
int cols = rock2.cols;
Vec3b** upPicture = NULL;
Vec3b** downPicture = NULL;

upPicture = (Vec3b**)calloc(cutRows, sizeof(Vec3b*));
downPicture = (Vec3b**)calloc(cutRows, sizeof(Vec3b*));

for (i = 0; i < cutRows; i++)  // calloc space for pixels from up to down
{
	upPicture[i] = (Vec3b*)calloc(cols, sizeof(Vec3b));
	downPicture[i] = (Vec3b*)calloc(cols, sizeof(Vec3b));
}

for (i = (rows - cutRows); i < rows; i++) {     // initialize upper part overlap pixel
	for (j = 0; j < cols; j++) {
		Vec3b rock1RGB = rock1.at<Vec3b>(i, j); // get pixel from upper picture
		upPicture[i - (rows - cutRows)][j] = rock1RGB; // assign pixel to upPicture pixel array
	}
}

for (i = 0; i < cutRows; i++) {  // initialize lower part overlap pixel
	for (j = 0; j < cols; j++) {
		Vec3b rock2RGB = rock2.at<Vec3b>(i, j); // get pixel from lower picture
		downPicture[i][j] = rock2RGB; // assign pixel to downPicture pixel array
	}
}

```

***

##### ( 2 ) 用重疊区域像素權重構造 Assembly-line
現在，在 `Mat **upPicture` 和 `Mat **downPicture` 二維陣列中，存放了上方圖像和下方圖像將要重疊的所有像素。

由**前言**中對像素權重的介紹可知，Assembly-line 中存放的是重疊區域每個像素的權重。每個像素的權重是根據`Mat **upPicture` 與 `Mat **downPicture` 對應像素點的 RGB 值的歐式距離計算得出的。

**重疊區域有多少個像素點，Assembly-line 就一一對應著多少個工作站。同時這每一個像素的權重，也代表著 Assembly-line 中對應工作站所需要耗費的時間。**
![](http://codingstory.net/content/images/2017/07/QQ--20170712161908-1.png)

```
double** Assembly_actually = NULL;
Vec3b** upPicture = NULL;
Vec3b** downPicture = NULL;

Assembly_actually = (double**)calloc(cutRows, sizeof(double*)); // assembly line array contains each pixel weight calculated by RGB
for (i = 0; i < cutRows; i++)
{
	Assembly_actually[i] = (double*)calloc(cols, sizeof(double));
}

for (i = 0; i < cutRows; i++) {
	for (j = 0; j < cols; j++) {
		// d == √ ( (R1 - R2)^2 + (B1 - B2)^2 + (G1 - G2)^2 )
		Assembly_actually[i][j] = sqrt(pow((upPicture[i][j].val[0] - downPicture[i][j].val[0]), 2) + pow((upPicture[i][j].val[1] - downPicture[i][j].val[1]), 2) + pow((upPicture[i][j].val[2] - downPicture[i][j].val[2]), 2));
	}
}

```

***

##### ( 3 ) 找出重疊區域的最短路徑，即無縫拼接中兩張圖像重疊部分的邊界路徑。
[使用 Dynamic Programming 解決生產線排程問題](http://codingstory.net/assembly-line-scheduling-zhuang-pei-xian-diao-du/)

現在 Assembly-line Scheduling 中各個工作站的權重已經準備好了。  
**在這個問題中**

* 1、Assembly-line 的起點和終點有多個。
* 2、時間耗費不需要計算 Transfer 的時間開銷。
* 3、到達下一個工作站的生產線，有 2 或 3 條可供選擇。
![](http://codingstory.net/content/images/2017/07/Assembly-line-Model.png)

###### 使用 Dynamic Programming 計算出 Assembly-line Scheduling 最短路徑

```
Mat rock1 = rock1_;
Mat rock2 = rock2_;
int rows = rock1.rows;
int cols = rock2.cols;
double** f = NULL; // record the value of the shorest distance from starting point to current pixel
double** Assembly_actually = NULL;
	int** l = NULL; // record assembly line path

	f = (double**)calloc(cutRows, sizeof(double*));
	for (i = 0; i < cutRows; i++) {
		f[i] = (double*)calloc(cols, sizeof(double));
	}

	// record assembly line path
	l = (int**)calloc(cols, sizeof(int*));
	for (i = 0; i < cutRows; i++) {
		l[i] = (int*)calloc(cols, sizeof(int));
	}

	for (int i = 0; i < cutRows; i++)
	{
		l[i][0] = i; // initialize first step as itself
	}
	// initialize the pixel value of the first col
	for (i = 0; i < cutRows; i++) {
		f[i][0] = Assembly_actually[i][0]; // initialize first step distance
	}

	for (i = 1; i < cols; i++) {
		for (j = 0; j < cutRows; j++) {
			// if there are three path can be choosed
			if (j != 0 && j != (cutRows - 1)) {
				// if middle path is shortest
				if ((f[j][i - 1] + Assembly_actually[j][i] < f[j - 1][i - 1] + Assembly_actually[j][i]) && (f[j][i - 1] + Assembly_actually[j][i] < f[j + 1][i - 1] + Assembly_actually[j][i])) {
					f[j][i] = f[j][i - 1] + Assembly_actually[j][i];
					l[j][i] = j;
				}
				// if upper path is shortest
				else if ((f[j - 1][i - 1] + Assembly_actually[j][i] < f[j][i - 1] + Assembly_actually[j][i]) && (f[j - 1][i - 1] + Assembly_actually[j][i] < f[j + 1][i - 1] + Assembly_actually[j][i])) {
					f[j][i] = f[j - 1][i - 1] + Assembly_actually[j][i];
					l[j][i] = j - 1;
				}
				// if lower path is shortest
				else if ((f[j + 1][i - 1] + Assembly_actually[j][i] < f[j][i - 1] + Assembly_actually[j][i]) && (f[j + 1][i - 1] + Assembly_actually[j][i] < f[j - 1][i - 1] + Assembly_actually[j][i])) {
					f[j][i] = f[j + 1][i - 1] + Assembly_actually[j][i];
					l[j][i] = j + 1;
				}
				// if there are two path distance are equal, choose middle path by default.  I am a lazy man...
				else {
					f[j][i] = f[j][i - 1] + Assembly_actually[j][i];
					l[j][i] = j;
				}
			}
			// if there are two path can be choosed, the top line or the bottom line
			else {
				// the top line
				if (j == 0) {
					// if middle path is shortest
					if (f[j][i - 1] + Assembly_actually[j][i] < f[j + 1][i - 1] + Assembly_actually[j][i]) {
						f[j][i] = f[j][i - 1] + Assembly_actually[j][i];
						l[j][i] = j;
					}
					// if lower path is shorest
					else if (f[j + 1][i - 1] + Assembly_actually[j][i] < f[j][i - 1] + Assembly_actually[j][i]) {
						f[j][i] = f[j + 1][i - 1] + Assembly_actually[j][i];
						l[j][i] = j + 1;
					}
					// if the middle and lower path are equal ,choose the middle path by default
					else {
						f[j][i] = f[j][i - 1] + Assembly_actually[j][i];
						l[j][i] = j;
					}

				}
				// the bottom line
				else {
					// if middle path is shortest
					if (f[j][i - 1] + Assembly_actually[j][i] < f[j - 1][i - 1] + Assembly_actually[j][i]) {
						f[j][i] = f[j][i - 1] + Assembly_actually[j][i];
						l[j][i] = j;
					}
					// if upper path is shortest
					else if (f[j - 1][i - 1] + Assembly_actually[j][i] < f[j][i - 1] + Assembly_actually[j][i]) {
						f[j][i] = f[j - 1][i - 1] + Assembly_actually[j][i];
						l[j][i] = j - 1;
					}
					// if middle and upper path are euqal, choose the middle path by default
					else {
						f[j][i] = f[j][i - 1] + Assembly_actually[j][i];
						l[j][i] = j;
					}
				}
			}
		}
	}

	double minDistance = DBL_MAX;
	int position = -1;
	int lineNum = -1;
	int* boundaryPosition;
	boundaryPosition = (int*)calloc(cols, sizeof(int));  // record the shortest distance path

														 // find the shortest distance from left boundary to right boundary
	for (i = 0; i < cutRows; i++) {
		if (f[i][cols - 1] < minDistance) {
			minDistance = f[i][cols - 1];
			position = i;
		}
		else
			continue;
	}

	// starting from the shortest distance path last pixel
	lineNum = l[position][cols - 1];

	for (i = cols - 1; i >= 0; i--) {
		lineNum = l[lineNum][i];  // from which row to here, upper, middle or lower ?
		boundaryPosition[i] = lineNum; // record the shortest distance path each step
	}
```

此時，在垂直拼接中，上下兩張圖像的邊界像素位置，存放在 `boundaryPosition[]` 陣列中。**( 注意，`boundaryPosition[]` 陣列中存放的边界像素位置，是相對於重疊區域上邊界來定位的。相對於上方圖像或下方圖像的位置，需要通過圖像的上邊界、圖像的總列數以及重疊區域的總列數來判斷)**

###### Example
![](http://codingstory.net/content/images/2017/07/boundaryPotion---2.png)

```
/*
    An example codes
*/

int upPicture.rows; // the number of up picture total rows
int downPicture.rows; // the number of down picture total rows
int cutRows; // the number of overlapping rows


if( boundaryPosition[x] == y )
{
  the edge pixel coordinate in upPicture == ( x, ( (upPicture.rows - cutRows) + y ) )

  the edge pixel coordinate in downPicture == ( x, y )
}

```

***

##### ( 4 ) 合成新圖像
至此我們已經找出了重疊區域的邊界，現在應該通過邊界這一關鍵的折線，將上方圖像和下方圖像合成為新圖像了。

新圖像中，座標在邊界，及邊界上方的像素點，用上方圖像的像素賦給新圖像。

新圖像中，座標在邊界下方的像素點，用下方圖像的像素賦給新圖像。

新圖像的 `rows number == upPicture.rows + downPicture.rows - cutRows;`   
**注意:** 新圖像像素的座標，和下方圖像像素座標的相對位置是不同的

```
Mat rock1 = rock1_;
Mat rock2 = rock2_;
Mat kimCreate;

int rows = rock1.rows;
int cutRows = 50; // number of overlapping rows
int cols = rock2.cols;
int* boundaryPosition;

kimCreate.create(2 * rows - cutRows, cols, CV_8UC3);  // synthesis upper picture and lower picture for one picture
	for (i = 0; i < (2 * rows - cutRows); i++) {
		for (j = 0; j < cols; j++) {
			if (i <= boundaryPosition[j] + (rows - cutRows)) {  // above the boundary edge
				kimCreate.at<Vec3b>(i, j) = rock1.at<Vec3b>(i, j);
			}
			else {
				kimCreate.at<Vec3b>(i, j) = rock2.at<Vec3b>((i - (rows - cutRows)), j);
			}
		}
	}
	return kimCreate;
```

###### 效果圖

![](http://codingstory.net/content/images/2017/07/QQ--20170712200223.png)

***

#### 2、水平拼貼
###### 水平拼貼與垂直拼貼的不同點是

* 1、邊界由橫向折線變為縱向折線
* 2、Assembly-line 也從橫向變為了縱向

###### 那麼這可以有兩種編程思想

* 1、將縱向的 Assembly-line 在程式碼中用橫向的方法來寫，但是要注意 rows and cols 不要弄混了。
* 2、重新實現一套縱向的 Assembly-line。

###### 水平拼貼與人常規思維相反，注意寫此處程式碼時頭腦一定要清晰冷靜！！！
```language-python line-numbers
#include "stdafx.h"

Mat Horizontal(Mat rock1_, Mat rock2_, int cutCols)
{
	Mat rock1 = rock1_;
	Mat rock2 = rock2_;
	int i, j;
	double** Assembly_actually = NULL;
	Mat kimCreate;
	Vec3b** leftPicture = NULL;
	Vec3b** rightPicture = NULL;
	int rows = rock1.rows;
	int cols = rock2.cols;

	leftPicture = (Vec3b**)calloc(cutCols, sizeof(Vec3b*));
	rightPicture = (Vec3b**)calloc(cutCols, sizeof(Vec3b*));

	for (i = 0; i < cutCols; i++)
	{
		leftPicture[i] = (Vec3b*)calloc(rows, sizeof(Vec3b));
		rightPicture[i] = (Vec3b*)calloc(rows, sizeof(Vec3b));
	}

	// initialize left picture array, from left to right boundary
	for (i = (cols - cutCols); i < cols; i++) {
		for (j = 0; j < rows; j++) {
			Vec3b rock1RGB = rock1.at<Vec3b>(j, i);
			leftPicture[i - (cols - cutCols)][j] = rock1RGB;
		}
	}

	// initialize right picture array, from left boundary to right
	for (i = 0; i < cutCols; i++) {
		for (j = 0; j < rows; j++) {
			Vec3b rock2RGB = rock2.at<Vec3b>(j, i);
			rightPicture[i][j] = rock2RGB;
		}
	}

	// initialize assembly line array, assembly line array contains each pixel weight in overlapping area
	Assembly_actually = (double**)calloc(cutCols, sizeof(double*));
	for (i = 0; i < cutCols; i++)
	{
		Assembly_actually[i] = (double*)calloc(rows, sizeof(double));
	}


	for (i = 0; i < cutCols; i++) {
		for (j = 0; j < rows; j++) {
			// d == √ ( (R1 - R2)^2 + (B1 - B2)^2 + (G1 - G2)^2 )
			Assembly_actually[i][j] = sqrt(pow((leftPicture[i][j].val[0] - rightPicture[i][j].val[0]), 2) + pow((leftPicture[i][j].val[1] - rightPicture[i][j].val[1]), 2) + pow((leftPicture[i][j].val[2] - rightPicture[i][j].val[2]), 2));
		}
	}

	double** f = NULL; // record the value of the shorest distance from starting point to current pixel
	int** l = NULL; // record assembly line path

	f = (double**)calloc(cutCols, sizeof(double*));
	for (i = 0; i < cutCols; i++) {
		f[i] = (double*)calloc(rows, sizeof(double));
	}
	l = (int**)calloc(cutCols, sizeof(int*));
	for (i = 0; i < cutCols; i++) {
		l[i] = (int*)calloc(rows, sizeof(int));
	}

	for (int i = 0; i < cutCols; i++)
	{
		l[i][0] = i; // initialize first step as itself
	}
	for (i = 0; i < cutCols; i++) {
		f[i][0] = Assembly_actually[i][0]; // initialize first step distance
	}

	for (i = 1; i < rows; i++) {
		for (j = 0; j < cutCols; j++) {
			// if there are three paths can be choosed
			if (j != 0 && j != (cutCols - 1)) {
				// if middle path is the shortest path
				if ((f[j][i - 1] + Assembly_actually[j][i] < f[j - 1][i - 1] + Assembly_actually[j][i]) && (f[j][i - 1] + Assembly_actually[j][i] < f[j + 1][i - 1] + Assembly_actually[j][i])) {
					f[j][i] = f[j][i - 1] + Assembly_actually[j][i];
					l[j][i] = j;
				}
				// if upper path is the shortest path
				else if ((f[j - 1][i - 1] + Assembly_actually[j][i] < f[j][i - 1] + Assembly_actually[j][i]) && (f[j - 1][i - 1] + Assembly_actually[j][i] < f[j + 1][i - 1] + Assembly_actually[j][i])) {
					f[j][i] = f[j - 1][i - 1] + Assembly_actually[j][i];
					l[j][i] = j - 1;
				}
				// if lower path is the shortest path
				else if ((f[j + 1][i - 1] + Assembly_actually[j][i] < f[j][i - 1] + Assembly_actually[j][i]) && (f[j + 1][i - 1] + Assembly_actually[j][i] < f[j - 1][i - 1] + Assembly_actually[j][i])) {
					f[j][i] = f[j + 1][i - 1] + Assembly_actually[j][i];
					l[j][i] = j + 1;
				}
				else {
					f[j][i] = f[j][i - 1] + Assembly_actually[j][i];
					l[j][i] = j;
				}
			}
			// if there are two paths can be choosed
			else {
				// left boundary
				if (j == 0) {
					// if middle path is the shortest path
					if (f[j][i - 1] + Assembly_actually[j][i] < f[j + 1][i - 1] + Assembly_actually[j][i]) {
						f[j][i] = f[j][i - 1] + Assembly_actually[j][i];
						l[j][i] = j;
					}
					// if upper path is the shortest path
					else if (f[j + 1][i - 1] + Assembly_actually[j][i] < f[j][i - 1] + Assembly_actually[j][i]) {
						f[j][i] = f[j + 1][i - 1] + Assembly_actually[j][i];
						l[j][i] = j + 1;
					}
					else {
						f[j][i] = f[j][i - 1] + Assembly_actually[j][i];
						l[j][i] = j;
					}

				}
				// right boundary
				else {
					if (f[j][i - 1] + Assembly_actually[j][i] < f[j - 1][i - 1] + Assembly_actually[j][i]) {
						f[j][i] = f[j][i - 1] + Assembly_actually[j][i];
						l[j][i] = j;
					}
					else if (f[j - 1][i - 1] + Assembly_actually[j][i] < f[j][i - 1] + Assembly_actually[j][i]) {
						f[j][i] = f[j - 1][i - 1] + Assembly_actually[j][i];
						l[j][i] = j - 1;
					}
					else {
						f[j][i] = f[j][i - 1] + Assembly_actually[j][i];
						l[j][i] = j;
					}
				}
			}
		}
	}

	double minDistance = DBL_MAX;
	int position = -1;
	int lineNum = -1;
	int* boundaryPosition;
	boundaryPosition = (int*)calloc(rows, sizeof(int));

	// find the shortest path from left boundary to right boundary
	for (i = 0; i < cutCols; i++) {
		if (f[i][rows - 1] < minDistance) {
			minDistance = f[i][rows - 1];
			position = i;
		}
		else
			continue;
	}

	//lineNum = l[rows - 1][position];
	lineNum = l[position][rows - 1];
	for (i = rows - 1; i >= 0; i--) {
		lineNum = l[lineNum][i];
		boundaryPosition[i] = lineNum;
	}

	/*
	The codes below are not in line with human thinking habits,
	it's difficult to understand them
	*/

	// create a picture , note the paras order --  rows, cols, type
	kimCreate.create(rows, 2 * cols - cutCols, CV_8UC3);
	// assign the new picture from left to right and up to down
	for (i = 0; i < rows; i++) {
		for (j = 0; j < (2 * cols - cutCols); j++) {
			if (j <= boundaryPosition[i] + (cols - cutCols)) {  // at the left of the boundary edge
				kimCreate.at<Vec3b>(i, j) = rock1.at<Vec3b>(i, j);
			}
			else {
				kimCreate.at<Vec3b>(i, j) = rock2.at<Vec3b>(i, (j - (cols - cutCols)));
			}

		}
	}

	return kimCreate;
}
```

###### 效果圖
![](http://codingstory.net/content/images/2017/07/QQ--20170712200333.png)

****

####  3、同時套用垂直拼貼與水平拼貼，合成更大的影像
此次機測最終目的是合成一個更大的圖像。

那麼先用水平拼貼將圖像水平擴展為更寬的圖像。

然後再用垂直拼貼將圖像垂直擴展為更長的圖像。( *順序可以調換* )

最後，根據用戶輸入，將圖像進行裁切微調，以用戶要求的尺寸進行顯示即可。
```language-python line-numbers
int main(int argc, char** argv) {
	string infileName1, infileName2, outfileName;
	int cutRows = -1;
	int goalSize = -1;
	for (int i = 0; i < 8; i++)
	{
		if (strcmp("-i", argv[i + 1]) == 0)
		{
			infileName1 = argv[i + 2];
			infileName2 = argv[i + 3];
			i += 2;
		}
		else if (strcmp("-o", argv[i + 1]) == 0)
		{
			outfileName = argv[i + 2];
			i++;
		}
		else if (strcmp("-r", argv[i + 1]) == 0)
		{
			cutRows = atoi(argv[i + 2]);
			i++;
		}
		else if (strcmp("-s", argv[i + 1]) == 0)
		{
			goalSize = atoi(argv[i + 2]);
			i++;
		}
	}

	int i = 0; // iterator variable
	int j = 0; // iterator variable
	int tempPicWidth = -1;
	int tempPicHeight = -1;
	int outputHSize = goalSize;
	int outputWSize = goalSize;
	double** Assembly_actuallyCopy = NULL;
	Mat rock1 = imread(infileName1);
	Mat rock2 = imread(infileName2);
	Mat tempPic;
	Mat outputPic;
	Mat tempPic1 = rock1;
	Mat tempPic2 = rock2;
	
	
	// Horizontal stretching
	while (true) {
		tempPic = Horizontal(tempPic1, tempPic2, cutRows);

		tempPicWidth = tempPic.cols;
		if (tempPicWidth < outputWSize) {
			tempPic1 = tempPic;
			tempPic2 = tempPic;
		}
		else {
			tempPic1 = tempPic;
			tempPic2 = tempPic;
			break;
		}
	}
	
	// Vertical stretching
	while (true) {
		tempPic = Vertical(tempPic1, tempPic2, cutRows);

		tempPicHeight = tempPic.rows;
		if (tempPicHeight < outputHSize) {
			tempPic1 = tempPic;
			tempPic2 = tempPic;
		}
		else {
			tempPic1 = tempPic;
			tempPic2 = tempPic;
			break;
		}
	}
	
	
//	outputPic = tempPic;
	outputPic = Mat(tempPic, Rect(0, 0, outputWSize, outputHSize));
	namedWindow("rock1  copyRight: codingstory", WINDOW_AUTOSIZE);
	resizeWindow("rock1  copyRight : codingstory", outputWSize, outputHSize);
	imshow("rock1  copyRight: codingstory", outputPic);
	imwrite(outfileName, outputPic);
	waitKey(0);

	return 0;
}
```

###### 最終效果
![](http://codingstory.net/content/images/2017/07/QQ--20170712202057.png)

****
# 以上
2017 年 7 月 12 日

