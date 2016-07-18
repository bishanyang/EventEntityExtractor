/*
 * tree_encoder.h
 *
 *  Created on: Feb 3, 2014
 *      Author: bishan
 */

#ifndef TREE_ENCODER_H_
#define TREE_ENCODER_H_
#include "tree_tagger.h"

namespace CRFPP {

class TreeEncoder {
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
	TreeEncoder() {
		freq           = 1;
		maxitr        = 100000; //default: 100000
		C              = 1.0;
		eta            = 0.0001;
		textmodel      = true;
		thread_num         = 1;
		shrinking_size = 20;

		alpha = NULL;
		feature_index = NULL;
	}

   ~TreeEncoder() {
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
  bool learn(const std::vector<TreeTagger* > &x, const char * modelfile);

  bool TreeLabelLikelihood(const std::vector<TreeTagger* > &x,
	      EncoderFeatureIndex *feature_index, double *alpha,
	      vector<double>& expected, double &value,
	      size_t maxitr, double C, double eta,
	      unsigned short thread_num, bool orthant);

  bool LBFGS_optimizer(const std::vector<TreeTagger* > &x,
        EncoderFeatureIndex *feature_index, double *alpha,
        vector<double>& expected, double &value,
        size_t maxitr, double C, double eta,
        bool orthant);
};

} /* namespace CRFPP */
#endif /* TREE_ENCODER_H_ */
