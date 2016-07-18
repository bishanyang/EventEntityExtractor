//
//  CRF++ -- Yet Another CRF toolkit
//
//  $Id: node.h 1595 2007-02-24 10:18:32Z taku $;
//
//  Copyright(C) 2005-2007 Taku Kudo <taku@chasen.org>
//
#ifndef NODE_H__
#define NODE_H__

#include <vector>
#include <cmath>
#include <algorithm>
#include <map>
#include "common.h"

#define MIN_VALUE 4.9E-324
#define LOG2 0.69314718055
#define LOG0 -1.7976931348623157E308
#define MINUS_LOG_EPSILON  50

namespace CRFPP {
   inline double logsumexp(double x, double y) {
    //if(fabs(x-y) < MIN_VALUE)
	//	return x+LOG2;
	const double vmin = _min(x, y);
    const double vmax = _max(x, y);
    if (vmax > vmin + MINUS_LOG_EPSILON) {
      return vmax;
    } else {
      return vmax + std::log(std::exp(vmin - vmax) + 1.0);
    }
  }
  // log(exp(x) + exp(y));
  //    this can be used recursivly
  // e.g., log(exp(log(exp(x) + exp(y))) + exp(z)) =
  // log(exp (x) + exp(y) + exp(z))
  inline double logsumexp(double x, double y, bool flg) {
    if (flg) return y;  // init mode
    const double vmin = _min(x, y);
    const double vmax = _max(x, y);
    if (vmax > vmin + MINUS_LOG_EPSILON) {
      return vmax;
    } else {
      return vmax + std::log(std::exp(vmin - vmax) + 1.0);
    }
  }

  inline double logsumexp(std::vector<double> x) {
	  if (x.size() == 1) {
		  return x[0];
	  }
 	  double sum = 0.0;
 	  for (int i=0; i<x.size(); i++) {
 	     sum = logsumexp(sum, x[i], i==0);
 	  }
 	  return sum;
  }

  struct Path;

  //only for inference
  class SegmentNode {
  public:
	  int start;
	  int end;
	  unsigned short int y; // opinion or non
	  unsigned short int polar_y; // pos, neg, neu, non
	  unsigned short int intensity_y;

	  double               cost;
	  double               cost_w_loss;
	  double               bestCost;
	  SegmentNode          *prev;

	  int                  *fvector; //segmentation vector
	  int                  *polarity_fvector; //polarity vector
	  int                  *intensity_fvector;

	  // shared
	  double               *embedding_word_vec;

	  double               exploss;
	  double               prob;
public:
	  SegmentNode(): start(0), end(0), y(0), polar_y(0), intensity_y(0), cost_w_loss(0.0),
	  	  	  	embedding_word_vec(0), intensity_fvector(0),
	            cost(0.0), bestCost(0.0), prev(0), fvector(0), polarity_fvector(0),
	            exploss(0.0), prob(0.0) {}
  };

  struct QueueSegElement {
    	SegmentNode *node;
        QueueSegElement *next;
        double fx;
        double gx;
    };

    class QueueSegElementComp {
    public:
      const bool operator()(QueueSegElement *q1,
    	  QueueSegElement *q2)
      { return(q1->fx > q2->fx); }
    };

  struct Node {
    unsigned int         x;
    unsigned short int   y;
    double               alpha;
    double               beta;
    double               cost;

    // for inference
    double               bestCost;
    Node                *prev;

    int                 *binary_fea_vector; //feature vector
    int                 *float_fea_vector; //feature vector
    double              *float_fv_vector;

    double               constraintcost;
    int                 *constraintvector;//for feature-type of constraints, each constraint is a feature

    std::vector<Path *>  lpath;
    std::vector<Path *>  rpath;

	double               exploss;
	double               prob;

public:
    void calcAlpha();
    void calcBeta();
    void calcExpectation(double *expected, double, size_t);

    void calcAlphaWithConstraints();
	void calcBetaWithConstraints();
	void calcProb(double Z_, size_t size);

	void calcFVExpectation(double *expected, double Z, size_t size);
	void calcFeatureExpectation(double *expected, double c) const;

    void clear() {
      x = y = 0;
      alpha = beta = cost = 0.0;
      prev = 0;

      binary_fea_vector = 0;
      float_fea_vector = 0;
      float_fv_vector = 0;

      constraintcost = 0.0;
      constraintvector = 0;
      prob = 0.0;

      lpath.clear();
      rpath.clear();

	  exploss = 0.0;
    }

    void zeros() {
         alpha = beta = cost = 0.0;
         constraintcost = 0.0;
         prob = 0.0;
   	     exploss = 0.0;
       }

    void shrink() {
      std::vector<Path *>(lpath).swap(lpath);
      std::vector<Path *>(rpath).swap(rpath);
    }

    Node(): x(0), y(0), alpha(0.0), beta(0.0),
            cost(0.0), bestCost(0.0), prev(0), binary_fea_vector(0), float_fea_vector(0), float_fv_vector(0), constraintvector(0),
            constraintcost(0.0), exploss(0), prob(0.0) {}
    ~Node() {}
  };

  struct Path {
    Node   *rnode;
    Node   *lnode;
    int    *binary_fea_vector;
    int    *float_fea_vector;
    double *float_fv_vector;

    int yindex;

    double  cost;

    int    *constraintvector; //a list for one type of constraint
    double  constraintcost;//ysize dimension

    double  prob;

    Path(): rnode(0), lnode(0), binary_fea_vector(0), float_fea_vector(0), float_fv_vector(0), cost(0.0),
    		constraintvector(0), constraintcost(0.0), prob(0.0), yindex(0) {}

    ~Path() {}

    // for CRF
    void calcExpectation(double *expected, double, size_t);
    void calcFeatureExpectation(double *expected, double c, size_t size) const;
    void calcProb(double Z, size_t size);

    void calcFVExpectation(double *expected, double Z, size_t size);

    void add(Node *_lnode, Node *_rnode, int y);

    void clear() {
      rnode = lnode = 0;
      binary_fea_vector = 0;
      float_fea_vector = 0;
      float_fv_vector = 0;
      cost = 0.0;
      prob = 0.0;

      constraintvector = 0;
      constraintcost = 0.0;

      yindex = 0;
    }

    void zeros() {
	  cost = 0.0;
	  prob = 0.0;
	  constraintcost = 0.0;
	}
  };

  class NodeCache {
   private:
	std::vector <Node *> node_freelist_;

   public:
    void free() {
      for (int i = 0; i < node_freelist_.size(); ++i) {
    	  	  delete node_freelist_[i];
    	  	  node_freelist_[i] = NULL;
      }
      //node_freelist_.clear();
      std::vector <Node *>().swap(node_freelist_);
    }

    Node* alloc() {
    		Node *n = new Node();
    		node_freelist_.push_back(n);
    		return n;
    }
    NodeCache() {}
    ~NodeCache() {}
  };

  class SegmentNodeCache {
     private:
  	std::vector <SegmentNode *> node_freelist_;

     public:
      void free() {
        for (int i = 0; i < node_freelist_.size(); ++i) {
      	  	  delete node_freelist_[i];
      	  	  node_freelist_[i] = NULL;
        }
        //node_freelist_.clear();
        std::vector <SegmentNode *>().swap(node_freelist_);
      }

      SegmentNode* alloc() {
    	  	  SegmentNode *n = new SegmentNode();
      	  node_freelist_.push_back(n);
      	  return n;
      }
      SegmentNodeCache() {}
      ~SegmentNodeCache() {}
    };


  class PathCache {
     private:
  	std::vector <Path *> freelist_;

     public:
      void free() {
        for (int i = 0; i < freelist_.size(); ++i) {
      	  	  delete freelist_[i];
      	  	freelist_[i] = NULL;
        }
        std::vector <Path *>().swap(freelist_);
      }

      Path* alloc() {
      		Path *n = new Path();
      	    freelist_.push_back(n);
      		return n;
      }

      PathCache() {}
      ~PathCache() {}
    };

  typedef std::vector<Path*>::const_iterator const_Path_iterator;
}

#endif
