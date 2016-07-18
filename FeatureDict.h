/*
 * FeatureDict.h
 *
 *  Created on: Apr 20, 2016
 *      Author: bishan
 */

#ifndef FEATUREDICT_H_
#define FEATUREDICT_H_

#include <map>
#include <set>
#include <string>
#include "Utils.h"

using namespace std;

class FeatureDict {
public:
	FeatureDict();
	virtual ~FeatureDict();

public:

	map<string, string> nom_to_verb;
	map<string, vector<string> > verb_classes;
	map<string, vector<string> > verb_frames;

	set<string> times;
	set<string> jobtitles;
	map<string, map<string, double> > event_priors;

	map<string, string> gazetteer_unigrams;
	map<string, string> gazetteer_bigrams;

	vector<vector<float> > word_embeddings;
	map<string, int>    word_lookup_table; //word to embedding index

public:

	vector<string> getFrameByLemma(string lemma, string pos);
	string getVerbForm(string lemma);

	bool   isTime(string lemma);
	bool   isJobTitle(string lemma);

	void   InitializeDict(string featurefile);
	void   LoadEmbeddings(string embedding_content);
};

#endif /* FEATUREDICT_H_ */
