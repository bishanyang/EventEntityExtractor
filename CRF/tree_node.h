//
//  CRF++ -- Yet Another CRF toolkit
//
//  $Id: node.h 1595 2007-02-24 10:18:32Z taku $;
//
//  Copyright(C) 2005-2007 Taku Kudo <taku@chasen.org>
//
#ifndef TREE_NODE_H__
#define TREE_NODE_H__

#include <vector>
#include <cmath>
#include <limits>
#include "common.h"

#define MINVALUE -numeric_limits<double>::infinity()

using namespace std;

namespace CRFPP {

class TreeTaggerNode;

class TreeTaggerPath {
public:
   TreeTaggerNode   *parent;
   TreeTaggerNode   *child;

   int y;

   double cost;
   double marginal;
   int    *binary_fea_vector;
   int    *float_fea_vector;
   double *float_fv_vector;

    // for pr
   	double  constraint_cost;
   	int    *constraint_fvector;
   	double *constraint_fv_vector;
   	double  q_marginal;

public:
   TreeTaggerPath(): parent(NULL), child(NULL), binary_fea_vector(0), float_fea_vector(0), float_fv_vector(0), y(0),
   	   	   	   	   cost(0.0), marginal(0.0), constraint_cost(0.0), constraint_fvector(0), constraint_fv_vector(0), q_marginal(0.0) {}

   ~TreeTaggerPath() {
   }

   void calcExpectation(double *expected);

   void calcConstraintExpectation(double *expected, double c);

   void add(TreeTaggerNode *_pnode, int c_index, int yindex, TreeTaggerNode *_cnode);

   void clear() {
	 binary_fea_vector = 0;
	 float_fea_vector = 0;
	 float_fv_vector = 0;
	 cost = 0.0;
	 marginal = 0.0;

	 y = 0;

	 parent = NULL;
	 child = NULL;

	 constraint_cost = 0.0;
	 constraint_fvector = 0;
	 constraint_fv_vector = 0;
	 q_marginal = 0.0;
   }
};

class TreeTaggerNode {
public:
	double parent_alpha;
	double parent_beta;
	vector<double> children_alpha;
	vector<double> children_beta;

	vector<TreeTaggerPath*>  parent_path;
	vector<vector<TreeTaggerPath*> > children_path;

	//feature vector
	int *binary_fea_vector;
	int *float_fea_vector;
	double *float_fv_vector;

	int x;
	int y;

	double cost;
	double marginal;

	// for pr
	double  constraint_cost;
	int    *constraint_fvector;
	double *constraint_fv_vector;
	double q_marginal;

public:
	TreeTaggerNode() : x(0), binary_fea_vector(0), float_fea_vector(0), float_fv_vector(0),
		parent_alpha(0.0), parent_beta(0.0),
		y(0), cost(0.0),
		marginal(0.0), constraint_cost(0.0), constraint_fvector(0), constraint_fv_vector(0), q_marginal(0.0) {
	}

	~TreeTaggerNode() {
	}

	void clear() {
		x = 0;
		y = 0;

		cost = 0.0;
		marginal = 0.0;

		constraint_cost = 0.0;
		constraint_fvector = 0;
		constraint_fv_vector = 0;
		q_marginal = 0.0;

		parent_alpha = 0.0;
		parent_beta = 0.0;
		children_alpha.clear();
		children_beta.clear();

		binary_fea_vector = 0;
		float_fv_vector = 0;
		float_fea_vector = 0;

		parent_path.clear();
		children_path.clear();
	}

	void calMarginal(double Z);

	void calcExpectation(double *expected);

	void calcConstraintExpectation(double *expected, double c);
};

class TaggerNodeCache {
   private:
	std::vector <TreeTaggerNode *> node_freelist_;

   public:
    void free() {
      for (int i = 0; i < node_freelist_.size(); ++i) {
    	  	  delete node_freelist_[i];
    	  	  node_freelist_[i] = NULL;
      }
      //node_freelist_.clear();
      std::vector <TreeTaggerNode *>().swap(node_freelist_);
    }

    TreeTaggerNode* alloc() {
    		TreeTaggerNode *n = new TreeTaggerNode();
    		node_freelist_.push_back(n);
    		return n;
    }

    TaggerNodeCache() {}
    ~TaggerNodeCache() {}
  };

  class TaggerPathCache {
     private:
  	std::vector <TreeTaggerPath *> freelist_;

     public:
      void free() {
        for (int i = 0; i < freelist_.size(); ++i) {
      	  	  delete freelist_[i];
      	  	freelist_[i] = NULL;
        }
        std::vector <TreeTaggerPath *>().swap(freelist_);
      }

      TreeTaggerPath* alloc() {
      		TreeTaggerPath *n = new TreeTaggerPath();
      	    freelist_.push_back(n);
      		return n;
      }

      TaggerPathCache() {}
      ~TaggerPathCache() {}
    };

}

#endif
