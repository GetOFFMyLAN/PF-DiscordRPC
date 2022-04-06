#include "WoutContainer.h"

using std::vector;
using std::string;
using std::ifstream;
using std::getline;

namespace workout {
	// 
	bool isNumber(string s) {
		const char* cvers = s.c_str();
		for (size_t i = 0; i < s.size(); i++) {
			int ascii = static_cast<int>(cvers[i]);
			if (ascii < 48 || ascii > 57) return false;
		}
		return true;
	}

	// Parses line following a csv format where data is separated
	// on a line by commas. There can only be 4 pieces of data (i.e
	// there should only be 4 commas and the vector size should be 4
	// the last two data must be numbers and must support discord rate limits
	vector<string> parser(string line) {
		vector<string> ret;
		size_t idx = line.find(',');
		while (idx != string::npos) {
			ret.push_back(line.substr(0, idx));
			line.erase(0, idx + 2);
			idx = line.find(',');
		}
		ret.push_back(line);

		// make sure enough data is passed and that data correct in file
		if (ret.size() != 4) {
			throw std::invalid_argument("invalid line: incorrect amount of arguments");
		} else if (!(isNumber(ret.at(2)) && isNumber(ret.at(3)))) {
			throw std::invalid_argument("invalid line: character instead of number used");
		}

		int reps = stoi(ret.at(2));
		int rtime = stoi(ret.at(3));
		// avoid overflow and negatives (overflow is avoided by using the rate limit)
		if (reps < 1) throw std::out_of_range("invalid line: rep count negative (range err)");
		if (rtime < MIN_WOUT_T || rtime > MAX_WOUT_T) throw std::out_of_range("invalid line: time out of bounds (range err)");
		// time specific error checking - Discord caps update rate at 5 updates per 20 seconds
		// ensure max time isnt too long and that ratio of time per rep is less than update cap
		if (rtime / reps < UPDATE_RATE) throw std::out_of_range("invalid line: rep count and workout time will exceed rate limits");

		return ret;
	}

	void Container::ldData(const string& fname) {
		ifstream fin(fname);
		if (!fin.is_open())
			throw std::invalid_argument(("cannot open file " + fname + " in ldData() call"));

		string line;
		getline(fin, line); // get first line in cfg file denoting formatting
		while (!fin.eof() && getline(fin, line)) {
			try {
				vector<string> data = parser(line);
				places.push_back(data.at(0));
				activities.push_back(data.at(1));
				maxReps.push_back(stoi(data.at(2)));
				totalTime.push_back(stoi(data.at(3)));
				size++;
			}
			catch (std::invalid_argument& err) { // catch invalid line
				continue;
			}
			catch (std::out_of_range& err) { // catch range issues with data
				continue;
			}
		}
	}
}