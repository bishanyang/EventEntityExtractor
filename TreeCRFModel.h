/*
 * TreeCRFModel.h
 *
 *  Created on: Oct 30, 2015
 *      Author: bishan
 */

#ifndef TREECRFMODEL_H_
#define TREECRFMODEL_H_

#include <iostream>
#include <fstream>
#include "./CRF/tree_encoder.h"
#include "./CRF/tree_tagger.h"
#include "CRF/feature_index.h"
#include "Param.h"
#include "Utils.h"
#include "Document.h"
#include "Dataset.h"

class TreeCRFLearner {
public:
	TreeCRFLearner();
	virtual ~TreeCRFLearner();
	vector<CRFPP::TreeTagger* > labelx;
	CRFPP::TreeEncoder encoder;

	Dataset *data;

public:
	void InitEncoder(int thread_num) {
		encoder.thread_num = thread_num;
		encoder.feature_index = new CRFPP::EncoderFeatureIndex(thread_num);
	}
	void LoadLabels(vector<string> lines);

	void LoadNERRoles(string inputfile);
	void LoadEventRoles(string filename);

	void outputModel(string modelfile);

	void Train(string modelfile);
	void LoadLabels();
};

class TreeCRFPredictor {
public:
	TreeCRFPredictor() {
		nbest = 1;
		feature_index = NULL;

		data = NULL;
	}
	~TreeCRFPredictor() {
		feature_index->clear();
		delete feature_index;
		feature_index = NULL;
	}
public:
	CRFPP::DecoderFeatureIndex *feature_index;
	int nbest;

	Dataset *data;

public:
	void LoadModel(string modelfile);
	void outputModel(string modelfile);

	string Predict(Document *doc);
};


#endif /* TREECRFMODEL_H_ */
