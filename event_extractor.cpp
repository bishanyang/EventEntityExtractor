#include "Logger.h"
#include "Config.h"

#include <vector>
#include <string>
#include "Logger.h"
#include "Config.h"
#include "FeatureDict.h"
#include "Dataset.h"

#include "TreeCRFModel.h"
#include "SequenceModel.h"
#include "AD3Inference.h"
#include "CRF/feature_index.h"

using namespace std;

FeatureDict global_feadict;
CRFPP::DecoderFeatureIndex *global_entity_mention_feature_index = NULL;
CRFPP::DecoderFeatureIndex *global_event_mention_feature_index = NULL;
TreeCRFPredictor global_tree_predictor;
CRFPredictor global_evpair_predictor;
double neg_infinity = -std::numeric_limits<double>::infinity();

void LoadParam(Config props, Param &param) {
	param.nbest = props.GetIntProperty("nbest");

	param.modelpath = props.GetProperty("modelpath");
	param.resourcepath = props.GetProperty("resourcepath");

	param.datapath = props.GetProperty("datapath");
	param.outputpath = props.GetProperty("outputpath");
}

string ReadFile(string filename) {
	string content = "";
	ifstream infile(filename.c_str(), ios::in);
	if (!infile) return content;
	string str;
	while(getline(infile, str)) {
		content += str + "\n";
	}
	infile.close();
	return content.substr(0, content.size()-1);
}

void GenerateEntityCandidates(Param param, CRFPP::DecoderFeatureIndex *entity_mention_feature_index, Document *doc) {
	CRFPredictor entity_mention_predictor;
	entity_mention_predictor.initParam(param);
	entity_mention_predictor.scheme = "BIO";
	entity_mention_predictor.feature_index = entity_mention_feature_index;

	entity_mention_predictor.PredictEntityCandidates(doc);
	for (int s = 0; s<doc->sentences.size(); ++s) {
		Sentence *sent = doc->sentences[s];
		for (int i = 0; i < sent->cand_entity_mentions.size(); ++i) {
			Mention *men = sent->cand_entity_mentions[i];
			men->index = doc->doc_id+"#"+Utils::int2string(sent->sent_id)+"#"+Utils::int2string(men->start)+"#"+Utils::int2string(men->end);
		}
	}
}

void GenerateEventCandidates(Param param, CRFPP::DecoderFeatureIndex *event_mention_feature_index, Document *doc) {
	CRFPredictor event_mention_predictor;
	event_mention_predictor.scheme = "BIO";
	event_mention_predictor.initParam(param);
	event_mention_predictor.feature_index = event_mention_feature_index;

	event_mention_predictor.PredictEventCandidates(doc);

	for (int s = 0; s<doc->sentences.size(); ++s) {
		Sentence *sent = doc->sentences[s];
		for (int i = 0; i < sent->cand_event_mentions.size(); ++i) {
			EventMention *event = sent->cand_event_mentions[i];
			event->index = doc->doc_id+"#"+Utils::int2string(sent->sent_id)+"#"+Utils::int2string(event->start)+"#"+Utils::int2string(event->end);
		}
	}
}

void LoadEventLabels(string input_content, set<string> &labels) {
	vector<string> lines;
	Utils::Split(input_content, '\n', lines);

	labels.clear();
	labels.insert("O");
	for (int l = 0; l < lines.size(); ++l) {
		string line = lines[l];

		int i = line.find("\t");
		string event = line.substr(0, i);
		labels.insert(event);
	}
}

void LoadEventRoles(string input_content, CRFPP::FeatureIndex* feature_index) {
	vector<string> lines;
	Utils::Split(input_content, '\n', lines);

	for (int l = 0; l < lines.size(); ++l) {
		string line = lines[l];

		int i = line.find("\t");
		string event = line.substr(0, i);
		int y = feature_index->lb2i[event];

		string values = line.substr(i+1);
		vector<string> fields;
		Utils::Split(values, ' ', fields);
		set<int> roles;
 		for (int j = 0; j < fields.size(); ++j) {
			int role = feature_index->arglb2i[fields[j]];
			roles.insert(role);
		}

 		feature_index->event_to_roles[y] = roles;
 		//cout<<"EventRole: "<<y<<" "<<roles.size()<<endl;
	}
}

void LoadNERRoles(string input_content, CRFPP::FeatureIndex* feature_index) {
	vector<string> lines;
	Utils::Split(input_content, '\n', lines);

	for (int l = 0; l < lines.size(); ++l) {
		string line = lines[l];

		int i = line.find("\t");
		string role = line.substr(0, i);
		int y = feature_index->arglb2i[role];

		string values = line.substr(i+1);
		vector<string> fields;
		Utils::Split(values, ' ', fields);
		set<int> ners;
 		for (int j = 0; j < fields.size(); ++j) {
			int k = fields[j].find(',');
			int ner = feature_index->argattrlb2i[fields[j].substr(k+1)];
			ners.insert(ner);
		}

 		feature_index->role_to_ners[y] = ners;
 		//cout<<"NER role: "<<y<<" "<<ners.size()<<endl;
	}
}

Dataset *LoadDataFile(string datafile) {
	vector<string> filenames;
	filenames.push_back(datafile);

	Dataset *data = new Dataset();
	data->LoadCoNLLData(filenames);

	return data;
}

void LoadSubset(Dataset *wholedata, string docidfile, Dataset *data) {
	set<string> doclist;
	ifstream infile(docidfile.c_str(), ios::in);
	string str;
	while(getline(infile, str)) {
		doclist.insert(str);
	}
	infile.close();

	for(int i = 0; i < wholedata->documents.size(); ++i) {
		string docid = wholedata->documents[i]->doc_id;
		if (doclist.find(docid) != doclist.end()) {
			data->documents.push_back(wholedata->documents[i]);
			data->id2doc[docid] = wholedata->documents[i];
		}
	}

	// build sentences and id2docs
	data->BuildSentences();
}

void TrainEntityTagger(Param param, string modelfile) {
	string datapath = param.datapath;
	Dataset *data = LoadDataFile(datapath+"ace2005_entity.conll");
	//Dataset *data = LoadDataFile(datapath+"ace2005_event.conll");

	string embeddingfile = param.resourcepath + "pretrained_ace_embeddings";
	string embeddingstr = ReadFile(embeddingfile);

	string featurefile = param.resourcepath + "featuredict";
	string featurestr = ReadFile(featurefile);

	FeatureDict feadict;
	feadict.InitializeDict(featurestr);
	feadict.LoadEmbeddings(embeddingstr);

	for (int i = 0; i < data->sentences.size(); ++i) {
		data->sentences[i]->SetFeatureDict(&feadict);
		data->sentences[i]->NormalizeWords();
	}

	Dataset *traindata = new Dataset();
	string trainfile = datapath + "train.filelist";
	Dataset *testdata = new Dataset();
	string testfile = datapath + "test.filelist";
	LoadSubset(data, trainfile, traindata);
	LoadSubset(data, testfile, testdata);

	cout<<"Training data "<<traindata->documents.size()<<" "<<traindata->sentences.size()<<endl;
	cout<<"Test data "<<testdata->documents.size()<<" "<<testdata->sentences.size()<<endl;

	string scheme = "BIO";

    // Training
	CRFLearner crf_extractor;
	crf_extractor.data = traindata;
	crf_extractor.InitEncoder(20);

	set<string> labelset;
	crf_extractor.data->CollectLabels(scheme, labelset);

	cout<<"label set "<<labelset.size()<<endl;
	crf_extractor.encoder.feature_index->LoadLabels(labelset);

	crf_extractor.encoder.feature_index->CopyEmbeddings(feadict.word_lookup_table, feadict.word_embeddings);
	crf_extractor.encoder.feature_index->CopyGazetteer(feadict.gazetteer_unigrams, feadict.gazetteer_bigrams);

	cout<<crf_extractor.encoder.feature_index->word_lookup_table.size()<<endl;
	cout<<crf_extractor.encoder.feature_index->gazetteer_unigrams.size()<<endl;

	crf_extractor.Train(modelfile);

	// Inference
	CRFPredictor chain_predictor;
	chain_predictor.feature_index = new CRFPP::DecoderFeatureIndex();

	string model_content = ReadFile(modelfile);
	chain_predictor.feature_index->open(model_content.c_str());

	chain_predictor.feature_index->CopyEmbeddings(feadict.word_lookup_table, feadict.word_embeddings);
	chain_predictor.feature_index->CopyGazetteer(feadict.gazetteer_unigrams, feadict.gazetteer_bigrams);

	param.nbest = 1;
	chain_predictor.scheme = "BIO";
	chain_predictor.eval_key = "PER,LOC,ORG,WEA,GPE,VALUE,FAC,VEH,TIME"; //"PER,LOC,ORG,WEA,GPE,VALUE,FAC,VEH,TIME";
	chain_predictor.testset = testdata;
	chain_predictor.initParam(param);

	string outputfile = param.datapath + "entity_crf_result";
	chain_predictor.TaggingPrediction(outputfile);
	//string outputfile = param.datapath + "test_entity_crf_k_50";
	//chain_predictor.NbestMentionPrediction(outputfile);

}

void TrainEventTagger(Param param, string modelfile) {
	string datapath = param.datapath;
	Dataset *data = LoadDataFile(datapath+"ace2005_event.conll");

	string embeddingfile = param.resourcepath + "pretrained_ace_embeddings";
	string embeddingstr = ReadFile(embeddingfile);

	string featurefile = param.resourcepath + "featuredict";
	string featurestr = ReadFile(featurefile);

	FeatureDict feadict;
	feadict.InitializeDict(featurestr);
	feadict.LoadEmbeddings(embeddingstr);

	for (int i = 0; i < data->sentences.size(); ++i) {
		data->sentences[i]->SetFeatureDict(&feadict);
		data->sentences[i]->NormalizeWords();
	}

	Dataset *traindata = new Dataset();
	Dataset *testdata = new Dataset();
	string trainfile = datapath + "event.train.filelist";
	string testfile = datapath + "test.filelist";
	LoadSubset(data, trainfile, traindata);
	LoadSubset(data, testfile, testdata);

	cout<<"Training data "<<traindata->documents.size()<<" "<<traindata->sentences.size()<<endl;
	cout<<"Test data "<<testdata->documents.size()<<" "<<testdata->sentences.size()<<endl;

	CRFLearner crf_extractor;
	crf_extractor.data = traindata;
	crf_extractor.InitEncoder(20);

	set<string> labelset;
	crf_extractor.data->CollectLabels("BIO", labelset);
	cout<<"label set "<<labelset.size()<<endl;
	crf_extractor.encoder.feature_index->LoadLabels(labelset);
	crf_extractor.encoder.feature_index->CopyEmbeddings(feadict.word_lookup_table, feadict.word_embeddings);

	crf_extractor.Train(modelfile);

	// Prediction
//	CRFPredictor chain_predictor;
//	chain_predictor.feature_index = new CRFPP::DecoderFeatureIndex();
//
//	string model_content = ReadFile(modelfile);
//	chain_predictor.feature_index->open(model_content.c_str());
//	chain_predictor.feature_index->CopyEmbeddings(feadict.word_lookup_table, feadict.word_embeddings);
//
//	param.nbest = 1;
//	chain_predictor.scheme = "BIO";
//	chain_predictor.eval_key = "EVENT";
//	chain_predictor.testset = testdata;
//	chain_predictor.initParam(param);
//
//	string outputfile = param.datapath + "test_event_crf";
//	chain_predictor.TaggingPrediction(outputfile);
	//chain_predictor.NbestMentionPrediction(outputfile);
}

void TrainTreeTagger(Param param, string modelfile) {
	string datapath = param.datapath;
	Dataset *data = LoadDataFile(datapath + "ace2005.conll");

	string embeddingfile = param.resourcepath + "pretrained_ace_embeddings";
	string embeddingstr = ReadFile(embeddingfile);
	string featurefile = param.resourcepath + "featuredict";
	string featurestr = ReadFile(featurefile);

	string eventrolefile = param.resourcepath + "subtype_role_dict.txt";
	string eventrole_content = ReadFile(eventrolefile);
	string rolenerfile = param.resourcepath + "argrole_dict.txt";
	string rolener_content = ReadFile(rolenerfile);

	FeatureDict feadict;
	feadict.InitializeDict(featurestr);
	feadict.LoadEmbeddings(embeddingstr);

	for (int i = 0; i < data->sentences.size(); ++i) {
		data->sentences[i]->SetFeatureDict(&feadict);
		data->sentences[i]->NormalizeWords();
	}

	data->LoadDependencies(datapath + "dependencies.txt");

	//data->LoadNELLEntities(datapath + "nell.predict.mentions");

	data->LoadStanfordEntities(datapath + "stanford.predict.mentions");

	data->LoadCRFEntities(datapath + "train_crf_entity_prediction_k_1");
	data->LoadCRFEntities(datapath + "test_crf_entity_prediction_k_1");

	data->LoadEventStructures(datapath + "train.crf.data");
	data->LoadEventStructures(datapath + "dev.crf.data");
	data->LoadEventStructures(datapath + "test.crf.data");

	Dataset *traindata = new Dataset();
	Dataset *testdata = new Dataset();
	string trainfile = datapath + "event.train.filelist";
	string testfile = datapath + "test.filelist";
	LoadSubset(data, trainfile, traindata);
	LoadSubset(data, testfile, testdata);

	cout<<"Training data "<<traindata->documents.size()<<" "<<traindata->sentences.size()<<endl;
	cout<<"Test data "<<testdata->documents.size()<<" "<<testdata->sentences.size()<<endl;

	TreeCRFLearner crf_extractor;
	crf_extractor.data = traindata;
	crf_extractor.InitEncoder(20);
	crf_extractor.LoadLabels();

	crf_extractor.encoder.feature_index->CopyEmbeddings(feadict.word_lookup_table, feadict.word_embeddings);
	LoadEventRoles(eventrole_content, crf_extractor.encoder.feature_index);
	LoadNERRoles(rolener_content, crf_extractor.encoder.feature_index);

	crf_extractor.Train(modelfile);
	//crf_extractor.outputModel(modelfile);

	// Prediction
//	TreeCRFPredictor tree_predictor;
//	tree_predictor.data = testdata;
//
//	string model_content = ReadFile(modelfile);
//	tree_predictor.LoadModel(model_content);
//
//	tree_predictor.feature_index->CopyEmbeddings(feadict.word_lookup_table, feadict.word_embeddings);
//	LoadEventRoles(eventrole_content, tree_predictor.feature_index);
//	LoadNERRoles(rolener_content, tree_predictor.feature_index);

//	string outputfile = datapath + "treecrf_prediction.output";
//	tree_predictor.Predict(outputfile);
}

void TrainEventSeqTagger(Param param, string modelfile) {
	string datapath = param.datapath;
	Dataset *data = LoadDataFile(datapath+"ace2005_event.conll");
	data->LoadDependencies(datapath + "dependencies.txt");
	data->LoadEventStructures(datapath + "train.crf.data");
	data->LoadEventStructures(datapath + "dev.crf.data");
	data->LoadEventStructures(datapath + "test.crf.data");

	string embeddingfile = param.resourcepath + "pretrained_ace_embeddings";
	string embeddingstr = ReadFile(embeddingfile);

	string featurefile = param.resourcepath + "featuredict";
	string featurestr = ReadFile(featurefile);
	FeatureDict feadict;
	feadict.InitializeDict(featurestr);
	feadict.LoadEmbeddings(embeddingstr);

	string eventrolefile = param.resourcepath + "subtype_role_dict.txt";
	string eventrole_content = ReadFile(eventrolefile);

	for (int i = 0; i < data->sentences.size(); ++i) {
		data->sentences[i]->SetFeatureDict(&feadict);
		data->sentences[i]->NormalizeWords();
	}

	Dataset *traindata = new Dataset();
	Dataset *testdata = new Dataset();
	string trainfile = datapath + "event.train.filelist";
	string testfile = datapath + "test.filelist";
	LoadSubset(data, trainfile, traindata);
	LoadSubset(data, testfile, testdata);

	cout<<"Training data "<<traindata->documents.size()<<" "<<traindata->sentences.size()<<endl;
	cout<<"Test data "<<testdata->documents.size()<<" "<<testdata->sentences.size()<<endl;

	// ==== Train ===
	CRFLearner crf_extractor;
	crf_extractor.data = traindata;
	crf_extractor.InitEncoder(20);

	set<string> labelset;
	LoadEventLabels(eventrole_content, labelset);
	cout<<"label set "<<labelset.size()<<endl;
	crf_extractor.encoder.feature_index->LoadLabels(labelset);
	crf_extractor.encoder.feature_index->CopyEmbeddings(feadict.word_lookup_table, feadict.word_embeddings);

	crf_extractor.TrainEventSeq(modelfile);

	// ===== Test ===
//	CRFPredictor chain_predictor;
//	chain_predictor.feature_index = new CRFPP::DecoderFeatureIndex();
//	string model_content = ReadFile(modelfile);
//	chain_predictor.feature_index->open(model_content.c_str());
//	chain_predictor.feature_index->CopyEmbeddings(feadict.word_lookup_table, feadict.word_embeddings);
//
//	param.nbest = 1;
//	chain_predictor.scheme = "IO";
//	chain_predictor.eval_key = "O";
//	for (int i = 1; i < chain_predictor.feature_index->ysize(); ++i) {
//		chain_predictor.eval_key += "," + chain_predictor.feature_index->i2lb[i];
//	}
//	chain_predictor.testset = testdata;
//	chain_predictor.initParam(param);
//
//	string outputfile = param.datapath + "event_seq_result";
//	chain_predictor.EventSeqPrediction(outputfile);
}

void LoadEmbeddings(string embedding_content, map<string, int> vocab,
		vector<vector<float> > &word_embeddings, map<string, int> &word_lookup_table) {
	word_lookup_table.clear();
	word_embeddings.clear();

	vector<string> lines;
	Utils::Split(embedding_content, '\n', lines);

	for (int l = 0; l < lines.size(); ++l) {
		string str = lines[l];
		int index = str.find('\t');
		string word = str.substr(0, index);
		if (vocab.find(word) != vocab.end()) {
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
}

void BuildEventInstances(Document *doc, CRFPP::DecoderFeatureIndex *local_feature_index,
		map<string, EventInstance *> &event_instances) {
	int pred_num_states = local_feature_index->ysize();
	int role_num_states = local_feature_index->argysize();
	int attr_num_states = local_feature_index->argattrsize();

	for (int s = 0; s < doc->sentences.size(); ++s) {
		Sentence *sent = doc->sentences[s];
		for (int i = 0; i < sent->cand_event_mentions.size(); ++i) {
			EventMention *event = sent->cand_event_mentions[i];

			EventInstance *inst = new EventInstance();
			inst->docid = doc->doc_id;
			inst->sent = sent;
			inst->event = event;
			inst->gold_subtype = "O";
			inst->pred_subtype = "O";
			inst->predicate_index = event->index;
			inst->pred_potential = new Potential();
			for (size_t k = 0; k < sent->cand_entity_mentions.size(); ++k) {
				Mention *men = sent->cand_entity_mentions[k];
				string gold_role = event->GetGoldArgRole(i);

				inst->arguments.push_back(men);
				inst->argument_indices.push_back(men->index);

				inst->gold_roles.push_back(gold_role);
				inst->pred_roles.push_back("O");
				inst->role_potentials.push_back(new Potential());
				inst->role_edge_potentials.push_back(new Potential());

				inst->gold_attrs.push_back(men->gold_ner_type);
				inst->pred_attrs.push_back("O");
				inst->attr_potentials.push_back(new Potential());
				inst->attr_edge_potentials.push_back(new Potential());
			}

			local_feature_index->clear();
			CRFPP::TreeTagger x;
			x.EventToTreeTagger(event, sent, local_feature_index, false);
			x.Inference();

			inst->pred_potential->potentials.resize(local_feature_index->ysize(), 0);
			for (int j = 0; j < x.predicate_nodes.size(); ++j) {
				int py = x.predicate_nodes[j]->y;
				inst->pred_potential->potentials[py] = log(x.predicate_nodes[j]->marginal);
			}

			for (int j = 0; j < inst->arguments.size(); ++j) {
				inst->role_potentials[j]->potentials.resize(local_feature_index->argysize(), neg_infinity);
				inst->role_edge_potentials[j]->potentials.resize(pred_num_states*role_num_states, neg_infinity);
				for (int k = 0; k < x.argument_role_nodes[j].size(); ++k) {
					int ry = x.argument_role_nodes[j][k]->y;
					inst->role_potentials[j]->potentials[ry] = log(x.argument_role_nodes[j][k]->marginal);

					for (int k1 = 0; k1 < x.argument_role_nodes[j][k]->parent_path.size(); ++k1) {
						CRFPP::TreeTaggerPath* path = x.argument_role_nodes[j][k]->parent_path[k1];
						int py = path->parent->y;
						inst->role_edge_potentials[j]->potentials[py*role_num_states+ry] = log(path->marginal);
					}
				}
			}

			for (int j = 0; j < inst->arguments.size(); ++j) {
				inst->attr_potentials[j]->potentials.resize(attr_num_states, neg_infinity);
				inst->attr_edge_potentials[j]->potentials.resize(role_num_states*attr_num_states, neg_infinity);
				for (int k = 0; k < x.argument_attr_nodes[j].size(); ++k) {
					int ay = x.argument_attr_nodes[j][k]->y;
					inst->attr_potentials[j]->potentials[ay] = log(x.argument_attr_nodes[j][k]->marginal);

					for (int k1 = 0; k1 < x.argument_attr_nodes[j][k]->parent_path.size(); ++k1) {
						CRFPP::TreeTaggerPath* path = x.argument_attr_nodes[j][k]->parent_path[k1];
						int ry = path->parent->y;
						inst->attr_edge_potentials[j]->potentials[ry*attr_num_states+ay] = log(path->marginal);
					}
				}
			}

			event_instances[event->index] = inst;
		}
	}
}

void BuildEntityInstances(Document *doc, map<string, map<string, double> > &mention_to_nerscores) {
	for (int s = 0; s < doc->sentences.size(); ++s) {
		Sentence *sent = doc->sentences[s];
		for (size_t k = 0; k < sent->cand_entity_mentions.size(); ++k) {
			Mention *men = sent->cand_entity_mentions[k];
			map<string, double> potentials;
			for (size_t j = 0; j < men->labels.size(); ++j) {
				potentials[men->labels[j]] = men->values[j];
			}
			mention_to_nerscores[men->index] = potentials;
		}
	}
}

string Inference(Document *doc) {
	string result = "";

	Param param;
	param.nbest = 50;
	GenerateEntityCandidates(param, global_entity_mention_feature_index, doc);

	param.nbest = 10;
	GenerateEventCandidates(param, global_event_mention_feature_index, doc);

	map<string, int> trigger_dict = global_tree_predictor.feature_index->lb2i;
	map<int, string> trigger_i2lb = global_tree_predictor.feature_index->i2lb;
	int pred_num_states = trigger_dict.size();

	map<int, string> role_i2lb = global_tree_predictor.feature_index->argi2lb;
	map<int, string> attr_i2lb = global_tree_predictor.feature_index->argattri2lb;
	int role_num_states = role_i2lb.size();
	int attr_num_states = attr_i2lb.size();

	map<string, EventInstance *> event_instances;
	BuildEventInstances(doc, global_tree_predictor.feature_index, event_instances);

	map<string, map<string, double> > mention_to_nerscores;
	BuildEntityInstances(doc, mention_to_nerscores);
	map<string, pair<int, string> > entity_map;

	AD3Inference ad3;
	for (map<string, map<string, double> >::iterator ait = mention_to_nerscores.begin();
			ait != mention_to_nerscores.end(); ++ait) {
		AD3::MultiVariable* entity_variable = ad3.factor_graph.CreateMultiVariable(attr_num_states);
		int varid = ad3.factor_graph.GetNumMultiVariables()-1;
		pair<int, string> var;
		var.first = varid;
		var.second = "O";
		entity_map[ait->first] = var;

		map<string, double> scores = mention_to_nerscores[ait->first];
		for (int a = 0; a < attr_num_states; ++a) {
			string nertype = attr_i2lb[a];
			//cout<<ait->first<<" "<<nertype<<" "<<scores[nertype]<<endl;
			entity_variable->SetLogPotential(a, log(scores[nertype]));
		}
	}

	// create variables and factors
	for (map<string, EventInstance*>::iterator eit = event_instances.begin();
			eit != event_instances.end(); ++eit) {
		EventInstance* inst = eit->second;

		AD3::MultiVariable* pred_variable = ad3.factor_graph.CreateMultiVariable(pred_num_states);
		for (int i = 0; i< pred_num_states; ++i)
			pred_variable->SetLogPotential(i, inst->pred_potential->potentials[i]);

		int varid = ad3.factor_graph.GetNumMultiVariables()-1;
		ad3.trigger_to_varid[inst->predicate_index] = varid;

		vector<AD3::MultiVariable* > arg_variables(inst->arguments.size());
		for (int j = 0; j < inst->arguments.size(); ++j) {
			arg_variables[j] = ad3.factor_graph.CreateMultiVariable(role_num_states);

			vector<AD3::MultiVariable*> pair_variables(2);
			pair_variables[0] = pred_variable;
			pair_variables[1] = arg_variables[j];
			ad3.factor_graph.CreateFactorDense(pair_variables, inst->role_edge_potentials[j]->potentials);
		}

		for (int j = 0; j < inst->arguments.size(); ++j) {
			AD3::MultiVariable* entity_var = ad3.factor_graph.GetMultiVariable(entity_map[inst->argument_indices[j]].first);

			vector<AD3::MultiVariable*> pair_variables(2);
			pair_variables[0] = arg_variables[j];
			pair_variables[1] = entity_var;

			ad3.factor_graph.CreateFactorDense(pair_variables, inst->attr_edge_potentials[j]->potentials);
		}
	}

	ad3.AddTriggerSequencePotentials(doc, global_evpair_predictor, trigger_dict, pred_num_states);

	// ad3 decoding whole document
	// Decoding.
	vector<double> posteriors;
	vector<double> additional_posteriors;
	double value;

	// Run AD3.
	//cout << "Running AD3..." << endl;
	ad3.factor_graph.SetVerbosity(0);
	ad3.factor_graph.SetEtaAD3(0.1);
	ad3.factor_graph.AdaptEtaAD3(true);
	ad3.factor_graph.SetMaxIterationsAD3(1000);
	ad3.factor_graph.SolveLPMAPWithAD3(&posteriors, &additional_posteriors, &value);

	// Read solutions
	int offset = 0;
	int idx = 0;
	for (map<string, map<string, double> >::iterator ait = mention_to_nerscores.begin();
					ait != mention_to_nerscores.end(); ++ait) {
		int best = 0;
		for (int k = 0; k < attr_num_states; ++k)
		if (posteriors[offset + k] > posteriors[offset + best]) {
			best = k;
		}
		entity_map[ait->first].second = attr_i2lb[best];
		offset += attr_num_states;
		idx++;
	}

	for (map<string, EventInstance*>::iterator eit = event_instances.begin(); eit != event_instances.end(); ++eit) {
		EventInstance* inst = eit->second;
		int best = 0;
		for (int k = 0; k < pred_num_states; ++k)
		if (posteriors[offset + k] > posteriors[offset + best]) {
			best = k;
		}
		inst->pred_subtype = trigger_i2lb[best];
		offset += pred_num_states;
		idx++;

		for (int j = 0; j < inst->arguments.size(); ++j) {
			int best = 0;
			for (int k = 0; k < role_num_states; ++k) {
				if (posteriors[offset + k] > posteriors[offset + best]) {
					best = k;
				}
			}
			inst->pred_roles[j] = role_i2lb[best];
			offset += role_num_states;
			idx++;
		}

		for (int j = 0; j < inst->arguments.size(); ++j) {
			inst->pred_attrs[j] = entity_map[inst->argument_indices[j]].second;
		}
	}

	std::stringstream ss;
	for (map<string, EventInstance*>::iterator eit = event_instances.begin(); eit != event_instances.end(); ++eit) {
		EventInstance* inst = eit->second;
		ss<<inst->predicate_index<<"\t"<<inst->pred_subtype<<endl;
		for (int j = 0; j < inst->arguments.size(); ++j) {
			ss<<inst->argument_indices[j]<<"#"<<inst->pred_attrs[j]<<"\t"<<inst->pred_roles[j]<<endl;
		}
		ss<<endl;
	}
	result = ss.str();

	return result;
}

void train(Param param) {
	//string modelfile = param.modelpath + "entity_mention_model";
	//TrainEntityTagger(param, modelfile);

	//string modelfile = param.modelpath + "event_mention_model";
	//TrainEventTagger(param, modelfile);

//	string modelfile = param.modelpath + "event_pair_model";
//	TrainEventSeqTagger(param, modelfile);

	string modelfile = param.modelpath + "event_argument_model";
	TrainTreeTagger(param, modelfile);
}

void test(Param param) {
	string entity_model = ReadFile(param.modelpath + "entity_mention_model");
	string event_model = ReadFile(param.modelpath + "event_mention_model");
	string tree_model = ReadFile(param.modelpath + "event_argument_model");
	string event_pair_model = ReadFile(param.modelpath + "event_pair_model");

	string datapath = param.datapath;
	Dataset *testdata = LoadDataFile(datapath + "ace.test.conll");
	testdata->LoadDependencies(datapath + "ace.test.dependencies.txt");
	testdata->LoadStanfordEntities(datapath + "ace.test.stanford.ner.txt");
	cout<<"Number of test documents: "<<testdata->documents.size()<<" sentences: "<<testdata->sentences.size()<<endl;

	string embeddingfile = param.resourcepath + "pretrained_ace_embeddings";
	string embeddingstr = ReadFile(embeddingfile);
	string featurefile = param.resourcepath + "featuredict";
	string featurestr = ReadFile(featurefile);

	string eventrolefile = param.resourcepath + "subtype_role_dict.txt";
	string eventrole_content = ReadFile(eventrolefile);
	string rolenerfile = param.resourcepath + "argrole_dict.txt";
	string rolener_content = ReadFile(rolenerfile);

	global_feadict.InitializeDict(featurestr);
	global_feadict.LoadEmbeddings(embeddingstr);

	for (int i = 0; i < testdata->sentences.size(); ++i) {
		testdata->sentences[i]->SetFeatureDict(&global_feadict);
		testdata->sentences[i]->NormalizeWords();
	}

	global_entity_mention_feature_index = new CRFPP::DecoderFeatureIndex();
	global_entity_mention_feature_index->open(entity_model);
	if (global_entity_mention_feature_index != NULL) {
		global_entity_mention_feature_index->CopyGazetteer(global_feadict.gazetteer_unigrams, global_feadict.gazetteer_bigrams);
		global_entity_mention_feature_index->CopyEmbeddings(global_feadict.word_lookup_table, global_feadict.word_embeddings);
	}

	global_event_mention_feature_index = new CRFPP::DecoderFeatureIndex();
	global_event_mention_feature_index->open(event_model);
	if (global_event_mention_feature_index != NULL) {
		global_event_mention_feature_index->CopyEmbeddings(global_feadict.word_lookup_table, global_feadict.word_embeddings);
	}

	global_tree_predictor.LoadModel(tree_model);
	LoadEventRoles(eventrole_content, global_tree_predictor.feature_index);
	LoadNERRoles(rolener_content, global_tree_predictor.feature_index);
	global_tree_predictor.feature_index->CopyEmbeddings(global_feadict.word_lookup_table, global_feadict.word_embeddings);

	global_evpair_predictor.feature_index = new CRFPP::DecoderFeatureIndex();
	global_evpair_predictor.feature_index->open(event_pair_model);
	global_evpair_predictor.feature_index->CopyEmbeddings(global_feadict.word_lookup_table, global_feadict.word_embeddings);

	string outputfile = param.outputpath + "joint.results.txt";
	ofstream resfile(outputfile.c_str(), ios::out);
	for (int i = 0; i < testdata->documents.size(); ++i) {
		resfile<<Inference(testdata->documents[i]);
	}
	resfile.close();
}

int main(int argc, const char** argv) {
	string configstr = ReadFile("default.config");
	Config props;
	props.ParseStr(configstr);

	Param param;
	LoadParam(props, param);

	//train(param);

	test(param);

	return 0;
}

