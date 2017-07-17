#include "stdafx.h"

Mat Vertical(Mat rock1_, Mat rock2_, int cutRows) {
	Mat rock1 = rock1_;
	Mat rock2 = rock2_;
	int i, j;
	double** Assembly_actually = NULL;
	Mat kimCreate;
	Vec3b** upPicture = NULL;
	Vec3b** downPicture = NULL;
	int rows = rock1.rows;
	int cols = rock2.cols;


	upPicture = (Vec3b**)calloc(cutRows, sizeof(Vec3b*));
	downPicture = (Vec3b**)calloc(cutRows, sizeof(Vec3b*));

	for (i = 0; i < cutRows; i++)  // calloc space for pixels from up to down
	{
		upPicture[i] = (Vec3b*)calloc(cols, sizeof(Vec3b));
		downPicture[i] = (Vec3b*)calloc(cols, sizeof(Vec3b));
	}

	Assembly_actually = (double**)calloc(cutRows, sizeof(double*)); // assembly line array contains each pixel weight calculated by RGB
	for (i = 0; i < cutRows; i++)
	{
		Assembly_actually[i] = (double*)calloc(cols, sizeof(double));
	}

	for (i = (rock1.rows - cutRows); i < rock1.rows; i++) {     // initialize upper part overlap pixel
		for (j = 0; j < rock1.cols; j++) {
			Vec3b rock1RGB = rock1.at<Vec3b>(i, j); // get pixel from upper picture
			upPicture[i - (rock1.rows - cutRows)][j] = rock1RGB; // assign pixel to upPicture pixel array
		}
	}

	for (i = 0; i < cutRows; i++) {  // initialize lower part overlap pixel
		for (j = 0; j < rock2.cols; j++) {
			Vec3b rock2RGB = rock2.at<Vec3b>(i, j); // get pixel from lower picture
			downPicture[i][j] = rock2RGB; // assign pixel to downPicture pixel array
		}
	}

	for (i = 0; i < cutRows; i++) {
		for (j = 0; j < cols; j++) {
			// d ==  ( (R1 - R2)^2 + (B1 - B2)^2 + (G1 - G2)^2 )
			Assembly_actually[i][j] = sqrt(pow((upPicture[i][j].val[0] - downPicture[i][j].val[0]), 2) + pow((upPicture[i][j].val[1] - downPicture[i][j].val[1]), 2) + pow((upPicture[i][j].val[2] - downPicture[i][j].val[2]), 2));
		}
	}

	double** f = NULL; // record the value of the shorest distance from starting point to current pixel
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
		lineNum = l[lineNum][i];  // from which row to here, upper, middle or lower 
		boundaryPosition[i] = lineNum; // record the shortest distance path each step
	}

	kimCreate.create(rock1.rows + rock2.rows - cutRows, cols, CV_8UC3);  // synthesis upper picture and lower picture for one picture
	for (i = 0; i < (rock1.rows + rock2.rows - cutRows); i++) {
		for (j = 0; j < cols; j++) {
			if (i <= boundaryPosition[j] + (rock1.rows - cutRows)) {  // above the boundary edge
				kimCreate.at<Vec3b>(i, j) = rock1.at<Vec3b>(i, j);
			}
			else {
				kimCreate.at<Vec3b>(i, j) = rock2.at<Vec3b>((i - (rock1.rows - cutRows)), j);
			}
		}
	}
	return kimCreate;
}