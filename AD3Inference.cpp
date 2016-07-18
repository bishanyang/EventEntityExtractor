/*
 * AD3Inference.cpp
 *
 *  Created on: Oct 27, 2015
 *      Author: bishan
 */

#include "AD3Inference.h"
#include "Utils.h"

#include <limits>

// Recover a valid integer assignment from a possibly fractional one.
// Use a simple heuristic that prefers the state with highest unary posterior.

void parse_line(string line, vector<string>& states, vector<double> &values) {
	states.clear();
	values.clear();
	vector<string> scores;
	Utils::Split(line, ' ', scores);
	for (int i = 0; i < scores.size(); ++i) {
		int j = scores[i].find(':');
		states.push_back(scores[i].substr(0,j));
		double v = atof(scores[i].substr(j+1).c_str());
		values.push_back(v);
	}
}

void loadSequenceData(string featurefile, Document *doc, vector<EventSequence*>& sequences) {
	ifstream infile(featurefile.c_str(), ios::in);
	string line;
	string cur_docid = "";
	vector<string> lines;
	while (getline(infile, line)) {
		if (line == "") {
			if (lines.size() > 0) {
				EventSequence *inst = new EventSequence();
				inst->docid = doc->doc_id;

				for (int i = 0; i < lines.size(); ++i) {
					if (lines[i][0] == 'T') {
						vector<string> fields;
						Utils::Split(lines[i], ' ', fields);
						inst->trigger_indices.push_back(fields[0]);
					}

					inst->featurelines.push_back(lines[i]);
				}

				sequences.push_back(inst);
				lines.clear();
				continue;
			}
		}

		if (line[0] == 'T') {
			int j = line.find('#');
			int i = line.find('_');
			cur_docid = line.substr(i+1, j-i-1);
		}
		if (cur_docid != doc->doc_id) continue;

		lines.push_back(line);
	}
	infile.close();
}

void AD3Inference::AddTriggerSequencePotentials(Document *doc, CRFPredictor predictor,
		map<string, int> trigger_dict, int pred_num_states) {

	for (int i = 0; i < doc->sentences.size(); ++i) {
		Sentence *sent = doc->sentences[i];
		for (int j = 0; j < sent->cand_event_mentions.size(); ++j) {
			for (int j1 = j+1; j1 < sent->cand_event_mentions.size(); ++j1) {
				CRFPP::TaggerImpl* tagger  = new CRFPP::TaggerImpl();
				tagger->PairToTagger(sent, sent->cand_event_mentions[j],
						sent->cand_event_mentions[j1], predictor.feature_index, false);
				tagger->Inference();

				string key = sent->cand_event_mentions[j]->index;
				assert (trigger_to_varid.find(key) !=  trigger_to_varid.end());

				int varid = trigger_to_varid[key];
				AD3::MultiVariable* pred_variable = factor_graph.GetMultiVariable(varid);
				for (int y = 0; y < predictor.feature_index->ysize(); ++y) {
					int py = trigger_dict[predictor.feature_index->i2lb[y]];
					double orig_p = pred_variable->GetLogPotential(py);
					pred_variable->SetLogPotential(py, orig_p + log(tagger->node_[0][y]->prob));
				}

				key = sent->cand_event_mentions[j1]->index;
				assert (trigger_to_varid.find(key) !=  trigger_to_varid.end());

				varid = trigger_to_varid[key];
				pred_variable = factor_graph.GetMultiVariable(varid);
				for (int y = 0; y < predictor.feature_index->ysize(); ++y) {
					int py = trigger_dict[predictor.feature_index->i2lb[y]];
					double orig_p = pred_variable->GetLogPotential(py);
					pred_variable->SetLogPotential(py, orig_p + log(tagger->node_[1][y]->prob));
				}
			}
		}
	}
}
