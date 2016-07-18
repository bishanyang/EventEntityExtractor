//
//  CRF++ -- Yet Another CRF toolkit
//
//  $Id: tagger.cpp 1601 2007-03-31 09:47:18Z taku $;
//
//  Copyright(C) 2005-2007 Taku Kudo <taku@chasen.org>
//
#include <iostream>
#include <vector>
#include <iterator>
#include <cmath>
#include <string>
#include <sstream>
#include "common.h"
#include "tagger.h"
#include <stdio.h>
#include <stdlib.h>

#include "../Utils.h"

//#define DEBUG_

namespace CRFPP {

bool TaggerImpl::open(FeatureIndex *f) {
    mode_ = LEARN;
    feature_index_ = f;
    ysize_ = feature_index_->ysize();

    return true;
}

bool TaggerImpl::clear() {
    nodes.clear();
    edges.clear();

    node_.clear();
    answer_.clear();
    result_.clear();

    Z_ = 0.0;

    return true;
}

double TaggerImpl::gradient(double *expected) {
    // reallocate the node structure
    //feature_index_->buildSequenceTagger(this);
	feature_index_->buildCRFTagger(this);

    recomputeCost();

	// forwardbackward
	for (int i = 0; i < static_cast<int>(nodes.size()); ++i) {
		 for (size_t j = 0; j < ysize_; ++j) {
			 node_[i][j]->calcAlpha();
		 }
	}
	for (int i = static_cast<int>(nodes.size() - 1); i >= 0;  --i) {
		 for (size_t j = 0; j < ysize_; ++j) {
			 node_[i][j]->calcBeta();
		 }
	}

	Z_ = 0.0;
	for (size_t j = 0; j < ysize_; ++j) {
		Z_ = logsumexp(Z_, node_[nodes.size() - 1][j]->alpha, j == 0);
	}

	// calculate marginal prob and expectation
	for (size_t i = 0; i < nodes.size(); ++i) {
		for (size_t j = 0; j < ysize_; ++j) {
			node_[i][j]->prob = std::exp(node_[i][j]->alpha + node_[i][j]->beta - Z_);
			for (const_Path_iterator it = node_[i][j]->lpath.begin(); it != node_[i][j]->lpath.end(); ++it) {
				(*it)->prob = std::exp((*it)->lnode->alpha + (*it)->cost + (*it)->rnode->cost + (*it)->rnode->beta - Z_);
			}
		}
	}

	for (size_t i = 0;   i < nodes.size(); ++i) {
	  for (size_t j = 0; j < ysize_; ++j) {
		node_[i][j]->calcExpectation(expected, Z_, ysize_);
	  }
	}

	double s = 0.0;
	for (size_t i = 0;   i < nodes.size(); ++i) {
		for (const int *f = node_[i][answer_[i]]->binary_fea_vector; *f != -1; ++f) {
			 expected[*f + answer_[i]] -= 1.0;
		}
		double *fv = node_[i][answer_[i]]->float_fv_vector;
		for (const int *f = node_[i][answer_[i]]->float_fea_vector; *f != -1; ++f) {
			 expected[*f + answer_[i]] -= *fv;
			 ++fv;
		}

	    s += node_[i][answer_[i]]->cost;  // UNIGRAM cost

	    const std::vector<Path *> &lpath = node_[i][answer_[i]]->lpath;
	    for (const_Path_iterator it = lpath.begin(); it != lpath.end(); ++it) {
	    	if ((*it)->lnode->y == answer_[(*it)->lnode->x]) {
	    		for (const int *f = (*it)->binary_fea_vector; *f != -1; ++f) {
					expected[*f +(*it)->yindex] -= 1.0;
	    	    }
	    		double *fv = (*it)->float_fv_vector;
	    		for (const int *f = (*it)->float_fea_vector; *f != -1; ++f) {
	    			expected[*f +(*it)->yindex] -= *fv;
	    			++fv;
	    		}
	    		s += (*it)->cost;  // BIGRAM COST
	    		break;
	    	}
	    }
	}

	return Z_ - s ;
}


void TaggerImpl::recomputeCost() {
	//clear node structure
	for (size_t i = 0; i < nodes.size(); ++i) {
	  for (size_t j = 0; j < ysize_; ++j) {
		 node_[i][j]->cost = 0.0;
		 vector<Path *> &lpath = node_[i][j]->lpath;
		 for (const_Path_iterator it = lpath.begin(); it != lpath.end(); ++it)
			 (*it)->cost = 0.0;
	  }
	}

	for (size_t i = 0; i < nodes.size(); ++i) {
	  for (size_t j = 0; j < ysize_; ++j) {
		 feature_index_->calcCost(node_[i][j]);

		 const std::vector<Path *> &lpath = node_[i][j]->lpath;
		 for (const_Path_iterator it = lpath.begin(); it != lpath.end(); ++it)
			 feature_index_->calcCost(*it);
	  }
	}
}

bool TaggerImpl::Inference() {
	//feature_index_->buildSequenceTagger(this);
	feature_index_->buildCRFTagger(this);

	recomputeCost();

	for (int i = 0; i < static_cast<int>(nodes.size()); ++i) {
		 for (size_t j = 0; j < ysize_; ++j) {
			 node_[i][j]->calcAlpha();
		 }
	 }

	 for (int i = static_cast<int>(nodes.size() - 1); i >= 0;  --i) {
		 for (size_t j = 0; j < ysize_; ++j) {
			 node_[i][j]->calcBeta();
		 }
	 }

	 // need to calculate prob
	 Z_ = 0.0;
	 for (size_t j = 0; j < ysize_; ++j) {
		Z_ = logsumexp(Z_, node_[nodes.size() - 1][j]->alpha, j == 0);
	}

	 // calculate marginal prob
	for (size_t i = 0; i < nodes.size(); ++i) {
		for (size_t j = 0; j < ysize_; ++j) {
			node_[i][j]->prob = std::exp(node_[i][j]->alpha + node_[i][j]->beta - Z_);
			for (const_Path_iterator it = node_[i][j]->lpath.begin(); it != node_[i][j]->lpath.end(); ++it) {
				(*it)->prob = std::exp((*it)->lnode->alpha + (*it)->cost + (*it)->rnode->cost + (*it)->rnode->beta - Z_);
			}
		}
	}

	viterbi_decoder.inference(nodes.size(), ysize_, node_, result_);

	return true;
}

double TaggerImpl::eval() {
	int acc = 0;
	int total = 0;
	for (int i = 0; i < answer_.size(); ++i) {
		if (answer_[i] != 0 || result_[i] != 0) {
			if (answer_[i] == result_[i]) acc++;
			total++;
		}
	}
	if (total == 0) return 0.0;
	return (double)acc/total;
}

double TaggerImpl::pairwise_log_potential(int y1, int y2) {
	// assume there are only two nodes
	double logp = node_[0][y1]->cost;
	const std::vector<Path *> &lpath = node_[1][y2]->lpath;
	for (const_Path_iterator it = lpath.begin(); it != lpath.end(); ++it) {
		if ((*it)->lnode->y == y1) {
			logp += (*it)->cost + node_[1][y2]->cost;
			break;
		}
	}

	return logp - Z_;
}

bool TaggerImpl::next() {
	return viterbi_decoder.next(result_);
}

double TaggerImpl::span_cost(int start, int end, int y) {
		double c = node_[start][y]->cost;
		for (int i = start+1; i <= end; ++i) {
			c += node_[i][y]->cost;
			for (const_Path_iterator it = node_[i][y]->lpath.begin(); it != node_[i][y]->lpath.end(); ++it) {
				if ((*it)->lnode->y == y) {
					c += (*it)->cost;
					break;
				}
			}
		}
		return c;
 }

double TaggerImpl::segment_prob(int start, int end, string ystr) {
	int y = feature_index_->lb2i[ystr];
	if (start == end) {
		return std::exp(node_[start][y]->alpha + node_[start][y]->beta - Z_);
	} else {
		double c = node_[start][y]->alpha;
		for (int k = start+1; k <= end; ++k) {
			for (const_Path_iterator it = node_[k][y]->lpath.begin(); it != node_[k][y]->lpath.end(); ++it) {
				if ((*it)->lnode->y == y) {
					c += (*it)->cost;
					break;
				}
			}
			c += node_[k][y]->cost;
		}
		c += node_[end][y]->beta;
		return std::exp(c - Z_);
	}
}

double TaggerImpl::segment_BIO_prob(int start, int end, string ystr) {
	int start_y = feature_index_->lb2i["B_"+ystr];
	int y = feature_index_->lb2i["I_"+ystr];
	if (start == end) {
		return std::exp(node_[start][start_y]->alpha + node_[start][start_y]->beta - Z_);
	} else {
		double c = node_[start][start_y]->alpha;
		int k = start+1;
		for (const_Path_iterator it = node_[k][y]->lpath.begin(); it != node_[k][y]->lpath.end(); ++it) {
			if ((*it)->lnode->y == start_y) {
				c += (*it)->cost;
				break;
			}
		}
		c += node_[k][y]->cost;

		for (k = start+2; k <= end; ++k) {
			for (const_Path_iterator it = node_[k][y]->lpath.begin(); it != node_[k][y]->lpath.end(); ++it) {
				if ((*it)->lnode->y == y) {
					c += (*it)->cost;
					break;
				}
			}
			c += node_[k][y]->cost;
		}
		c += node_[end][y]->beta;
		return std::exp(c - Z_);
	}
}


bool TaggerImpl::SentenceToTagger(Sentence *sent, CRFPP::FeatureIndex *feature_index, bool train) {
	open(feature_index);

	int length = sent->size();
	if (length == 0) return false;

	clear();

	answer_.resize(length, 0);
	result_.resize(length, 0);
	nodes.resize(length);

	// read labels
	for(int i=0; i<sent->token_labels.size(); i++) {
		string label = sent->token_labels[i];
		answer_[i] = feature_index->lb2i[label];
	}

	x_.resize(length);
	for (int i = 0; i < sent->words.size(); ++i) {
		x_[i] = sent->words[i];
	}
	node_.resize(length);
	for (int i = 0; i < length; ++i) {
		node_[i].resize(ysize_);
		nodes[i] = i;
	}

//	for (int i = 0; i < length-1; ++i) {
//		edges.push_back(make_pair(i, i+1));
//	}

	feature_index_->buildCRFFeatures(this, sent, train);

	return true;
}
bool TaggerImpl::PairToTagger(Sentence *sent, EventMention *m1, EventMention *m2, CRFPP::FeatureIndex *feature_index, bool train) {
	open(feature_index);

	int length = 2;

	clear();

	answer_.resize(length, 0);
	result_.resize(length, 0);
	nodes.resize(length);

	// read labels
	answer_[0] = feature_index->lb2i[m1->gold_subtype];
	answer_[1] = feature_index->lb2i[m2->gold_subtype];

	x_.resize(length);
	x_[0] = m1->menstr;
	x_[1] = m2->menstr;

	node_.resize(length);
	for (int i = 0; i < length; ++i) {
		node_[i].resize(ysize_);
		nodes[i] = i;
	}

	feature_index_->buildPairwiseFeatures(this, sent, m1, m2, train);

	return true;
}
}



