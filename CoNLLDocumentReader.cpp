/*
 * CoNLLDocumentReader.cpp
 *
 *  Created on: Sep 11, 2013
 *      Author: bishan
 */

#include "CoNLLDocumentReader.h"
#include <fstream>
#include <assert.h>
#include <stack>

CoNLLDocumentReader::CoNLLDocumentReader(string filename) {
	// TODO Auto-generated constructor stub
	infile.open(filename.c_str(), ios::in);
}

CoNLLDocumentReader::~CoNLLDocumentReader() {
	// TODO Auto-generated destructor stub
}

bool CoNLLDocumentReader::ReadDocument(Document *doc) {
	string line;
	while (getline(infile, line) && line.find("#begin document") == string::npos) {
	}

	if (line == "") return false;

	assert(doc != NULL);

	int start_index = line.find('(');
	int end_index = line.find(')');
	doc->doc_id = line.substr(start_index+1, end_index-start_index-1);

	// Read sentences.
	int sent_id = 0;
	string sent_str = "";
	while (getline(infile, line) && line.find("#end document") == string::npos) {
		if (line == "") {
			Sentence *sent = new Sentence();
			if(ReadSentence(sent_str, sent)) {
				sent->docid = doc->doc_id;
				doc->sentences.push_back(sent);
			} else {
				delete sent;
				sent = NULL;
			}
			sent_str = "";
		} else {
			sent_str += line + "\n";
		}
	}

	return true;
}

bool CoNLLDocumentReader::ReadSentence(string sent_str, Sentence* sent) {
	assert(sent != NULL);

	vector<string> lines;
	Utils::Split(sent_str, '\n', lines);
	//cout<<lines.size()<<endl;

	string parse_tree = "";
	for (int i = 0; i < lines.size(); ++i) {
		vector<string> fields;
		Utils::Split(lines[i], '\t', fields);

		if (fields.size() != 12) {
			cout<<lines[i]<<endl;
			continue;
		}

		sent->sent_id = atoi(fields[1].c_str());

		string word = "";
		string pos = "";
		string lemma = "";
		word = fields[3];
		pos = fields[4];

		lemma = fields[6];

		sent->words.push_back(word);
		sent->postags.push_back(pos);
		sent->lemmas.push_back(lemma);

		// Read parse tree info.
		string parse_tag = fields[5];
		int index = parse_tag.find('*');
		if (index >= 0) {
			parse_tree += parse_tag.substr(0, index) +
					"(" + pos + "[" + word + "/" + pos + "]" + " " + word + ")"
					+ parse_tag.substr(index+1);
		}

		// read token tags
		string tag = fields[fields.size()-1];

		sent->token_labels.push_back(tag);
		sent->pred_token_labels.push_back("O");
	}

	//cout<<sent->sent_id<<" "<<sent->token_labels.size()<<endl;

	// Build parse tree.
	if (parse_tree != "")
		sent->buildPennTree(parse_tree);

	return true;
}
