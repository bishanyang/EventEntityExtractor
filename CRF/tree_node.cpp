//
//  CRF++ -- Yet Another CRF toolkit
//
//  $Id: node.cpp 1595 2007-02-24 10:18:32Z taku $;
//
//  Copyright(C) 2005-2007 Taku Kudo <taku@chasen.org>
//
#include <cmath>
#include "tree_node.h"
#include "common.h"
#include <assert.h>

namespace CRFPP {

void TreeTaggerNode::calcExpectation(double *expected) {
	for (int *f = binary_fea_vector; *f != -1; ++f) {
		expected[*f + y] += marginal;
	}
	double *fv = float_fv_vector;
	for (int *f = float_fea_vector; *f != -1; ++f) {
		expected[*f + y] += marginal * (*fv);
		++fv;
	}

	for (int i = 0; i < children_path.size(); ++i) {
		for (int j = 0; j < children_path[i].size(); ++j) {
			children_path[i][j]->calcExpectation(expected);
		}
	}
}

void TreeTaggerNode::calcConstraintExpectation(double *expected, double c) {
	double *fv = constraint_fv_vector;
	for (int *f = constraint_fvector; *f != -1; ++f) {
		expected[*f + y] += q_marginal * (*fv) * c;
		++fv;
	}

//	for (int i = 0; i < children_path.size(); ++i) {
//		for (int j = 0; j < children_path[i].size(); ++j) {
//			children_path[i][j]->calcConstraintExpectation(expected, c);
//		}
//	}
}

void TreeTaggerNode::calMarginal(double log_Z) {
	double c = parent_beta + cost;
	for (int j = 0; j < children_alpha.size(); ++j) {
		c += children_alpha[j];
	}
	marginal = exp(c - log_Z);

	for (int i = 0; i < children_path.size(); ++i) {
		for (int j = 0; j < children_path[i].size(); ++j) {
			TreeTaggerPath* p = children_path[i][j];
			p->marginal = exp(p->cost + p->parent->children_beta[i] + p->child->parent_alpha - log_Z);
		}
	}
}

void TreeTaggerPath::add(TreeTaggerNode *_pnode, int c_index, int yindex, TreeTaggerNode *_cnode) {
	parent = _pnode;
	child = _cnode;

	y = yindex;

	child->parent_path.push_back(this);
	parent->children_path[c_index].push_back(this);
}

void TreeTaggerPath::calcExpectation(double *expected) {
	 for (int *f = binary_fea_vector; *f != -1; ++f) {
	        expected[*f + y] += marginal;
	  }

	  double *fv = float_fv_vector;
      for (int *f = float_fea_vector; *f != -1; ++f) {
        expected[*f + y] += marginal * (*fv);
        ++fv;
      }
}

void TreeTaggerPath::calcConstraintExpectation(double *expected, double c) {
	  double *fv = constraint_fv_vector;
      for (int *f = constraint_fvector; *f != -1; ++f) {
    	  expected[*f + y] += q_marginal * (*fv) * c;
          ++fv;
      }
}


}


