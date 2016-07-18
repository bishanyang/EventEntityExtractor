/*
 * Document.h
 *
 *  Created on: Mar 19, 2013
 *      Author: bishan
 */

#ifndef DOCUMENT_H_
#define DOCUMENT_H_

#include "Sentence.h"

using namespace std;

class Document{
public:
	Document() {};
	~Document() {
		for (int i = 0; i < sentences.size(); ++i) {
			delete sentences[i];
			sentences[i] = NULL;
		}
	};

	int SentNum() { return sentences.size(); }
	void SetDocID(string id) { doc_id = id; }
	string DocID() { return doc_id; }

	Sentence *findSentence(int sentid);
	void OutputEventAnnos(string outputfile);

	void BuildVocab(map<string, int> &vocabulary);

public:
	string doc_id;
	vector<Sentence *> sentences;

};

#endif /* DOCUMENT_H_ */
