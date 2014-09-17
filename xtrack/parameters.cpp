/*******************************************************************************
 * Copyright (c) 2014 itemis AG (http://www.itemis.eu) and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/

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

bool boolParam(std::unordered_map<std::string, std::string> parameters,
		std::string key, bool defaultValue) {
	bool result = defaultValue;
	if (parameters.find(key) != parameters.end()) {
		result = (parameters[key] == "true");
	}
	return result;
}
