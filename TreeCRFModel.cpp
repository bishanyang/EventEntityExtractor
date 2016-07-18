/*
 * TreeCRFModel.cpp
 *
 *  Created on: Oct 30, 2015
 *      Author: bishan
 */

#include "TreeCRFModel.h"

TreeCRFLearner::TreeCRFLearner() {
	// TODO Auto-generated constructor stub
	data = NULL;
}

TreeCRFLearner::~TreeCRFLearner() {
	// TODO Auto-generated destructor stub
}

void TreeCRFLearner::outputModel(string modelfile) {
	   std::string filename2 = modelfile;
	   filename2 += ".txt";

	   std::ofstream tofs(filename2.c_str());

	   tofs << "maxid: "       << encoder.feature_index->size() << std::endl;
	   tofs << std::endl;

	  tofs << std::endl;
	  tofs.setf(std::ios::fixed, std::ios::floatfield);
	  tofs.precision(16);
	  for (map<string, int>::iterator it = encoder.feature_index->lb2i.begin(); it != encoder.feature_index->lb2i.end(); ++it) {
		  tofs<<it->first<<"\t"<<it->second<<endl;
	  }
	  for (map<string, int>::iterator it = encoder.feature_index->arglb2i.begin(); it != encoder.feature_index->arglb2i.end(); ++it) {
		  tofs<<it->first<<"\t"<<it->second<<endl;
	  }
	  for (map<string, int>::iterator it = encoder.feature_index->argattrlb2i.begin(); it != encoder.feature_index->argattrlb2i.end(); ++it) {
		  tofs<<it->first<<"\t"<<it->second<<endl;
	  }


	  vector<pair<string, double> > feature_weights;
	  map<string, int> fstr2id;
	  encoder.feature_index->get_featuremap(fstr2id);
	  for (map<string, int>::iterator it = fstr2id.begin(); it != fstr2id.end(); ++it) {
		  string fstr = it->first;
		  int fid = it->second;
		  if (fstr[0] == 'P') {
			  for (map<string, int>::iterator it1 = encoder.feature_index->lb2i.begin(); it1 != encoder.feature_index->lb2i.end(); ++it1) {
				  feature_weights.push_back((make_pair(fstr+"_"+it1->first, encoder.feature_index->get_alpha(fid+it1->second))));
			  }
		  } else if (fstr[0] == 'A') {
			  for (map<string, int>::iterator it1 = encoder.feature_index->argattrlb2i.begin(); it1 != encoder.feature_index->argattrlb2i.end(); ++it1) {
				  feature_weights.push_back((make_pair(fstr+"_"+it1->first, encoder.feature_index->get_alpha(fid+it1->second))));
			  }
		  } else if (fstr[0] == 'R') {
			  for (map<string, int>::iterator it1 = encoder.feature_index->arglb2i.begin(); it1 != encoder.feature_index->arglb2i.end(); ++it1) {
				  feature_weights.push_back((make_pair(fstr+"_"+it1->first, encoder.feature_index->get_alpha(fid+it1->second))));
			  }
		  }
	   }

	  //sort(feature_weights.begin(), feature_weights.end(), Utils::decrease_second<string, double>);

	  for (int i = 0; i < feature_weights.size(); ++i) {
		  tofs<<feature_weights[i].first<<"\t"<<feature_weights[i].second<<endl;
	  }

	  tofs.close();

}

void TreeCRFLearner::LoadEventRoles(string inputfile) {
	map<string, set<string> > translabels;
	set<string> p;
	p.insert("O");
//	p.insert("TIME");
//	p.insert("Place");
	translabels["O"] = p;

	string line;
	ifstream infile(inputfile.c_str(), ios::in);
	while (getline(infile, line)) {
		int i = line.find("\t");
		string trigger = line.substr(0, i);
		if (encoder.feature_index->lb2i.find(trigger) == encoder.feature_index->lb2i.end()) continue;

		string roles = line.substr(i+1);
		vector<string> fields;
		Utils::Split(roles, ' ', fields);
		set<string> labels;
 		for (int j = 0; j < fields.size(); ++j) {
			labels.insert(fields[j]);
		}
 		translabels[trigger] = labels;
	}
	infile.close();

	encoder.feature_index->SetTransY(translabels);

	cout<<"load transition labels "<<encoder.feature_index->transition_i2lb.size()<<endl;
}

void TreeCRFLearner::LoadLabels(vector<string> lines) {
	//collect labels
	// set labels
	map<string, int> predicate_label2i;
	map<string, int> argument_label2i;
	map<string, int> argattr_label2i;

	predicate_label2i.clear();
	predicate_label2i["O"] = 0;

	argument_label2i.clear();
	argument_label2i["O"] = 0;

	argattr_label2i["O"] = 0;

	int pi = 1;
	int ai = 1;
	int ti = 1;
	for (int i = 0; i < lines.size(); ++i) {
		string line = lines[i];
		if (line[0] == 'T') {
			vector<string> fields;
			Utils::Split(line, ' ', fields);
			if (predicate_label2i.find(fields[1]) == predicate_label2i.end())
				predicate_label2i[fields[1]] = pi++;
		} else if (line[0] == 'R') {
			vector<string> fields;
			Utils::Split(line, ' ', fields);
			if (argument_label2i.find(fields[1]) == argument_label2i.end())
				argument_label2i[fields[1]] = ai++;

			vector<string> splits;
			Utils::Split(fields[0], '#', splits);
			int k = splits.size()-1;
			if (argattr_label2i.find(splits[k]) == argattr_label2i.end())
				argattr_label2i[splits[k]] = ti++;
		}
	}

	encoder.feature_index->SetY(predicate_label2i);
	encoder.feature_index->SetArgY(argument_label2i);
	encoder.feature_index->SetArgAttrY(argattr_label2i);

	cout<<"collect "<<predicate_label2i.size()<<" predicate labels"<<endl;
	for (map<string, int>::iterator it = predicate_label2i.begin(); it != predicate_label2i.end(); ++it) {
		cout<<it->first<<"\t"<<it->second<<endl;
	}
	cout<<"collect "<<argument_label2i.size()<<" argument labels"<<endl;
	for (map<string, int>::iterator it = argument_label2i.begin(); it != argument_label2i.end(); ++it) {
		cout<<it->first<<"\t"<<it->second<<endl;
	}
	cout<<"collect "<<argattr_label2i.size()<<" argument attribute labels"<<endl;
	for (map<string, int>::iterator it = argattr_label2i.begin(); it != argattr_label2i.end(); ++it) {
		cout<<it->first<<"\t"<<it->second<<endl;
	}
}

void TreeCRFLearner::LoadLabels() {
	map<string, int> predicate_label2i;
	map<string, int> argument_label2i;
	map<string, int> argattr_label2i;

	predicate_label2i["O"] = 0;
	argument_label2i["O"] = 0;
	argattr_label2i["O"] = 0;

	int pi = 1;
	int ai = 1;
	int ti = 1;
	for (int i = 0; i < data->sentences.size(); ++i) {
		for (int j = 0; j < data->sentences[i]->cand_event_mentions.size(); ++j) {
			EventMention *men = data->sentences[i]->cand_event_mentions[j];
			if (predicate_label2i.find(men->gold_subtype) == predicate_label2i.end())
				predicate_label2i[men->gold_subtype] = pi++;

			for (map<int, string>::iterator it = men->gold_arg_to_role.begin(); it != men->gold_arg_to_role.end(); ++it) {
				if (argument_label2i.find(it->second) == argument_label2i.end())
					argument_label2i[it->second] = ai++;
			}
		}
		for (int j = 0; j < data->sentences[i]->cand_entity_mentions.size(); ++j) {
			Mention *men = data->sentences[i]->cand_entity_mentions[j];
			if (argattr_label2i.find(men->gold_ner_type) == argattr_label2i.end())
				argattr_label2i[men->gold_ner_type] = ti++;
		}
	}

	encoder.feature_index->SetY(predicate_label2i);
	encoder.feature_index->SetArgY(argument_label2i);
	encoder.feature_index->SetArgAttrY(argattr_label2i);

	cout<<"collect "<<predicate_label2i.size()<<" predicate labels"<<endl;
	for (map<string, int>::iterator it = predicate_label2i.begin(); it != predicate_label2i.end(); ++it) {
		cout<<it->first<<"\t"<<it->second<<endl;
	}
	cout<<"collect "<<argument_label2i.size()<<" argument labels"<<endl;
	for (map<string, int>::iterator it = argument_label2i.begin(); it != argument_label2i.end(); ++it) {
		cout<<it->first<<"\t"<<it->second<<endl;
	}
	cout<<"collect "<<argattr_label2i.size()<<" argument attribute labels"<<endl;
	for (map<string, int>::iterator it = argattr_label2i.begin(); it != argattr_label2i.end(); ++it) {
		cout<<it->first<<"\t"<<it->second<<endl;
	}
}

void TreeCRFLearner::Train(string modelfile) {
	labelx.clear();

	int xid = 0;
	for (int i = 0; i < data->sentences.size(); ++i) {
		Sentence *sent = data->sentences[i];

		for (int j = 0; j < sent->cand_event_mentions.size(); ++j) {
			CRFPP::TreeTagger* x  = new CRFPP::TreeTagger();
			if (x->EventToTreeTagger(sent->cand_event_mentions[j], sent, encoder.feature_index, true)) {
				xid = labelx.size();
				x->set_thread_id(xid % encoder.feature_index->thread_num_);
				labelx.push_back(x);
			}
		}
	}

	cout<<"Feature size "<<encoder.feature_index->size()<<endl;
	cout << "Number of instances: " << labelx.size() << std::endl;
	// allocate nodes, paths, and fvectors
	encoder.buildAlpha();
	encoder.learn(labelx, modelfile.c_str());
}

void TreeCRFPredictor::LoadModel(string modelfile) {
	feature_index = new CRFPP::DecoderFeatureIndex();
	feature_index->open(modelfile.c_str());
}

string TreeCRFPredictor::Predict(Document *doc) {
	std::stringstream ss;
	for (int s = 0; s < doc->sentences.size(); ++s) {
		Sentence *sent = doc->sentences[s];
		int sentid = sent->sent_id;

		for (int i = 0; i < sent->cand_event_mentions.size(); ++i) {
			EventMention *event = sent->cand_event_mentions[i];

			feature_index->clear();

			CRFPP::TreeTagger x;
			x.EventToTreeTagger(event, sent, feature_index, false);

			x.Inference();

			string trigger_result = feature_index->i2lb[x.predicate_result_];
			sent->cand_event_mentions[i]->pred_subtype = trigger_result;
			ss<<doc->doc_id<<"#"<<sentid<<"#"<<event->start<<"#"<<event->end<<"\t"<<trigger_result<<"\n";

			string event_str = sent->GetSpanText(event->start, event->end);
			for (int j = 0; j < sent->cand_entity_mentions.size(); ++j) {
				Mention *en = sent->cand_entity_mentions[j];
				string role = feature_index->argi2lb[x.argrole_result_[j]];
				string ner = feature_index->argattri2lb[x.argattr_result_[j]];
				event->pred_arg_to_role[j] = role;
				en->pred_ner_type = feature_index->argattri2lb[x.argattr_result_[j]];

				ss<<doc->doc_id<<"#"<<sentid<<"#"<<en->start<<"#"<<en->end<<"#"<<ner<<"\t"<<role<<"\n";
			}

			ss<<"\n";
		}
	}
	return ss.str();
}

void TreeCRFPredictor::outputModel(string modelfile) {
	   std::string filename2 = modelfile;
	   filename2 += ".txt";

	   std::ofstream tofs(filename2.c_str());

	   tofs << "maxid: "       << feature_index->size() << std::endl;
	   tofs << std::endl;

	  tofs << std::endl;
	  tofs.setf(std::ios::fixed, std::ios::floatfield);
	  tofs.precision(16);
	  for (map<string, int>::iterator it = feature_index->lb2i.begin(); it != feature_index->lb2i.end(); ++it) {
		  tofs<<it->first<<"\t"<<it->second<<endl;
	  }
	  for (map<string, int>::iterator it = feature_index->arglb2i.begin(); it != feature_index->arglb2i.end(); ++it) {
		  tofs<<it->first<<"\t"<<it->second<<endl;
	  }
	  for (map<string, int>::iterator it = feature_index->argattrlb2i.begin(); it != feature_index->argattrlb2i.end(); ++it) {
		  tofs<<it->first<<"\t"<<it->second<<endl;
	  }


	  vector<pair<string, double> > feature_weights;
	  map<string, int> fstr2id = feature_index->feature_dict_;

	  for (map<string, int>::iterator it = fstr2id.begin(); it != fstr2id.end(); ++it) {
		  string fstr = it->first;
		  int fid = it->second;
		  if (fstr[0] == 'P') {
			  for (map<string, int>::iterator it1 = feature_index->lb2i.begin(); it1 != feature_index->lb2i.end(); ++it1) {
				  feature_weights.push_back((make_pair(fstr+"_"+it1->first, feature_index->get_alpha(fid+it1->second))));
			  }
		  } else if (fstr[0] == 'A') {
			  for (map<string, int>::iterator it1 = feature_index->argattrlb2i.begin(); it1 != feature_index->argattrlb2i.end(); ++it1) {
				  feature_weights.push_back((make_pair(fstr+"_"+it1->first, feature_index->get_alpha(fid+it1->second))));
			  }
		  } else if (fstr[0] == 'R') {
			  for (map<string, int>::iterator it1 = feature_index->arglb2i.begin(); it1 != feature_index->arglb2i.end(); ++it1) {
				  feature_weights.push_back((make_pair(fstr+"_"+it1->first, feature_index->get_alpha(fid+it1->second))));
			  }
		  }
	   }

	  //sort(feature_weights.begin(), feature_weights.end(), Utils::decrease_second<string, double>);

	  for (int i = 0; i < feature_weights.size(); ++i) {
		  tofs<<feature_weights[i].first<<"\t"<<feature_weights[i].second<<endl;
	  }

	  tofs.close();

}
