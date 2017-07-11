#include "stdafx.h"


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

	// Horizontal stretching
	while (true) {
		tempPic = Horizontal(tempPic1, tempPic2, cutRows);

		tempPicWidth = tempPic.cols;
		if (tempPicWidth < outputWSize) {
			tempPic1 = tempPic;
			tempPic2 = tempPic;
		}else {
			tempPic1 = tempPic;
			tempPic2 = tempPic;
			break;
		}
	}

	outputPic = Mat(tempPic, Rect(0, 0, outputWSize, outputHSize));
	namedWindow("rock1  copyRight: codingstory", WINDOW_AUTOSIZE);
	resizeWindow("rock1  copyRight : codingstory", outputWSize, outputHSize);
	imshow("rock1  copyRight: codingstory", outputPic);

	waitKey(0);

	return 0;
}


