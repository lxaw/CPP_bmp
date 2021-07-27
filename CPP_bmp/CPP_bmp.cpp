/*
Written (with love) by Lex Whalen

This project could not have been completed without the great help
of: https://github.com/sol-prog/cpp-bmp-images/blob/master/BMP.h

Most of the initial code to write BMP files is largely inspired from this repository.
*/

/*
A BMP file can be divided into 4 regions.
1. File header
2. Bitmap header
3. Color hear
4. Pixel data
*/
#include <iostream>

#include "BMPFile.h"
#include "BGRAColor.h"

int main()
{
//	BMPFile bmp(255,255);
//	BMPFile bmp(50,50);
	BMPFile bmp(1000,500);
//	BGRAColor red{ 0,0,255,255 };
//	BGRAColor green { 0, 255, 0,255};
//	BGRAColor blue { 255, 0, 0,255};
//	bmp.fillRegion(0, 0, 20, 20, red);
//	bmp.fillRegion(20, 50, 50, 70, blue);
//	bmp.fillRegion(50, 20, 80, 200, green);
//	bmp.write("regions.bmp");

//	BGRAColor a{ 100,100,255,255 };
//	BGRAColor b{ 50,50,140,255 };
//	BGRAColor c = (a + b);
//	BGRAColor d = (a - b);
//
//	bmp.fillRegion(0, 0, 800, 600, a);
//	bmp.write("a.bmp");
//
//	bmp.fillRegion(0, 0, 800, 600, b);
//	bmp.write("b.bmp");
//
//	bmp.fillRegion(0, 0, 800, 600, c);
//	bmp.write("c.bmp");
//
//	bmp.fillRegion(0, 0, 800, 600, d);
//	bmp.write("d.bmp");
//	BGRAColor c1{ 0,0,0,255 };
//	BGRAColor c2{ 255,255,255,255 };
	BGRAColor c1{ 100,255,50,255 };
	BGRAColor c2{ 255,0,255,255 };

	bmp.verticalGradient(c1,c2);
	bmp.write("vert_grad.bmp");

	bmp.horizontalGradient(c2,c1);
	bmp.write("horizontal_grad.bmp");

	
}

