/*
Written (with love) by Lex Whalen
*/
#pragma once

#include <stdint.h>
#include <iostream>

struct BGRAColor
{
	// blue chan
	int _B{ 0 };
	// green chan
	int _G{ 0 };
	// redchan
	int _R{ 0 };
	// alpha chan
	int _A{ 0 };

	const int MAX_VAL{ 255 };
	const int MIN_VAL{ 0 };

	BGRAColor(int B, int G, int R, int A) {
		_B = B;
		_G = G;
		_R = R;
		_A = A;
	}
	int min_bgra(int a, int b) {
		if (a < b) {
			return a;
		}
		else {
			return b;
		}
	}
	int max_bgra(int a, int b) {
		if (a > b) {
			return a;
		}
		return b;
	}
	BGRAColor operator + (int const& anInt) {
		int newB = min_bgra(_B + anInt, MAX_VAL);
		int newG = min_bgra(_G + anInt, MAX_VAL);
		int newR = min_bgra(_R + anInt, MAX_VAL);

		return BGRAColor{ newB,newG,newR,MAX_VAL };
	}
	BGRAColor operator / (int const& anInt) {
		int newB = max_bgra(_B / anInt, 1);
		int newG = max_bgra(_G / anInt, 1);
		int newR = max_bgra(_R / anInt, 1);

		return BGRAColor{ newB, newG, newR, MAX_VAL };
	}
	BGRAColor operator + (BGRAColor const& aBGRA) {
		int newB = min_bgra(_B + aBGRA._B,MAX_VAL);
		int newG = min_bgra(_G + aBGRA._G,MAX_VAL);
		int newR = min_bgra(_R + aBGRA._R,MAX_VAL);

		return BGRAColor { newB,newG,newR,MAX_VAL };
	}
	BGRAColor operator - (BGRAColor const& aBGRA) {
		int newB = min_bgra(_B - aBGRA._B,MIN_VAL);
		int newG = min_bgra(_G - aBGRA._G,MIN_VAL);
		int newR = min_bgra(_R - aBGRA._R,MIN_VAL);

		return BGRAColor { newB,newG,newR,MAX_VAL};
	}
	void print() {
		std::cout << "B value: " << _B << "\nG value: " << _G << "\nR value: " << _R << "\n";
	}
};

