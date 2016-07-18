#pragma once
#include <iostream>
#include <vector>
#include <queue>
#include <fstream>
#include <set>
#include <string>
#include <algorithm>
#include <assert.h>
#include <map>

#include "Utils.h"
#include "./Parsing/PennTree.h"
#include "./Parsing/GraphNode.h"
#include "FeatureDict.h"

using namespace std;

class Mention {
public:
	Mention() : mid(-1), sentid(-1), start(-1), end(-1), coref_id(-1) {
		gold_ner_type = "O";
		pred_ner_type = "O";
	}
	~Mention() {}
public:
	string index;

	int mid;
	int sentid;
	int start;
	int end;

	string gold_ner_type;
	string pred_ner_type;

	int coref_id;

	vector<string> labels;
	vector<double> values;
public:
	int len() {if(start == -1 || end == -1) return 0; else return end-start+1;}
	double get_confidence(string label) {
		for (int i = 0; i < labels.size(); ++i) {
			if (labels[i] == label) return values[i];
		}
		return 0.0;
	}
	bool equals(Mention *m) {
		if (m->start == start && m->end == end) return true;
		return false;
	}
};

class EventMention {
public:
	EventMention() : mid(-1), sentid(-1), start(-1), end(-1) {
		gold_subtype = "O";
		pred_subtype = "O";
	}

	~EventMention() {}

	int len() {if(start == -1 || end == -1) return 0; else return end-start+1;}

public:
	string index;

	int mid;
	int sentid;
	int start;
	int end;

	string menstr;

	string gold_subtype;
	string pred_subtype;

	map<int, string> gold_arg_to_role;
	map<int, string> pred_arg_to_role;
public:
	string GetGoldArgRole(int i) {
		if (gold_arg_to_role.find(i) == gold_arg_to_role.end()) return "O";
		return gold_arg_to_role[i];
	}
};

class Sentence
{
public:
	Sentence() {sent_id = 0; penntree = NULL; G = NULL; feature_dict = NULL;}
	~Sentence();
public:
	string docid;
	int sent_id;
	string sent; //original sentence

	vector<string> words;
	vector<string> lemmas;
	vector<string> postags;

	vector<string> token_labels;
	vector<string> pred_token_labels;

	vector<Mention*> entity_mentions;
	vector<EventMention*> event_mentions;

	vector<Mention*> cand_entity_mentions;
	vector<EventMention*> cand_event_mentions;

	vector<Mention*> crf_entities;
	vector<Mention*> stanford_entities;
	vector<Mention*> nell_entities;

	FeatureDict *feature_dict;

public:
	PennTree* penntree;
	DependencyGraph* G;

public:
	void SetSentenceID(int i) {sent_id = i;}
	int SentenceID() {return sent_id;}

	int size() {return words.size();}

	string toString() {
		string str = "";
		for(int i=0; i<words.size(); i++)
			str += words[i]+" ";
		return str;
	}

	void SetMentionStr(EventMention *m) {
		string lemma_str = "";
		for (int i = m->start; i <= m->end; ++i) {
			lemma_str += lemmas[i] + " ";
		}
		m->menstr = lemma_str.substr(0, lemma_str.size()-1);
	}

	void NormalizeWords();
	void SetFeatureDict(FeatureDict *pdict) { feature_dict = pdict;}

	string getCRFLabel(Mention *men, double &prob);
	string getNELLLabel(Mention *men);
	string getStanfordNERLabel(Mention *men);

	bool buildPennTree(string parse_tree);
	bool buildDepGraph(vector<string> &lines);
	bool buildPennTree(vector<string> &lines);

	void genBasicFeatures(int cur, vector<string> &features);
	void genCRFWindowFeatures(int offset, int cur, vector<string> &features);
	void genEmbeddingFeatures(int start, int end,
			map<string, int> &lookup_table,
			vector<vector<float> > &word_vec,
			vector<double>& feature_values);

	void genRelationContextFeatures(int window, int s1, int e1, int s2, int e2, map<string, double> &features);

	bool containEvent() {
		for (int i = 0; i < cand_event_mentions.size(); ++i) {
			if (cand_event_mentions[i]->gold_subtype != "O")
				return true;
		}
		return false;
	}
	string GetContext(int start, int end);
	string GetSpanText(int start, int end);

	EventMention *findEventMention(int start, int end);
	int findEntityMention(int start, int end);

	EventMention *findCandidateEventMention(int start, int end);
	int findCandidateEntityMention(int start, int end);

	void genTriggerWindowFeatures(int offset, int cur, map<string, double> &features);

	void genTriggerFeatures(EventMention *event, map<string, double> &features);
	void genTriggerArgFeatures(EventMention *event, vector<int> entity_flags, vector<map<string, double> > &features);
	void genArgAttributeFeatures(vector<map<string, double> > &features);

	void genTriggerPairFeatures(EventMention *m1, EventMention *m2, map<string, double> &features);

};


