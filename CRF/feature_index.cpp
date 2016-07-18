//
//  CRF++ -- Yet Another CRF toolkit
//
//  $Id: feature_index.cpp 1587 2007-02-12 09:00:36Z taku $;
//
//  Copyright(C) 2005-2007 Taku Kudo <taku@chasen.org>
//
#include "feature_index.h"

#include <iostream>
#include <fstream>
#include <cstring>
#include <set>
#include <vector>

#include "common.h"


namespace CRFPP {

static inline char *read_ptr(char **ptr, size_t size) {
  char *r = *ptr;
  *ptr += size;
  return r;
}

template <class T> static inline void read_static(char **ptr,
                                                  T *value) {
  char *r = read_ptr(ptr, sizeof(T));
  memcpy(value, r, sizeof(T));
}

int DecoderFeatureIndex::addfID(const char *key, int ysize) {
	string fstr = key;
	if (dic_.find(fstr) != dic_.end()) {
		return dic_[fstr];
	}
	return -1;
}

int DecoderFeatureIndex::getfID(const char *key) {
	string fstr = key;
	if (dic_.find(fstr) != dic_.end()) {
		return dic_[fstr];
	}
	return -1;
}

int DecoderFeatureIndex::addConstraintID(const char *key, vector<double> p) {
	string fstr = key;
	if (constraint_dic_.find(fstr) != constraint_dic_.end()) {
		return constraint_dic_[fstr];
	}
	return -1;
}

int DecoderFeatureIndex::getConstraintID(const char *key) {
	string fstr = key;
	if (constraint_dic_.find(fstr) != constraint_dic_.end()) {
		return constraint_dic_[fstr];
	}
	return -1;
}

int EncoderFeatureIndex::addfID(const char *key, int ysize) {
  std::map <std::string, std::pair<int, int> >::iterator it = dic_.find(key);
  if (it == dic_.end()) {
    dic_.insert(std::make_pair<std::string, std::pair<int, int> >
                (key, std::make_pair<int, int>(maxid_, 1)));
    int n = maxid_;

	maxid_ += ysize;
	return n;
  } else {
    it->second.second++;
    return it->second.first;
  }
  return -1;
}

int EncoderFeatureIndex::getfID(const char *key)
{
  std::map <std::string, std::pair<int, int> >::iterator it = dic_.find(key);
  if (it == dic_.end())
     return -1;
  else
    return it->second.first;
}

int EncoderFeatureIndex::addConstraintID(const char *key, vector<double> p) {
  std::map <std::string, int >::iterator it = constraint_dic_.find(key);
  if (it == constraint_dic_.end()) {
	  constraint_dic_.insert(std::make_pair<std::string, int>(key, maxmuid_));
	  int n = maxmuid_;

	  for (int i = 0; i < p.size(); ++i) {
		  constraint_prior[n+i] = p[i];
	  }

	  maxmuid_ += p.size();
	  return n;
  } else {

	  int n = it->second;
      for (int i = 0; i < p.size(); ++i) {
		  constraint_prior[n+i] += p[i];
	  }

      return it->second;
  }
}

int EncoderFeatureIndex::getConstraintID(const char *key) {
  std::map <std::string, int>::iterator it = constraint_dic_.find(key);
  if (it == constraint_dic_.end())
     return -1;
  else
    return it->second;
}

bool DecoderFeatureIndex::open(string modelstr) {
  vector<string> lines;
  Utils::Split(modelstr, '\n', lines);

  int l = 0;
  maxid_ = atoi(lines[l++].c_str());
  maxmuid_ = atoi(lines[l++].c_str());

  int size = atoi(lines[l++].c_str());
  int c = 0;
  while (c < size) {
    y_.push_back(lines[l++].c_str());
    c++;
  }
  for(int i=0; i<y_.size(); i++) {
	  string y = y_[i];
	  lb2i[y] = i;
	  i2lb[i] = y;
  }

  size = atoi(lines[l++].c_str());
  c = 0;
  while (c < size) {
    arg_y_.push_back(lines[l++].c_str());
    c++;
  }
    for(int i=0; i<arg_y_.size(); i++) {
  	  string y = arg_y_[i];
  	  arglb2i[y] = i;
  	  argi2lb[i] = y;
    }

    size = atoi(lines[l++].c_str());
    c = 0;
    while (c < size) {
    	arg_attr_y_.push_back(lines[l++].c_str());
    	c++;
    }
	for(int i=0; i<arg_attr_y_.size(); i++) {
	  string y = arg_attr_y_[i];
	  argattrlb2i[y] = i;
	  argattri2lb[i] = y;
	}

	size = atoi(lines[l++].c_str());
	c = 0;
	while (c < size) {
     string label = lines[l++];

     int i = label.find('_');
     string y1 = label.substr(0, i);
     string y2 = label.substr(i+1);

     int ilabel = lb2i[y1] * y_.size() + lb2i[y2];

     transition_i2lb[c] = ilabel;
     transition_lb2i[ilabel] = c;
     c++;
   }

	size = atoi(lines[l++].c_str());
	c = 0;
	while (c < size) {
		string line = lines[l++];
		int i = line.find('\t');
		string fstr = line.substr(0,i);
		int fid = atoi(line.substr(i+1).c_str());
		dic_[fstr] = fid;
		c++;
	}

	size = atoi(lines[l++].c_str());
	alpha_ = new double[size];
	c = 0;
	while (c < size) {
		alpha_[c] = atof(lines[l++].c_str());
		c++;
	}

	size = atoi(lines[l++].c_str());
	c = 0;
	while (c < size) {
		string line = lines[l++];
		int i = line.find('\t');
		string fstr = line.substr(0,i);
		int fid = atoi(line.substr(i+1).c_str());
		constraint_dic_[fstr] = fid;
		c++;
	}

	size = atoi(lines[l++].c_str());
	mu_ = new double[size];
	c = 0;
	while (c < size) {
		mu_[c] = atof(lines[l++].c_str());
		c++;
	}

	if (l != (int)lines.size()) {
		return false;
	}

  return true;
}

void EncoderFeatureIndex::get_featuremap(map<string, int> &fstr2id) {
	fstr2id.clear();
	for (std::map<std::string, std::pair<int, int> >::iterator it = dic_.begin(); it != dic_.end(); ++it) {
	  string fstr = it->first;
	  int fid = it->second.first;
	  fstr2id[fstr] = fid;
	}
}

void EncoderFeatureIndex::get_constraintmap(map<string, int> &cstr2id) {
	cstr2id.clear();
	for (std::map<std::string, int >::iterator it = constraint_dic_.begin(); it != constraint_dic_.end(); ++it) {
	  string fstr = it->first;
	  int fid = it->second;
	  cstr2id[fstr] = fid;
	}
}

string EncoderFeatureIndex::get_constraint_str(int k) {
	for (map<string, int>::iterator it = constraint_dic_.begin(); it != constraint_dic_.end(); ++it) {
		if (it->second == k) {
			return it->first;
		}
	}
	return "";
}

void FeatureIndex::clear() {
  char_freelist_.free();
  feature_cache_.clear();
  feature_value_cache_.clear();

  for (size_t i = 0; i < thread_num_; ++i) {
    node_freelist_[i].free();
    path_freelist_[i].free();

    treenode_freelist_[i].free();
    treepath_freelist_[i].free();
  }
}

bool EncoderFeatureIndex::save(const char *filename, bool textmodelfile) {
	std::ofstream bofs;
	bofs.open(filename, OUTPUT_MODE);
    CHECK_FALSE(bofs) << "open failed: " << filename;

    bofs<<maxid_<<endl;
    bofs<<maxmuid_<<endl;

    bofs<<y_.size()<<endl;
	for (size_t i = 0; i < y_.size(); ++i) {
		bofs<<y_[i]<<endl;
	}

	bofs<<arg_y_.size()<<endl;
	for (size_t i = 0; i < arg_y_.size(); ++i) {
		bofs<<arg_y_[i]<<endl;
	}

	bofs<<arg_attr_y_.size()<<endl;
	for (size_t i = 0; i < arg_attr_y_.size(); ++i) {
		bofs<<arg_attr_y_[i]<<endl;
	}

	// transition label info
	bofs<<transition_i2lb.size()<<endl;
	for (size_t i = 0; i < transition_i2lb.size(); ++i) {
		int ilabel = transition_i2lb[i];
		int y1 = ilabel/y_.size();
		int y2 = ilabel%y_.size();
		bofs<<std::string(y_[y1]) + "_" + std::string(y_[y2])<<endl;
	}

	//feature dic_
	bofs<<dic_.size()<<endl;
	for (std::map<std::string, std::pair<int, int> >::iterator it = dic_.begin(); it != dic_.end(); ++it) {
		bofs<<it->first<<"\t"<<it->second.first<<endl;
	}

    //feature param
	bofs<<maxid_<<endl;
    for (size_t i  = 0; i < maxid_; ++i) {
    	bofs<<alpha_[i]<<endl;
    }

    //constraint dic_
    bofs<<constraint_dic_.size()<<endl;
	for (std::map<std::string, int >::iterator it = constraint_dic_.begin(); it != constraint_dic_.end(); ++it) {
		bofs<<it->first.c_str()<<"\t"<<it->second<<endl;
	}

	//constraint param
	bofs<<maxmuid_<<endl;
	for (size_t i  = 0; i < maxmuid_; ++i) {
		bofs<<mu_[i]<<endl;
	}

    bofs.close();


   return true;
}

char *FeatureIndex::strdup(const char *p) {
  size_t len = std::strlen(p);
  char *q = char_freelist_.alloc(len+1);
  std::strcpy(q, p);
  return q;
}

void FeatureIndex::calcCost(Node *n) {
  n->cost = 0.0;

#define ADD_COST(T, A)                                                  \
  { T c = 0;                                                            \
    for (int *f = n->binary_fea_vector; *f != -1; ++f) c += (A)[*f + n->y];       \
    double *fv = n->float_fv_vector; \
    for (int *f = n->float_fea_vector; *f != -1; ++f) { c += (A)[*f + n->y] * (*fv);       \
    ++fv; }\
    n->cost = (T)c; }

    ADD_COST(double, alpha_);
#undef ADD_COST
}

void FeatureIndex::calcCost(Path *p) {
  p->cost = 0.0;

#define ADD_COST(T, A)                                          \
  { T c = 0.0;                                                  \
    for (int *f = p->binary_fea_vector; *f != -1; ++f)                    \
      c += (A)[*f + p->lnode->y * y_.size() + p->rnode->y];     \
    double *fv = p->float_fv_vector; \
    for (int *f = p->float_fea_vector; *f != -1; ++f) { \
      c += (A)[*f + p->lnode->y * y_.size() + p->rnode->y] * (*fv);       \
          ++fv; }\
    p->cost =(T)c; }

    ADD_COST(double, alpha_);
#undef ADD_COST
}

void FeatureIndex::calcTreeNodeCost(TreeTaggerNode *n) {
  n->cost = 0.0;

#define ADD_COST(T, A)                                                  \
  { T c = 0;                                                            \
    for (int *f = n->binary_fea_vector; *f != -1; ++f) c += (A)[*f + n->y];       \
    double *fv = n->float_fv_vector; \
    for (int *f = n->float_fea_vector; *f != -1; ++f) { c += (A)[*f + n->y] * (*fv);       \
    ++fv; }\
    n->cost = (T)c; }

    ADD_COST(double, alpha_);
#undef ADD_COST
}

void FeatureIndex::calcTreePathCost(TreeTaggerPath *p) {
  p->cost = 0.0;

#define ADD_COST(T, A)                                          \
  { T c = 0.0;                                                  \
    for (int *f = p->binary_fea_vector; *f != -1; ++f)                    \
      c += (A)[*f + p->y];     \
    double *fv = p->float_fv_vector; \
    for (int *f = p->float_fea_vector; *f != -1; ++f) { \
      c += (A)[*f + p->y] * (*fv);       \
          ++fv; }\
    p->cost =(T)c; }

    ADD_COST(double, alpha_);
#undef ADD_COST
}

void FeatureIndex::calcTreeNodeConstraintCost(TreeTaggerNode *n) {
  n->constraint_cost = 0.0;
  double c = 0.0;
  double *fv = n->constraint_fv_vector;
  for (int *f = n->constraint_fvector; *f != -1; ++f) {
	  c += mu_[*f + n->y] * (*fv);
	  ++fv;
  }
  n->constraint_cost = c;
}

void FeatureIndex::calcTreePathConstraintCost(TreeTaggerPath *p) {
  p->constraint_cost = 0.0;
  double c = 0.0;
  double *fv = p->constraint_fv_vector;
  for (int *f = p->constraint_fvector; *f != -1; ++f)   {
      c += mu_[*f + p->y] * (*fv);
      ++fv;
  }
  p->constraint_cost = c;
}

void FeatureIndex::CopyEmbeddings(const map<string, int> &pword_lookup_table, const vector<vector<float> > &pword_embeddings) {
	word_lookup_table.clear();
	word_embeddings.clear();
	for (map<string, int>::const_iterator it = pword_lookup_table.begin(); it != pword_lookup_table.end(); ++it) {
		word_lookup_table[it->first] = it->second;
	}
	for (int i = 0; i < pword_embeddings.size(); ++i) {
		word_embeddings.push_back(pword_embeddings[i]);
	}
	if (word_embeddings.size() == 0) {
		word_vec_size = 0;
		return;
	}
	word_vec_size = word_embeddings[0].size();
}


void FeatureIndex::LoadEmbeddings(string infilename) {
	word_lookup_table.clear();
	word_embeddings.clear();

	ifstream infile(infilename.c_str(), ios::in);
	if (!infile) return;

	string str;
	while (getline(infile, str)) {
		int index = str.find('\t');
		string word = str.substr(0, index);
		string values = str.substr(index+1);
		vector<string> fields;
		Utils::Split(values, ' ', fields);
		vector<float> vec(fields.size());
		for (int i = 0; i <fields.size(); ++i) vec[i] = atof(fields[i].c_str());
		int wid = word_lookup_table.size();
		word_lookup_table[word] = wid;
		word_embeddings.push_back(vec);
	}
	infile.close();
}

void FeatureIndex::InitEmbeddingLabelSize() {
	if (word_embeddings.size() == 0) {
		word_vec_size = 0;
		return;
	}
	word_vec_size = word_embeddings[0].size();
}

void FeatureIndex::ClearWordEmbeddings() {
	word_embeddings.clear();
	word_lookup_table.clear();
}

void FeatureIndex::CopyGazetteer(map<string, string> &p_gazetteer_unigrams,
		map<string, string> &p_gazetteer_bigrams) {
	gazetteer_unigrams.clear();
	gazetteer_bigrams.clear();
	for (map<string, string>::iterator it = p_gazetteer_unigrams.begin(); it != p_gazetteer_unigrams.end(); ++it) {
		gazetteer_unigrams[it->first] = it->second;
	}
	for (map<string, string>::iterator it = p_gazetteer_bigrams.begin(); it != p_gazetteer_bigrams.end(); ++it) {
		gazetteer_bigrams[it->first] = it->second;
	}
}

}
