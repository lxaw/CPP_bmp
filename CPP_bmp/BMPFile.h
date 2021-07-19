/*
Written (with love) by Lex Whalen
*/
#pragma once

#include "BMPColorHeader.h"
#include "BMPFileHeader.h"
#include "BMPInfoHeader.h"

#include <vector>
#include <stdint.h>
#include <string>
#include <ios>
#include <fstream>


struct BMPFile
{
	BMPFileHeader _file_header;
	BMPInfoHeader _info_header;
	BMPColorHeader _color_header;

	std::vector<uint8_t> _data;

	BMPFile(const char* fileName) {
		read(fileName);
	}

	void read(const char* fileName) {
		std::ifstream inp{ fileName,std::ios_base::binary };
		if (inp) {
			inp.read((char*)& _file_header, sizeof(_file_header));

			if (_file_header._file_type != 0x4D42) {
				throw std::runtime_error("Error: Incorrect file format.");
			}
			inp.read((char*)&_info_header, sizeof(_info_header));

			// _color_header only used for transpar imgs
			if (_info_header._bit_count == 32) {
				// See if file has bitmask color info
				if (_info_header._size >= (sizeof(BMPInfoHeader) + sizeof(BMPColorHeader))) {
					inp.read((char*)&_color_header, sizeof(_color_header));
					// See if pix data stored as BGRA and if color space sRGB
					checkColorHeader(_color_header);
				}
				else {
					throw std::runtime_error("Error: Cannot read file's bit mask info.");
				}
			}
			// Move to pix data location
			inp.seekg(_file_header._offset_data, inp.beg);

			// Adjust header field for output
			// Only need to save the headers and data
			if (_info_header._bit_count == 32) {
				_info_header._size = sizeof(BMPInfoHeader) + sizeof(BMPColorHeader);
				_file_header._offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + sizeof(BMPColorHeader);
			}
			else {
				_info_header._size = sizeof(BMPInfoHeader);
				_file_header._offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
			}
			_file_header._file_size = _file_header._offset_data;

			if (_info_header._height < 0) {
				throw std::runtime_error("Error: Only accepts BMP images having origin in bottom left.");
			}

			_data.resize(_info_header._width * _info_header._height * _info_header._bit_count / 8);

			// See if need to deal with row padding
			if (_info_header._width % 4 == 0) {
				inp.read((char*)_data.data(), _data.size());
				_file_header._file_size += static_cast<uint32_t>(_data.size());
			}
			else {
				_row_stride = _info_header._width * _info_header._bit_count / 8;
				uint32_t new_stride = makeStrideAligned(4);
				std::vector<uint8_t> padding_row(new_stride - _row_stride);

				for (int y = 0; y < _info_header._height; ++y) {
					inp.read((char*)(_data.data() + _row_stride * y), _row_stride);
					inp.read((char*)padding_row.data(), padding_row.size());
				}
				_file_header._file_size += static_cast<uint32_t>(_data.size()) + _info_header._height * static_cast<uint32_t>(padding_row.size());
			}
		}
		else {
			throw std::runtime_error("Error: Cannot open the file.");
		}
	}


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

	void fillRegion(uint32_t x0, uint32_t y0, uint32_t w, uint32_t h, uint8_t B, uint8_t G, uint8_t R, uint8_t A) {
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
					_data[chans * (y * _info_header._width + x) + 0] = B;
					_data[chans * (y * _info_header._width + x) + 1] = G;
					_data[chans * (y * _info_header._width + x) + 2] = R;

					if (chans == 4) {
						_data[chans * (y * _info_header._width + x) + 3] = A;
					}
				}
			}
		}
	}
	
	void setPixel(uint32_t x0, uint32_t y0, uint8_t B, uint8_t G, uint8_t R, uint8_t A) {
		if (x0 >= (uint32_t)_info_header._width || y0 >= (uint32_t)_info_header._height || x0 < 0 || y0 < 0) {
			// Dimension issue
			throw std::runtime_error("Error: Point to set is outside of the image itself.");
		}
		else {
			// Note the use of y0 here
			uint32_t chans = _info_header._bit_count / 8;
			_data[chans * (y0 * _info_header._width + x0) + 0] = B;
			_data[chans * (y0 * _info_header._width + x0) + 1] = G;
			_data[chans * (y0 * _info_header._width + x0) + 2] = R;
			
			if (chans == 4) {
				_data[chans * (y0 * _info_header._width + x0) + 3] = A;
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

