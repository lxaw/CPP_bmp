/*
Written (with love) by Lex Whalen
*/

#include <stdint.h>

#pragma once

/*
Color masks init'ed to BGRA format, only used for imgs w/
transparency (32 bit depth here)
*/
struct BMPColorHeader
{
	// The bit mask for red chan
	uint32_t _red_mask{ 0x00ff0000 };
	// The bit mask for green chan
	uint32_t _green_mask{ 0x0000ff00 };
	// bit mask for blue chan
	uint32_t _blue_mask{ 0x000000ff };
	// bit mask for alpha chan
	uint32_t _alpha_mask{ 0xff000000 };
	// The default sRGB
	uint32_t _color_space_type{ 0x73524742 };
	// The unused data for sRGB color space
	uint32_t _unused[16]{ 0 };
};

