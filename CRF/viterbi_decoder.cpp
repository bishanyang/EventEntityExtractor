/*
 * viterbi_decoder.cpp
 *
 *  Created on: Feb 2, 2014
 *      Author: bishan
 */

#include "viterbi_decoder.h"

namespace CRFPP {

bool ViterbiDecoder::initNbest(vector<vector<Node*> > &node_) {
  if (!agenda_.get()) {
    agenda_.reset(new std::priority_queue <QueueElement*,
                  std::vector<QueueElement *>, QueueElementComp>);
    nbest_freelist_.reset(new FreeList <QueueElement>(128));
  }

  nbest_freelist_->free();
  while (!agenda_->empty()) agenda_->pop();   // make empty

  size_t k = xsize_-1;
  for (size_t i = 0; i < ysize_; ++i) {
    QueueElement *eos = nbest_freelist_->alloc();
    eos->node = node_[k][i];
    eos->fx = -node_[k][i]->bestCost;
    eos->gx = -node_[k][i]->cost;
    eos->next = 0;
    agenda_->push(eos);
  }

  return true;
}

bool ViterbiDecoder::next(vector<unsigned short int> &result_) {
  while (!agenda_->empty()) {
    QueueElement *top = agenda_->top();
    agenda_->pop();
    Node *rnode = top->node;

    if (rnode->x == 0) {
      for (QueueElement *n = top; n; n = n->next)
        result_[n->node->x] = n->node->y;
      cost_ = top->gx;
      return true;
    }

    for (const_Path_iterator it = rnode->lpath.begin();
         it != rnode->lpath.end(); ++it) {
      QueueElement *n =nbest_freelist_->alloc();
      n->node = (*it)->lnode;
      n->gx   = -(*it)->lnode->cost     -(*it)->cost +  top->gx;
      n->fx   = -(*it)->lnode->bestCost -(*it)->cost +  top->gx;
      //          |              h(x)                 |  |  g(x)  |
      n->next = top;
      agenda_->push(n);
    }
  }

  return 0;
}

void ViterbiDecoder::viterbiwithConstraints(vector<vector<Node*> > &node_,
		vector<unsigned short int> &result_) {
	 for (size_t i = 0;   i < xsize_; ++i) {
		for (size_t j = 0; j < ysize_; ++j)
		{
			double bestc = -1e37;
			Node *best = 0;
			const std::vector<Path *> &lpath = node_[i][j]->lpath;
			for (const_Path_iterator it = lpath.begin(); it != lpath.end(); ++it)
			{
			  double cost = (*it)->lnode->bestCost +(*it)->cost + (*it)->constraintcost + node_[i][j]->cost +node_[i][j]->constraintcost;
			  if (cost > bestc) {
				bestc = cost;
				best  = (*it)->lnode;
			  }
			}
			node_[i][j]->prev     = best;
			node_[i][j]->bestCost = best ? bestc : node_[i][j]->cost+node_[i][j]->constraintcost;
		}
	}

    double bestc = -1e37;
    Node *best = 0;
    size_t s = xsize_-1;
    for (size_t j = 0; j < ysize_; ++j) {
      if (bestc < node_[s][j]->bestCost) {
        best  = node_[s][j];
        bestc = node_[s][j]->bestCost;
      }
    }

    for (Node *n = best; n; n = n->prev)
      result_[n->x] = n->y;

    cost_ = -node_[xsize_-1][result_[xsize_-1]]->bestCost;
  }

void ViterbiDecoder::viterbi(vector<vector<Node*> > &node_,
		vector<unsigned short int> &result_) {
	 for (size_t i = 0;   i < xsize_; ++i) {
		for (size_t j = 0; j < ysize_; ++j) {
			double bestc = -1e37;
			Node *best = 0;
			const std::vector<Path *> &lpath = node_[i][j]->lpath;
			for (const_Path_iterator it = lpath.begin(); it != lpath.end(); ++it) {
			  double cost = (*it)->lnode->bestCost +(*it)->cost + node_[i][j]->cost;
			  if (cost > bestc) {
				bestc = cost;
				best  = (*it)->lnode;
			  }
			}
			node_[i][j]->prev     = best;
			node_[i][j]->bestCost = best ? bestc : node_[i][j]->cost;
		}
	}

   double bestc = -1e37;
   Node *best = 0;
   size_t s = xsize_-1;
   for (size_t j = 0; j < ysize_; ++j) {
     if (bestc < node_[s][j]->bestCost) {
       best  = node_[s][j];
       bestc = node_[s][j]->bestCost;
     }
   }

   for (Node *n = best; n; n = n->prev)
     result_[n->x] = n->y;

   cost_ = -node_[xsize_-1][result_[xsize_-1]]->bestCost;
 }

void ViterbiDecoder::inference(int xsize, int ysize, vector<vector<Node*> > &chain,
		vector<unsigned short int> &result_) {
	xsize_ = xsize;
	ysize_ = ysize;

	viterbi(chain, result_);
	if (nbest_) initNbest(chain);
}

} /* namespace CRFPP */
