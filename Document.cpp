#include "Document.h"

Sentence *Document::findSentence(int sentid) {
	for (int i = 0; i < sentences.size(); ++i) {
		if (sentences[i]->sent_id == sentid)
			return sentences[i];
	}
	return NULL;
}

void Document::BuildVocab(map<string, int> &vocabulary) {
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

void Document::OutputEventAnnos(string outputfile) {
	if (sentences.size() == 0) return;

	if (doc_id == "APW_ENG_20030322.0119") {

	}

	int entity_id = 0;
	int event_id = 0;

	int ch_offset = 0;

	string sentfile = outputfile + ".txt";
	ofstream sentencefile(sentfile.c_str(), ios::out);
	for (int i = 0; i < sentences.size(); ++i) {
		Sentence *sent = sentences[i];
		if (sent->words[0] == "u.s.") {
			Utils::capitalize_word(sent->words[0]);
		}

		string sentencestr = sent->toString();

		Utils::capitalize(sentencestr[0]);

		sentencefile<<sentencestr<<endl;
	}
	sentencefile.close();

	string annofile = outputfile + ".ann";
	ofstream resultfile(annofile.c_str(), ios::out);
	for (int i = 0; i < sentences.size(); ++i) {
		Sentence *sent = sentences[i];

		vector<int> offset(sent->words.size());
		int k = 0;
		for (int w = 0; w < sent->words.size(); ++w) {
			offset[w] = ch_offset + k;
			k += sent->words[w].size();
			k++; //blank
		}


		map<int, int> entityidmap; // sent_en_id, anno_id
		for (int j = 0; j < sent->cand_entity_mentions.size(); ++j) {
			string label = sent->cand_entity_mentions[j]->pred_ner_type;
			if (label == "O") continue;

			string enid = "T" + Utils::int2string(entity_id);
			entityidmap[j] = entity_id;

			int op_start = sent->cand_entity_mentions[j]->start;
			int op_end = sent->cand_entity_mentions[j]->end;
			string startoffset = "";
			string endoffset = "";

			startoffset = Utils::int2string(offset[op_start]);
			endoffset = Utils::int2string(offset[op_end]+sent->words[op_end].size());
			resultfile<<enid<<"\t"<<label<<" "<<startoffset<<" "<<endoffset<<"\t"<<sent->GetSpanText(op_start, op_end)<<endl;

			entity_id++;
		}

		map<int, int> eventidmap; // sent_en_id, anno_id
		for (int j = 0; j < sent->cand_event_mentions.size(); ++j) {
			EventMention *ev_men = sent->cand_event_mentions[j];
			string label = ev_men->pred_subtype;
			if (label == "O") continue;

			string enid = "T" + Utils::int2string(entity_id);
			eventidmap[j] = entity_id;

			int op_start = ev_men->start;
			int op_end = ev_men->end;
			string startoffset = "";
			string endoffset = "";

			startoffset = Utils::int2string(offset[op_start]);
			endoffset = Utils::int2string(offset[op_end]+sent->words[op_end].size());
			resultfile<<enid<<"\t"<<label<<" "<<startoffset<<" "<<endoffset<<"\t"<<sent->GetSpanText(op_start, op_end)<<endl;

			entity_id++;

			// event relations
			if (ev_men->pred_arg_to_role.size() == 0) continue;

            string event_str = "E" + Utils::int2string(event_id++) + "\t" + label + ":" + enid + " ";
            for (map<int, string>::iterator it = ev_men->pred_arg_to_role.begin(); it != ev_men->pred_arg_to_role.end(); ++it) {
                Mention *men = sent->cand_entity_mentions[it->first];
                string role = it->second;

                startoffset = Utils::int2string(offset[men->start]);
                endoffset = Utils::int2string(offset[men->end]+sent->words[men->end].size());

                string argid = "T" + Utils::int2string(entityidmap[it->first]);
                event_str += role + ":" + argid + " ";
            }
            resultfile<<event_str<<endl;
		}

		string sentencestr = sent->toString();
		ch_offset += sentencestr.size() + 1;
	}

	resultfile.close();
}
