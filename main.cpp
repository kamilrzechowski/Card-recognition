#include "lipa.h"
#include <iostream>
#include "Graph.h"
#include <cmath>
#include <math.h>
#include <stdlib.h> 

// threshold function
#define THRESHOLD(size, c) (c/size)

struct colors {
	int id;
	float color;
	colors() : id(-1),
		color(0) {}
};

struct Point
{
	int x;
	int y;
	Point() :x(0),
		y(0) {}
};

struct Rect{
	Point topLeft;
	Point bottomRight;
};

struct Card {
	int color;		// 0-no color, 1-trefl, 2-karo, 3-kier, 4-pik
	std::string colorName;
	int number;
	int x1, y1, x2, y2;
	Card() :color(0), colorName(""),
		number(0), x1(0), y1(0), x2(0), y2(0) {}
};

int powierzchnia = 0, obwod = 0;
Point minCorner, maxCorner;
//Rect karty2[4];	//0
static int liczbaKart = 4;
//LipaLib - Learning Image Processing Autonomic Library

// =========== FUNCTION EXAMPLE =================//
void rgbTogray(Image3CH& rgbImg, Image1CH& grayImg) // arguments with & because we want to save changes to images after performing funtion
{
	//Check if image sizes are equal
	if (rgbImg.width() == grayImg.width() && rgbImg.height() == grayImg.height())
	{
		for (int i = 0; i < rgbImg.width(); i++) //iterate by rows
			for (int j = 0; j < rgbImg.height(); j++) //iterate by cols
			{
				grayImg(i, j).Intensity() = (rgbImg(i, j).Red() + rgbImg(i, j).Green() + rgbImg(i, j).Blue()) / 3; // I = (1/3)*(R+G+B) (i,j) - (number of row, number of col)
			}
	}
	else
	{
		std::cerr << "Image sizes mismatch"<<std::endl; //print error
		return;
	}
}
//================================================================================//
bool findColerance(Image3CH& orginalImg, int x1, int y1, int x2, int y2) {
	//Image3CH cutImg(karty[0].bottomRight.x-karty[0].topLeft.x, karty[0].bottomRight.y-karty[0].topLeft[0].y);
	//Image3CH kier(karty[0].bottomRight.x - karty[0].topLeft.x, karty[0].bottomRight.y - karty[0].topLeft[0].y);
	Image3CH cutImg(x2 - x1, y2 - y1);
	Image3CH kier(x2 - x1, y2 - y1);
	kier.LoadImage("img\\kier_black_red.png", LPL_LOAD_FITTED);
	double sumaWagRed = 0.0, sumaWagGreen = 0.0, sumaWagBlue = 0.0;
	double sumeRed = 0.0, sumeGreen = 0.0, sumeBlue = 0.0;

	for (int i = 0; i < kier.width(); i++) {
		for (int j = 0; j < kier.height(); j++) {
			cutImg(i, j).Red() = orginalImg(i + x1, j + y1).Red();
			cutImg(i, j).Green() = orginalImg(i + x1, j + y1).Green();
			cutImg(i, j).Blue() = orginalImg(i + x1, j + y1).Blue();

			sumeRed = sumeRed + cutImg(i, j).Red()*kier(i, j).Red();
			sumeGreen = sumeGreen + cutImg(i, j).Green()*kier(i, j).Green();
			sumeBlue = sumeBlue + cutImg(i, j).Blue()*kier(i, j).Blue();

			sumaWagRed += kier(i, j).Red();
			sumaWagBlue += kier(i, j).Blue();
			sumaWagGreen += kier(i, j).Green();
		}
	}
	double red = sumeRed / sumaWagRed, green = sumeGreen / sumaWagGreen, blue = sumeBlue / sumaWagBlue;
	double sum = red + green + blue;
	if (sum > 0.7)
		return true;
	
	return false;
}
//=================================================================================================//
bool findColeranceBinaryImage(Image1CH& mImg, int x1, int y1, int x2, int y2) {
	Image1CH cutImg(x2 - x1, y2 - y1);
	Image3CH trefl(x2 - x1, y2 - y1);
	trefl.LoadImage("img\\trefl.jpg", LPL_LOAD_FITTED);
	double sumaWag = 0.0;
	double sume = 0.0;

	for (int i = 0; i < trefl.width(); i++) {
		for (int j = 0; j < trefl.height(); j++) {
			cutImg(i, j).Intensity() = mImg(i + x1, j + y1).Intensity();

			sume = sume + cutImg(i, j).Intensity()*(trefl(i, j).Red()+ trefl(i, j).Green()+ trefl(i, j).Blue());
			
			sumaWag += (trefl(i, j).Red() + trefl(i, j).Green() + trefl(i, j).Blue());
		}
	}
	double colorOut = sume / sumaWag;
	if (colorOut > 0.7)
		return true;

	return false;
}
//==========================================================================

void threshold(Image1CH& inImg, Image1CH& outImg, double limit) {
	if (inImg.width() != outImg.width() || inImg.height() != outImg.height())
		return;

	for (int i = 0; i < inImg.width(); i++) {
		for (int j = 0; j < inImg.height(); j++) {
			if (inImg(i, j).Intensity() > limit) {
				outImg(i, j).Intensity() = 1;
			}
			else {
				outImg(i, j).Intensity() = 0;
			}
		}
	}
}
//=================================================================================//
void thresholdColor(Image3CH& inImg, Image1CH& outImg, double limit) {
	if (inImg.width() != outImg.width() || inImg.height() != outImg.height())
		return;

	for (int i = 0; i < inImg.width(); i++) {
		for (int j = 0; j < inImg.height(); j++) {
			if (inImg(i, j).R() < limit && inImg(i, j).G() < limit && inImg(i, j).B() < limit) {
				outImg(i, j).Intensity() = 1;
			}
			else {
				outImg(i, j).Intensity() = 0;
			}
		}
	}
}
//=======================================================================//
void convertRed(Image3CH& inImg, Image3CH& outImg, double limit, int **tab) {
	if (inImg.width() != outImg.width() || inImg.height() != outImg.height())
		return;

	for (int i = 0; i < inImg.width(); i++) {
		for (int j = 0; j < inImg.height(); j++) {
			if (inImg(i, j).Green() < 0.3 && inImg(i, j).Blue() < 0.3 && inImg(i, j).Red() > 0.3) {
				outImg(i, j).Red() = 1;
				outImg(i, j).Green() = 0;
				outImg(i, j).Blue() = 0;
				tab[i][j] = 1;
			}
			else {
				outImg(i, j).Red() = 0;
				outImg(i, j).Green() = 0;
				outImg(i, j).Blue() = 0;
				tab[i][j] = 0;
			}
		}
	}
}
//=======================================================================//
void calculateSurface(int **tab, int xWidth, int yHeight, int x, int y) {
	if (x >= 0 && x<xWidth && y >= 0 && y<yHeight) {
		if (tab[x][y] == 1) {
			powierzchnia++;
			tab[x][y] = 2;		//mark field as calculated
			calculateSurface(tab, xWidth, yHeight, x, y - 1);
			calculateSurface(tab, xWidth, yHeight, x, y + 1);
			calculateSurface(tab, xWidth, yHeight, x - 1, y);
			calculateSurface(tab, xWidth, yHeight, x + 1, y);
		}
		else if(tab[x][y]==0) {
			obwod++;
			tab[x][y] = -1;	//mark field as calculated
		}
		if (minCorner.x > x)
			minCorner.x = x;
		if (minCorner.y > y)
			minCorner.y = y;
		if (maxCorner.x < x)
			maxCorner.x = x;
		if (maxCorner.y < y)
			maxCorner.y = y;
	}
}

void calculate(int **tab, int width, int height, Image1CH& outImage, Card* foundCards, Image3CH& colorImage) {
	double cardArea = (foundCards[0].x2 - foundCards[0].x1)*(foundCards[0].y2 - foundCards[0].y1);
	double colorArea = 1.0;

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			if (tab[x][y] == 1) {
				calculateSurface(tab, width, height, x, y);
				if (obwod == 0)
					obwod++;
				colorArea = (maxCorner.x - minCorner.x)*(maxCorner.y - minCorner.y);
				if (cardArea/colorArea < 230) {
					for (int i = minCorner.x; i < maxCorner.x; i++) {
						for (int j = minCorner.y; j < maxCorner.y; j++) {
							outImage(i, j).Intensity() = 1;
						}
					}
					for (int i = 0; i < liczbaKart; i++) {
						if (foundCards[i].x1 < minCorner.x && foundCards[i].y1 < minCorner.y && foundCards[i].x2 > maxCorner.x
							&& foundCards[i].y2 > maxCorner.y) {
							foundCards[i].number++;
							if (foundCards[i].color == 0) {
								if (findColerance(colorImage, minCorner.x, minCorner.y, maxCorner.x, maxCorner.y)) {
									foundCards[i].color = 3;
									foundCards[i].colorName = "kier";
								}
								else {
									foundCards[i].color = 2;
									foundCards[i].colorName = "karo";
								}
							}
						}
					}
				}
				powierzchnia = 0;
				obwod = 0;
				minCorner.x = width;
				minCorner.y = height;
				maxCorner.x = 0;
				maxCorner.y = 0;
			}
		}
	}
}
//==========================================================================//
void find4Cards(Image1CH& img, int width, int height, Card* karty) {
	Rect allCardFiled;
	allCardFiled.topLeft.x = width;
	allCardFiled.topLeft.y = height;

	for (int x = 1; x < width; x++) {
		for (int y = 1; y < height; y++) {
			if (img(x,y).Intensity() == 0) {
				if (allCardFiled.bottomRight.x < x)
					allCardFiled.bottomRight.x = x;
				if (allCardFiled.bottomRight.y < y)
					allCardFiled.bottomRight.y = y;
				if (allCardFiled.topLeft.x > x)
					allCardFiled.topLeft.x = x;
				if (allCardFiled.topLeft.y > y)
					allCardFiled.topLeft.y = y;
			}
		}
	}

	karty[0].x1 = allCardFiled.topLeft.x;
	karty[0].y1 = allCardFiled.topLeft.y;
	karty[0].x2 = allCardFiled.topLeft.x + (allCardFiled.bottomRight.x - allCardFiled.topLeft.x)/2;
	karty[0].y2 = allCardFiled.topLeft.y + (allCardFiled.bottomRight.y - allCardFiled.topLeft.y) / 2;


	karty[1].x1 = allCardFiled.topLeft.x + (allCardFiled.bottomRight.x - allCardFiled.topLeft.x)/2;
	karty[1].y1 = allCardFiled.topLeft.y;
	karty[1].x2 = allCardFiled.bottomRight.x;
	karty[1].y2 = allCardFiled.topLeft.y + (allCardFiled.bottomRight.y - allCardFiled.topLeft.y) / 2;

	karty[2].x1 = allCardFiled.topLeft.x;
	karty[2].y1 = allCardFiled.topLeft.y + (allCardFiled.bottomRight.y - allCardFiled.topLeft.y) / 2;
	karty[2].x2 = allCardFiled.topLeft.x + (allCardFiled.bottomRight.x - allCardFiled.topLeft.x) / 2;
	karty[2].y2 = allCardFiled.bottomRight.y;

	karty[3].x1 = allCardFiled.topLeft.x + (allCardFiled.bottomRight.x - allCardFiled.topLeft.x) / 2;
	karty[3].y1 = allCardFiled.topLeft.y + (allCardFiled.bottomRight.y - allCardFiled.topLeft.y) / 2;
	karty[3].x2 = allCardFiled.bottomRight.x;
	karty[3].y2 = allCardFiled.bottomRight.y;
}

float diff(float intensity1, float intensity2) {
	return abs(intensity1 - intensity2);
}

graphUniverse* segment_graph(int num_vertices, int num_edges, field *edges, float tresholdFactor){ // sort edges by weight
	//std::sort(edges, edges + num_edges, biColorLess);

	// make a disjoint-set forest
	graphUniverse *u = new graphUniverse(num_vertices);

	// init thresholds
	float *threshold = new float[num_vertices];
	for (int i = 0; i < num_vertices; i++)
		threshold[i] = THRESHOLD(1, tresholdFactor);

	// for each edge, in non-decreasing weight order...
	for (int i = 0; i < num_edges; i++) {
		field *pedge = &edges[i];

		// components conected by this edge
		int a = u->find(pedge->a);
		int b = u->find(pedge->b);
		if (a != b) {
			if ((pedge->w <= threshold[a]) &&
				(pedge->w <= threshold[b])) {
				u->join(a, b);
				a = u->find(a);
				threshold[a] = pedge->w + THRESHOLD(u->size(a), tresholdFactor);
			}
		}
	}

	// free up
	delete threshold;
	return u;
}

void calcField(Image1CH& img, Image1CH& out, float tresholdFactor) {
	int width = img.width(), height = img.height();

	field *field1 = new field[width*height * 4];

	int num = 0, ar;
	float wAvr;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int px0y0 = img(x, y).Intensity();
			ar = y * width + x;

			if (x < width - 1) {
				int px1y0 = img(x + 1, y).Intensity();
				field1[num].a = ar;
				field1[num].b = y * width + (x + 1);
				field1[num].w = diff(px0y0, px1y0);
				num++;
			}

			if (y < height - 1) {
				int px0y1 = img(x, y+1).Intensity();
				field1[num].a = ar;
				field1[num].b = (y + 1) * width + x;
				field1[num].w = diff(px0y0, px0y1);
				num++;
			}

			if ((x < width - 1) && (y < height - 1)) {
				int px1y1 = img(x+1, y+1).Intensity();
				field1[num].a = ar;
				field1[num].b = (y + 1) * width + (x + 1);
				field1[num].w = diff(px0y0, px1y1);
				num++;
			}

			if ((x < width - 1) && (y > 0)) {
				int px1ym1 = img(x+1, y-1).Intensity();
				field1[num].a = ar;
				field1[num].b = (y - 1) * width + (x + 1);
				field1[num].w = diff(px0y0, px1ym1);
				num++;
			}
		}
	}

	graphUniverse *u = segment_graph(width*height, num, field1, tresholdFactor);
	delete[] field1;
	int num_ccs = u->num_sets();

	// pick random colors for each component
	colors* randColorsArray = new colors[num_ccs];
	for (int j = 0; j < num_ccs; j++) {
		randColorsArray[j].color =  ((float)(rand() % 1000))/1000;
	}

	int comp, colorNum = 0;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			comp = u->find(y * width + x);
			colorNum = 0;
			while(randColorsArray[colorNum].id != comp && colorNum < num_ccs) {
				colorNum++;
			}
			if (randColorsArray[colorNum].id == comp) {
				out(x, y).Intensity() = randColorsArray[colorNum].color;
			}
			else {
				colorNum = 0;
				while (randColorsArray[colorNum].id != -1 && colorNum < num_ccs) {
					colorNum++;
				}
				if (randColorsArray[colorNum].id == -1) {
					out(x, y).Intensity() = randColorsArray[colorNum].color;
					randColorsArray[colorNum].id = comp;
				}
			}
		}
	}
}

//====================================================================================================//
void calculateBlackCard(Image1CH& binaryImg, Card& blackCardInfo){
	int width = blackCardInfo.x2 - blackCardInfo.x1, height = blackCardInfo.y2 - blackCardInfo.y1;
	Image1CH blackCardImg(width, height);
	Image1CH blackCardTempImg(width, height);
	/*int** blackArray = new int*[width];
	for (int i = 0; i < width; ++i)
		blackArray[i] = new int[height];*/

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			//blackArray[x][y] = binaryImg(x + blackCardInfo.x1, y + blackCardInfo.y1).Intensity();
			blackCardImg(x,y).Intensity() = binaryImg(x + blackCardInfo.x1, y + blackCardInfo.y1).Intensity();
			blackCardTempImg(x, y).Intensity() = binaryImg(x + blackCardInfo.x1, y + blackCardInfo.y1).Intensity();
		}
	}
	//blackCardImg.ShowImage("res");

	//blurr
	int mSize = 40;
	int** kernel = new int*[mSize];
	for (int i = 0; i < mSize; ++i)
		kernel[i] = new int[mSize];

	for (int x = 0; x < mSize; x++)
		for (int y = 0; y < mSize; y++)
			kernel[x][y] = 1;

	double sum = 0.0;
	double waga = mSize*mSize;

	int startPoint = mSize / 2;
	for (int i = startPoint; i < blackCardImg.width() - startPoint; i++) {
		for (int j = startPoint; j < blackCardImg.height() - startPoint; j++) {
			sum = 0.0;
			for (int k = 0; k < mSize; k++) {
				for (int m = 0; m < mSize; m++) {
					sum += blackCardImg(i + k - startPoint, j + m - startPoint).Intensity()*kernel[k][m];
				}
			}
			blackCardTempImg(i, j).Intensity() = sum / waga;
		}
	}
	//blackCardTempImg.ShowImage("blurred");

	//orginal - treshold(blur) = same znaki na karcie
	int** blackArray = new int*[width];
	for (int i = 0; i < width; ++i)
		blackArray[i] = new int[height];

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			if (blackCardTempImg(x, y).Intensity() > 0.35)
				blackCardTempImg(x, y).Intensity() = 1;
			else
				blackCardTempImg(x, y).Intensity() = 0;
			blackCardImg(x,y).Intensity()-= blackCardTempImg(x, y).Intensity();
			blackArray[x][y] = blackCardImg(x, y).Intensity();
		}
	}
	//blackCardImg.ShowImage("res");

	double colorArea = 1.0;
	double cardArea = width*height;
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			if (blackArray[x][y] == 1) {
				calculateSurface(blackArray,width, height, x, y);
				if (obwod == 0)
					obwod++;
				colorArea = (maxCorner.x - minCorner.x)*(maxCorner.y - minCorner.y);
				if (cardArea / colorArea < 230) {
					if (blackCardInfo.color == 0) {
						if (findColeranceBinaryImage(blackCardImg, minCorner.x, minCorner.y, maxCorner.x, maxCorner.y)) {
							blackCardInfo.color = 1;
							blackCardInfo.colorName = "trefl";
						}
						else {
							blackCardInfo.color = 4;
							blackCardInfo.colorName = "pik";
						}
					}
					for (int i = minCorner.x; i < maxCorner.x; i++) {
						for (int j = minCorner.y; j < maxCorner.y; j++) {
							blackCardImg(i, j).Intensity() = 1;
						}
					}
					blackCardInfo.number++;
				}
				powierzchnia = 0;
				obwod = 0;
				minCorner.x = width;
				minCorner.y = height;
				maxCorner.x = 0;
				maxCorner.y = 0;
			}
		}
	}
	blackCardImg.ShowImage("res");

}
// ==============================================//

int main()
{
	Image3CH ColourImage(600, 800); // Create new image Image3CH - three channels image (width,height)
	ColourImage.LoadImage("img\\blurred.png", LPL_LOAD_FITTED); 
	
	Image1CH GrayscaleImage(ColourImage.width(), ColourImage.height()); 
	//easy version, no function creation needed
	for (int i = 0; i < ColourImage.width(); i++)
		for (int j = 0; j < ColourImage.height(); j++)
		{
			GrayscaleImage(i, j).Intensity() =  (ColourImage(i, j).Red() + ColourImage(i, j).Green() + ColourImage(i, j).Blue()) / 3;
		}

	/*minCorner.x = ColourImage.width();
	minCorner.y = ColourImage.height();

	GrayscaleImage.ShowImage("res"); //Show image

	Image1CH BinaryImgage(ColourImage.width(), ColourImage.height());
	thresholdColor(ColourImage, BinaryImgage, 0.2);
	BinaryImgage.ShowImage("res");
	
	Image3CH RedImage(ColourImage.width(), ColourImage.height());
	Image1CH FilteredImage(ColourImage.width(), ColourImage.height());

	int** redArray = new int*[ColourImage.width()];
	for (int i = 0; i < ColourImage.width(); ++i)
		redArray[i] = new int[ColourImage.height()];

	Card* foundCards = new Card[liczbaKart];
	
	find4Cards(BinaryImgage, BinaryImgage.width(), BinaryImgage.height(),foundCards);
	convertRed(ColourImage, RedImage, 0.5, redArray);
	calculate(redArray, ColourImage.width(), ColourImage.height(), FilteredImage,foundCards, RedImage);
	//ColourImage.DrawLine(foundCards[0].x1, foundCards[0].y1, foundCards[0].x2, foundCards[0].y2);
	//ColourImage.DrawLine(foundCards[1].x1, foundCards[1].y1, foundCards[1].x2, foundCards[1].y2);
	//ColourImage.DrawLine(foundCards[2].x1, foundCards[2].y1, foundCards[2].x2, foundCards[2].y2);
	//ColourImage.DrawLine(foundCards[3].x1, foundCards[3].y1, foundCards[3].x2, foundCards[3].y2);
	//ColourImage.ShowImage("res");
	FilteredImage.ShowImage("res");

	for (int i = 0; i < liczbaKart; i++) {
		if (foundCards[i].number == 0) {		//black card
			calculateBlackCard(BinaryImgage, foundCards[i]);
		}
		std::cout << foundCards[i].number << foundCards[i].colorName << std::endl;
	}*/

	ColourImage.ShowImage("res");
	Image1CH BinaryImgage(ColourImage.width(), ColourImage.height());
	thresholdColor(ColourImage, BinaryImgage, 0.2);
	BinaryImgage.ShowImage("res");
	Image1CH segImg(ColourImage.width(), ColourImage.height());
	calcField(BinaryImgage, segImg, 0.7);
	segImg.ShowImage("res");

	//ColourImage.ShowHistogram(); // Show image histograms
	int a;
	std::cin >> a;

	return 0;
}
