/*
 * AD3Inference.h
 *
 *  Created on: Oct 27, 2015
 *      Author: bishan
 */

#ifndef AD3INFERENCE_H_
#define AD3INFERENCE_H_

#include "AD3/FactorGraph.h"
#include "AD3/Utils.h"
#include <cstdlib>
#include <map>
#include <string>
#include <fstream>

#include "Document.h"
#include "Sentence.h"
#include "SequenceModel.h"

class Potential {
public:
	Potential() {}
public:
	vector<double> potentials; // size: arg_num_states+1
};

class EventInstance {
public:
	string docid;
	Sentence *sent;

	EventMention *event;
	vector<Mention*> arguments;

	string predicate_index;
	vector<string> argument_indices;

	string gold_subtype;
	vector<string> gold_roles;
	vector<string> gold_attrs;

	string pred_subtype;
	vector<string> pred_roles;
	vector<string> pred_attrs;

	Potential *pred_potential;
	vector<Potential*> role_potentials;
	vector<Potential*> role_edge_potentials;
	vector<Potential*> attr_potentials;
	vector<Potential*> attr_edge_potentials;
};

class EventSequence {
public:
	string docid;

	vector<string> trigger_indices;
	vector<string> featurelines;
};


class AD3Inference {
public:
	AD3Inference() {

	}
	~AD3Inference() {
	}

public:
	AD3::FactorGraph factor_graph;
	map<string, int> trigger_to_varid;
	map<string, int> triggerpair_to_factorid;

	//map<string, vector<int> > argument_to_varids; // for ner consistency

public:
	void AddTriggerSequencePotentials(Document *doc, CRFPredictor predictor,
			map<string, int> trigger_dict, int pred_num_states);

};

#endif /* AD3INFERENCE_H_ */
