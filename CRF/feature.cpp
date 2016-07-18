#include "common.h"
#include "feature_index.h"
#include "node.h"
#include "tree_tagger.h"
#include "tagger.h"

namespace CRFPP {

void FeatureIndex::SetY(map<string, int> labels) {
  	y_.clear();

  	for (map<string, int>::iterator it = labels.begin(); it != labels.end(); ++it) {
  		lb2i[it->first] = it->second;
  		i2lb[it->second] = it->first;
  	}
  	for (int i = 0; i < i2lb.size(); ++i) {
  		y_.push_back(i2lb[i].c_str());
  	}
}

void FeatureIndex::SetArgY(map<string, int> labels) {
	arg_y_.clear();

	for (map<string, int>::iterator it = labels.begin(); it != labels.end(); ++it) {
		arglb2i[it->first] = it->second;
		argi2lb[it->second] = it->first;
	}
	for (int i = 0; i < argi2lb.size(); ++i) {
		arg_y_.push_back(argi2lb[i].c_str());
	}
}

void FeatureIndex::SetArgAttrY(map<string, int> labels) {
	arg_attr_y_.clear();

	for (map<string, int>::iterator it = labels.begin(); it != labels.end(); ++it) {
		argattrlb2i[it->first] = it->second;
		argattri2lb[it->second] = it->first;
	}
	for (int i = 0; i < argattri2lb.size(); ++i) {
		arg_attr_y_.push_back(argattri2lb[i].c_str());
	}
}

void FeatureIndex::SetTransY(map<string, set<string> > translabels) {
	int ysize = y_.size();
	int argysize = arg_y_.size();
	int idx = 0;
	for (int y1 = 0; y1 < ysize; ++y1) {
		for (int y2 = 0; y2 < argysize; ++y2) {
			int label = y1 * argysize + y2;
			string y1str = y_[y1];
			string y2str = arg_y_[y2];

			if (translabels.find(y1str) == translabels.end()) continue;
			if (translabels[y1str].find(y2str) == translabels[y1str].end()) continue;

			transition_lb2i[label] = idx;
			transition_i2lb[idx] = label;
			idx++;
		}
	}
}

void FeatureIndex::LoadLabels(set<string> &labels) {
	y_.clear();

	// make sure O corresponds to index 0
	lb2i["O"] = 0;
	i2lb[0] = "O";
	y_.push_back("O");

	for (set<string>::iterator it = labels.begin(); it != labels.end(); ++it) {
		string label = *it;
		if (label == "O") continue;

		lb2i[label] = y_.size();
		i2lb[y_.size()] = label;
		y_.push_back(this->strdup(label.c_str()));
	}


	set<string> trans_labels;
	// all possible transitions
	for (int i = 0; i < i2lb.size(); ++i) {
		for (int j = 0; j < i2lb.size(); ++j) {
			string y1 = i2lb[i];
			string y2 = i2lb[j];
			trans_labels.insert(y1+"_"+y2);
		}
	}

	SetTransLabels(trans_labels);

	cout<<"collect "<<labels.size()<<" labels"<<endl;
	cout<<"collect "<<trans_labels.size()<<" trans labels"<<endl;
}


void FeatureIndex::SetLabels(set<string> labels) {
  	y_.clear();

  	// make sure O corresponds to index 0
  	lb2i["O"] = 0;
  	i2lb[0] = "O";
  	y_.push_back("O");

  	for (set<string>::iterator it = labels.begin(); it != labels.end(); ++it) {
  		string label = *it;
  		if (label == "O") continue;

  		lb2i[label] = y_.size();
  		i2lb[y_.size()] = label;
  		y_.push_back(this->strdup(label.c_str()));
  	}

  	// copy arg_y
  	for (int i = 0; i < y_.size(); ++i) {
  		arg_y_.push_back(y_[i]);
  		arglb2i[y_[i]] = lb2i[y_[i]];
  		argi2lb[i] = i2lb[i];
  	}
}

void FeatureIndex::SetTransLabels(set<string> translabels) {
	transition_lb2i.clear();
	transition_i2lb.clear();

	int ysize = y_.size();
	int idx = 0;
  	for (set<string>::iterator it = translabels.begin(); it != translabels.end(); ++it) {
  		string label = *it;
  		int i = label.find('_');
  		string y1 = label.substr(0, i);
  		string y2 = label.substr(i+1);
  		int ilabel = lb2i[y1]*ysize+lb2i[y2];
  		transition_lb2i[ilabel] = idx;
  		transition_i2lb[idx] = ilabel;
  		idx++;
  	}
}

bool FeatureIndex::buildTreeTagger(TreeTagger *tagger) {
    size_t fid = tagger->feature_id();
    size_t fvid = tagger->feature_value_id();

    unsigned short thread_id = tagger->thread_id();

    treenode_freelist_[thread_id].free();
    treepath_freelist_[thread_id].free();

    // load predicate nodes
    int *binary_f = feature_cache_[fid++];
    int *float_f = feature_cache_[fid++];
    double *fv = feature_value_cache_[fvid++];
    for (int i = 0; i < ysize(); ++i) {
		TreeTaggerNode *n = treenode_freelist_[thread_id].alloc();
		n->clear();
		n->x = 0;
		n->y = i;
		n->binary_fea_vector = binary_f;
		n->float_fea_vector = float_f;
	    n->float_fv_vector = fv;

	    n->children_alpha.resize(tagger->arguments.size(), 0.0);
	    n->children_beta.resize(tagger->arguments.size(), 0.0);
	    n->children_path.resize(tagger->arguments.size());
		tagger->predicate_nodes[i] = n;
    }

    // load relations
    for (int i = 0; i < tagger->arguments.size(); ++i) {
       	binary_f = feature_cache_[fid++];
       	float_f = feature_cache_[fid++];
       	fv = feature_value_cache_[fvid++];
       	for (int cy = 0; cy < argysize(); ++cy) {
       		TreeTaggerNode *n = treenode_freelist_[thread_id].alloc();
			n->clear();
			n->x = i;
			n->y = cy;
			n->binary_fea_vector = binary_f;
			n->float_fea_vector = float_f;
			n->float_fv_vector = fv;

			n->children_alpha.resize(1, 0.0);
			n->children_beta.resize(1, 0.0);
			n->children_path.resize(1);
			tagger->argument_role_nodes[i][cy] = n;
       	}
    }

    int *edgef = feature_cache_[fid++];
    int *edgeff = feature_cache_[fid++];
    double *edgefv = feature_value_cache_[fvid++];
    for (int py = 0; py < ysize(); ++py) {
    	for (int i = 0; i < tagger->arguments.size(); ++i) {
			for (int cy = 0; cy < argysize(); ++cy) {
				if (py == 0 && cy != 0) continue;
				// check event-role compatibility
				if (py != 0 && event_to_roles[py].find(cy) == event_to_roles[py].end()) continue;

				TreeTaggerPath *p = treepath_freelist_[thread_id].alloc();
				p->clear();
				p->binary_fea_vector = edgef;
				p->float_fea_vector = edgeff;
				p->float_fv_vector = edgefv;
				p->add(tagger->predicate_nodes[py], i, py*argysize()+cy, tagger->argument_role_nodes[i][cy]);
			}
		}
    }

    // load argument nodes
	for (int i = 0; i < tagger->arguments.size(); ++i) {
		binary_f = feature_cache_[fid++];
		float_f = feature_cache_[fid++];
		fv = feature_value_cache_[fvid++];
		for (int ay = 0; ay < argattrsize(); ++ay) {
			TreeTaggerNode *n = treenode_freelist_[thread_id].alloc();
			n->clear();
			n->x = i;
			n->y = ay;
			n->binary_fea_vector = binary_f;
			n->float_fea_vector = float_f;
			n->float_fv_vector = fv;
			tagger->argument_attr_nodes[i][ay] = n;
		}
	}

	edgef = feature_cache_[fid++];
	edgeff = feature_cache_[fid++];
	edgefv = feature_value_cache_[fvid++];
	for (int i = 0; i < tagger->arguments.size(); ++i) {
		for (int cy = 0; cy < argysize(); ++cy) {
			for (int ay = 0; ay < argattrsize(); ++ay) {
				// check role-attr compatibility
				//if (cy == 0 && ay != 0) continue;
				if (cy != 0 && role_to_ners[cy].find(ay) == role_to_ners[cy].end()) continue;

				TreeTaggerPath *p = treepath_freelist_[thread_id].alloc();
				p->clear();
				p->binary_fea_vector = edgef;
				p->float_fea_vector = edgeff;
				p->float_fv_vector = edgefv;
				p->add(tagger->argument_role_nodes[i][cy], 0, cy*argattrsize()+ay, tagger->argument_attr_nodes[i][ay]);
			}
		}
	}


    return true;
}

void FeatureIndex::buildEventTreeFeatures(TreeTagger *tagger,
		EventMention *event,
		Sentence *sent,
		bool train) {
	int id;
    std::vector <int> binary_features;
    vector<int> float_features;
    vector<double> float_values;

    tagger->set_feature_id(feature_cache_.size());
    tagger->set_feature_value_id(feature_value_cache_.size());

    map<string, double> predicate_features;
    sent->genTriggerFeatures(event, predicate_features);

    // predicate features
    for(map<string, double>::iterator it = predicate_features.begin(); it != predicate_features.end(); ++it) {
		string fv = "P_"+it->first;
		if (train) {
			id = addfID(fv.c_str(), ysize());
		} else {
			id = getfID(fv.c_str());
		}
		if (id != -1) {
			if (it->first[0] != 'P') {
				binary_features.push_back(id);
			} else {
				float_features.push_back(id);
				float_values.push_back(it->second);
			}
		}
	}

	// embedding features
	if (word_lookup_table.size() > 0 && word_lookup_table.size() > 0) {
		vector<double> embedding;
		sent->genEmbeddingFeatures(event->start, event->end, word_lookup_table, word_embeddings, embedding);
		for (int k = 0; k < word_vec_size; ++k) {
			string fv = "P_Embedding_"+Utils::int2string(k);
			if (train) {
				id = addfID(fv.c_str(), ysize());
			} else {
				id = getfID(fv.c_str());
			}
			if (id != -1) {
				float_features.push_back(id);
				float_values.push_back(embedding[k]);
			}
		}
	}

	feature_cache_.add(binary_features);
	feature_cache_.add(float_features);
	feature_value_cache_.add(float_values);
	binary_features.clear();
	float_features.clear();
	float_values.clear();


	// relation features
	vector<int> entity_flags(sent->size(), 0);
	for (int i = 0; i < sent->cand_entity_mentions.size(); ++i) {
		for (int j = sent->cand_entity_mentions[i]->start; j <= sent->cand_entity_mentions[i]->end; ++j) {
			entity_flags[j] = 1;
		}
	}
	vector<map<string, double> > relation_features;
	sent->genTriggerArgFeatures(event, entity_flags, relation_features);

	for (size_t i = 0; i < relation_features.size(); ++i) {
		for(map<string, double>::iterator it = relation_features[i].begin(); it != relation_features[i].end(); ++it) {
			string fv = "R_"+it->first;
			if (train) {
				id = addfID(fv.c_str(), argysize());
			} else {
				id = getfID(fv.c_str());
			}
			if (id != -1) {
				binary_features.push_back(id);
			}
		}
		feature_cache_.add(binary_features);
		feature_cache_.add(float_features);
		feature_value_cache_.add(float_values);
		binary_features.clear();
		float_features.clear();
		float_values.clear();
	}

	//path feature
	string edgefstr = "R_edge";
	if (train) {
		id = addfID(edgefstr.c_str(), ysize()*argysize());
	} else {
		id = getfID(edgefstr.c_str());
	}
	if (id != -1) {
		binary_features.push_back(id);
	}
	feature_cache_.add(binary_features);
	feature_cache_.add(float_features);
	feature_value_cache_.add(float_values);
	binary_features.clear();
	float_features.clear();
	float_values.clear();


	// argument attribute features
	vector<map<string, double> > attribute_features;
	sent->genArgAttributeFeatures(attribute_features);

	for (size_t i = 0; i < attribute_features.size(); ++i) {
		for(map<string, double>::iterator it = attribute_features[i].begin(); it != attribute_features[i].end(); ++it) {
			string fv = "A_"+it->first;
			if (train) {
				id = addfID(fv.c_str(), argattrsize());
			} else {
				id = getfID(fv.c_str());
			}
			if (id != -1) {
				if (it->first[0] != 'P') {
					binary_features.push_back(id);
				} else {
					float_features.push_back(id);
					float_values.push_back(it->second);
				}
			}
		}
		feature_cache_.add(binary_features);
		feature_cache_.add(float_features);
		feature_value_cache_.add(float_values);
		binary_features.clear();
		float_features.clear();
		float_values.clear();
	}

	//path feature
	edgefstr = "A_edge";
	if (train) {
		id = addfID(edgefstr.c_str(), argysize()*argattrsize());
	} else {
		id = getfID(edgefstr.c_str());
	}
	if (id != -1) {
		binary_features.push_back(id);
	}
	feature_cache_.add(binary_features);
	feature_cache_.add(float_features);
	feature_value_cache_.add(float_values);
	binary_features.clear();
	float_features.clear();
	float_values.clear();
}


void FeatureIndex::LoadGazetteerFeatures(const map<string, int>& vocab, string filename) {
	  string gazetteer_files = filename + "/gazetteers-list.txt";
	  ifstream filelist(gazetteer_files.c_str(), ios::in);
	  if (!filelist) return;

	  vector<string> files;
	  string str;
	  while (getline(filelist, str)) {
		  files.push_back(str);
	  }
	  filelist.close();

	  for (int i = 0; i < files.size(); ++i) {
		  vector<string> splits;
		  Utils::Split(files[i], '/', splits);
		  string key = splits[splits.size()-1];
		  int index = key.find(".");
		  key = key.substr(0,index);

		  string file = filename + "/" + files[i];
		  ifstream infile(file.c_str(), ios::in);
		  if (!infile) continue;

		  while (getline(infile, str)) {
			  vector<string> words;
			  Utils::Split(str, ' ', words);
			  if (words.size() == 1) {
				  string word = words[0];
				  word = Utils::toLower(word);
				  if (vocab.find(word) != vocab.end())
					  gazetteer_unigrams[word] = key;
			  } else if (words.size() == 2) {
				  string word = words[0]+"_"+words[1];
				  word = Utils::toLower(word);
				  if (vocab.find(word) != vocab.end())
					  gazetteer_bigrams[word] = key;
			  }
//			  else if (words.size() > 2) {
//				  string text = words[0];
//				  for (int j = 1; j < min((int)words.size(),3); ++j) text += "_"+words[j];
//				  text = Utils::toLower(text);
//				  if (vocab.find(text) != vocab.end())
//					  gazetteer_trigrams[text] = key;
//			  }
		  }
		  infile.close();
	  }
  }

void FeatureIndex::buildCRFTagger(TaggerImpl *tagger) {
	size_t fid = tagger->feature_id();
	size_t fvid = tagger->feature_value_id();

	unsigned short thread_id = tagger->thread_id();

	node_freelist_[thread_id].free();
	path_freelist_[thread_id].free();

	for (size_t cur = 0; cur < tagger->size(); ++cur) {
	  int *binary_f = feature_cache_[fid++];
	  int *float_f = feature_cache_[fid++];
	  double *fv = feature_value_cache_[fvid++];

	  for (size_t i = 0; i < y_.size(); ++i) {
		  Node *n = node_freelist_[thread_id].alloc();

		  n->clear();
		  n->x = cur;
		  n->y = i;
		  n->binary_fea_vector = binary_f;
		  n->float_fea_vector = float_f;
		  n->float_fv_vector = fv;
		  tagger->set_node(n, cur, i);
	  }
	}

	int *binary_f = feature_cache_[fid++];
	int *float_f = feature_cache_[fid++];
	double *fv = feature_value_cache_[fvid++];

	for (size_t cur = 1; cur < tagger->size(); ++cur) {
		for (size_t j = 0; j < y_.size(); ++j) {
		  for (size_t i = 0; i < y_.size(); ++i) {
			Path *p = path_freelist_[thread_id].alloc();
			p->clear();
			p->add(tagger->node(cur-1, j), tagger->node(cur, i), j*y_.size()+i);
			p->binary_fea_vector = binary_f;
			p->float_fea_vector = float_f;
			p->float_fv_vector = fv;
		  }
		}
	}
}

bool FeatureIndex::buildCRFFeatures(TaggerImpl *tagger, Sentence *sent, bool train) {
	int id;
	string_buffer os;
	std::vector <int> binary_features;
	vector<int> float_features;
	vector<double> float_values;

	tagger->set_feature_id(feature_cache_.size());
	tagger->set_feature_value_id(feature_value_cache_.size());

	for (size_t cur = 0; cur < tagger->size(); ++cur) {
		vector<string> strfeatures;

		 // basic features: word, pos

		sent->genBasicFeatures(cur, strfeatures);

		// window features, only consider word and lemma by default
		for (int k = 1; k <= windowsize; ++k) {
			sent->genCRFWindowFeatures(k, cur, strfeatures);
			sent->genCRFWindowFeatures(-1*k, cur, strfeatures);
		}

		 // segment features
		if (gazetteer_unigrams.size() > 0) {
			string word = sent->words[cur];
			word = Utils::toLower(word);
			if (gazetteer_unigrams.find(word) != gazetteer_unigrams.end()) {
				string fstr = gazetteer_unigrams[word];
				strfeatures.push_back(fstr);
			}
		}

		if (gazetteer_bigrams.size() > 0) {
			if ((int)cur < (int)sent->words.size()-1) {
				string word = sent->words[cur] + "_" + sent->words[cur+1];
				word = Utils::toLower(word);
				if (gazetteer_bigrams.find(word) != gazetteer_bigrams.end()) {
					string fstr = "E_"+gazetteer_bigrams[word];
					strfeatures.push_back(fstr);
				}
			}

			if ((int)cur >= 1) {
				string word = sent->words[cur-1] + "_" + sent->words[cur];
				word = Utils::toLower(word);
				if (gazetteer_bigrams.find(word) != gazetteer_bigrams.end()) {
					string fstr = "I_"+gazetteer_bigrams[word];
					strfeatures.push_back(fstr);
				}
			}
		}

		for(int col=0; col<strfeatures.size(); col++) {
			string fv = "F_"+strfeatures[col]; //w_-1_0_be
			if (train) {
				id = addfID(fv.c_str(), ysize());
			} else {
				id = getfID(fv.c_str());
			}
			if (id != -1) {
				binary_features.push_back(id);
			}
		}

		// embedding features
		if (word_lookup_table.size() > 0 && word_lookup_table.size() > 0) {
			vector<double> embedding;
			sent->genEmbeddingFeatures(cur, cur, word_lookup_table, word_embeddings, embedding);
			for (int k = 0; k < word_vec_size; ++k) {
				string fv = "F_Embedding_"+Utils::int2string(k);
				if (train) {
					id = addfID(fv.c_str(), ysize());
				} else {
					id = getfID(fv.c_str());
				}
				if (id != -1) {
					float_features.push_back(id);
					float_values.push_back(embedding[k]);
				}
			}
		}

		feature_cache_.add(binary_features);
		feature_cache_.add(float_features);
		feature_value_cache_.add(float_values);
		binary_features.clear();
		float_features.clear();
		float_values.clear();
	}


	//default edge feature
	string fv = "B_";
	if (train) {
		id = addfID(fv.c_str(), ysize()*ysize());
	} else {
		id = getfID(fv.c_str());
	}
	if (id != -1) {
		binary_features.push_back(id);
	}
	feature_cache_.add(binary_features);
	feature_cache_.add(float_features);
	feature_value_cache_.add(float_values);
	binary_features.clear();
	float_features.clear();
	float_values.clear();
    return true;
}

bool FeatureIndex::buildPairwiseFeatures(TaggerImpl *tagger, Sentence *sent,
		EventMention *m1, EventMention *m2, bool train) {
	int id;
	string_buffer os;
	std::vector <int> binary_features;
	vector<int> float_features;
	vector<double> float_values;

	tagger->set_feature_id(feature_cache_.size());
	tagger->set_feature_value_id(feature_value_cache_.size());

	map<string, double> predicate_features_1;
	sent->genTriggerFeatures(m1, predicate_features_1);
	for(map<string, double>::iterator it = predicate_features_1.begin();
			it != predicate_features_1.end(); ++it) {
		string fv = "P_"+it->first;
		if (train) {
			id = addfID(fv.c_str(), ysize());
		} else {
			id = getfID(fv.c_str());
		}
		if (id != -1) {
			if (it->first[0] != 'P') {
				binary_features.push_back(id);
			} else {
				float_features.push_back(id);
				float_values.push_back(it->second);
			}
		}
	}
	vector<double> embedding_1;
	if (word_lookup_table.size() > 0 && word_lookup_table.size() > 0) {
		sent->genEmbeddingFeatures(m1->start, m1->end, word_lookup_table, word_embeddings, embedding_1);
		for (int k = 0; k < word_vec_size; ++k) {
			string fv = "F_Embedding_"+Utils::int2string(k);
			if (train) {
				id = addfID(fv.c_str(), ysize());
			} else {
				id = getfID(fv.c_str());
			}
			if (id != -1) {
				float_features.push_back(id);
				float_values.push_back(embedding_1[k]);
			}
		}
	}
	feature_cache_.add(binary_features);
	feature_cache_.add(float_features);
	feature_value_cache_.add(float_values);
	binary_features.clear();
	float_features.clear();
	float_values.clear();

	map<string, double> predicate_features_2;
	sent->genTriggerFeatures(m2, predicate_features_2);
	for(map<string, double>::iterator it = predicate_features_2.begin();
				it != predicate_features_2.end(); ++it) {
		string fv = "P_"+it->first;
		if (train) {
			id = addfID(fv.c_str(), ysize());
		} else {
			id = getfID(fv.c_str());
		}
		if (id != -1) {
			if (it->first[0] != 'P') {
				binary_features.push_back(id);
			} else {
				float_features.push_back(id);
				float_values.push_back(it->second);
			}
		}
	}
	vector<double> embedding_2;
	if (word_lookup_table.size() > 0 && word_lookup_table.size() > 0) {
		sent->genEmbeddingFeatures(m2->start, m2->end, word_lookup_table, word_embeddings, embedding_2);
		for (int k = 0; k < word_vec_size; ++k) {
			string fv = "F_Embedding_"+Utils::int2string(k);
			if (train) {
				id = addfID(fv.c_str(), ysize());
			} else {
				id = getfID(fv.c_str());
			}
			if (id != -1) {
				float_features.push_back(id);
				float_values.push_back(embedding_2[k]);
			}
		}
	}
	feature_cache_.add(binary_features);
	feature_cache_.add(float_features);
	feature_value_cache_.add(float_values);
	binary_features.clear();
	float_features.clear();
	float_values.clear();

	//default edge feature
	map<string, double> pair_features;
	sent->genTriggerPairFeatures(m1, m2, pair_features);
	for(map<string, double>::iterator it = pair_features.begin(); it != pair_features.end(); ++it) {
		string fv = "E_"+it->first;
		if (train) {
			id = addfID(fv.c_str(), ysize()*ysize());
		} else {
			id = getfID(fv.c_str());
		}
		if (id != -1) {
			binary_features.push_back(id);
		}
	}

	if (word_lookup_table.size() > 0 && word_lookup_table.size() > 0) {
		double norm_1 = 0.0;
		for (int j = 0; j < embedding_1.size(); ++j) {
			norm_1 += embedding_1[j] * embedding_1[j];
		}
		norm_1 = sqrt(norm_1);

		double norm_2 = 0.0;
		for (int j = 0; j < embedding_2.size(); ++j) {
			norm_2 += embedding_2[j] * embedding_2[j];
		}
		norm_2 = sqrt(norm_2);

		if (norm_1 != 0 && norm_2 != 0) {
			double sim = 0.0;
			for (int j = 0; j < embedding_1.size(); ++j) {
				sim += embedding_1[j] * embedding_2[j];
			}
			sim = sim/(norm_1* norm_2);

			string fv = "Embedding_sim";
			if (train) {
				id = addfID(fv.c_str(), ysize()*ysize());
			} else {
				id = getfID(fv.c_str());
			}
			if (id != -1) {
				float_features.push_back(id);
				float_values.push_back(sim);
			}
		}
	}

	feature_cache_.add(binary_features);
	feature_cache_.add(float_features);
	feature_value_cache_.add(float_values);
	binary_features.clear();
	float_features.clear();
	float_values.clear();

    return true;
}
}


