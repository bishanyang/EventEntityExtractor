//
//  CRF++ -- Yet Another CRF toolkit
//
//  $Id: encoder.h 1588 2007-02-12 09:03:39Z taku $;
//
//  Copyright(C) 2005-2007 Taku Kudo <taku@chasen.org>
//
#ifndef CRFPP_ENCODER_H_
#define CRFPP_ENCODER_H_

#include "common.h"
#include <vector>
#include <string>

#include "feature_index.h"
#include "tagger.h"

using namespace std;

namespace CRFPP {

class Encoder {
private:
	whatlog what_;
public:
	size_t         freq; //use features that occuer no less than INT(default 1)
	size_t         maxitr; //set INT for max iterations in LBFGS routine(default 10k)
	double         C; //set FLOAT for cost parameter(default 1.0)
	double         eta; //set FLOAT for termination criterion(default 0.0001)
	bool           textmodel;
	unsigned short thread_num;
	unsigned short shrinking_size;//set INT for number of iterations variable needs to be optimal before considered for shrinking. (default 20)

	double         *alpha;

	//vector<PRConstraint> constraintsList;
	EncoderFeatureIndex* feature_index;

public:
   Encoder() {
		freq           = 1;
		//maxitr = 3;
		maxitr        = 100000; //default: 100000
		C              = 1.0;
		eta            = 0.0001;
		textmodel      = true;
		thread_num         = 1;
		shrinking_size = 20;

		alpha = NULL;
		feature_index = NULL;
	}

   ~Encoder() {
	   if (alpha != NULL) {
		   delete alpha;
		   alpha = NULL;
	   }

	   if (feature_index != NULL) {
		   delete feature_index;
		   feature_index = NULL;
	   }
   }

  void buildAlpha();

  //batch learning
  bool learn(const std::vector<TaggerImpl* > &x, const char * modelfile);

  const char* what() { return what_.str(); }
};

}
#endif
