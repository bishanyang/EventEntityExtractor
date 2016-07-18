/*
 * Config.h
 *
 *  Created on: Nov 28, 2013
 *      Author: bishan
 */
#pragma once

#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <assert.h>
#include "Utils.h"

using namespace std;

class Config {
public:
	Config() {}

	void ParseStr(string config_str) {
		vector<string> lines;
		Utils::Split(config_str, '\n', lines);
		for (int i = 0; i < lines.size(); ++i) {
			string str = lines[i];
			if (str[0] == '#') continue;
			int index = str.find("=");
			if (index < 0) continue;

			string key = str.substr(0, index);
			string value = str.substr(index+1);
			props[key] = value;
		}
	}

	void ReadFile(string filename) {
		ifstream infile(filename.c_str(), ios::in);
		if (!infile) return;
		string str;
		while(getline(infile, str)) {
			if (str[0] == '#') continue;
			int index = str.find("=");
			if (index < 0) continue;

			string key = str.substr(0, index);
			string value = str.substr(index+1);
			props[key] = value;
		}
		infile.close();
	}

	bool IsEmpty() {
		if (props.size() == 0) return true;
		return false;
	}

	string GetProperty(string key) {
		if (props.find(key) == props.end()) {
			//cout<<"couldn't find key "<<key<<endl;
			return "";
		}
		return props[key];
	}

	bool GetBoolProperty(string key) {
		string v = GetProperty(key);

		if(v != "true" && v != "false") {
			return false;
		}

		return (v == "true");
	}

	double GetDoubleProperty(string key) {
		string v = GetProperty(key);
		return atof(v.c_str());
	}

	int GetIntProperty(string key) {
		string v = GetProperty(key);
		return atoi(v.c_str());
	}

	void SetProperty(string key, string value) {
		props[key] = value;
	}
private:
	map<string, string> props;
};
