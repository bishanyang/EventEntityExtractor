//
//  CRF++ -- Yet Another CRF toolkit
//
//  $Id: node.cpp 1595 2007-02-24 10:18:32Z taku $;
//
//  Copyright(C) 2005-2007 Taku Kudo <taku@chasen.org>
//
#include <cmath>
#include "node.h"
#include "common.h"
#include <assert.h>

namespace CRFPP {
  void Node::calcAlpha() {
    alpha = 0.0;
    for (const_Path_iterator it = lpath.begin(); it != lpath.end(); ++it)
      alpha = logsumexp(alpha,
          (*it)->cost +(*it)->lnode->alpha,
          (it == lpath.begin()));
    alpha += cost;
  }

  void Node::calcAlphaWithConstraints() {
      alpha = 0.0;
      for (const_Path_iterator it = lpath.begin(); it != lpath.end(); ++it)
        alpha = logsumexp(alpha,
            (*it)->cost + (*it)->constraintcost + (*it)->lnode->alpha,
            (it == lpath.begin()));
      alpha += cost+constraintcost; //node->constraintcost
    }

  void Node::calcBeta() {
    beta = 0.0;
    for (const_Path_iterator it = rpath.begin(); it != rpath.end(); ++it)
      beta = logsumexp(beta,
          (*it)->cost +(*it)->rnode->beta+(*it)->rnode->cost,
          (it == rpath.begin()));
  }

  void Node::calcBetaWithConstraints() {
      beta = 0.0;
      for (const_Path_iterator it = rpath.begin(); it != rpath.end(); ++it)
        beta = logsumexp(beta,
            (*it)->cost + (*it)->constraintcost + (*it)->rnode->beta +(*it)->rnode->cost + (*it)->rnode->constraintcost,
            (it == rpath.begin()));
    }

  void Node::calcExpectation(double *expected, double Z, size_t size) {
    for (int *f = binary_fea_vector; *f != -1; ++f) {
    	expected[*f + y] += prob;
    }
    double *fv = float_fv_vector;
    for (int *f = float_fea_vector; *f != -1; ++f) {
       	expected[*f + y] += prob * (*fv);
       	++fv;
    }

    for (const_Path_iterator it = lpath.begin(); it != lpath.end(); ++it) {
        (*it)->calcExpectation(expected, Z, size);
    }
  }

  void Path::calcExpectation(double *expected, double Z, size_t size) {
     for (int *f = binary_fea_vector; *f != -1; ++f)
       expected[*f + yindex] += prob;

     double *fv = float_fv_vector;
     for (int *f = float_fea_vector; *f != -1; ++f) {
       expected[*f + yindex] += prob * (*fv);
       ++fv;
     }
  }

  void Node::calcProb(double Z, size_t size) {
	  prob = std::exp(alpha + beta - Z);
	  for (const_Path_iterator it = lpath.begin(); it != lpath.end(); ++it)
			(*it)->calcProb(Z, size);
  }

  void Path::calcProb(double Z, size_t size) {
	  prob = std::exp(lnode->alpha + cost + constraintcost + rnode->cost + rnode->constraintcost + rnode->beta - Z);
  }

  void Path::add(Node *_lnode, Node *_rnode, int y) {
    lnode = _lnode;
    rnode = _rnode;
    lnode->rpath.push_back(this);
    rnode->lpath.push_back(this);

    yindex = y;
  }

}


