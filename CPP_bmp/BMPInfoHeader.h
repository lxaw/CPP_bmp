/*
Written (with love) by Lex Whalen
*/

#include <stdint.h>

#pragma once
struct BMPInfoHeader
{
	// The size of the header in bytes
	uint32_t _size{ 0 };
	// The width of the bitmap in pix
	int32_t _width{ 0 };
	// The height of the bitmap in pix
	int32_t _height{ 0 };
	// The number of planes for the device, here 1
	uint16_t _planes{ 1 };
	// The number of bits per pix
	uint16_t _bit_count{ 0 };
	// 0 or 3, for now we only use uncompressed
	//  0 for imgs w/ 24 bits/pix and 3 for 32 bits/pix
	uint32_t _compression{ 0 };
	// 0 for uncompressed imgs
	uint32_t _size_image{ 0 };
	int32_t _x_pixels_per_meter{ 0 };
	int32_t _y_pixels_per_meter{ 0 };
	// The number of color indices in color table
	// 0 is max num of colors allowed by _bit_count
	uint32_t _colors_used{ 0 };
	// The number of colors used to display bitmap
	// If 0 all colors needed
	uint32_t _colors_important{ 0 };
};

