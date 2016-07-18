/*
 * Logger.cpp
 *
 *  Created on: Sep 28, 2013
 *      Author: bishan
 */

#include "Logger.h"
#include <ctime>

Logger::~Logger() {
	// TODO Auto-generated destructor stub
	outfile.close();
}

Logger::Logger() {
	std::time_t now = std::time(NULL);
	std::tm * ptm = std::localtime(&now);
	char buffer[32];
	// Format: Mo, 15.06.2009 20:20:00
	std::strftime(buffer, 32, "%d_%m_%Y_%H_%M_%S", ptm);
	timestr = buffer;

	string logfile = "./log/"+timestr+".txt";
	outfile.open(logfile.c_str(), ios::out);
}

Logger::Logger(string filename) {
	string logfile = "./log/";

	std::time_t now = std::time(NULL);
	std::tm * ptm = std::localtime(&now);
	char buffer[32];
	// Format: Mo, 15.06.2009 20:20:00
	std::strftime(buffer, 32, "%d_%m_%Y_%H_%M_%S", ptm);
	timestr = buffer;

	logfile += timestr + "_" + filename;
	outfile.open(logfile.c_str(), ios::out);
}

