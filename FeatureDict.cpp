/*
 * FeatureDict.cpp
 *
 *  Created on: Apr 20, 2016
 *      Author: bishan
 */

#include "FeatureDict.h"

FeatureDict::FeatureDict() {
	// TODO Auto-generated constructor stub

}

FeatureDict::~FeatureDict() {
	// TODO Auto-generated destructor stub
}


void FeatureDict::InitializeDict(string feature_content) {
	vector<string> lines;
	Utils::Split(feature_content, '\n', lines);

	for (int i = 0; i < lines.size(); ++i) {
		vector<string> fields;
		Utils::Split(lines[i], '\t', fields);
		if (fields.size() != 3) continue;

		if (fields[0] == "FrameNet") {
			vector<string> frames;
			Utils::Split(fields[2], ',', frames);
			verb_frames[fields[1]] = frames;
		} else if (fields[0] == "VerbNet") {
			vector<string> frames;
			Utils::Split(fields[2], ',', frames);
			verb_classes[fields[1]] = frames;
		} else if (fields[0] == "NOMLEX") {
			nom_to_verb[fields[1]] = fields[2].c_str();
		} else if (fields[0] == "TIME") {
			times.insert(fields[1]);
		} else if (fields[0] == "JOBTITLE") {
			jobtitles.insert(fields[1]);
		} else if (fields[0] == "EventPrior") {
			map<string, double> probs;
			vector<string> subfields;
			Utils::Split(fields[2], ' ', subfields);
			for (int i = 0; i < subfields.size(); ++i) {
				int j = subfields[i].find(":");
				string lb = subfields[i].substr(0, j);
				double v = atof(subfields[i].substr(j+1).c_str());
				probs[lb] = v;
			}
			event_priors[fields[1]] = probs;
		} else if (fields[0] == "Gazetteer") {
			vector<string> words;
			Utils::Split(fields[2], ' ', words);
			if (words.size() == 1) {
			  string word = words[0];
			  word = Utils::toLower(word);
			  gazetteer_unigrams[word] = fields[1];
			} else if (words.size() == 2) {
			  string word = words[0]+"_"+words[1];
			  word = Utils::toLower(word);
			  gazetteer_bigrams[word] = fields[1];
			}
		}
	}
}

void FeatureDict::LoadEmbeddings(string embedding_content) {
	word_lookup_table.clear();
	word_embeddings.clear();

	vector<string> lines;
	Utils::Split(embedding_content, '\n', lines);

	for (int i = 0; i < lines.size(); ++i) {
		string str = lines[i];
		int index = str.find('\t');
		string word = str.substr(0, index);
		string values = str.substr(index+1);
		vector<string> fields;
		Utils::Split(values, ' ', fields);
		vector<float> vec(fields.size());
		for (int i = 0; i <fields.size(); ++i) vec[i] = atof(fields[i].c_str());
		int wid = word_lookup_table.size();
		word_lookup_table[word] = wid;
		word_embeddings.push_back(vec);
	}
}

vector<string> FeatureDict::getFrameByLemma(string lemma, string pos) {
	vector<string> frames;

	string framepos = "";
	if (pos[0] == 'V') framepos = "v";
	else if (pos[0] == 'N') framepos = "n";
	else if (pos[0] == 'J') framepos = "a";
	else if (pos.substr(0,2) == "RB") framepos = "adv";
	else if (pos == "CD") framepos = "num";
	else if (pos == "CC") framepos = "c";
	else if (pos == "UH") framepos = "intj";
	else if (pos == "IN") framepos = "prep";

	if (framepos == "") return frames;
	string key = lemma + "." + framepos;
	if (verb_frames.find(key) != verb_frames.end()) return verb_frames[key];
	return frames;
}

string FeatureDict::getVerbForm(string lemma) {
	if (nom_to_verb.find(lemma) != nom_to_verb.end()) return nom_to_verb[lemma];
	return "";
}

bool FeatureDict::isTime(string lemma) {
	if(times.find(lemma) != times.end())
		return true;
	else
		return false;
}

bool FeatureDict::isJobTitle(string lemma) {
	if(jobtitles.find(lemma) != jobtitles.end())
		return true;
	else
		return false;
}
