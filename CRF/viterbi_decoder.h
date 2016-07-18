/*
 * viterbi_decoder.h
 *
 *  Created on: Feb 2, 2014
 *      Author: bishan
 */

#ifndef VITERBI_DECODER_H_
#define VITERBI_DECODER_H_
#include <iostream>
#include <vector>
#include <queue>
#include "node.h"
#include "scoped_ptr.h"
#include "freelist.h"

using namespace std;

namespace CRFPP {

struct QueueElement {
   Node *node;
   QueueElement *next;
   double fx;
   double gx;
};

class QueueElementComp {
public:
	const bool operator()(QueueElement *q1,
			QueueElement *q2)
	{ return(q1->fx > q2->fx); }
};

class ViterbiDecoder {
public:
	ViterbiDecoder() : nbest_(0), inference_type(0), cost_(0),
	xsize_(0), ysize_(0) {}
	~ViterbiDecoder() {
	}

private:
	scoped_ptr<std::priority_queue <QueueElement*, std::vector <QueueElement *>,
	        QueueElementComp> > agenda_;
	scoped_ptr<FreeList <QueueElement> > nbest_freelist_;

	int xsize_;
	int ysize_;
	int           inference_type ;

	void viterbiwithConstraints(vector<vector<Node*> > &node_,
			vector<unsigned short int> &result_);
	void viterbi(vector<vector<Node*> > &node_,
			vector<unsigned short int> &result_);
	bool initNbest(vector<vector<Node*> > &node_);

public:
	unsigned int nbest_  : 11;
	double        cost_;
	bool next(vector<unsigned short int> &result_);
	void inference(int xsize, int ysize, vector<vector<Node*> > &chain,
			vector<unsigned short int> &result_);

	size_t nbest() const { return nbest_; }
	void set_nbest(size_t nbest) { nbest_ = nbest; }
};

} /* namespace CRFPP */
#endif /* VITERBI_DECODER_H_ */
