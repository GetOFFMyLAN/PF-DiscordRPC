#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#define MAX_WOUT_T 600
#define MIN_WOUT_T 60
#define UPDATE_RATE 4

namespace workout {
	bool isNumber(std::string);
	std::vector<std::string> parser(std::string);

	struct Container {
		std::vector<std::string> places;
		std::vector<std::string> activities;
		std::vector<unsigned int> maxReps;
		std::vector<unsigned int> totalTime;
		size_t size = 0;

		void ldData(const std::string&);
	};
}

