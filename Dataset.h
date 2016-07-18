/*
 * Dataset.h
 *
 *  Created on: Nov 9, 2012
 *      Author: babishan
 */

#ifndef DATASET_H_
#define DATASET_H_

#include <vector>
#include <string>
#include <map>

#include "Document.h"

using namespace std;

class Dataset {
public:
	Dataset();
	~Dataset();
public:
	vector<Sentence*> sentences;
	vector<Document*> documents;
	map<string, Document*> id2doc;

public:
	void BuildSentences();

	void CollectLabels(string scheme, set<string> &labelset);

	void BuildVocab(map<string, int> &vocabulary);

	void LoadCoNLLData(vector<string> filenames);

	void OutputCoNLL(string outputfile);

	void LoadDependencies(string dependencyfile);

	void LoadGoldEntities(string inputfile);
	void LoadCRFEntities(string inputfile);
	void LoadNELLEntities(string inputfile);
	void LoadStanfordEntities(string inputfile);

	void LoadEntityCandidates(string inputfile);
	void LoadEventCandidates(string inputfile);

	void LoadEventStructures(string inputfile);
};

#endif /* DATASET_H_ */
