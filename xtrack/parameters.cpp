// Parameter access

#include "parameters.h"

int intParam(std::unordered_map<std::string, std::string> parameters,
		std::string key, int defaultValue) {
	int result = defaultValue;
	if (parameters.find(key) != parameters.end()) {
		std::stringstream convert(parameters[key]);
		if (!(convert >> result)) {
			std::cerr << "Invalid value set for parameter " << key << "\n";
			throw 1;
		}
	}
	return result;
}

std::string stringParam(std::unordered_map<std::string, std::string> parameters,
		std::string key, std::string defaultValue) {
	std::string result = defaultValue;
	if (parameters.find(key) != parameters.end()) {
		result = parameters[key];
	}
	return result;
}
