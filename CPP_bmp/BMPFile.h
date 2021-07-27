/*
Written (with love) by Lex Whalen
*/
#pragma once

#include "BMPColorHeader.h"
#include "BMPFileHeader.h"
#include "BMPInfoHeader.h"

#include "BGRAColor.h"

#include <vector>
#include <stdint.h>
#include <string>
#include <ios>
#include <fstream>
#include <stdlib.h>


struct BMPFile
{
	BMPFileHeader _file_header;
	BMPInfoHeader _info_header;
	BMPColorHeader _color_header;

	std::vector<uint8_t> _data;


	BMPFile(int32_t w, int32_t h, bool hasAlpha = true) {
		if (w <= 0 || h <= 0) {
			throw std::runtime_error("Error: Image width and height must be positive.");
		}

		_info_header._width = w;
		_info_header._height = h;
		if (hasAlpha) {
			_info_header._size = sizeof(BMPInfoHeader) + sizeof(BMPColorHeader);
			_file_header._offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + sizeof(BMPColorHeader);
			
			_info_header._bit_count = 32;
			_info_header._compression = 3;
			_row_stride = w * 4;
			_data.resize(_row_stride * h);
			_file_header._file_size = _file_header._offset_data + _data.size();
		}
		else {
			// no alpha
			_info_header._size = sizeof(BMPInfoHeader);
			_file_header._offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
			
			_info_header._bit_count = 24;
			_info_header._compression = 0;
			_row_stride = w * 3;
			_data.resize(_row_stride * h);

			uint32_t new_stride = makeStrideAligned(4);
			_file_header._file_size = _file_header._offset_data + static_cast<uint32_t>(_data.size()) + _info_header._height * (new_stride - _row_stride);
		}
	}
	
	void write(const std::string fileName) {
		std::ofstream of{ fileName, std::ios_base::binary };
		if (of) {
			if (_info_header._bit_count == 32) {
				writeHeadersAndData(of);
			}
			else if (_info_header._bit_count == 24) {
				if (_info_header._width % 4 == 0) {
					writeHeadersAndData(of);
				}
				else {
					// Need to get a new stride
					uint32_t new_stride = makeStrideAligned(4);
					std::vector<uint8_t> padding_row(new_stride - _row_stride);

					writeHeaders(of);

					for (int y = 0;y < _info_header._height;++y) {
						of.write((const char*)(_data.data() + _row_stride * y), _row_stride);
						of.write((const char*)padding_row.data(), padding_row.size());
					}
				}
			}
			else {
				throw std::runtime_error("Error: Unsupported bit per pixel. Need 24 or 32.");
			}
		}
		else {
			throw std::runtime_error("Error: Cannot open output image file.");
		}
	}

	void fillRegion(uint32_t x0, uint32_t y0, uint32_t w, uint32_t h, BGRAColor aBGRA) {
		// Fills a region with color
		if (x0 + w > (uint32_t)_info_header._width || y0 + h > _info_header._height) {
			// To large
			throw std::runtime_error("Error: Region to fill is larger than the image itself.");
		}
		else {
			uint32_t chans = _info_header._bit_count / 8;
			for (uint32_t y = y0;y < y0 + h; ++y) {
				// Note the use of y here
				for (uint32_t x = x0; x < x0 + w; ++x) {
					_data[chans * (y * _info_header._width + x) + 0] = aBGRA._B;
					_data[chans * (y * _info_header._width + x) + 1] = aBGRA._G;
					_data[chans * (y * _info_header._width + x) + 2] = aBGRA._R;

					if (chans == 4) {
						_data[chans * (y * _info_header._width + x) + 3] = aBGRA._A;
					}
				}
			}
		}
	}
	int getOneOrAvg(int numerator, int denominator) {
		int one = 1;
		if (numerator < 0 || denominator < 0) {
			one *= -1;
		}
		if (denominator == 0) {
			std::cout << "Division by zero: return -1 or 1\n";
			return one;
		}
		int avg = numerator / denominator;
		if (avg != 0) {
			return avg;
		}
		else {
			return one;
		}
	}
	int addOrBound(int a, int b, int lb, int ub) {
		int sum = a + b;
		if (sum <= lb) {
			return lb;
		}
		else if (sum > lb && sum < ub) {
			return sum;
		}
		else {
			return ub;
		}
	}
	int intMax(int a, int b) {
		if (a > b) {
			return a;
		}
		return b;
	}
	void verticalGradient(BGRAColor bgra1, BGRAColor bgra2) {
		// horizontal gradient

		int dB;
		int dG;
		int dR;

		// note: do not use the subtraction for BGRA color here, since want to be able to 
		// get negative
		int diff_b = bgra2._B - bgra1._B;
		int diff_g = bgra2._G - bgra1._G;
		int diff_r = bgra2._R - bgra1._R;

		int height = _info_header._height;

		// now divide total width by differences	
		// lowest for positive is 1, negative is -1
		if (std::abs(diff_b) > height) {
			dB = getOneOrAvg(diff_b, height);
		}
		else {
			dB = getOneOrAvg(height, diff_b);
		}
		if (std::abs(diff_g) > height) {
			dG = getOneOrAvg(diff_g, height);
		}
		else {
			dG = getOneOrAvg(height, diff_g);
		}
		if (std::abs(diff_r) > height) {
			dR = getOneOrAvg(diff_r, height);
		}
		else {
			dR = getOneOrAvg(height, diff_r);
		}
		int multiplier_b = intMax(height / intMax(bgra1._B,bgra2._B), 1);
		int multiplier_g = intMax(height / intMax(bgra1._G,bgra2._G), 1);
		int multiplier_r = intMax(height / intMax(bgra1._R,bgra2._R), 1);


		uint8_t tempB = bgra1._B;
		uint8_t tempG = bgra1._G;
		uint8_t tempR = bgra1._R;

		for (uint32_t y = 0;y < _info_header._height;++y) {

			for (uint32_t x = 0; x < _info_header._width;++x) {
				setPixel(x, y, tempB, tempG, tempR, 255);
			}

			if (y!= 0 && y % std::abs(multiplier_b * dB) == 0) {
				tempB = addOrBound(tempB, dB, 0,255);
			}
			if (y!= 0 && y % std::abs(multiplier_g * dG) == 0) {
				tempG = addOrBound(tempG, dG, 0,255);
			}
			if (y != 0 &&  y % std::abs(multiplier_r * dR) == 0) {
				tempR = addOrBound(tempR, dR, 0,255);
			}
		}
	}
	void horizontalGradient(BGRAColor bgra1, BGRAColor bgra2) {
		// horizontal gradient

		int dB;
		int dG;
		int dR;

		// note: do not use the subtraction for BGRA color here, since want to be able to 
		// get negative
		int diff_b = bgra2._B - bgra1._B;
		int diff_g = bgra2._G - bgra1._G;
		int diff_r = bgra2._R - bgra1._R;

		int width = _info_header._width;

		// now divide total width by differences	
		// lowest for positive is 1, negative is -1
		if (std::abs(diff_b) > width) {
			dB = getOneOrAvg(diff_b, width);
		}
		else {
			dB = getOneOrAvg(width, diff_b);
		}
		if (std::abs(diff_g) > width) {
			dG = getOneOrAvg(diff_g, width);
		}
		else {
			dG = getOneOrAvg(width, diff_g);
		}
		if (std::abs(diff_r) > width) {
			dR = getOneOrAvg(diff_r, width);
		}
		else {
			dR = getOneOrAvg(width, diff_r);
		}
		int multiplier_b = intMax(width / intMax(bgra1._B,bgra2._B), 1);
		int multiplier_g = intMax(width / intMax(bgra1._G,bgra2._G), 1);
		int multiplier_r = intMax(width / intMax(bgra1._R,bgra2._R), 1);


		for (uint32_t y = 0;y < _info_header._height;++y) {
			uint8_t tempB = bgra1._B;
			uint8_t tempG = bgra1._G;
			uint8_t tempR = bgra1._R;


			for (uint32_t x = 0; x < _info_header._width;++x) {
				setPixel(x, y, tempB, tempG, tempR, 255);


				if (x!= 0 && x % std::abs(multiplier_b * dB) == 0) {
					tempB = addOrBound(tempB, dB, 0,255);
				}
				if (x!= 0 && x % std::abs(multiplier_g * dG) == 0) {
					tempG = addOrBound(tempG, dG, 0,255);
				}
				if (x != 0 &&  x % std::abs(multiplier_r * dR) == 0) {
					tempR = addOrBound(tempR, dR, 0,255);
				}
			}
		}
	}
	
	void setPixel(uint32_t x0, uint32_t y0, int b, int g, int r, int a) {
		if (x0 >= (uint32_t)_info_header._width || y0 >= (uint32_t)_info_header._height || x0 < 0 || y0 < 0) { // Dimension issue
			throw std::runtime_error("Error: Point to set is outside of the image itself.");
		}
		else {
			// Note the use of y0 here
			uint32_t chans = _info_header._bit_count / 8;
			_data[chans * (y0 * _info_header._width + x0) + 0] = b;
			_data[chans * (y0 * _info_header._width + x0) + 1] = r;
			_data[chans * (y0 * _info_header._width + x0) + 2] = g;
			
			if (chans == 4) {
				_data[chans * (y0 * _info_header._width + x0) + 3] = a;
			}
		}
	}
private:
	uint32_t _row_stride{ 0 };

	void writeHeaders(std::ofstream& of) {
		of.write((const char*)&_file_header, sizeof(_file_header));
		of.write((const char*)&_info_header, sizeof(_info_header));
		
		if (_info_header._bit_count == 32) {
			of.write((const char*)&_color_header, sizeof(_color_header));
		}
	}

	void writeHeadersAndData(std::ofstream& of) {
		writeHeaders(of);
		of.write((const char*)_data.data(), _data.size());
	}

	// Keep adding 1 to _row_stride until aligns correctly
	uint32_t makeStrideAligned(uint32_t alignStride) {
		uint32_t new_stride = _row_stride;
		while (new_stride % alignStride != 0) {
			new_stride++;
		}
		return new_stride;
	}

	// See if pix data stored as BGRA / if color space sRGB
	void checkColorHeader(BMPColorHeader& bmp_color_header) {
		BMPColorHeader model_color_header;
		if (model_color_header._red_mask != bmp_color_header._red_mask ||
			model_color_header._blue_mask != bmp_color_header._blue_mask ||
			model_color_header._green_mask != bmp_color_header._green_mask ||
			model_color_header._alpha_mask != bmp_color_header._alpha_mask) {
			throw std::runtime_error("Incorrect color mask format. Expecting BGRA pixel data.");
		}
		if (model_color_header._color_space_type != bmp_color_header._color_space_type) {
			throw std::runtime_error("Incorrect color space type. Expecting sRGB values.");
		}
	}
};

