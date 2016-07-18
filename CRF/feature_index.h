//
//  CRF++ -- Yet Another CRF toolkit
//
//  $Id: feature_index.h 1588 2007-02-12 09:03:39Z taku $;
//
//  Copyright(C) 2005-2007 Taku Kudo <taku@chasen.org>
//
#ifndef CRFPP_FEATURE_INDEX_H_
#define CRFPP_FEATURE_INDEX_H_

//#define _DEBUG

#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <assert.h>

#include "common.h"
#include "scoped_ptr.h"
#include "feature_cache.h"
#include "node.h"
#include "tree_node.h"
#include "freelist.h"
#include "mmap.h"
#include "darts.h"

#include "../Utils.h"
#include "../Sentence.h"

using namespace std;
namespace CRFPP {

class Tagger;
class TaggerImpl;
class TreeTagger;
class TreeTaggerNode;
class TreeTaggerPath;

class FeatureIndex {
protected:
	int            maxid_; //number of features
	double             *alpha_; //crf parameters
	double             node_cost_factor_;
	double             path_cost_factor_;

  std::vector<const char*>  y_;
  std::vector<const char*>  arg_y_;
  std::vector<const char*>  arg_attr_y_;

  FeatureCache        feature_cache_;
  FeatureValueCache   feature_value_cache_;

  int       windowsize;

  // for pr
  int            maxmuid_;
  double         *mu_; //constraint parameters

  FreeList<char>      char_freelist_;

  // For CRF
  scoped_array<NodeCache>  node_freelist_;
  scoped_array<PathCache>  path_freelist_;

  scoped_array<TaggerPathCache > treepath_freelist_;
  scoped_array<TaggerNodeCache > treenode_freelist_;

public:
   virtual int addfID(const char *, int ysize) = 0;
   virtual int getfID(const char *key) = 0; //get id without adding

   virtual int addConstraintID(const char *, vector<double> p) = 0;
   virtual int getConstraintID(const char *key) = 0; //get id without adding

   whatlog             what_;
   size_t              thread_num_;

public:
  map<string, int>   lb2i;
  map<int, string>   i2lb;

  map<string, int>   arglb2i;
  map<int, string>   argi2lb;

  map<string, int>   argattrlb2i;
  map<int, string>   argattri2lb;

  map<int, int>   transition_lb2i;
  map<int, int>   transition_i2lb;

  map<int, set<int> > role_to_ners;
  map<int, set<int> > event_to_roles;

  map<int, double> constraint_prior;

  int       word_vec_size;
  vector<vector<float> > word_embeddings;
  map<string, int>    word_lookup_table; //word to embedding index

  map<string, string> gazetteer_unigrams;
  map<string, string> gazetteer_bigrams;
  map<string, string> gazetteer_trigrams;

  static const unsigned int version = MODEL_VERSION;

public:
  inline void set_alpha(long int index, double c) { alpha_[index] = c; }
  inline double get_alpha(long int index) { return alpha_[index]; }

  inline void set_mu(long int index, double c) { mu_[index] = c; }
  inline double get_mu(long int index) { return mu_[index]; }

  // clear everything
  void clear();

  void init() {
	  path_freelist_.reset(new PathCache [thread_num_]);
	  node_freelist_.reset(new NodeCache [thread_num_]);

	  treepath_freelist_.reset(new TaggerPathCache [thread_num_]);
	  treenode_freelist_.reset(new TaggerNodeCache [thread_num_]);
  }

  void freelist() {
	  for(int thread_id=0; thread_id<thread_num_; thread_id++) {
		  path_freelist_[thread_id].free();
		  node_freelist_[thread_id].free();

		  treenode_freelist_[thread_id].free();
		  treepath_freelist_[thread_id].free();
	  }
  }

  size_t size() const  { return maxid_; }
  size_t constraint_size() const {return maxmuid_;}

  int get_lb2i(string str) {
	  if (lb2i.find(str) == lb2i.end()) return -1;
	  return lb2i[str];
  }
  //string fstr (int id) { return fid2str[id2fid[id]]; }

  size_t ysize() const { return y_.size(); }
  size_t argysize() const { return arg_y_.size(); }
  size_t argattrsize() const { return arg_attr_y_.size(); }

  int yID(string l) {
	  if(lb2i.find(l) == lb2i.end())
		  return -1;
	  else
		  return lb2i[l];
  }

  void   set_alpha(double *alpha) { alpha_ = alpha; }
  const double *weight() const { return const_cast<double *>(alpha_); }

  void   set_mu(double *mu) { mu_ = mu; }

  void set_node_cost_factor(double cost_factor) { node_cost_factor_ = cost_factor; }
  double node_cost_factor() const { return node_cost_factor_; }
  void set_path_cost_factor(double cost_factor) { path_cost_factor_ = cost_factor; }
  double path_cost_factor() const { return path_cost_factor_; }

  void LoadGazetteerFeatures(const map<string, int>& vocab, string filename);
  void LoadEmbeddings(string infilename);
  void InitEmbeddingLabelSize();
  void ClearWordEmbeddings();
  void CopyEmbeddings(const map<string, int> &pword_lookup_table, const vector<vector<float> > &pword_embeddings);

  void CopyGazetteer(map<string, string> &gazetteer_unigrams, map<string, string> &gazetteer_bigrams);

  void SetY(map<string, int> labels);
  void SetArgY(map<string, int> labels);
  void SetArgAttrY(map<string, int> labels);
  void SetTransY(map<string, set<string> > translabels);

  void LoadLabels(set<string> &labels);
  void SetLabels(set<string> labels);
  void SetTransLabels(set<string> translabels);

  void calcCost(Node *n);
  void calcCost(Path *p);

  void calcTreeNodeCost(TreeTaggerNode *n);
  void calcTreePathCost(TreeTaggerPath *p);

  bool buildConstraints(TreeTaggerNode *);
  void calcTreeNodeConstraintCost(TreeTaggerNode *n);
  void calcTreePathConstraintCost(TreeTaggerPath *p);

  void buildCRFTagger(TaggerImpl *tagger);
  bool buildCRFFeatures(TaggerImpl *tagger, Sentence *sent, bool train);

  bool buildPairwiseFeatures(TaggerImpl *tagger, Sentence *sent,
  		EventMention *m1, EventMention *m2, bool train);

  bool buildTreeTagger(TreeTagger *tagger);
  void buildEventTreeFeatures(TreeTagger *tagger,
  		EventMention *event,
  		Sentence *sent,
  		bool train);


  const char* what() { return what_.str(); }
  char *strdup(const char *);

  explicit FeatureIndex(): maxid_(0), alpha_(0), windowsize(1), word_vec_size(0),
                           node_cost_factor_(1.0), path_cost_factor_(1.0),
                           thread_num_(1), char_freelist_(8192), mu_(0), maxmuid_(0)
                           {}
  virtual ~FeatureIndex() {}
};

class EncoderFeatureIndex: public FeatureIndex {
public:
 explicit EncoderFeatureIndex(size_t n=1) {
	  thread_num_ = n;
     init();
 }


private:
  //feature str -> feature id, count
  std::map <std::string, std::pair<int, int> > dic_;
  map<string, int> constraint_dic_;

  int addfID(const char *, int ysize);
  int getfID(const char*);

  int addConstraintID(const char *, vector<double> p);
  int getConstraintID(const char*);

public:
  bool save(const char *, bool);
  //void shrink(size_t) ;

  void get_featuremap(map<string, int> &fstr2id);
  void get_constraintmap(map<string, int> &cstr2id);
  string get_constraint_str(int k);
};

class DecoderFeatureIndex: public FeatureIndex {
private:
	std::map <std::string, int> dic_;
	map<string, int> constraint_dic_;

	int addfID(const char *, int ysize);
	int getfID(const char*);

    int addConstraintID(const char *, vector<double> p);
    int getConstraintID(const char*);

public:
    // for debugging
    map<string, int> feature_dict_;

    explicit DecoderFeatureIndex() {
	   init();
    }
    bool open(string str); //open model
};
}

#endif
