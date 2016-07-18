/*
 * tree_tagger.h
 *
 *  Created on: Feb 2, 2014
 *      Author: bishan
 */

#ifndef TREE_TAGGER_H_
#define TREE_TAGGER_H_

#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include "scoped_ptr.h"
#include "tree_node.h"
#include "../Utils.h"
#include "feature_index.h"
#include "../Sentence.h"

namespace CRFPP {

class TreeTagger {
public:
	TreeTagger() : feature_index_(0), feature_id_(0), feature_value_id_(0), thread_id_(0),
				log_Z(0.0), predicate_answer_(0), predicate_result_(0) {}
	~TreeTagger() {
		clear();
	}

public:
    FeatureIndex             *feature_index_;

    int predicate_answer_; // root: 0, following children
    int predicate_result_;
    vector<int> argrole_result_;
    vector<int> argrole_answer_;
    vector<int> argattr_result_;
    vector<int> argattr_answer_;

    string     predicate; // unique mids?
    vector<string>  arguments;
    vector<string>  arg_attributes;

    double log_Z;

	vector<TreeTaggerNode*> predicate_nodes; // |PY|
	vector<vector<TreeTaggerNode*> > argument_role_nodes; //|Arg| * |R|
	vector<vector<TreeTaggerNode*> > argument_attr_nodes; //|Arg| * |A|

	size_t                   feature_id_;
	size_t                   feature_value_id_;
	unsigned short           thread_id_;

public:
	void   set_feature_id(size_t id) { feature_id_  = id; }
	size_t feature_id() const { return feature_id_; }
	void   set_feature_value_id(size_t id) { feature_value_id_  = id; }
	size_t feature_value_id() const { return feature_value_id_; }

	void   set_thread_id(unsigned short id) { thread_id_ = id; }
	unsigned short thread_id() const { return thread_id_; }

public:
	bool clear();

	bool EventToTreeTagger(EventMention *event, Sentence *sent, CRFPP::FeatureIndex *feature_index, bool train);

	void check_gradient(double *expected, int alpha_index);

	double gradient(double *expected);
	void SumProduct();
	void SumProdLeafToRoot();
	void SumProdRootToLeaf();

	void Inference();

	void DeleteTree(TreeTaggerNode *node);
	bool open(FeatureIndex *f);

	void ComputeCost();
	void ComputeCost(TreeTaggerNode *node);

	void ClearCost(TreeTaggerNode* node);

};

} /* namespace CRFPP */
#endif /* TREE_TAGGER_H_ */
