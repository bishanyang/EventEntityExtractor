/*
 * Logger.h
 *
 *  Created on: Sep 28, 2013
 *      Author: bishan
 */

#ifndef LOGGER_H_
#define LOGGER_H_
#include <fstream>

using namespace std;

class Logger {
public:
	Logger();
	Logger(string filename);
	virtual ~Logger();

	void Write(string str) { outfile<<str; }
	void WriteLine(string str) { outfile<<str<<endl; }
	void Close() { outfile.close(); }
private:
	ofstream outfile;
public:
	string timestr;
};

#endif /* LOGGER_H_ */
