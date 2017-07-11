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
			// d == ¡Ì ( (R1 - R2)^2 + (B1 - B2)^2 + (G1 - G2)^2 )
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