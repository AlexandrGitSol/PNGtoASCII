#ifndef CONVERTORASCII_H
#define CONVERTORASCII_H

#include "FileManager.h"

#include <iostream>
#include <map>
#include <utility>
#include <algorithm>
#include <cmath>
#include <string>
#include <sstream>
#include <vector>
using namespace std;

class ConvertorASCII {
	struct Settings {
		bool stop = false;
		int max_w = 40;
		double aspect_ratio = 2.5;
	};

	Settings stgs;
	FileManagerBK fm;
	vector<string> ends = { "*.jpg", "*.png" };

	int gcd(int a, int b);
	pair<int, int> resize(int width, int height, int k);
	unsigned char ptouc(unsigned char r, unsigned char g, unsigned char b);
	string PNGtoASCII(unsigned char* data, int original_width, int original_height, int width_art, int height_art);
	string remake_console(std::string pixels, double aspect_ratio);
	string converted(string path_);

	template<typename T>
	bool can_convert(const std::string& str) {
		std::istringstream iss(str);
		T value;
		return (iss >> value) && (iss.eof());
	}

	bool is_int(const std::string& str) {
		return can_convert<int>(str);
	}

	bool is_float(const std::string& str) {
		return can_convert<float>(str);
	}

	bool is_double(const std::string& str) {
		return can_convert<double>(str);
	}

	bool is_long(const std::string& str) {
		return can_convert<long>(str);
	}
public:
	void asking();
};

#endif //CONVERTORASCII_H