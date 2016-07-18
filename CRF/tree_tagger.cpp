/*
 * tree_tagger.cpp
 *
 *  Created on: Feb 2, 2014
 *      Author: bishan
 */

#include "tree_tagger.h"
#include <assert.h>

namespace CRFPP {

void TreeTagger::DeleteTree(TreeTaggerNode *node) {
	for (int i = 0; i < node->children_path.size(); ++i) {
		for (int j = 0; j < node->children_path[i].size(); ++j) {
			DeleteTree(node->children_path[i][j]->child);
			delete node->children_path[i][j];
		}
	}
	delete node;
}

void TreeTagger::check_gradient(double *expected, int alpha_index)
{
/*	double eps = 1.49011611938e-08;

	double orig_a = feature_index_->get_alpha(alpha_index);
	double orig_log_Z = log_Z;

	double s = 0.0;
	int py = 0;
	int cy = 0;

	py = predicate_answer_;
	s += predicate_nodes[py]->cost;
	for (int i = 0; i < argument_nodes.size(); ++i) {
		cy = argument_answer_[i];
		s += argument_nodes[i][cy]->cost;
		for (int j = 0; j < predicate_nodes[py]->children_path[i].size(); ++j) {
			TreeTaggerPath *p = predicate_nodes[py]->children_path[i][j];
			if (p->child->y == cy) {
				s += p->cost;
				break;
			}
		}
	}

	double f_before = s-log_Z;

	// for each dimension compute gradient by ...
	feature_index_->set_alpha(alpha_index, orig_a+eps);

	ComputeCost();

	SumProduct();

	// cal numerator
	s = 0.0;
	py = predicate_answer_;
	s += predicate_nodes[py]->cost;
	for (int i = 0; i < argument_nodes.size(); ++i) {
		cy = argument_answer_[i];
		s += argument_nodes[i][cy]->cost;
		for (int j = 0; j < predicate_nodes[py]->children_path[i].size(); ++j) {
			TreeTaggerPath *p = predicate_nodes[py]->children_path[i][j];
			if (p->child->y == cy) {
				s += p->cost;
				break;
			}
		}
	}

	double f_after = s - log_Z;

	double approx_grad = -(double) (f_after-f_before)/eps;

	if (fabs(approx_grad - expected[alpha_index]) >0.000001) {
		cout<<"gradient is wrong!!!"<<endl;
	} else {
		cout<<"gradient check pass"<<fabs(approx_grad - expected[alpha_index])<<endl;
	}

	feature_index_->set_alpha(alpha_index, orig_a);
	log_Z = orig_log_Z;*/
}

double TreeTagger::gradient(double *expected)
{
	// reallocate tree structure
	feature_index_->buildTreeTagger(this);

	ComputeCost();

    //calculate log_Z and marginal
    SumProduct();

    // cal expectation
	for (size_t i = 0; i < predicate_nodes.size(); ++i) {
		predicate_nodes[i]->calcExpectation(expected);
	}

	for (int i = 0; i < argument_role_nodes.size(); ++i) {
		for (int j = 0; j < argument_role_nodes[i].size(); ++j) {
			argument_role_nodes[i][j]->calcExpectation(expected);
		}
	}

	for (int i = 0; i < argument_attr_nodes.size(); ++i) {
		for (int j = 0; j < argument_attr_nodes[i].size(); ++j) {
			argument_attr_nodes[i][j]->calcExpectation(expected);
		}
	}

	// cal gradient
	double s = 0.0;
	int py = predicate_answer_;
	for (const int *f = predicate_nodes[py]->binary_fea_vector; *f != -1; ++f) {
		expected[*f + py] -= 1.0;
	}
	double *fv = predicate_nodes[py]->float_fv_vector;
	for (const int *f = predicate_nodes[py]->float_fea_vector; *f != -1; ++f) {
		expected[*f + py] -= (*fv);
		++fv;
	}
	s += predicate_nodes[py]->cost;

	for (int i = 0; i < argument_role_nodes.size(); ++i) {
		int cy = argrole_answer_[i];
		for (const int *f = argument_role_nodes[i][cy]->binary_fea_vector; *f != -1; ++f) {
			expected[*f + cy] -= 1.0;
		}
		fv = argument_role_nodes[i][cy]->float_fv_vector;
		for (const int *f = argument_role_nodes[i][cy]->float_fea_vector; *f != -1; ++f) {
			expected[*f + cy] -= (*fv);
			++fv;
		}
		s += argument_role_nodes[i][cy]->cost;

		for (int j = 0; j < predicate_nodes[py]->children_path[i].size(); ++j) {
			TreeTaggerPath *p = predicate_nodes[py]->children_path[i][j];
			if (p->y == py * feature_index_->argysize() + cy) {
				for (const int *f = p->binary_fea_vector; *f != -1; ++f) {
					expected[*f + p->y] -= 1.0;
				}
				fv = p->float_fv_vector;
				for (const int *f = p->float_fea_vector; *f != -1; ++f) {
					expected[*f + p->y] -= (*fv);
					++fv;
				}
				s += p->cost;
				break;
			}
		}
	}

	for (int i = 0; i < argument_attr_nodes.size(); ++i) {
		int cy = argrole_answer_[i];
		int ay = argattr_answer_[i];
		for (const int *f = argument_attr_nodes[i][ay]->binary_fea_vector; *f != -1; ++f) {
			expected[*f + ay] -= 1.0;
		}
		fv = argument_attr_nodes[i][ay]->float_fv_vector;
		for (const int *f = argument_attr_nodes[i][ay]->float_fea_vector; *f != -1; ++f) {
			expected[*f + ay] -= (*fv);
			++fv;
		}
		s += argument_attr_nodes[i][ay]->cost;

		for (int j = 0; j < argument_role_nodes[i][cy]->children_path[0].size(); ++j) {
			TreeTaggerPath *p = argument_role_nodes[i][cy]->children_path[0][j];
			if (p->y == cy * feature_index_->argattrsize() + ay) {
				for (const int *f = p->binary_fea_vector; *f != -1; ++f) {
					expected[*f + p->y] -= 1.0;
				}
				fv = p->float_fv_vector;
				for (const int *f = p->float_fea_vector; *f != -1; ++f) {
					expected[*f + p->y] -= (*fv);
					++fv;
				}
				s += p->cost;
				break;
			}
		}
	}

	return log_Z - s;
}

// Marginals are defined for all factors
void TreeTagger::SumProduct() {
	// Messages along leaf-to-root order, then root-to-leaf
	SumProdLeafToRoot();
	SumProdRootToLeaf();

	// compute partition function
	log_Z = 0.0;
	for (int i = 0; i < predicate_nodes.size(); ++i) {
		double c = predicate_nodes[i]->cost;
		for (int j = 0; j < predicate_nodes[i]->children_alpha.size(); ++j) {
			c += predicate_nodes[i]->children_alpha[j];
		}
		log_Z = logsumexp(log_Z, c, i == 0);
	}

	// compute marginals
	for (int i = 0; i < predicate_nodes.size(); ++i) {
		predicate_nodes[i]->calMarginal(log_Z);
	}
	for (int i = 0; i < argument_role_nodes.size(); ++i) {
		for (int j = 0; j < argument_role_nodes[i].size(); ++j)
			argument_role_nodes[i][j]->calMarginal(log_Z);
	}
	for (int i = 0; i < argument_attr_nodes.size(); ++i) {
		for (int j = 0; j < argument_attr_nodes[i].size(); ++j)
			argument_attr_nodes[i][j]->calMarginal(log_Z);
	}
}


// Marginals are defined for all factors
void TreeTagger::Inference() {
	feature_index_->buildTreeTagger(this);

	ComputeCost();

	// need probability output
	SumProduct();

	double bestc = -1e37;
	double besty = 0;
	for (int i = 0; i < predicate_nodes.size(); ++i) {
		if (predicate_nodes[i]->marginal > bestc) {
			bestc = predicate_nodes[i]->marginal;
			besty = predicate_nodes[i]->y;
		}
	}
	predicate_result_ = besty;

	for (int i = 0; i < argument_role_nodes.size(); ++i) {
		bestc = -1e37;
		besty = 0;
		for (int j = 0; j < argument_role_nodes[i].size(); ++j) {
			if (argument_role_nodes[i][j]->marginal > bestc) {
				bestc = argument_role_nodes[i][j]->marginal;
				besty = argument_role_nodes[i][j]->y;
			}
		}
		argrole_result_[i] = besty;
	}

	for (int i = 0; i < argument_attr_nodes.size(); ++i) {
		bestc = -1e37;
		besty = 0;
		for (int j = 0; j < argument_attr_nodes[i].size(); ++j) {
			if (argument_attr_nodes[i][j]->marginal > bestc) {
				bestc = argument_attr_nodes[i][j]->marginal;
				besty = argument_attr_nodes[i][j]->y;
			}
		}
		argattr_result_[i] = besty;
	}
}

void TreeTagger::ComputeCost(TreeTaggerNode* node) {
	feature_index_->calcTreeNodeCost(node);
	for (int i = 0; i < node->children_path.size(); ++i) {
		for (int j = 0; j < node->children_path[i].size(); ++j) {
			feature_index_->calcTreePathCost(node->children_path[i][j]);
		}
	}
}

void TreeTagger::ClearCost(TreeTaggerNode* node) {
	node->cost = 0.0;
	for (int i = 0; i < node->children_path.size(); ++i) {
		for (int j = 0; j < node->children_path[i].size(); ++j)
			node->children_path[i][j]->cost = 0.0;
	}
}

void TreeTagger::ComputeCost() {
	// clear cost
	for (size_t y = 0; y < predicate_nodes.size(); ++y) {
		ClearCost(predicate_nodes[y]);
	}
	for (int i = 0; i < argument_role_nodes.size(); ++i) {
		for (int y = 0; y < argument_role_nodes[i].size(); ++y)
			ClearCost(argument_role_nodes[i][y]);
	}
	for (int i = 0; i < argument_attr_nodes.size(); ++i) {
		for (int y = 0; y < argument_attr_nodes[i].size(); ++y)
			ClearCost(argument_attr_nodes[i][y]);
	}

	// calculate node cost
	for (size_t y = 0; y < predicate_nodes.size(); ++y) {
		ComputeCost(predicate_nodes[y]);
	}
	for (int i = 0; i < argument_role_nodes.size(); ++i) {
		for (int y = 0; y < argument_role_nodes[i].size(); ++y)
			ComputeCost(argument_role_nodes[i][y]);
	}
	for (int i = 0; i < argument_attr_nodes.size(); ++i) {
		for (int y = 0; y < argument_attr_nodes[i].size(); ++y)
			ComputeCost(argument_attr_nodes[i][y]);
	}
}

void TreeTagger::SumProdLeafToRoot() {
	// attr -> factor
	for (int argi = 0; argi < argument_attr_nodes.size(); ++argi) {
		for (int ay = 0; ay < argument_attr_nodes[argi].size(); ++ay) {
			TreeTaggerNode *node = argument_attr_nodes[argi][ay];
			node->parent_alpha = node->cost;
		}
	}

	// factor -> role
	for (int argi = 0; argi < argument_role_nodes.size(); ++argi) {
		for (int ry = 0; ry < argument_role_nodes[argi].size(); ++ry) {
			TreeTaggerNode *node = argument_role_nodes[argi][ry];

			double c = 0.0;
			for (int ay = 0; ay < node->children_path[0].size(); ++ay) {
				TreeTaggerPath* p = node->children_path[0][ay];
				c = logsumexp(c, p->cost + p->child->parent_alpha, (ay == 0));
			}
			node->children_alpha[0] = c;
		}
	}

	// role -> factor
	for (int argi = 0; argi < argument_role_nodes.size(); ++argi) {
		for (int ry = 0; ry < argument_role_nodes[argi].size(); ++ry) {
			TreeTaggerNode *node = argument_role_nodes[argi][ry];
			node->parent_alpha = node->cost + node->children_alpha[0];
		}
	}

	// factor -> predicate
	for (int py = 0; py < predicate_nodes.size(); ++py) {
		TreeTaggerNode *node = predicate_nodes[py];
		for (int argi = 0; argi < node->children_alpha.size(); ++argi) {
			double c = 0.0;
			for (int k = 0; k < node->children_path[argi].size(); ++k) {
				TreeTaggerPath* p = node->children_path[argi][k];
				c = logsumexp(c, p->cost + p->child->parent_alpha, (k == 0));
			}
			node->children_alpha[argi] = c;
		}
	}
}

void TreeTagger::SumProdRootToLeaf() {
	// predicate -> factor
	for (int py = 0; py < predicate_nodes.size(); ++py) {
		TreeTaggerNode *node = predicate_nodes[py];
		for (int argi = 0; argi < node->children_beta.size(); ++argi) {
			double c = node->cost;
			for (int k = 0; k < node->children_alpha.size(); ++k) {
				if (k == argi) continue;
				c += node->children_alpha[k];
			}
			node->children_beta[argi] = c;
		}
	}

	// factor -> role
	for (int argi = 0; argi < argument_role_nodes.size(); ++argi) {
		for (int ry = 0; ry < argument_role_nodes[argi].size(); ++ry) {
			TreeTaggerNode *node = argument_role_nodes[argi][ry];
			double c = 0.0;
			for (int k = 0; k < node->parent_path.size(); ++k) {
				TreeTaggerPath *p = node->parent_path[k];
				c = logsumexp(c, p->cost + p->parent->children_beta[argi], (k == 0));
			}
			node->parent_beta = c;
		}
	}

	// role -> factor
	for (int argi = 0; argi < argument_role_nodes.size(); ++argi) {
		for (int ry = 0; ry < argument_role_nodes[argi].size(); ++ry) {
			TreeTaggerNode *node = argument_role_nodes[argi][ry];
			node->children_beta[0] = node->cost + node->parent_beta;
		}
	}

	// factor -> attr
	for (int argi = 0; argi < argument_attr_nodes.size(); ++argi) {
		for (int ay = 0; ay < argument_attr_nodes[argi].size(); ++ay) {
			TreeTaggerNode *node = argument_attr_nodes[argi][ay];
			double c = 0.0;
			for (int k = 0; k < node->parent_path.size(); ++k) {
				TreeTaggerPath *p = node->parent_path[k];
				c = logsumexp(c, p->cost + p->parent->children_beta[0], (k == 0));
			}
			node->parent_beta = c;
		}
	}
}

bool TreeTagger::open(FeatureIndex *f) {
    feature_index_ = f;

    return true;
}

bool TreeTagger::clear() {
	predicate_nodes.clear();
	std::vector<TreeTaggerNode*>().swap(predicate_nodes);

	argument_role_nodes.clear();
	std::vector<vector<TreeTaggerNode*> >().swap(argument_role_nodes);

	argument_attr_nodes.clear();
	std::vector<vector<TreeTaggerNode*> >().swap(argument_attr_nodes);

	arguments.clear();
	std::vector<string>().swap(arguments);

	predicate_answer_ = 0;
	predicate_result_ = 0;

	argrole_answer_.clear();
	std::vector<int>().swap(argrole_answer_);
	argattr_answer_.clear();
	std::vector<int>().swap(argattr_answer_);
	argrole_result_.clear();
	std::vector<int>().swap(argrole_result_);
	argattr_result_.clear();
	std::vector<int>().swap(argattr_result_);

	log_Z = 0.0;

    return true;
}

bool TreeTagger::EventToTreeTagger(EventMention *event, Sentence *sent, CRFPP::FeatureIndex *feature_index, bool train) {

	open(feature_index);

	clear();

	// predicate string
	predicate = event->index;
	if (train) predicate_answer_ = feature_index->lb2i[event->gold_subtype];
	else predicate_answer_ = 0;

	// read features name label f:v f:v ...
	argrole_answer_.clear();
	argattr_answer_.clear();

	for (size_t i = 0; i < sent->cand_entity_mentions.size(); ++i) {
		Mention *entity = sent->cand_entity_mentions[i];
		arguments.push_back(entity->index);
		if (train) {
			string gold_role = event->GetGoldArgRole(i);
			argrole_answer_.push_back(feature_index->arglb2i[gold_role]);
		} else {
			argrole_answer_.push_back(0);
		}

		arg_attributes.push_back(entity->index);
		if (train) {
			string gold_ner = entity->gold_ner_type;
			argattr_answer_.push_back(feature_index->argattrlb2i[gold_ner]);
		} else {
			argattr_answer_.push_back(0);
		}
	}

	predicate_result_ = 0;
	argrole_result_.resize(argrole_answer_.size(), 0);
	argattr_result_.resize(argattr_answer_.size(), 0);

	predicate_nodes.resize(feature_index->ysize());
	argument_role_nodes.resize(arguments.size(), vector<TreeTaggerNode*>(feature_index_->argysize()));
	argument_attr_nodes.resize(arguments.size(), vector<TreeTaggerNode*>(feature_index_->argattrsize()));

	feature_index_->buildEventTreeFeatures(this, event, sent, train);

	return true;
}

} /* namespace CRFPP */
