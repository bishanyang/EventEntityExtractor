/*
 * tree_encoder.cpp
 *
 *  Created on: Feb 3, 2014
 *      Author: bishan
 */

#include <fstream>
#include "timer.h"
#include "lbfgs.h"
#include "common.h"
#include "scoped_ptr.h"
#include "thread.h"
#include <assert.h>
#include <math.h>
#include "tree_encoder.h"

#include "feature_index.h"

namespace CRFPP {

class TreeLabelLikelihoodThread: public thread {
    public:
      TreeTagger **x;
      unsigned short start_i;
      unsigned short thread_num;

      size_t size;
      double obj; //loglikelihood
      std::vector<double> expected; //gradient

      void run() {
        obj = 0.0;
        std::fill(expected.begin(), expected.end(), 0.0);
        for (size_t i = start_i; i < size; i += thread_num) {
          obj += x[i]->gradient(&expected[0]);
        }
      }
};


bool TreeEncoder::TreeLabelLikelihood(const std::vector<TreeTagger* > &x,
      EncoderFeatureIndex *feature_index, double *alpha,
      vector<double>& expected, double &value,
      size_t maxitr, double C, double eta,
      unsigned short thread_num, bool orthant) {
    double old_obj = 1e+37;
    int    converge = 0;
    LBFGS lbfgs;
    std::vector<TreeLabelLikelihoodThread> thread(thread_num);

    for (size_t i = 0; i < thread_num; i++) {
      thread[i].start_i = i;
      thread[i].size = x.size();
      thread[i].thread_num = thread_num;
      thread[i].x = const_cast<TreeTagger **>(&x[0]);
      thread[i].expected.resize(feature_index->size());
    }


    for (size_t itr = 0; itr < maxitr; ++itr) {
      for (size_t i = 0; i < thread_num; ++i) thread[i].start();
      for (size_t i = 0; i < thread_num; ++i) thread[i].join();

      for (size_t i = 1; i < thread_num; ++i) {
        thread[0].obj += thread[i].obj;
      }

      //combine gradient
      for (size_t i = 1; i < thread_num; ++i) {
        for (size_t k = 0; k < feature_index->size(); ++k)
          thread[0].expected[k] += thread[i].expected[k];
      }

      for (size_t k = 0; k < feature_index->size(); ++k) {
			thread[0].obj += (alpha[k] * alpha[k] /(2.0 * C));
			thread[0].expected[k] += alpha[k] / C;
	  }

      double diff = (itr == 0 ? 1.0 :
          std::fabs(old_obj - thread[0].obj)/old_obj);
      std::cout << "iter="  << itr
        << " obj=" << thread[0].obj
        << " diff="  << diff << std::endl;
      old_obj = thread[0].obj;

      if (diff < eta)
        converge++;
      else
        converge = 0;

      if (itr > maxitr || converge == 3)  break;  // 3 is ad-hoc

      if (lbfgs.optimize(feature_index->size(),
            &alpha[0],
            thread[0].obj,
            &thread[0].expected[0], orthant, C) <= 0)
        return false;
    }

    value = -thread[0].obj;

    return true;
  }


void TreeEncoder::buildAlpha() {
	alpha = new double[feature_index->size()];           // parameter
	for (size_t i = 0; i < feature_index->size(); ++i) alpha[i] = 0.0;

	feature_index->set_alpha(alpha);
}

bool TreeEncoder::LBFGS_optimizer(const std::vector<TreeTagger* > &x,
      EncoderFeatureIndex *feature_index, double *alpha,
      vector<double>& expected, double &value,
      size_t maxitr, double C, double eta,
      bool orthant) {

    double old_obj = 1e+37;
    int    converge = 0;
    LBFGS lbfgs;

   expected.resize(feature_index->size());

   double obj = 0.0;
   // LBFGS loop
    for (size_t itr = 0; itr < maxitr; ++itr) {
    	obj = 0.0;

    	std::fill(expected.begin(), expected.end(), 0.0);
    	// sum up gradient
		for (size_t i = 0; i < x.size(); i ++) {
			obj += x[i]->gradient(&expected[0]);
			// check gradient
			//if (i == 0)
			//	x[i]->check_gradient(&expected[0], 22);
		}

		// L2 regularizer
		size_t num_nonzero = 0;
		for (size_t k = 0; k < feature_index->size(); ++k) {
			obj += (alpha[k] * alpha[k] /(2.0 * C));
			expected[k] += alpha[k] / C;
		}

		double diff = (itr == 0 ? 1.0 :
        std::fabs(old_obj - obj)/old_obj);

		std::cout << "iter="  << itr
		<< " obj=" << obj
		<< " diff="  << diff << std::endl;

		old_obj = obj;

		if (diff < eta) converge++;
		else converge = 0;

		if (itr > maxitr || converge == 3)  break;  // 3 is ad-hoc

		if (lbfgs.optimize(feature_index->size(),
            alpha,
            obj,
            &expected[0], orthant, C) <= 0)
        return false;
    }

    value = -obj;

    return true;
}

//----------------Learning using LBFGS-------------------
bool TreeEncoder::learn(const std::vector<TreeTagger* > &x, const char * modelfile) {
	int ysize = feature_index->ysize();
	double tolerance = 0.001;
	double oldValue = 0.0;

	//---------for labeled instances------------
	//regular CRF optimization (compute loglikelihood and gradient, then LBFGS)
	vector<double> expected(feature_index->size());
	fill(expected.begin(), expected.end(), 0.0);

	double label_value = 0;

	// Multi-thread
	if (thread_num > 1) {
		TreeLabelLikelihood(x, feature_index, alpha, expected, label_value, maxitr, C, eta, thread_num, false);
	} else {
		// Single-thread
		if (!LBFGS_optimizer(x, feature_index, alpha, expected, label_value,
				maxitr, C, eta, false)) {
			cout<<"LBFGS error!"<<endl;
		}
	}

	if (!feature_index->save(modelfile, textmodel)) return false;

	std::cout << "\nDone!";

	return true;
}
} /* namespace CRFPP */
