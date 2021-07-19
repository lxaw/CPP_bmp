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

int main()
{
	BMPFile bmp0("Test.bmp");
	BMPFile bmp1(200, 200, false);
}

