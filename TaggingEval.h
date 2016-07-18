/*
 * Eval.h
 *
 *  Created on: Feb 10, 2014
 *      Author: bishan
 */

#ifndef TAGGINGEVAL_H_
#define TAGGINGEVAL_H_

#include <vector>
#include <fstream>
#include "Utils.h"

struct EvalParam {
	int overlap; // 0: strict, 1: overlap, 2: proportional
	string key;
	string scheme;
	EvalParam() : overlap(0), key(""), scheme("IO") {}
};

struct EvalSpan : public Span {
	vector<string> labels;
	vector<double> values;

	EvalSpan() {
		start = -1;
		end = -1;
	}
};

struct SpanScore {
	double pre;
	double rec;
	double f1;

	string toString() {
		std::stringstream ss;
		ss<<"pre: "<<pre<<" rec: "<<rec<<" f: "<<f1<<endl;
		return ss.str();
	}

	void Copy(SpanScore score) {
		pre = score.pre;
		rec = score.rec;
		f1 = score.f1;
	}
};

class EvalScore {
public:
	SpanScore extraction;
public:
	void Copy(EvalScore score) {
		extraction.Copy(score.extraction);
	}

	string toString() {
		std::stringstream ss;
		ss<<"For Extraction"<<endl;
		ss<<extraction.toString();

		return ss.str();
	}
};

static double ExactMatch(vector<Span> &true_spans, vector<Span> &pred_spans) {
	double correctp = 0;
	for (int j = 0; j < true_spans.size(); ++j) {
		for (int k = 0; k < pred_spans.size(); ++k) {
			if (true_spans[j].isEqual(pred_spans[k])) {
				correctp += 1.0;
				break;
			}
		}
	}
	return correctp;
}

static void SoftMatch(vector<Span> &true_spans, vector<Span> &pred_spans,
		double &softr, double &softp) {
	int j,k;
	for (j = 0; j < true_spans.size(); ++j) {
		for (k = 0; k < pred_spans.size(); ++k) {
			if (true_spans[j].overlap(pred_spans[k]) > 0) {
				//assert(true_spans[j].isEqual(pred_spans[k]));
				softr += 1.0;
				break;
			}
		}
	}

	for (j = 0; j < pred_spans.size(); ++j) {
		for (k = 0; k < true_spans.size(); ++k) {
			if (pred_spans[j].overlap(true_spans[k]) > 0) {
				//assert(pred_spans[j].isEqual(true_spans[k]));
				softp += 1.0;
				break;
			}
		}
	}
}

static void ProportionMatch(vector<Span> &true_spans, vector<Span> &pred_spans,
		double &propr, double &propp) {
	for (int i = 0; i < pred_spans.size(); ++i) {
		double n = 0;
		for (int j = 0; j < true_spans.size(); ++j) {
			n += pred_spans[i].overlap(true_spans[j]);
		}
		propp += float(n)/(pred_spans[i].len());
	}
	for (int i = 0; i < true_spans.size(); ++i) {
		double n = 0;
		for (int j = 0; j < pred_spans.size(); ++j) {
			n += pred_spans[j].overlap(true_spans[i]);
		}
		propr += float(n)/(true_spans[i].len());
	}
}

static void GetSpans(vector<string> labels, vector<Span> &spans, string key, string scheme="IO") {
// BIO
	if (scheme == "BIO") {
		spans.clear();
		int j = 0;
		while (j < labels.size()) {
			if (labels[j].substr(0,2) == "B_" && labels[j].substr(2) == key) {
				Span s;
				s.start = j;
				j++;
				while (j < labels.size() && labels[j] == "I_"+key) {
					j++;
				}
				s.end = j-1;
				spans.push_back(s);
			} else {
				j++;
			}
		}
	} else if (scheme == "IO") {
		// IO
		spans.clear();
		int j = 0;
		while (j < labels.size()) {
			if (labels[j] == key) {
				Span s;
				s.start = j;
				j++;
				while (j < labels.size() && labels[j] == key) {
					j++;
				}
				s.end = j-1;
				spans.push_back(s);
			} else {
				j++;
			}
		}
	}
}
static void GetAllSpans(vector<string> labels, vector<Span> &spans, string scheme="IO") {
// BIO
	if (scheme == "BIO") {
		spans.clear();
		int j = 0;
		while (j < labels.size()) {
			if (labels[j].substr(0,2) == "B_") {
				Span s;
				s.start = j;
				string key = labels[j].substr(2);
				j++;
				while (j < labels.size() && labels[j] == "I_"+key) {
					j++;
				}
				s.end = j-1;
				s.label = key;
				spans.push_back(s);
			} else {
				j++;
			}
		}
	} else if (scheme == "IO") {
		// IO
		spans.clear();
		int j = 0;
		while (j < labels.size()) {
			if (labels[j] != "O") {
				Span s;
				s.start = j;
				string key = labels[j];
				j++;
				while (j < labels.size() && labels[j] == key) {
					j++;
				}
				s.end = j-1;
				s.label = key;
				spans.push_back(s);
			} else {
				j++;
			}
		}
	}
}

static void ComputeMicroAvg(vector<vector<string> > answers, vector<vector<string> > results,
		set<string> &eval_labels, EvalParam &param, std::stringstream &ss) {

	double correctp = 0;
	double truep = 0;
	double predictp = 0;
	double propp = 0;
	double propr = 0;
	double softp = 0;
	double softr = 0;

	for(int i=0; i<answers.size(); ++i) {
		vector<string> labels = answers[i];
		vector<string> pred_labels = results[i];

		for (set<string>::iterator it = eval_labels.begin(); it != eval_labels.end(); ++it) {
			vector<Span> true_spans;
			vector<Span> pred_spans;
			GetSpans(labels, true_spans, *it, param.scheme);
			GetSpans(pred_labels, pred_spans, *it, param.scheme);
			truep += true_spans.size();
			predictp += pred_spans.size();
			if (param.overlap == 0) {
				correctp += ExactMatch(true_spans, pred_spans);
			} else if (param.overlap == 1) {
				SoftMatch(true_spans, pred_spans, softr, softp);
			} else if (param.overlap == 2) {
				ProportionMatch(true_spans, pred_spans, propr, propp);
			}
		}
	}

	if (param.overlap == 0) {
		double pre = predictp==0 ? 0 : correctp/predictp;
		double rec = truep==0 ? 0 : correctp/truep;
		double f = pre+rec==0 ? 0 : 2.0*pre*rec/(pre+rec);
		ss<<"pre: "<<pre<<" rec: "<<rec<<" f: "<<f<<endl;
		cout<<"pre: "<<pre<<" rec: "<<rec<<" f: "<<f<<endl;
	} else if (param.overlap == 1) {
		double pre = predictp==0 ? 0 : softp/predictp;
		double rec = truep==0 ? 0 : softr/truep;
		double f = pre+rec==0 ? 0 : 2.0*pre*rec/(pre+rec);
		ss<<"pre: "<<pre<<" rec: "<<rec<<" f: "<<f<<endl;
		cout<<"pre: "<<pre<<" rec: "<<rec<<" f: "<<f<<endl;
	} else {
		double pre = propp==0 ? 0 : propp/predictp;
		double rec = propr==0 ? 0 : propr/truep;
		double f = pre+rec==0 ? 0 : 2.0*pre*rec/(pre+rec);
		ss<<"pre: "<<pre<<" rec: "<<rec<<" f: "<<f<<endl;
		cout<<"pre: "<<pre<<" rec: "<<rec<<" f: "<<f<<endl;
	}
}

static void ComputeScore(vector<vector<string> > answers, vector<vector<string> > results, EvalParam &param, std::stringstream &ss) {

	vector<string> keylabels;
	Utils::Split(param.key, ',', keylabels);

	for (int l = 0; l < keylabels.size(); ++l) {
		double correctp = 0;
		double truep = 0;
		double predictp = 0;
		double propp = 0;
		double propr = 0;
		double softp = 0;
		double softr = 0;

		int valid_sent = 0;
		string label = keylabels[l];
		cout<<"answer size: "<<answers.size()<<endl;
		for(int i=0; i<answers.size(); ++i) {
			vector<string> labels = answers[i];
			vector<string> pred_labels = results[i];

			vector<Span> true_spans;
			vector<Span> pred_spans;
			GetSpans(labels, true_spans, label, param.scheme);
			GetSpans(pred_labels, pred_spans, label, param.scheme);

			// only evaluated on annotated sentence...
			/*bool valid = false;
			for (int k = 0; k < labels.size(); ++k) {
				if (labels[k] != "O") {
					valid = true;
					break;
				}
			}
			if (!valid) continue;
			valid_sent += 1;*/

			truep += true_spans.size();
			predictp += pred_spans.size();
			if (param.overlap == 0) {
				correctp += ExactMatch(true_spans, pred_spans);
			} else if (param.overlap == 1) {
				SoftMatch(true_spans, pred_spans, softr, softp);
			} else if (param.overlap == 2) {
				ProportionMatch(true_spans, pred_spans, propr, propp);
			}
		}

		//cout<<"valid sent: "<<valid_sent<<endl;

		ss<<"For "<<label<<endl;
		cout<<"For "<<label<<endl;
		if (param.overlap == 0) {
			cout<<"strict match "<<truep<<" "<<predictp<<" "<<correctp<<endl;
			double pre = predictp==0 ? 0 : correctp/predictp;
			double rec = truep==0 ? 0 : correctp/truep;
			double f = pre+rec==0 ? 0 : 2.0*pre*rec/(pre+rec);
			ss<<"pre: "<<pre<<" rec: "<<rec<<" f: "<<f<<endl;
			cout<<"pre: "<<pre<<" rec: "<<rec<<" f: "<<f<<endl;
		} else if (param.overlap == 1) {
			double pre = predictp==0 ? 0 : softp/predictp;
			double rec = truep==0 ? 0 : softr/truep;
			double f = pre+rec==0 ? 0 : 2.0*pre*rec/(pre+rec);
			ss<<"pre: "<<pre<<" rec: "<<rec<<" f: "<<f<<endl;
			cout<<"pre: "<<pre<<" rec: "<<rec<<" f: "<<f<<endl;
		} else {
			double pre = propp==0 ? 0 : propp/predictp;
			double rec = propr==0 ? 0 : propr/truep;
			double f = pre+rec==0 ? 0 : 2.0*pre*rec/(pre+rec);
			ss<<"pre: "<<pre<<" rec: "<<rec<<" f: "<<f<<endl;
			cout<<"pre: "<<pre<<" rec: "<<rec<<" f: "<<f<<endl;
		}
	}
}

static void ComputeAccScore(vector<string> answers, vector<string> results,
		EvalParam &param, std::stringstream &ss) {

	vector<string> keylabels;
	Utils::Split(param.key, ',', keylabels);

	int labelsize = keylabels.size();

	map<string, int> lb2i;
	map<int, string> i2lb;
	for (int i = 0; i < keylabels.size(); ++i) {
		lb2i[keylabels[i]] = i;
		i2lb[i] = keylabels[i];
	}

	map<pair<int, int>, int> confusion_matrix;
	confusion_matrix.clear();
	for (int i = 0; i < labelsize; ++i) {
		for (int j = 0; j < labelsize; ++j)
			confusion_matrix[make_pair(i,j)] = 0;
	}

	int correct = 0;
	int all = 0;
	//ofstream outfile("mallet_test.dat", ios::out);
	for(int i=0; i<answers.size(); ++i) {
		string label_str = answers[i];
		string pred_label_str = results[i];

		int true_label = lb2i[label_str];
		int pred_label = lb2i[pred_label_str];
		confusion_matrix[make_pair(true_label, pred_label)] += 1;

		if (true_label == pred_label) correct++;
		all++;
	}
	cout<<"Token accuracy: "<<(double)correct/all<<endl;
	ss<<"Token accuracy: "<<(double)correct/all<<endl;

	for (int i = 0; i < labelsize; ++i) {
		for (int j = 0; j < labelsize; ++j) {
			ss<<i2lb[i]<<","<<i2lb[j]<<","<<confusion_matrix[make_pair(i,j)]<<" ";
		}
		ss<<endl;
	}
	ss<<endl;

	// compute F1
	for (int i = 0; i < labelsize; ++i) {
		double true_sum = 0.0;
		for (int j = 0; j < labelsize; ++j) {
			true_sum += confusion_matrix[make_pair(i,j)];
		}
		double pred_sum = 0.0;
		for (int j = 0; j < labelsize; ++j) {
			pred_sum += confusion_matrix[make_pair(j,i)];
		}
		double correct = confusion_matrix[make_pair(i,i)];
		double p = correct/pred_sum;
		double r = correct/true_sum;
		double f1 = 2.0*p*r/(p+r);

		ss<<i2lb[i]<<" F1: "<<f1<<endl;
		cout<<i2lb[i]<<" F1: "<<f1<<endl;
	}
	ss<<endl;
}

static double EvaluateSegmentation(vector<vector<Span> > answers,
		vector<vector<Span> > results, EvalParam &param, EvalScore &eval_score) {

	double score = 0.0;

	vector<string> fields;
	Utils::Split(param.key, ',', fields);

	for (int k = 0; k < fields.size(); ++k) {
		string key = fields[k];
		double truep = 0.0;
		double predictp = 0.0;
		double correctp = 0.0;
		double softr = 0.0;
		double softp = 0.0;
		double propr = 0.0;
		double propp = 0.0;

		for(int i=0; i<answers.size(); ++i) {
			vector<Span> true_spans;
			vector<Span> pred_spans;
			for (int j = 0; j < answers[i].size(); ++j) {
				if (answers[i][j].label == key)
					true_spans.push_back(Span(answers[i][j].start, answers[i][j].end));
			}
			for (int j = 0; j < results[i].size(); ++j) {
				if (results[i][j].label == key)
					pred_spans.push_back(Span(results[i][j].start, results[i][j].end));
			}

			truep += true_spans.size();
			predictp += pred_spans.size();
			if (param.overlap == 0) {
				correctp += ExactMatch(true_spans, pred_spans);
			} else if (param.overlap == 1) {
				SoftMatch(true_spans, pred_spans, softr, softp);
			} else if (param.overlap == 2) {
				ProportionMatch(true_spans, pred_spans, propr, propp);
			}
		}

		double pre = 0;
		double rec = 0;
		double f = 0;
		if (param.overlap == 0) {
			pre = predictp==0 ? 0 : correctp/predictp;
			rec = truep==0 ? 0 : correctp/truep;
			f = pre+rec==0 ? 0 : 2.0*pre*rec/(pre+rec);
		} else if (param.overlap == 1) {
			pre = predictp==0 ? 0 : softp/predictp;
			rec = truep==0 ? 0 : softr/truep;
			f = pre+rec==0 ? 0 : 2.0*pre*rec/(pre+rec);
		} else {
			pre = propp==0 ? 0 : propp/predictp;
			rec = propr==0 ? 0 : propr/truep;
			f = pre+rec==0 ? 0 : 2.0*pre*rec/(pre+rec);
		}

		eval_score.extraction.pre = pre;
		eval_score.extraction.rec = rec;
		eval_score.extraction.f1 = f;

		cout<<"For "<<key<<endl;
		cout<<"pre: "<<pre<<" rec: "<<rec<<" f: "<<f<<endl;

		score += f;
	}

	score = (double)score/fields.size();
	return score;
}

static double EvaluateSegmentationPerCategory(vector<vector<EvalSpan> > answers, vector<vector<EvalSpan> > results,
		EvalParam &param) {

	vector<string> keylabels;
	Utils::Split(param.key, ',', keylabels);

	double overall_score = 0;
	//ofstream outfile("mallet_test.dat", ios::out);
	for (int a = 0; a < keylabels.size(); ++a) {
		string key = keylabels[a];
		if (key == "O") continue;

		double correctp = 0;
		double truep = 0;
		double predictp = 0;
		double propp = 0;
		double propr = 0;
		double softp = 0;
		double softr = 0;

		for(int i=0; i<answers.size(); ++i) {
			vector<Span> true_spans;
			vector<Span> pred_spans;
			for (int j = 0; j < answers[i].size(); ++j) {
				for (int k = 0; k < answers[i][j].labels.size(); ++k) {
					if (answers[i][j].labels[k] == key) {
						true_spans.push_back(Span(answers[i][j].start, answers[i][j].end));
						break;
					}
				}
			}
			for (int j = 0; j < results[i].size(); ++j) {
				for (int k = 0; k < results[i][j].labels.size(); ++k) {
					if (results[i][j].labels[k] == key) {
						pred_spans.push_back(Span(results[i][j].start, results[i][j].end));
						break;
					}
				}
			}

			truep += true_spans.size();
			predictp += pred_spans.size();
			if (param.overlap == 0) {
				correctp += ExactMatch(true_spans, pred_spans);
			} else if (param.overlap == 1) {
				SoftMatch(true_spans, pred_spans, softr, softp);
			} else if (param.overlap == 2) {
				ProportionMatch(true_spans, pred_spans, propr, propp);
			}
		}

		cout<<"For label: "<<key<<endl;

		double pre = 0;
		double rec = 0;
		double f = 0;
		if (param.overlap == 0) {
			pre = predictp==0 ? 0 : correctp/predictp;
			rec = truep==0 ? 0 : correctp/truep;
			f = pre+rec==0 ? 0 : 2.0*pre*rec/(pre+rec);
		} else if (param.overlap == 1) {
			pre = predictp==0 ? 0 : softp/predictp;
			rec = truep==0 ? 0 : softr/truep;
			f = pre+rec==0 ? 0 : 2.0*pre*rec/(pre+rec);
		} else {
			pre = propp==0 ? 0 : propp/predictp;
			rec = propr==0 ? 0 : propr/truep;
			f = pre+rec==0 ? 0 : 2.0*pre*rec/(pre+rec);
		}

		cout<<"pre: "<<pre<<" rec: "<<rec<<" f: "<<f<<endl;
		overall_score += f;
	}

	return overall_score;
}


#endif /* EVAL_H_ */
