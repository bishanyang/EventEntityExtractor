#include "Sentence.h"

Sentence::~Sentence() {
	vector<string>().swap(words);
	vector<string>().swap(postags);
	vector<string>().swap(lemmas);

	if (penntree != NULL) delete penntree;
	if (G != NULL) delete G;
	//if (dict != NULL) delete dict;
}

string Sentence::GetSpanText(int start, int end) {
	string str = "";
	for (int i = start; i <= end; ++i) {
		str += words[i] + " ";
	}
	return str.substr(0, str.size()-1);
}

string Sentence::GetContext(int start, int end) {
	int context_start = max(0, start - 3);
	int context_end = min(end + 3, (int)words.size() - 1);
	string str = "";
	for (int i = context_start; i < start; ++i) {
		str += words[i] + " ";
	}
	str += "[";
	for (int i = start; i <= end; ++i) {
		str += words[i] + " ";
	}
	str += "] ";

	for (int i = end + 1; i <= context_end; ++i) {
		str += words[i] + " ";
	}
	return str.substr(0, str.size()-1);
}

void Sentence::NormalizeWords() {
	for (int s = 0; s < words.size(); ++s) {
		string word = words[s];

		// normalize digits
		bool is_number = true;
		for (int i = 0; i < word.size(); ++i) {
			if (!(word[i] >= '0' && word[i] <= '9')) {
				is_number = false;
				break;
			}
		}
		if (is_number) {
			lemmas[s] = "NUMNUM";
		} else if (word.find('@') != string::npos && word.find(".com") != string::npos) {
			lemmas[s] = "@EMAIL";
		} else {
			bool has_alphabet = false;
			for (int i = 0; i < word.size(); ++i) {
				if ((word[i] >= 'a' && word[i] <= 'z') || (word[i] >= 'A' && word[i] <= 'Z')) {
					has_alphabet = true;
					break;
				}
			}
			// if no alphabet, normalize digit to '#'
			if (!has_alphabet) {
				for (int i = 0; i < word.size(); ++i) {
					if ((word[i] >= '0' && word[i] <= '9')) {
						lemmas[s][i] = '#';
					}
				}
			}
		}
	}
}

bool Sentence::buildDepGraph(vector<string> &lines)
{
	vector<pair<int, int> > dep_pairs;
	vector<string> dep_relations;

	//map< int, vector<string> > depmap;
    G = NULL;
    G = new DependencyGraph();

    int max_wid = -1;
	for(int i=0; i<lines.size(); i++) {
		if (lines[i] == "") continue;

		int sindex = lines[i].find("(");
		int eindex = lines[i].rfind(")");
		if (sindex < 0 || eindex < 0) continue;

		string dep = lines[i].substr(0, sindex);
		string pairs = lines[i].substr(sindex+1, eindex-sindex-1);

		vector<string> tmp;
		sindex = pairs.find(", ");
		if (sindex < 0) continue;

		tmp.push_back(pairs.substr(0, sindex));
		tmp.push_back(pairs.substr(sindex+2));
		assert(tmp.size() == 2);

		sindex = tmp[0].size()-1;
		while(sindex >= 0 && tmp[0][sindex] != '-') sindex--;
		int wid1 = atoi(tmp[0].substr(sindex+1).c_str()); //wordid-1, since id 0 refers to ROOT

		sindex = tmp[1].size()-1;
		while(sindex >= 0 && tmp[1][sindex] != '-') sindex--;
		int wid2 = atoi(tmp[1].substr(sindex+1).c_str());

		// excluding root dependencies
		if (wid1 == 0 || wid2 == 0) continue;

		wid1 = wid1 - 1;
		wid2 = wid2 - 1;
		if (wid1 > max_wid) {
			max_wid = wid1;
		}
		if (wid2 > max_wid) {
			max_wid = wid2;
		}

		dep_pairs.push_back(make_pair(wid1, wid2));
		dep_relations.push_back(dep);
	}

	if (max_wid+1 > (int)words.size() || (int)dep_relations.size() == 0) {
		delete G;
		return false;
	}

	if (!G->buildGraph(dep_pairs, dep_relations, words.size())) {
		delete G;
		return false;
	}

    //return G->buildGraph(depmap, words.size());
	return true;
}

bool Sentence::buildPennTree(vector<string> &lines)
{
	string text = "";
	for(int i=0; i<lines.size(); i++)
	{
		string str = lines[i];
		Utils::Trim(str);
		text += str;
	}
	penntree = new PennTree();
	penntree->ReadTree(text);

	return true;

}

bool Sentence::buildPennTree(string parse_tree)
{
	if (penntree) {
		delete penntree;
		penntree = NULL;
	}
	penntree = new PennTree();
	if (!penntree->ReadTree(parse_tree)) {
		delete penntree;
		penntree = NULL;
		return false;
	}
	if (penntree->leaves.size() != words.size()) return false;
	return true;
}


EventMention *Sentence::findEventMention(int start, int end) {
	for (int i = 0; i < event_mentions.size(); ++i) {
		if (event_mentions[i]->start == start && event_mentions[i]->end == end) {
			return event_mentions[i];
		}
	}
	return NULL;
}

int Sentence::findEntityMention(int start, int end) {
	for (int i = 0; i < entity_mentions.size(); ++i) {
		if (entity_mentions[i]->start == start && entity_mentions[i]->end == end) {
			return i;
		}
	}
	return -1;
}

EventMention *Sentence::findCandidateEventMention(int start, int end) {
	for (int i = 0; i < cand_event_mentions.size(); ++i) {
		if (cand_event_mentions[i]->start == start && cand_event_mentions[i]->end == end) {
			return cand_event_mentions[i];
		}
	}
	return NULL;
}

int Sentence::findCandidateEntityMention(int start, int end) {
	for (int i = 0; i < cand_entity_mentions.size(); ++i) {
		if (cand_entity_mentions[i]->start == start && cand_entity_mentions[i]->end == end) {
			return i;
		}
	}
	return -1;
}

void Sentence::genBasicFeatures(int cur, vector<string> &features) {
	string word = words[cur];

	// unigram
	//features.push_back(words[cur]);
	//if (lemmas[cur] != words[cur])
	features.push_back(lemmas[cur]);
	if (lemmas[cur] == "NUMNUM") features.push_back(words[cur]);

	string postag = "";
	if (postags.size() > 0) {
		postag = postags[cur];
		features.push_back(postag);
	}

	bool all_capitalize = true;
	for (int i = 0; i < word.size(); ++i) {
		if (!(word[0] >= 'A' && word[0] <= 'Z')) {
			all_capitalize = false;
			break;
		}
	}
	if (all_capitalize) {
		features.push_back("ALL_CAPITALIZE");
	} else if (word[0] >= 'A' && word[0] <= 'Z') {
		features.push_back("IS_CAPITALIZE");
	}

}

void Sentence::genCRFWindowFeatures(int offset, int cur, vector<string> &features) {
	if (offset == 0 || cur < 0 || cur >= (int)words.size()) return;
	string prefix = Utils::int2string(offset);

	 int index = cur + offset;
	 if (index >= 0 && index < (int)words.size()) {
		 //features.push_back(prefix+"_"+words[index]);
		 //if (words[index] != lemmas[index])
		 features.push_back(prefix+"_"+lemmas[index]);

		 string postag = "";
		 if (postags.size() > 0) {
			 postag = postags[index];
			 features.push_back(prefix+"_"+postag);
		 }

		 string word = words[index];
		 bool all_capitalize = true;
		 for (int i = 0; i < word.size(); ++i) {
			 if (!(word[0] >= 'A' && word[0] <= 'Z')) {
				 all_capitalize = false;
			 }
		 }
		 if (all_capitalize) {
			 features.push_back(prefix+"_"+"ALL_CAPITALIZE");
		 } else if (word[0] >= 'A' && word[0] <= 'Z') {
			 features.push_back(prefix+"_"+"IS_CAPITALIZE");
		 }
	 }
}

void Sentence::genEmbeddingFeatures(int start, int end,
		map<string, int> &lookup_table,
		vector<vector<float> > &word_vec,
		vector<double>& feature_values)
{
	 if (lookup_table.size() == 0 || word_vec.size() == 0) return;

	 feature_values.resize(word_vec[0].size(), 0.0);

	 int c = 0;
	 for (int cur = start; cur <= end; ++cur) {
		 //string word = Utils::toLower(words[cur]);
		 //string lemma = Utils::toLower(lemmas[cur]);
		 string word = words[cur];
		 string lemma = lemmas[cur];

		 if (lookup_table.find(word) != lookup_table.end()) {
			 int id = lookup_table[word];
			 for (int i = 0; i < word_vec[id].size(); ++i) {
				 feature_values[i] += word_vec[id][i];
			 }
			 c++;
		 } else if (lookup_table.find(lemma) != lookup_table.end()) {
			 int id = lookup_table[lemma];
			 for (int i = 0; i < word_vec[id].size(); ++i) {
				 feature_values[i] += word_vec[id][i];
			 }
			 c++;
		 }
	 }
	 // average word_vec
	 if (c > 0) {
		 for (int i = 0; i < feature_values.size(); ++i) {
			 feature_values[i] = (double)feature_values[i]/c;
		 }
	 }
}

void Sentence::genTriggerWindowFeatures(int offset, int cur, map<string, double> &features) {
	if (offset == 0 || cur < 0 || cur >= (int)words.size()) return;
	string prefix = Utils::int2string(offset);

	 int index = cur + offset;
	 if (index >= 0 && index < (int)words.size()) {
		 //features[prefix+"_"+words[index]] = 1.0;
		 features[prefix+"_"+lemmas[index]] = 1.0;
	 }
}

void Sentence::genTriggerFeatures(EventMention *event, map<string, double> &features) {
	string trigger_str = "";
	string lemma_str = "";
	for (int i = event->start; i <= event->end; ++i) {
		trigger_str += words[i] + " ";
		lemma_str += lemmas[i] + " ";
	}
	trigger_str = trigger_str.substr(0, trigger_str.size()-1);
	lemma_str = lemma_str.substr(0, lemma_str.size()-1);
	features["Lex_"+lemma_str] = 1.0;

	if (event->start != event->end) {
		for (int i = event->start; i <= event->end; ++i) {
			features["Token_"+lemmas[i] + "_"+postags[i]] = 1.0;
		}
	}

	if (feature_dict->event_priors.find(lemma_str) != feature_dict->event_priors.end()) {
		map<string, double> priors = feature_dict->event_priors[lemma_str];
		for (map<string, double>::iterator it = priors.begin(); it != priors.end(); ++it) {
			if (it->second > 0.5) {
				features["Prior_"+it->first] = it->second;
			}
		}
	}

	vector<string> frames = feature_dict->getFrameByLemma(lemma_str, postags[event->start]);
	for (int i = 0; i < frames.size(); ++i) {
		features["Frame_"+frames[i]] = 1.0;
	}

	// context features
	genTriggerWindowFeatures(-1, event->start, features);
	genTriggerWindowFeatures(1, event->end, features);

	// trigger seeds
//	if (dict->event_priors.find(lemma_str) != dict->event_priors.end()) {
//	    for subtype, seeds in subtype_seeds.items():
//	        wordnet_dict.gen_dict_sim_features(subtype, seeds, event_word, ev_pos, feature_vecs)
//	}

	string verb = feature_dict->getVerbForm(lemma_str);
	if (verb != "") features["VerbForm_" + verb] = 1.0;

	if (postags[event->start].size() > 3 && postags[event->start].substr(0,3) == "PRP") {
		features["isPronoun"] = 1.0;
	}

	if (event->start > 0 && event->end < words.size()-1) {
		string lstr = words[event->start-1];
		string rstr = words[event->end+1];
		if ((lstr == "\"" || lstr == "``" || lstr == "`") && (rstr == "\"" || rstr == "``" || rstr == "`"))
			features["insideQuote"] = 1.0;
	}

	// Dependency features
	for (int i = event->start; i <= event->end; ++i) {
		vector<string> neighbors;
		G->getNeighbors(i, neighbors);
		for (int j = 0; j < neighbors.size(); ++j) {
	        int k = neighbors[j].find('/');
	        string type = neighbors[j].substr(0,k);
	        int v = atoi(neighbors[j].substr(k+1).c_str());

	        features[type] = 1.0;
	        if (feature_dict->isTime(lemmas[v])) {
	        	features[type + "_TIME"] = 1.0;
	        } else if (feature_dict->isJobTitle(lemmas[v])) {
	        	features[type + "_JOBTITLE"] = 1.0;
	        }

	        features[type+"_"+lemmas[v]] = 1.0;
		}
	}

	features["bias"] = 1.0;

}

void Sentence::genTriggerPairFeatures(EventMention *m1, EventMention *m2, map<string, double> &features) {
	for (int i = m1->start; i <= m2->end; ++i) {
		vector<string> neighbors;
		G->getNeighbors(i, neighbors);
		for (int j = 0; j < neighbors.size(); ++j) {
			int k = neighbors[j].find('/');
			string type = neighbors[j].substr(0,k);
			int v = atoi(neighbors[j].substr(k+1).c_str());
			if (type.find("conj") != string::npos && v >= m2->start && v <= m2->end) {
				features["Dep_" + type] = 1.0;
			}
		}
	}

	// Frame similarity
	vector<string> frames_1 = feature_dict->getFrameByLemma(m1->menstr, postags[m1->start]);
	vector<string> frames_2 = feature_dict->getFrameByLemma(m2->menstr, postags[m2->start]);
	bool hit = false;
	for (int i = 0; i < frames_1.size(); ++i) {
		for (int j = 0; j < frames_2.size(); ++j) {
			if (frames_1[i] == frames_2[j]) {
				features["sim_frame"] = 1.0;
				hit = true;
				break;
			}
		}
		if (hit) break;
	}

	// lexical similarity
	hit = false;
	for (int i = m1->start; i <= m1->end; ++i) {
		for (int j = m2->start; j <= m2->end; ++j) {
			if (lemmas[i] == lemmas[j]) {
				features["sim_lex"] = 1.0;
				hit = true;
				break;
			}
		}
		if (hit) break;
	}

	features["bias"] = 1.0;
}

void Sentence::genRelationContextFeatures(int window, int s1, int e1, int s2, int e2, map<string, double> &features) {
    // only unigram from now
    string key = "";
    if (s2 > e1 && s2 - e1 <= window) {//predidcate, context, arg
    	key = "R_0#";
    	for (int i = e1+1; i < s2; ++i) {
            key += words[i] + " ";
    	}
    	key = key.substr(0, key.size()-1);
    	features[key] = 1.0;
    } else if (s1 > e2 && s1 - e2 <= window) {
    	key = "R_1#";
		for (int i = e2+1; i < s1; ++i) {
			key += words[i] + " ";
		}
		key = key.substr(0, key.size()-1);
		features[key] = 1.0;
    }
}

void Sentence::genTriggerArgFeatures(EventMention *event, vector<int> entity_flags, vector<map<string, double> > &allfeatures) {
	string trigger_lemma_str = "";
	for (int i = event->start; i <= event->end; ++i) {
		trigger_lemma_str += lemmas[i] + " ";
	}
	trigger_lemma_str = trigger_lemma_str.substr(0, trigger_lemma_str.size()-1);

	for (int i = 0; i < cand_entity_mentions.size(); ++i) {
		map<string, double> features;
		Mention *men = cand_entity_mentions[i];

		string lemma_str = "";
		for (int i = men->start; i <= men->end; ++i) {
			lemma_str += lemmas[i] + " ";
		}
		lemma_str = lemma_str.substr(0, lemma_str.size()-1);
		features["Arg_"+lemma_str] = 1.0;

		if (men->start != men->end) {
			for (int i = men->start; i <= men->end; ++i) {
				features["Token_"+lemmas[i]] = 1.0;
			}
		}

		features["R_pred_" +trigger_lemma_str] = 1.0;

		features["R_NER_" + men->pred_ner_type] = men->get_confidence(men->pred_ner_type);

		features["R_pred_NER" + trigger_lemma_str + "_" + men->pred_ner_type] = men->get_confidence(men->pred_ner_type);

		// positiion
		if (men->start == event->start && men->end == event->end) {
			features["R_Equal"] = 1.0;
		} else if (event->end < men->start) {
			features["R_after_trigger"] = 1.0;
			int k = event->end+1;
			for (; k < men->start; ++k) {
				if (entity_flags[k] != 0) break;
			}
			if (k == men->start)
				features["R_"+lemma_str+"_RIGHT"] = 1.0;
		} else if (men->end < event->start) {
			features["R_before_trigger"] = 1.0;
			int k = men->end+1;
			for (; k < event->start; ++k) {
				if (entity_flags[k] != 0) break;
			}
			if (k == event->start)
				features["R_"+lemma_str+"_LEFT"] = 1.0;
		} else if (event->start >= men->start && event->end <= men->end) {
			features["R_arg_contain_pred"] = 1.0;
		} else if (men->start >= event->start && men->end <= event->end) {
			features["R_pred_contain_arg"] = 1.0;
		}

		// context features
		genRelationContextFeatures(4, event->start, event->end, men->start, men->end, features);

		// same clause
		if (!penntree->inClause(Span(event->start, event->end), Span(men->start, men->end))) {
			features["R_not_same_clause"] = 1.0;
		}

		string shortest_path = G->getShortestPath(Span(event->start, event->end), Span(men->start, men->end));

		vector<string> splits;
		Utils::Split(shortest_path, ',', splits);
		if (splits.size() >= 3) {
			// s, r, ..., r, t
			int j = 1;
			vector<string> dep_words;
			vector<string> dep_edges;
			while (j < splits.size()-1) {
				dep_edges.push_back(splits[j]);

				int v = atoi(splits[j+1].c_str());
				string word = lemmas[v];

				if (feature_dict->isTime(word))
					word = "TIME";
				if (feature_dict->isJobTitle(word))
					word = "JOBTITLE";
				dep_words.push_back(word);

				j += 2;
			}

			if (dep_edges.size() == 1) {
				features["R_"+dep_edges[0]] = 1.0;
				//feature_vecs["R_"+dep_words[0]+"_"+dep_edges[0]+"_"+dep_words[1]] = 1
			} else if (dep_edges.size() <= 3) {
				string dep_str = "";
				for (int j = 0; j < dep_edges.size(); ++j) dep_str += dep_edges[j] + "_";
				dep_str = dep_str.substr(0, dep_str.size()-1);
				features["R_"+dep_str] = 1.0;

				dep_str = dep_edges[0] + "_*_" + dep_edges[dep_edges.size()-1];
				features["R_"+dep_str] = 1.0;
			}
		}

		features["bias"] = 1.0;
		allfeatures.push_back(features);
	}
}

string Sentence::getCRFLabel(Mention *men, double &prob) {
	for (int i = 0; i < crf_entities.size(); ++i) {
		if (men->equals(crf_entities[i])) {
			string label = crf_entities[i]->pred_ner_type;
			prob = crf_entities[i]->get_confidence(label);
			return label;
		}
	}
	prob = 1.0;
	return "O";
}

string Sentence::getNELLLabel(Mention *men) {
	for (int i = 0; i < nell_entities.size(); ++i) {
		if (men->equals(nell_entities[i])) {
			return nell_entities[i]->pred_ner_type;
		}
	}
	return "O";
}

string Sentence::getStanfordNERLabel(Mention *men) {
	for (int i = 0; i < stanford_entities.size(); ++i) {
		if (men->equals(stanford_entities[i])) {
			return stanford_entities[i]->pred_ner_type;
		}
	}
	return "O";
}

void Sentence::genArgAttributeFeatures(vector<map<string, double> > &allfeatures) {
	for (int i = 0; i < cand_entity_mentions.size(); ++i) {
		map<string, double> features;
		Mention *men = cand_entity_mentions[i];
		string lemma_str = "";
		for (int i = men->start; i <= men->end; ++i) {
			lemma_str += lemmas[i] + " ";
		}
		lemma_str = lemma_str.substr(0, lemma_str.size()-1);
		features["Arg_"+lemma_str] = 1.0;

		if (men->start != men->end) {
			for (int i = men->start; i <= men->end; ++i) {
				features["Token_"+lemmas[i]] = 1.0;
			}
		}

		double prob = 0.0;
		string label = getCRFLabel(men, prob);

		features["P_CRF_" + label] = prob;

		label = getStanfordNERLabel(men);
		if (label != "O") {
			features["N_Stanford_" + label] = 1.0;
		}

		label = getNELLLabel(men);
		if (label != "O") {
			features["N_NELL_" + label] = 1.0;
		}

		features["bias"] = 1.0;

		allfeatures.push_back(features);
	}

}
