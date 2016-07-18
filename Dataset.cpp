/*
 * Dataset.cpp
 *
 *  Created on: Nov 9, 2012
 *      Author: babishan
 */
#include "Dataset.h"
#include "CoNLLDocumentReader.h"

#include <fstream>
#include <set>
#include <math.h>

Dataset::Dataset() {
	// TODO Auto-generated constructor stub
}

Dataset::~Dataset() {
	// TODO Auto-generated destructor stub

//	for (int i = 0; i < sentences.size(); ++i) {
//		delete sentences[i];
//		sentences[i] = NULL;
//	}
}


void Dataset::CollectLabels(string scheme, set<string> &labelset) {
	labelset.clear();
	for (int i = 0; i < sentences.size(); ++i) {
		for (int j = 0; j <sentences[i]->token_labels.size(); ++j) {
			string label = sentences[i]->token_labels[j];
			if (scheme == "IO") {
				if (label.find('_') != string::npos) {
					label = label.substr(2);
					sentences[i]->token_labels[j] = label;
				}
			}
			labelset.insert(label);
		}
	}
}

void Dataset::BuildSentences() {
	sentences.clear();
	for (int i = 0; i < documents.size(); ++i) {
		for (int j = 0; j < documents[i]->sentences.size(); ++j) {
			sentences.push_back(documents[i]->sentences[j]);
		}
	}
}

void Dataset::BuildVocab(map<string, int> &vocabulary) {
	for (int i = 0; i < sentences.size(); ++i) {
		for (int j = 0; j < sentences[i]->words.size(); ++j) {
			string word = sentences[i]->words[j];
			if (vocabulary.find(word) == vocabulary.end()) {
				int id = vocabulary.size();
				vocabulary[word] = id;
			}
			string lemma = sentences[i]->lemmas[j];
			if (word != lemma) {
				if (vocabulary.find(lemma) == vocabulary.end()) {
					int id = vocabulary.size();
					vocabulary[lemma] = id;
				}
			}
			if (j > 0) {
				word = sentences[i]->words[j]+"_"+sentences[i]->words[j-1];
				word = Utils::toLower(word);
				if (vocabulary.find(word) == vocabulary.end()) {
					int id = vocabulary.size();
					vocabulary[word] = id;
				}
			}
		}
	}
}

void Dataset::LoadCoNLLData(vector<string> filenames) {
	documents.clear();
	id2doc.clear();
	sentences.clear();

	// Each file presents a topic.
	for (int i = 0; i < filenames.size(); ++i) {
		// Reading training documents.
		CoNLLDocumentReader read(filenames[i]);
		//cout<<filenames[i]<<endl;

		while (true) {
			Document *doc = new Document();
			if (!read.ReadDocument(doc)) {
				delete doc;
				doc = NULL;
				break;
			}

			for (int j = 0; j < doc->sentences.size(); ++j) {
				sentences.push_back(doc->sentences[j]);
			}
			documents.push_back(doc);
			id2doc[doc->doc_id] = doc;
			//cout<<"Read document "<<doc->doc_id<<endl;
		}
	}
}

void Dataset::OutputCoNLL(string outputfile) {
	ofstream outf(outputfile.c_str(), ios::out);
	for (int d = 0; d < documents.size(); ++d) {
		outf<<"#begin document ("<<documents[d]->doc_id<<"); part 000"<<endl;
		for (int i = 0; i < documents[d]->sentences.size(); ++i) {
			Sentence *sent = documents[d]->sentences[i];
			for (int k = 0; k < sent->size(); ++k) {
				outf<<documents[d]->doc_id<<"\t0\t"<<k<<"\t"<<sent->words[k]<<"\t"<<sent->postags[k]<<"\t"<<sent->token_labels[k]<<"\t"<<sent->pred_token_labels[k]<<endl;
			}
			outf<<endl;
		}
		outf<<"#end document"<<endl;
	}
	outf.close();
}

void Dataset::LoadEntityCandidates(string inputfile) {
	ifstream infile(inputfile.c_str(), ios::in);
	string line;
	while (getline(infile, line)) {
		vector<string> fields;
		Utils::Split(line, '\t', fields);
		string docid = fields[1];

		if (id2doc.find(docid) == id2doc.end()) {
			//cout<<"skip "<<docid<<endl;
			continue;
		}

		int sentid = atoi(fields[2].c_str());
		int j = fields[3].find(',');
		int start = atoi(fields[3].substr(0,j).c_str());
		int end = atoi(fields[3].substr(j+1).c_str());

		vector<string> subfields;
		Utils::Split(fields[4], ' ', subfields);

		Mention *men = new Mention();
		men->index = docid + "#" + fields[2] + "#" + Utils::int2string(start) + "#" + Utils::int2string(end);
		men->start = start;
		men->end = end;
		men->pred_ner_type = fields[0];
		for (int i = 0; i < subfields.size(); ++i) {
			int j = subfields[i].find(':');
			string lb = subfields[i].substr(0, j);
			double v = atof(subfields[i].substr(j+1).c_str());
			men->labels.push_back(lb);
			men->values.push_back(v);
		}

		id2doc[docid]->sentences[sentid]->cand_entity_mentions.push_back(men);
	}
	infile.close();
}

void Dataset::LoadEventCandidates(string inputfile) {
	ifstream infile(inputfile.c_str(), ios::in);
	string line;
	while (getline(infile, line)) {
		vector<string> fields;
		Utils::Split(line, '\t', fields);
		string docid = fields[1];
		if (id2doc.find(docid) == id2doc.end()) {
			//cout<<"skip "<<docid<<endl;
			continue;
		}

		int sentid = atoi(fields[2].c_str());
		int j = fields[3].find(',');
		int start = atoi(fields[3].substr(0,j).c_str());
		int end = atoi(fields[3].substr(j+1).c_str());

		EventMention *men = new EventMention();
		men->index = docid + "#" + fields[2] + "#" + Utils::int2string(start) + "#" + Utils::int2string(end);
		men->start = start;
		men->end = end;

		id2doc[docid]->sentences[sentid]->cand_event_mentions.push_back(men);
	}
	infile.close();
}

void Dataset::LoadEventStructures(string inputfile) {
	ifstream infile(inputfile.c_str(), ios::in);
	string str;
	vector<string> lines;
	while (getline(infile, str)) {
		if (str == "") {
			int j = lines[0].find(' ');
			string index = lines[0].substr(0,j);
			string label = lines[0].substr(j+1);
			vector<string> fields;
			Utils::Split(index, '#', fields);
			string docid = fields[0].substr(2);

			if (id2doc.find(docid) == id2doc.end()) {
				//cout<<"skip "<<docid<<endl;
				continue;
			}

			int sentid = atoi(fields[1].c_str());
			int start = atoi(fields[2].c_str());
			int end = atoi(fields[3].c_str());

			Sentence *sent = id2doc[docid]->sentences[sentid];

			EventMention *evmen = new EventMention();
			evmen->index = index;
			evmen->start = start;
			evmen->end = end;
			evmen->gold_subtype = label;
			sent->SetMentionStr(evmen);

			for (int k = 1; k < lines.size(); ++k) {
				j = lines[k].find(' ');
				index = lines[k].substr(0,j);
				label = lines[k].substr(j+1);

				vector<string> fields;
				Utils::Split(index, '#', fields);
				start = atoi(fields[2].c_str());
				end = atoi(fields[3].c_str());

				int en_j = sent->findCandidateEntityMention(start, end);
				if (en_j < 0) {
					Mention *men = new Mention();
					men->index = index;
					men->start = start;
					men->end = end;
					men->gold_ner_type = fields[4];
					en_j = sent->cand_entity_mentions.size();
					sent->cand_entity_mentions.push_back(men);
				}

				if (label != "O")
					evmen->gold_arg_to_role[en_j] = label;
			}

			sent->cand_event_mentions.push_back(evmen);

			lines.clear();
		} else {
			lines.push_back(str);
		}

	}
	infile.close();
}

void Dataset::LoadCRFEntities(string inputfile) {
	ifstream infile(inputfile.c_str(), ios::in);
	string line;
	while (getline(infile, line)) {
		vector<string> fields;
		Utils::Split(line, '\t', fields);
		string docid = fields[1];
		if (id2doc.find(docid) == id2doc.end()) {
			//cout<<"skip "<<docid<<endl;
			continue;
		}

		int sentid = atoi(fields[2].c_str());
		int j = fields[3].find(',');
		int start = atoi(fields[3].substr(0,j).c_str());
		int end = atoi(fields[3].substr(j+1).c_str());

		Mention *men = new Mention();
		men->start = start;
		men->end = end;
		men->pred_ner_type = fields[0];
		vector<string> subfields;
		Utils::Split(fields[4], ' ', subfields);
		for (int i = 0; i < subfields.size(); ++i) {
			int j = subfields[i].find(':');
			string lb = subfields[i].substr(0, j);
			double v = atof(subfields[i].substr(j+1).c_str());
			men->labels.push_back(lb);
			men->values.push_back(v);
		}

		id2doc[docid]->sentences[sentid]->crf_entities.push_back(men);
	}
	infile.close();
}

void Dataset::LoadGoldEntities(string inputfile) {
	ifstream infile(inputfile.c_str(), ios::in);
	string line;
	while (getline(infile, line)) {
		vector<string> fields;
		Utils::Split(line, '\t', fields);
		string docid = fields[1];
		if (id2doc.find(docid) == id2doc.end()) {
			//cout<<"skip "<<docid<<endl;
			continue;
		}

		int sentid = atoi(fields[2].c_str());
		int j = fields[3].find(',');
		int start = atoi(fields[3].substr(0,j).c_str());
		int end = atoi(fields[3].substr(j+1).c_str());

		Mention *men = new Mention();
		men->index = docid + "#" + fields[2] + "#" + Utils::int2string(start) + "#" + Utils::int2string(end);
		men->start = start;
		men->end = end;
		men->gold_ner_type = fields[0];

		id2doc[docid]->sentences[sentid]->entity_mentions.push_back(men);
	}
	infile.close();
}

void Dataset::LoadNELLEntities(string inputfile) {
	ifstream infile(inputfile.c_str(), ios::in);
	string line;
	while (getline(infile, line)) {
		vector<string> fields;
		Utils::Split(line, '\t', fields);
		string docid = fields[1];
		if (id2doc.find(docid) == id2doc.end()) {
			//cout<<"skip "<<docid<<endl;
			continue;
		}

		int sentid = atoi(fields[2].c_str());
		int j = fields[3].find(',');
		int start = atoi(fields[3].substr(0,j).c_str());
		int end = atoi(fields[3].substr(j+1).c_str());

		Mention *men = new Mention();
		men->start = start;
		men->end = end;
		men->pred_ner_type = fields[0];

		id2doc[docid]->sentences[sentid]->nell_entities.push_back(men);
	}
	infile.close();
}

void Dataset::LoadStanfordEntities(string inputfile) {
	ifstream infile(inputfile.c_str(), ios::in);
	string line;
	while (getline(infile, line)) {
		vector<string> fields;
		Utils::Split(line, '\t', fields);
		string docid = fields[1];
		if (id2doc.find(docid) == id2doc.end()) {
			//cout<<"skip "<<docid<<endl;
			continue;
		}

		int sentid = atoi(fields[2].c_str());
		int j = fields[3].find(',');
		int start = atoi(fields[3].substr(0,j).c_str());
		int end = atoi(fields[3].substr(j+1).c_str());

		Mention *men = new Mention();
		men->start = start;
		men->end = end;
		men->pred_ner_type = fields[0];

		id2doc[docid]->sentences[sentid]->stanford_entities.push_back(men);
	}
	infile.close();
}

void Dataset::LoadDependencies(string dependencyfile) {
	if (dependencyfile == "") return;

	ifstream depfile(dependencyfile.c_str(), ios::in);
	if (!depfile) return;

	int sentid = 0;
	vector<string> lines;
	string str;
	while(getline(depfile, str))
	{
		if(str == "" || str == "\n" || str == "\r")
		{
			sentences[sentid]->buildDepGraph(lines);
			lines.clear();
			sentid++;
		}
		else
			lines.push_back(str);
	}
	if(str != "" && str != "\n" && str != "\r")
	{
		sentences[sentid]->buildDepGraph(lines);
	}
	depfile.close();
}
