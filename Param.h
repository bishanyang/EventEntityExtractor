/*
 * Param.h
 *
 *  Created on: Feb 10, 2014
 *      Author: bishan
 */

#ifndef PARAM_H_
#define PARAM_H_

#include <string>
using namespace std;

class Param {
public:
	Param() : nbest(0)
    {
    }

public:
	int nbest;
	string modelpath;
	string resourcepath;

	string datapath;
	string outputpath;
};


#endif /* PARAM_H_ */
