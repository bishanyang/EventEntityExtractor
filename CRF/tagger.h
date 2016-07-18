//
//  CRF++ -- Yet Another CRF toolkit
//
//  $Id: tagger.h 1588 2007-02-12 09:03:39Z taku $;
//
//  Copyright(C) 2005-2007 Taku Kudo <taku@chasen.org>
//
#ifndef CRFPP_TAGGER_H__
#define CRFPP_TAGGER_H__

#include <iostream>
#include <vector>
#include <map>

#include "feature_index.h"
#include "param.h"
#include "scoped_ptr.h"
#include "viterbi_decoder.h"

using namespace std;

namespace CRFPP {

static inline double toprob(Node *n, double Z) {
	return std::exp(n->alpha + n->beta - n->cost - Z);
}

static inline double toprob(double cost, double Z) {
	return std::exp(cost - Z);
}

class Tagger {
public:
	Tagger() {
		feature_id_ = 0;
		feature_value_id_ = 0;
		thread_id_ = 0;
		feature_index_ = 0;
	};
	virtual ~Tagger(){}

public:
  	size_t                   feature_id_;
	size_t                   feature_value_id_;
	unsigned short           thread_id_;

	FeatureIndex             *feature_index_;

  	std::vector<unsigned short int>  answer_;
  	std::vector<unsigned short int>  result_;

public:
  	virtual  double   gradient(double *)=0;
  	virtual  bool     Inference()=0;
  	virtual  size_t   size() const=0;
  	virtual  bool     next()=0;

   void   set_feature_id(size_t id) { feature_id_  = id; }
   size_t feature_id() const { return feature_id_; }
   void   set_feature_value_id(size_t id) { feature_value_id_  = id; }
   size_t feature_value_id() const { return feature_value_id_; }

   void   set_thread_id(unsigned short id) { thread_id_ = id; }
   unsigned short thread_id() const { return thread_id_; }
};

class TaggerImpl : public Tagger {
  enum { TEST, LEARN };
	  //enum {MINRISK, VITERBI};
  private:
      size_t                            ysize_;

  public:
      unsigned int mode_   : 2;
      unsigned int vlevel_ : 3;

      double        Z_;

      vector<int>   nodes;
      vector<pair<int, int> > edges;

      std::vector <string> x_;
      std::vector<std::vector <Node *> > node_;

      whatlog what_;
      string_buffer os_;

      ViterbiDecoder viterbi_decoder;

public:
      explicit TaggerImpl(): mode_(TEST), vlevel_(0),
                             ysize_(0), Z_(0) {
      }
	  ~TaggerImpl() {
//		  for (int i = 0; i < node_.size(); ++i) {
//			  for (int j = 0; j <node_[i].size(); ++j) {
//				  delete node_[i][j];
//				  node_[i][j] = NULL;
//			  }
//		  }
	  }

public:
	  double eval();

	  double pairwise_log_potential(int y1, int y2);

	  bool SentenceToTagger(Sentence *sent, CRFPP::FeatureIndex *feature_index, bool train);
	  bool PairToTagger(Sentence *sent, EventMention *m1, EventMention *m2, CRFPP::FeatureIndex *feature_index, bool train);

	  bool next();

	  void calcCost(Node *n);

	  double span_cost(int start, int end, int y);
	  double segment_prob(int start, int end, string ystr);
	  double segment_BIO_prob(int start, int end, string ystr);

	  bool Inference();

	  string toString();

      //clear data structure
      bool clear();

      bool open(FeatureIndex *);

      void    recomputeCost();
      void    computerZ();
      double  gradient(double *expected);

	  int get_lb2i(string str) {return feature_index_->yID(str);}

	  size_t result(size_t i)  const    { return result_[i]; }
	  size_t answer(size_t i) const { return answer_[i]; }

      Node  *node(size_t i, size_t j) const { return node_[i][j]; }
      Path  *path(size_t cur, size_t i, size_t j) {return node_[cur][i]->lpath[j];}
      void   set_node(Node *n, size_t i, size_t j) { node_[i][j] = n; }

      size_t       size() const { return nodes.size(); } //seq_len
      size_t       ysize() const { return ysize_; }

	unsigned int vlevel() const { return vlevel_; }
	float cost_factor() const { return feature_index_->node_cost_factor(); }

	//use in nbest inference
	double  prob() const { return std::exp(- viterbi_decoder.cost_ - Z_); }

	void set_vlevel(unsigned int vlevel) { vlevel_ = vlevel;}
	void set_cost_factor(float cost_factor) {
		if (cost_factor > 0)
		  feature_index_->set_node_cost_factor(cost_factor);
	}

	const char* what() { return what_.str(); }
};
}

#endif
