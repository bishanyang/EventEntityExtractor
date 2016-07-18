/*
 * CRFLearner.cpp
 *
 *  Created on: Jan 18, 2013
 *      Author: bishan
 */

#include "SequenceModel.h"

void CRFLearner::Train(string modelfile) {
	labelx.clear();
	for (int i = 0; i < data->sentences.size(); ++i) {
		// only train on annotated sentences
		bool valid = false;
		for (int j = 0; j < data->sentences[i]->token_labels.size(); ++j) {
			if (data->sentences[i]->token_labels[j] != "O") valid = true;
		}
		if (!valid) {
			continue;
		}

		CRFPP::TaggerImpl* x  = new CRFPP::TaggerImpl();
		if (x->SentenceToTagger(data->sentences[i], encoder.feature_index, true)) {
			int xid = labelx.size();
			x->set_thread_id(xid % encoder.feature_index->thread_num_);
			labelx.push_back(x);
		} else {
			delete x;
			x = NULL;
		}
	}

	cout<<"Loaded "<<labelx.size()<<" instances..."<<endl;

	encoder.buildAlpha();

	// for debugging
	//encoder.maxitr = 1;

	encoder.learn(labelx, modelfile.c_str());
}

void CRFLearner::TrainEventSeq(string modelfile) {
	labelx.clear();

	for (int i = 0; i < data->sentences.size(); ++i) {
		Sentence *sent = data->sentences[i];
		for (int j = 0; j < sent->cand_event_mentions.size(); ++j) {
			for (int j1 = j+1; j1 < sent->cand_event_mentions.size(); ++j1) {
				CRFPP::TaggerImpl* x  = new CRFPP::TaggerImpl();
				if (x->PairToTagger(sent, sent->cand_event_mentions[j],
						sent->cand_event_mentions[j1], encoder.feature_index, true)) {
					int xid = labelx.size();
					x->set_thread_id(xid % encoder.feature_index->thread_num_);
					labelx.push_back(x);
				} else {
					delete x;
					x = NULL;
				}
			}
		}
	}

	cout<<"Feature size "<<encoder.feature_index->size()<<endl;

	// allocate nodes, paths, and fvectors
	encoder.buildAlpha();
	encoder.learn(labelx, modelfile.c_str());
}

void CRFLearner::outputLabels(string filename) {
	ofstream outfile(filename.c_str(), ios::out);
	for (int y = 0; y < encoder.feature_index->ysize(); ++y) {
		outfile<<encoder.feature_index->i2lb[y]<<"\t"<<y<<endl;
	}
	outfile.close();
}

void CRFLearner::outputModel(string modelfile) {
	   std::string filename2 = modelfile;
	   filename2 += ".txt";

	   std::ofstream tofs(filename2.c_str());

	   tofs << "maxid: "       << encoder.feature_index->size() << std::endl;
	   tofs << std::endl;

	  tofs << std::endl;
	  tofs.setf(std::ios::fixed, std::ios::floatfield);
	  tofs.precision(16);

	  vector<pair<string, double> > feature_weights;
	  map<string, int> fstr2id;
	  encoder.feature_index->get_featuremap(fstr2id);
	  for (map<string, int>::iterator it = fstr2id.begin(); it != fstr2id.end(); ++it) {
		  string fstr = it->first;
		  int fid = it->second;
		  if (fstr[0] != 'B') { //only output unigram features
			  for (int y = 0; y < encoder.feature_index->ysize(); ++y) {
				  string key = fstr+"_"+encoder.feature_index->i2lb[y];
				   feature_weights.push_back((make_pair(key, encoder.feature_index->get_alpha(fid+y))));
			  }
		  } else {
			  for (int y = 0; y < encoder.feature_index->transition_i2lb.size(); ++y) {
				  int ilabel = encoder.feature_index->transition_i2lb[y];
				  int y1 = ilabel/encoder.feature_index->ysize();
				  int y2 = ilabel%encoder.feature_index->ysize();
				  string key = fstr+"_"+encoder.feature_index->i2lb[y1] + "_" + encoder.feature_index->i2lb[y2];
				  feature_weights.push_back((make_pair(key, encoder.feature_index->get_alpha(fid+y))));
			  }
		  }
	   }

	  sort(feature_weights.begin(), feature_weights.end(), Utils::decrease_second<string, double>);

	  for (int i = 0; i < feature_weights.size(); ++i) {
		  tofs<<feature_weights[i].first<<"\t"<<feature_weights[i].second<<endl;
	  }
	  tofs.close();
}

void CRFPredictor::AddEntityCandidate(Span span, CRFPP::TaggerImpl* tagger, vector<Mention*> &pred_mentions) {
	for (int j = 0; j < pred_mentions.size(); ++j) {
		if (span.overlap(Span(pred_mentions[j]->start, pred_mentions[j]->end)) > 0) {
			return;
		}
	}

	Mention *men = new Mention();
	men->start = span.start;
	men->end = span.end;

	double bestprob = 0.0;
	string besty = "O";

	for (int y = 0; y < feature_index->i2lb.size(); ++y) {
		string label = feature_index->i2lb[y];
		if (scheme == "BIO" && label != "O" && label.substr(0,2) == "I_") continue;
		if (scheme == "BIO" && label != "O") label = label.substr(2);

		double prob = 0.0;
		if (scheme == "BIO")
			prob = tagger->segment_BIO_prob(men->start, men->end, label);
		else
			prob = tagger->segment_prob(men->start, men->end, label);
		if (prob > bestprob) {
			bestprob = prob;
			besty = label;
		}
		men->labels.push_back(label);
		men->values.push_back(prob);
	}

	men->pred_ner_type = besty;
	pred_mentions.push_back(men);
}

void CRFPredictor::AddOutputSpan(Span span, CRFPP::TaggerImpl* tagger, vector<EvalSpan> &pred_mentions) {
	for (int j = 0; j < pred_mentions.size(); ++j) {
		//if (pred_entity_mentions[j].start == pred_spans[i].start && pred_entity_mentions[j].end == pred_spans[i].end) {
		if (pred_mentions[j].overlap(span) > 0) {
			return;
		}
	}

	EvalSpan sp;
	sp.start = span.start;
	sp.end = span.end;

	double bestprob = 0.0;
	string besty = "O";

	for (int y = 0; y < feature_index->i2lb.size(); ++y) {
		string label = feature_index->i2lb[y];
		if (scheme == "BIO" && label != "O" && label.substr(0,2) == "I_") continue;
		if (scheme == "BIO" && label != "O") label = label.substr(2);

		double prob = 0.0;
		if (scheme == "BIO")
			prob = tagger->segment_BIO_prob(sp.start, sp.end, label);
		else
			prob = tagger->segment_prob(sp.start, sp.end, label);
		if (prob > bestprob) {
			bestprob = prob;
			besty = label;
		}
		sp.labels.push_back(label);
		sp.values.push_back(prob);
	}

	sp.label = besty;
	pred_mentions.push_back(sp);
}

void CRFPredictor::PredictEntityCandidates(Document *doc) {
	for (int s = 0; s < doc->sentences.size(); ++s) {
		Sentence *sent = doc->sentences[s];

		feature_index->clear();
		CRFPP::TaggerImpl* tagger  = new CRFPP::TaggerImpl();
		tagger->SentenceToTagger(sent, feature_index, false);

		tagger->viterbi_decoder.nbest_ = nbest;

		tagger->Inference();

		sent->cand_entity_mentions.clear();
		int n = 0;
		while(n < nbest) {
			vector<unsigned short int> result;
			result.resize(tagger->size(), 0);

			if (!tagger->viterbi_decoder.next(result)) break;

			vector<string> tags;
			for (size_t i = 0; i < tagger->size(); ++i) {
				tags.push_back(feature_index->i2lb[result[i]]);
			}
			vector<Span> pred_spans;
			GetAllSpans(tags, pred_spans, scheme);
			for (int i = 0; i < pred_spans.size(); ++i) {
				AddEntityCandidate(pred_spans[i], tagger, sent->cand_entity_mentions);
			}
			if (n == 0) {
				// add crf entities
				for (int i = 0; i < sent->cand_entity_mentions.size(); ++i) {
					sent->crf_entities.push_back(sent->cand_entity_mentions[i]);
				}
			}
			n++;
		}
	}
}

void CRFPredictor::AddEventCandidate(Span span, CRFPP::TaggerImpl* tagger, vector<EventMention*> &pred_mentions) {
	for (int j = 0; j < pred_mentions.size(); ++j) {
		if (span.overlap(Span(pred_mentions[j]->start, pred_mentions[j]->end)) > 0) {
			return;
		}
	}

	EventMention *men = new EventMention();
	men->start = span.start;
	men->end = span.end;
	pred_mentions.push_back(men);
}

void CRFPredictor::PredictEventCandidates(Document *doc) {
	for (int s = 0; s < doc->sentences.size(); ++s) {
		Sentence *sent = doc->sentences[s];

		feature_index->clear();

		CRFPP::TaggerImpl* tagger  = new CRFPP::TaggerImpl();
		tagger->SentenceToTagger(sent, feature_index, false);

		tagger->viterbi_decoder.nbest_ = nbest;

		tagger->Inference();

		sent->cand_event_mentions.clear();
		int n = 0;
		while(n < nbest) {
			vector<unsigned short int> result;
			result.resize(tagger->size(), 0);

			if (!tagger->viterbi_decoder.next(result)) break;

			vector<string> tags;
			for (size_t i = 0; i < tagger->size(); ++i) {
				tags.push_back(feature_index->i2lb[result[i]]);
			}
			vector<Span> pred_spans;
			GetAllSpans(tags, pred_spans, scheme);
			for (int i = 0; i < pred_spans.size(); ++i) {
				AddEventCandidate(pred_spans[i], tagger, sent->cand_event_mentions);
			}
			n++;
		}
	}
}

void CRFPredictor::Evaluate(vector<vector<string> > answers,
		vector<vector<string> > results, string resultfile) {
	vector<string> keylabels;
	Utils::Split(eval_key, ',', keylabels);

	ofstream outfile(resultfile.c_str(), ios::out);
	for (int l = 0; l < keylabels.size(); ++l) {
		double correctp = 0;
		double truep = 0;
		double predictp = 0;

		string label = keylabels[l];

		for(int i=0; i<answers.size(); ++i) {
			vector<string> labels = answers[i];
			vector<string> pred_labels = results[i];

			vector<Span> true_spans;
			vector<Span> pred_spans;
			GetSpans(labels, true_spans, label, scheme);
			GetSpans(pred_labels, pred_spans, label, scheme);

			truep += true_spans.size();
			predictp += pred_spans.size();
			for (int j = 0; j < true_spans.size(); ++j) {
				for (int k = 0; k < pred_spans.size(); ++k) {
					if (true_spans[j].isEqual(pred_spans[k])) {
						correctp += 1.0;
						outfile<<label<<"\t"
								<<true_spans[j].start<<","<<true_spans[j].end<<endl;
						break;
					}
				}
			}
		}

		cout<<"For "<<label<<endl;
		cout<<"strict match "<<truep<<" "<<predictp<<" "<<correctp<<endl;
		double pre = predictp==0 ? 0 : correctp/predictp;
		double rec = truep==0 ? 0 : correctp/truep;
		double f = pre+rec==0 ? 0 : 2.0*pre*rec/(pre+rec);
		cout<<"pre: "<<pre<<" rec: "<<rec<<" f: "<<f<<endl;
	}
	outfile.close();
}

void CRFPredictor::TaggingPrediction(string resultfile) {
	// Evaluation
	cout<<"test instance size: "<<testset->sentences.size()<<endl;

	vector<pair<string, int> > answer_index;
	vector<vector<string> > answers;
	vector<vector<string> > results;

	CRFPP::TaggerImpl* tagger  = new CRFPP::TaggerImpl();
	for (int i = 0; i < testset->sentences.size(); ++i) {

		tagger->clear();
		tagger->SentenceToTagger(testset->sentences[i], feature_index, false);
		tagger->Inference();

		vector<string> labels;
		vector<string> pred_labels;
		for(int j=0; j<tagger->size(); j++) {
			int itrue = tagger->answer_[j];
			int ipred = tagger->result_[j];

			string pred_label = feature_index->i2lb[ipred];
			string label = feature_index->i2lb[itrue];

			labels.push_back(label);
			pred_labels.push_back(pred_label);
			//cout<<label<<" "<<itrue<<"\t"<<pred_label<<" "<<ipred<<"\t"<<endl;
		}
		//cout<<endl;

		answer_index.push_back(make_pair(testset->sentences[i]->docid, testset->sentences[i]->sent_id));
		answers.push_back(labels);
		results.push_back(pred_labels);

		// set prediction results in sentence
		for (int k = 0; k < pred_labels.size(); ++k) {
			testset->sentences[i]->pred_token_labels[k] = pred_labels[k];
		}
	}
	delete tagger;
	tagger = NULL;

	Evaluate(answers, results, resultfile);
}

void CRFPredictor::EventSeqPrediction(string resultfile) {
	// Evaluation
	cout<<"test instance size: "<<testset->sentences.size()<<endl;

	vector<vector<string> > answers;
	vector<vector<string> > results;

	int acc = 0;
	int total = 0;
	for (int i = 0; i < testset->sentences.size(); ++i) {
		Sentence *sent = testset->sentences[i];
		for (int j = 0; j < sent->cand_event_mentions.size(); ++j) {
			for (int j1 = j+1; j1 < sent->cand_event_mentions.size(); ++j1) {
				CRFPP::TaggerImpl* tagger  = new CRFPP::TaggerImpl();
				tagger->PairToTagger(sent, sent->cand_event_mentions[j],
						sent->cand_event_mentions[j1], feature_index, false);
				tagger->Inference();

				vector<string> labels;
				vector<string> pred_labels;
				for(int j=0; j<tagger->size(); j++) {
					int itrue = tagger->answer_[j];
					int ipred = tagger->result_[j];

					string pred_label = feature_index->i2lb[ipred];
					string label = feature_index->i2lb[itrue];
					if (pred_label != "O" || label != "O") {
						if (pred_label == label) acc++;
						total++;
					}

					labels.push_back(label);
					pred_labels.push_back(pred_label);
				}

				answers.push_back(labels);
				results.push_back(pred_labels);

				delete tagger;
				tagger = NULL;
			}
		}
	}

	Evaluate(answers, results, resultfile);
	cout<<"Accuracy: "<<(float)acc/total<<endl;
}

void CRFPredictor::NbestMentionPrediction(string mentionfile) {
	cout<<"test instance size: "<<testset->sentences.size()<<endl;
	ofstream output(mentionfile.c_str(), ios::out);

	vector<vector<string> > all_answers;
	vector<vector<string> > all_results;

	int true_entities = 0;

	for (int s = 0; s < testset->sentences.size(); ++s) {
		CRFPP::TaggerImpl* tagger  = new CRFPP::TaggerImpl();
		tagger->SentenceToTagger(testset->sentences[s], feature_index, false);

		tagger->viterbi_decoder.nbest_ = nbest;

		tagger->Inference();


		vector<string> labels(testset->sentences[s]->size(), "O");
		vector<string> pred_labels(testset->sentences[s]->size(), "O");

		vector<Span> true_spans;
		GetAllSpans(testset->sentences[s]->token_labels, true_spans, "BIO");

		for (int i = 0; i < true_spans.size(); ++i) {
			labels[true_spans[i].start] = "B_ENTITY";
			for (int k = true_spans[i].start+1; k <= true_spans[i].end; ++k)
				labels[k] = "I_ENTITY";
		}

		vector<EvalSpan> pred_entity_mentions;

		// for training nfold prediction for training
//		for (int i = 0; i < true_spans.size(); ++i) {
//			AddOutputSpan(true_spans[i], tagger, pred_entity_mentions);
//		}

		int n = 0;
		while(n < nbest) {
			vector<unsigned short int> result;
			result.resize(tagger->size(), 0);

			if (!tagger->viterbi_decoder.next(result)) break;

			vector<string> tags;
			for (size_t i = 0; i < tagger->size(); ++i) {
				tags.push_back(feature_index->i2lb[result[i]]);
			}
			vector<Span> pred_spans;
			GetAllSpans(tags, pred_spans, scheme);
			for (int i = 0; i < pred_spans.size(); ++i) {
				AddOutputSpan(pred_spans[i], tagger, pred_entity_mentions);
			}

			n++;
		}

		// output predicted mentions
		for (int i = 0; i < pred_entity_mentions.size(); ++i) {
			string text = testset->sentences[s]->GetSpanText(pred_entity_mentions[i].start, pred_entity_mentions[i].end);
			output<<pred_entity_mentions[i].label<<"\t"<<testset->sentences[s]->docid<<"\t"<<testset->sentences[s]->sent_id
					<<"\t"<<pred_entity_mentions[i].start<<","<<pred_entity_mentions[i].end<<"\t";
			for (int j = 0; j < pred_entity_mentions[i].labels.size(); ++j) {
				output<<pred_entity_mentions[i].labels[j]<<":"<<pred_entity_mentions[i].values[j]<<" ";
			}
			output<<"\t"<<text<<endl;
		}

		for (int i = 0; i < pred_entity_mentions.size(); ++i) {
			pred_labels[pred_entity_mentions[i].start] = "B_ENTITY";
			for (int j = pred_entity_mentions[i].start+1; j <= pred_entity_mentions[i].end; ++j)
				pred_labels[j] = "I_ENTITY";
		}

		all_answers.push_back(labels);
		all_results.push_back(pred_labels);
	}


	std::stringstream ss;
	EvalParam eval_param;
	eval_param.key = eval_key;
	eval_param.scheme = "BIO";
	ComputeScore(all_answers, all_results, eval_param, ss);

	output.close();

}
