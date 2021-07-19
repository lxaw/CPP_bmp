/*
Written (with love) by Lex Whalen
*/
#pragma once

#include <stdint.h>


/*
BMP files need these 5 elements in their file header.
Info on file type, size, and location of pix data.
*/
struct BMPFileHeader
{
	// The file type
	uint16_t _file_type{ 0x4D42 };
	// The size of file in bytes
	uint32_t _file_size{ 0 };
	// A reserved value of 0
	uint16_t _reserved1{ 0 };
	// A reserved value of 0
	uint16_t _reserved2{ 0 };
	// The start position of the pixel data
	// Essentially the bytes from beginning of file
	uint32_t _offset_data{ 0 };
};

