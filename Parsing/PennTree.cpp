#include "PennTree.h"
#include <assert.h>

PennTree::PennTree(void)
{
	root = NULL;
}

PennTree::~PennTree()
{
	clear();
}

void PennTree::clear()
{
	if(root != NULL)
    {
        deleteNode(root);
    }
	root = NULL;
}

bool PennTree::isBuilt()
{
	if(root == NULL)
		return false;
	else
		return true;
}

void PennTree::deleteNode(ParseTreeNode *currentNode)
{
    for (int i = 0; i < currentNode->getChildrenSize(); i++) {
		ParseTreeNode* child = (ParseTreeNode*)currentNode->getChildrenAt(i);
        deleteNode(child);
	}   
    delete currentNode;
	currentNode = NULL;
}

int PennTree::getNumberOfNodes(ParseTreeNode *currentNode)
{
    int result = 0;
    for (int i = 0; i < currentNode->getChildrenSize(); i++) {
		ParseTreeNode* child = (ParseTreeNode*)currentNode->getChildrenAt(i);
        result += getNumberOfNodes(child);
	}   
    result += 1;
    return result;
}

int PennTree::getNumberOfNodes()
{
    return getNumberOfNodes(root);
}

void PennTree::genPos(ParseTreeNode* currentNode, int& offset)
{
	if(currentNode->getChildrenSize() == 0)
	{
		currentNode->setStartPos(offset);
		currentNode->setEndPos(offset);
		offset++;
		return;
	}

	int childnum = currentNode->getChildrenSize();
    for (int i = 0; i < childnum; i++) {
		ParseTreeNode* child = (ParseTreeNode*)currentNode->getChildrenAt(i);
        genPos(child, offset);
	}   

	currentNode->setStartPos(((ParseTreeNode*)currentNode->getChildrenAt(0))->getStartPos());
	currentNode->setEndPos(((ParseTreeNode*)currentNode->getChildrenAt(childnum-1))->getEndPos());
}

void PennTree::genPos()
{
	int offset = 0;
	genPos(root, offset);
}

void PennTree::genHeadIndex() {
	genHeadIndex(root);
}

void PennTree::genHeadIndex(ParseTreeNode* currentNode) {
	currentNode->getHeadWordIndex();
	for (int i = 0; i < currentNode->getChildrenSize(); ++i) {
		genHeadIndex((ParseTreeNode*)currentNode->getChildrenAt(i));
	}
}

bool PennTree::inClause(Span e1, Span e2)
{
	for(int i=0; i<clauses.size(); i++)
	{
		int start = clauses[i]->getStartPos();
		int end = clauses[i]->getEndPos();
		if(e1.inRange(start, end) && e2.inRange(start, end))
			return true;
	}
	return false;
	/*int start = min(e1.start, e2.start);
	int end = max(e1.end, e2.end);

	int i = 0;
	while(i<=root->getEndPos())
	{
		if((i>=e1.start && i<=e1.end) ||(i>=e2.start && i<=e2.end))
		{
			TreeNode* tnode = leaves[i];
			while(tnode->getParent() != NULL && tnode->inRange(start, end))
			{
				tnode = tnode->getParent();
				if(tnode->getValue()[0] == 'S') //a clause
				{
					if(tnode->getStartPos() > e2.end || tnode->getEndPos() < e2.start)
						return false;
				}
			}
		}
		i++;
	}
	return true;*/
}

string PennTree::GetParseTag(int start, int end) {
	string tag = "";
	vector<ParseTreeNode*> segnodes;
	getSegmentInfo(start, end, segnodes);
	for (int i = 0; i < segnodes.size(); ++i) {
		tag += segnodes[i]->getValue()+"_";
	}
	return tag.substr(0, tag.size()-1);
}

/*
int PennTree::GetHeadIndex(int start, int end) {
	vector<TreeNode*> segnodes;
	getSegmentInfo(start, end, segnodes);
	if (segnodes.size() > 1) {
		return segnodes[segnodes.size() - 1]->getHeadWordIndex();
	} else {
		return segnodes[0]->getHeadWordIndex();
	}
}
*/

void PennTree::getSegmentInfo(int start, int end, vector<ParseTreeNode*>& segnodes)
{
	getSegmentInfo(root, start, end, segnodes);
}

string PennTree::getSubTree(int start, int end)
{
	string path = "SubTree_";
	ParseTreeNode* subroot = getCommonRoot(start, end);
	getSubTree(subroot, start, end, path);

	return path;
}

void PennTree::getSubTree(ParseTreeNode* curnode, int start, int end, string& path)
{
	if(curnode->getStartPos() > end || curnode->getEndPos() < start)
		return;

	path += curnode->getValue()+"_";
	
	if(curnode->isLeafPhrase())
		return;
	
	for(int i=0; i<curnode->getChildrenSize(); i++)
	{
		getSubTree((ParseTreeNode*)curnode->getChildrenAt(i), start, end, path);
	}
}

ParseTreeNode* PennTree::getCommonRoot(int start, int end)
{
	vector<ParseTreeNode*> segnodes;
	getSegmentInfo(root, start, end, segnodes);
	//common root
	ParseTreeNode* comroot = segnodes[0];
	while(comroot != NULL)
	{
		if(comroot->getEndPos() >= end)
			break;
		comroot = comroot->getParent();
	}
	return comroot;
}

string PennTree::getTreePath(Span s, Span t)
{
	ParseTreeNode* sroot = getCommonRoot(s.start, s.end);
	ParseTreeNode* troot = getCommonRoot(t.start, t.end);
	return getTreePath(sroot, troot);
}
string PennTree::getTreePath(ParseTreeNode* s, ParseTreeNode* t)
{
	//s<t
	string path = "";
	ParseTreeNode* comroot = s;
	while(comroot != NULL)
	{
		path += comroot->getValue()+">";
		if(comroot->getEndPos() >= t->getEndPos()) //contain t
			break;
		comroot = comroot->getParent();
	}
	vector<ParseTreeNode*> revpaths;
	ParseTreeNode* end = t;
	while(end != comroot)
	{
		revpaths.push_back(end);
		end = end->getParent();
	}
	for(int i=revpaths.size()-1; i>=0; i--)
		path += "<"+revpaths[i]->getValue();
	return path;
}

void PennTree::getSegmentInfo(ParseTreeNode* currentNode, int start, int end, vector<ParseTreeNode*>& segnodes)
{
	if(currentNode->getChildrenSize() == 0)
		return;

	for(int i=0; i<currentNode->getChildrenSize(); i++)
		getSegmentInfo((ParseTreeNode*)currentNode->getChildrenAt(i), start, end, segnodes);

	if(currentNode->inRange(start, end) && 
		(currentNode->getParent() == NULL || !(currentNode->getParent()->inRange(start, end))))
	{
		segnodes.push_back(currentNode);
	}
}

void PennTree::getSegmentation(ParseTreeNode* curNode, vector<Span>& phrases)
{
	int start = curNode->getStartPos();
	int end = start;
	int i = 0;
	while (i<curNode->getChildrenSize()) {
		if (((ParseTreeNode*)curNode->getChildrenAt(i))->isParseLeaf()) {
			i++;
		} else {
			break;
		}
	}

	if (i-1 >= 0) {
		end = ((ParseTreeNode*)curNode->getChildrenAt(i-1))->getEndPos();
		Span en(start, end);
		phrases.push_back(en);
	}

	for (; i < curNode->getChildrenSize(); ++i) {
		getSegmentation((ParseTreeNode*)curNode->getChildrenAt(i), phrases);
	}
}

int PennTree::GetSegmentGroupBoundary(int i) {
	return GetMultiUnitParent(i)->getEndPos();
}

bool PennTree::isSentenceNode(ParseTreeNode *node) {
	if (node->getValue()[0] == 'S')
		return true;
	return false;
}

ParseTreeNode *PennTree::GetMultiUnitParent(int s) {
	ParseTreeNode *node = leaves[s];
	//while (node->getParseParent() != NULL && !isSentenceNode(node->getParseParent())) {
	while (node->getParseParent() != NULL) {
		node = node->getParseParent();
		if (!node->isLeafUnit()) break;
	}
	return node;
}

void PennTree::GetSegmentFeatures(int start, int end, vector<string> &features) {
	if (start == end) return;
	vector<ParseTreeNode *> parsenodes;
	getSegmentInfo(start, end, parsenodes);
	string str = "";
	for (int i = 0; i < parsenodes.size(); ++i) {
		str += parsenodes[i]->getValue()+"_";
		features.push_back(parsenodes[i]->getValue());
	}
	//features.push_back(str);
}

string PennTree::GetSegmentParse(int start, int end) {
	if (start == end) return "";
	vector<ParseTreeNode *> parsenodes;
	getSegmentInfo(start, end, parsenodes);
	string str = "";
	for (int i = 0; i < parsenodes.size(); ++i) {
		str += parsenodes[i]->getValue()+"_";
	}
	return str;
}

void PennTree::MergeUnitSegments(vector<Span> unit_spans, vector<Span> &merge_spans) {
	// across multiple phrases, then check the multi-unit parent
	int i = 0;
	int right_boundary = GetMultiUnitParent(unit_spans[i].end)->getEndPos();
	int start = unit_spans[i].start;
	i++;
	while (i < unit_spans.size()) {
		int rb = GetMultiUnitParent(unit_spans[i].end)->getEndPos();
		if (rb != right_boundary) {
			Span en(start, unit_spans[i-1].end);
			merge_spans.push_back(en);
			right_boundary = rb;
			start = unit_spans[i].start;
		}
		i++;
	}
	Span en(start, unit_spans[i-1].end);
	merge_spans.push_back(en);
}

bool PennTree::GetMultiUnitSegments(int start, int end, vector<Span> &spans) {
	// get the start unit
	assert(start != end);

	spans.clear();

	// check if start and end belongs to a unit
	for (int i = 0; i < units.size(); ++i) {
		if (units[i].start == start && units[i].end == end) {
			spans.push_back(units[i]);
			return true;
		}
	}

	// across multiple phrases, then check the multi-unit parent
	int right_boundary = GetMultiUnitParent(start)->getEndPos();
	int s = start;
	for (int i = start+1; i<=end; ++i) {
		int rb = GetMultiUnitParent(i)->getEndPos();
		if (rb != right_boundary) {
			Span en(s, i-1);
			spans.push_back(en);
			right_boundary = rb;
			s = i;
		}
	}
	return true;
}

string PennTree::GetSyntacticPath(int start, int end) {
	if (start == end) {
		return leaves[start]->getValue();
	}

	// get the starting leaf phrase
	ParseTreeNode *start_leaf = leaves[start];
	ParseTreeNode *end_leaf = leaves[end];

	// get the common root
	string syn_path = "";
	ParseTreeNode *left_common_root = start_leaf;
	while (left_common_root->getParseParent()->getEndPos() < end) {
		left_common_root = left_common_root->getParseParent();
		syn_path += left_common_root->getValue() + "_";
	}
	assert(left_common_root->getParseParent() != NULL);
	left_common_root = left_common_root->getParseParent();
	syn_path += "->" + left_common_root->getValue();

	// get the right path
	vector<string> right_paths;
	ParseTreeNode * right_common_root = end_leaf;
	while (right_common_root->getParseParent()->getStartPos() > start) {
		right_common_root = right_common_root->getParseParent();
		right_paths.push_back(right_common_root->getValue());
	}
	right_common_root = right_common_root->getParseParent();
	assert(right_common_root == left_common_root);

	syn_path += "<-";
	for (int i = right_paths.size()-1; i >=0; --i) {
		syn_path += right_paths[i] + "_";
	}

	return syn_path;
}

void PennTree::getSegmentation()
{
	if(root == NULL)
		return;

	units.clear();
	getSegmentation(root, units);
}

string PennTree::getChunkPath(Span e1, Span e2)
{
	//e1 < e2
	int i = 0;
	while(i<chunks.size())
	{
		if(chunks[i].end >= e1.end)
			break;
		i++;
	}
	int j = i;
	while(j<chunks.size())
	{
		if(chunks[j].end >= e2.end)
			break;
		j++;
	}
	string path = "";
	for(int k=i; k<=j; k++)
		path += chunks[k].label+"_";
	return path;
}

void PennTree::getChunks()
{
	if(leaves.size() == 0)
		return;

	chunks.clear();
	int i = 0;
	while(i<leaves.size())
	{
		string v = leaves[i]->getParent()->getValue();
		int start = i;
		string label = v;
		i++;
		while(i<leaves.size() && leaves[i]->getParent()->getValue() == label)
			i++;
		int end = i-1;
		Span en(start, end, label);
		chunks.push_back(en);
	}
}

void PennTree::getClauses()
{
	if(root->getEndPos() == 0)
		return;
	clauses.clear();
	getClauses(root, clauses);
}

void PennTree::getClauses(ParseTreeNode* curNode, vector<ParseTreeNode*>& clauses)
{
	if(curNode->getValue()[0] == 'S')
		clauses.push_back(curNode);
	for(int i=0; i<curNode->getChildrenSize(); i++)
		getClauses((ParseTreeNode*)curNode->getChildrenAt(i), clauses);
}

void PennTree::getWidth(ParseTreeNode* currentNode, int& num)
{
	if(currentNode->getChildrenSize() == 0)
	{
		num++;
	}

	for(int i=0; i<currentNode->getChildrenSize(); i++)
		getWidth((ParseTreeNode*)currentNode->getChildrenAt(i), num);
}

int PennTree::getWidth()
{
	int num = 0;
	getWidth(root, num);
	return num;
}
void PennTree::getChildrenSize(ParseTreeNode *currentNode, double &total, double &numerator)
{
    if(currentNode->getChildrenSize() == 0)
        return;
    total += currentNode->getChildrenSize();
    numerator += 1;
    for (int i = 0; i < currentNode->getChildrenSize(); i++) {
        ParseTreeNode* child = (ParseTreeNode*)currentNode->getChildrenAt(i);
		getChildrenSize(child, total, numerator);
	}   
}

bool PennTree::ReadTree(string text)
{
	vector<string> fields;
	Utils::Split(text, ' ', fields);

	stack<ParseTreeNode*>* prenodestack = new stack<ParseTreeNode*>;
	int i = 0;
	while(i < (int)fields.size())
	{
		if(fields[i][fields[i].size()-1] != ')')
		{
			string value = fields[i].substr(1);
			ParseTreeNode* node = new ParseTreeNode();
			node->setValue(value);
			
			if(!prenodestack->empty())
			{
				ParseTreeNode* p = prenodestack->top();
				p->addChild(node);
				node->setParent(p);
			}
			else
				root = node;

			prenodestack->push(node);
		} else {
			int j = 0;
			while (j < fields[i].size()) {
				if (fields[i][j] == ')') break;
				j++;
			}
			string value = fields[i].substr(0,j);

			ParseTreeNode* p = prenodestack->top();
			ParseTreeNode* newnode = new ParseTreeNode();
			newnode->setValue(value);
			newnode->setParent(p);
			p->addChild(newnode);

			prenodestack->pop();
		}
		i++;
	}

	genPos();

	getLeaves();

	//genHeadIndex();

	getSegmentation();

	//getLeafSegments();
	//getClauses();
	//getChunks();
	return true;
}

Span PennTree::getLeafSegment(int i)
{
	for(int j=0; j<units.size(); j++)
	{
		if(units[j].start<=i && units[j].end>=i)
			return units[j];
	}
	return Span(-1,-1);
}

void PennTree::getSegmentUnit(int p, Span &en) {
	for (int i = 0; i < units.size(); ++i) {
		if (units[i].start <= p && units[i].end >= p) {
			en.start = units[i].start;
			en.end = units[i].end;
			return;
		}
	}
}

void PennTree::ReadDiscourseTree(string text)
{
	stack<ParseTreeNode*>* prenodestack = new stack<ParseTreeNode*>;
	int i = 0;
	while(i < (int)text.size())
	{
		if(text[i] == '(')
		{
			string value = "";
			i++;
			while(text[i] != '(' && text[i] != ')')
			{
				value += text[i];
				i++;
			}
			Utils::Trim(value);
			ParseTreeNode* node = new ParseTreeNode();
			node->parseValue(value);

			if(!prenodestack->empty())
			{
				ParseTreeNode* p = prenodestack->top();
				p->addChild(node);
				node->setParent(p);
			}
			else
				root = node;

			prenodestack->push(node);
		}
		else if(text[i] == ')')
		{
			ParseTreeNode* p = prenodestack->top();
			string value = p->getValue();
			if(value.find(" ") != string::npos)
			{
				int index = value.find(" ");
				string firstv = value.substr(0, index);
				string secondv = value.substr(index+1);
				p->parseValue(firstv);

				ParseTreeNode* newnode = new ParseTreeNode();
				newnode->parseValue(secondv);
				newnode->setParent(p);
				p->addChild(newnode);
			}

			prenodestack->pop();
			i++;
		}
		else if(text[i] == ' ')
			i++;
	}

	genPos();
	getLeaves();

	getSegmentation();

	//getClauses();
	getChunks();

	getConnectives();

}

void PennTree::getConnectives()
{
	connectives.clear();
	int start = 0;
	int i = 0;
	while(i<leaves.size())
	{
		//the child of the leaf is the word node
		ParseTreeNode *child0 = (ParseTreeNode*)leaves[i]->getChildrenAt(0);
		if(child0->discourse_id != "")
		{
			Connective con;
			con.start = i;
			con.id = child0->discourse_id;
			con.sense = child0->sense;
			while(i<leaves.size() && child0->discourse_id == con.id) {
				con.str += child0->word + " ";
				i++;
			}
			// Remove the space at the end.
			con.str = con.str.substr(0, con.str.size()-1);
			con.end = i-1;
			connectives.push_back(con);
		}
		else
			i++;
	}
}

string PennTree::ToString() {

	string text;
	if (root == NULL)
		text = "Tree not built yet";
	else {
		text = ToString(root, 1);
	}
	return text;
}

string PennTree::ToString(ParseTreeNode* currentNode, int level) { 
	string text = currentNode->getValue();
	
	string lev;
	for (int i = 0; i<currentNode->getChildrenSize(); i++) {
		ParseTreeNode* child = (ParseTreeNode*)currentNode->getChildrenAt(i);
		text += "\n";
		for (int k = 0; k < level; k++) {
            text += "  ";
		}

		text += ToString(child, level + 1);
	}
	return text;
}

ParseTreeNode* PennTree::getRoot()
{
    return root;
}

void PennTree::getLeaves()
{
	leaves.clear();
	getLeaves(root, leaves); //should be in order, depth-first search
}

void PennTree::getLeaves(ParseTreeNode* currentNode, vector<ParseTreeNode*>& leaves)
{
	if(currentNode->isParseLeaf()) {
		leaves.push_back(currentNode);
		return;
	}
	//depth first
	for (int i = 0; i<currentNode->getChildrenSize(); i++) {
		getLeaves((ParseTreeNode*)currentNode->getChildrenAt(i), leaves);
	}
}

void PennTree::getNumberOfLeaves(ParseTreeNode* currentNode, int& num)
{
    for (int i = 0; i<currentNode->getChildrenSize(); i++) {
		ParseTreeNode* child = (ParseTreeNode*)currentNode->getChildrenAt(i);
		if(child->getChildrenSize() == 0)
			num++;
		else
			getNumberOfLeaves(child, num);
	}
}

void PennTree::breadthFirstTravel()
{
	queue<ParseTreeNode*> aQueue;
	ParseTreeNode* pointer=root;
	if(pointer)
	{
		aQueue.push(pointer);
		while(!aQueue.empty())
		{
			pointer=aQueue.front();

			//pointer->Value()

			aQueue.pop();
			for(int i=0; i<pointer->getChildrenSize(); i++)
			{
				aQueue.push((ParseTreeNode*)pointer->getChildrenAt(i));
			}	
		}
	}
}

void PennTree::getStructuralFeatures(ParseTreeNode* curnode, int start, int end, vector<string>& features)
{
	if(curnode->getStartPos() > end || curnode->getEndPos() < start)
		return;

	string fstr = "";
	string value = curnode->getValue();
	/*if(value.substr(0,2) == "JJ")
		features.push_back("JJ_"+value+"/"+curnode->getHeadWord());
	else if(value == "ADVP" || value == "ADJP")
		features.push_back("JJ_"+value+"/"+curnode->getHeadWord());
	*/
	/*if(value == "PP")
	{
		if(curnode->getChildrenSize() == 1)
			fstr = "PP_"+curnode->getChildrenAt(0)->getValue()+"/"+curnode->getChildrenAt(0)->getHeadWord();
		else if(curnode->getChildrenSize() > 1)
		{
			if(curnode->getChildrenAt(0)->isLeafPhrase() && curnode->getChildrenAt(1)->isLeafPhrase())
				fstr = "PP_"+curnode->getChildrenAt(0)->getValue()+"_"+curnode->getChildrenAt(1)->getValue()+"/"+curnode->getChildrenAt(1)->getHeadWord();
		}
		features.push_back(fstr);
	}*/

	if(curnode->isLeafPhrase()) 
	{
		//if(value.substr(0,2) == "JJ")
		//	features.push_back("JJ_"+curnode->getHeadWord());
		
		//if(value == "ADVP" || value == "ADJP")
		//	features.push_back("JJ_"+value+"/"+curnode->getHeadWord());
		//else
		/*{
			for(int i=0; i<curnode->getChildrenSize(); i++)
			{
				if(curnode->getChildrenAt(i)->getValue().substr(0,2) == "JJ")
					features.push_back("JJ_"+value+"/"+curnode->getChildrenAt(i)->getHeadWord());
			}
		}*/
		//if(value == "NP")
		//	features.push_back("NP_"+curnode->getHeadTag()+"_"+curnode->getHeadWord());
	}

	//features.push_back(fstr);
	/*if(curnode->getPreviousSibling() != NULL && curnode->getPreviousSibling()->isVerb())
	{
		fstr = "preVerb_"+curnode->getPreviousSibling()->getHeadWord()+"_"+curnode->getValue()+"/"+curnode->getHeadWord();
		features.push_back(fstr);
	}*/

	/*if(curnode->getValue() == "VP")
	{
		fstr = "VP->";
		for(int i=0; i<curnode->getChildrenSize(); i++)
		{
			if(curnode->getChildrenAt(i)->getStartPos() > end || curnode->getChildrenAt(i)->getEndPos() < start)
				continue;
			fstr += curnode->getChildrenAt(i)->getValue()+"_";
		}
		features.push_back(fstr);
	}*/
	//	features.push_back("VPhead_"+curnode->getHeadWord());

	// VP leaf (work)
	/*if(curnode->isLeafVP())
	{
		fstr = "VPleaf_"+curnode->getChildrenAt(0)->getHeadTag()+"/"+curnode->getChildrenAt(0)->getHeadWord();
		if(curnode->getChildrenAt(0)->inRange(start, end))
			features.push_back(fstr);

		if(curnode->getChildrenSize() > 1)
		{
			fstr += "_ARG_"+curnode->getChildrenAt(1)->getHeadTag()+"/"+curnode->getChildrenAt(1)->getHeadWord();
			if(curnode->getChildrenAt(1)->inRange(start, end))
				features.push_back(fstr);
		}
	}*/

	/*if(curnode->isRootVP() && curnode->getPreviousSibling() != NULL)
	{
		fstr = "subjVP_"+curnode->getPreviousSibling()->getHeadTag()+"/"+curnode->getPreviousSibling()->getHeadWord();
		features.push_back(fstr);
	}*/
	//structure
	if(curnode->isLeafPhrase())
		return;

  /* subtree (not work)  
    fstr = curnode->getValue()+"->";
	for(int i=0; i<curnode->getChildrenSize(); i++)
	{
		if(curnode->getChildrenAt(i)->getStartPos() > end || curnode->getChildrenAt(i)->getEndPos() < start)
			continue;
		fstr += curnode->getChildrenAt(i)->getValue()+",";
	}
	features.push_back(fstr);
	*/
	for(int i=0; i<curnode->getChildrenSize(); i++) {
		getStructuralFeatures((ParseTreeNode*)curnode->getChildrenAt(i), start, end, features);
	}
	
}

bool PennTree::addCandidate(map<int, set<int> >& candidates, int start, int end)
{
	if(start > end)
		return false;

	bool insert = false;
	if(candidates.find(start) == candidates.end())
	{
		set<int> newset;
		newset.insert(end);
		candidates[start] = newset;
		insert = true;
	}
	else
	{
		if(candidates[start].find(end) == candidates[start].end())
		{
			candidates[start].insert(end);
			insert = true;
		}
	}

	return insert;
}

