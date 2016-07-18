/*
 * CRFLearner.h
 *
 *  Created on: Jan 18, 2013
 *      Author: bishan
 */

#ifndef SEQUENCEMODEL_H_
#define SEQUENCEMODEL_H_
#include <iostream>
#include "./CRF/encoder.h"
#include "./CRF/tagger.h"
#include "./CRF/feature_index.h"
#include "Param.h"
#include "Utils.h"
#include "Document.h"
#include "TaggingEval.h"
#include "Dataset.h"


class CRFLearner {
public:
	CRFLearner() {
		data = NULL;
	}
	~CRFLearner() {
		for (int i = 0; i < labelx.size(); ++i)
			delete labelx[i];
	}

	vector<CRFPP::TaggerImpl* > labelx;
	CRFPP::Encoder encoder;
	Dataset *data;

public:
	void InitEncoder(int thread_num) {
		encoder.thread_num = thread_num;
		encoder.feature_index = new CRFPP::EncoderFeatureIndex(thread_num);
	}
	void Train(string modelfile);
	void TrainEventSeq(string modelfile);

	void outputLabels(string filename);
	void outputModel(string modelfile);

};

class CRFPredictor {
public:
	CRFPredictor() {
		testset = NULL;
		nbest = 1;
		feature_index = NULL;
		scheme = "BIO";
	}
	~CRFPredictor() {
	}
public:
	CRFPP::DecoderFeatureIndex *feature_index;
	int nbest;
	Dataset *testset;

	string scheme;
	string eval_key;
public:

	void initParam(Param param) {
		nbest = param.nbest;
	}

	void PredictEntityCandidates(Document *doc);
	void PredictEventCandidates(Document *doc);

	void AddEntityCandidate(Span span, CRFPP::TaggerImpl* tagger, vector<Mention*> &pred_mentions);
	void AddEventCandidate(Span span, CRFPP::TaggerImpl* tagger, vector<EventMention*> &pred_mentions);

	void TaggingPrediction(string resultfile);

	void EventSeqPrediction(string resultfile);

	void AddOutputSpan(Span span, CRFPP::TaggerImpl* tagger, vector<EvalSpan> &pred_mentions);
	void NbestMentionPrediction(string mentionfile);

	void Evaluate(vector<vector<string> > answers,
			vector<vector<string> > results, string resultfile);
};


#endif /* SEQUENCEMODEL_H_ */
