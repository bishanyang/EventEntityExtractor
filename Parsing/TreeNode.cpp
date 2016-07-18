#include "TreeNode.h"

TreeNode::TreeNode() {
	node_id = -1;
	children.clear();
	parent = NULL;
}

TreeNode::~TreeNode() {
}

int TreeNode::getChildrenSize()
{
	return (int)children.size();
}

void TreeNode::setChildrenAt(int i, TreeNode* node)
{
	if(i >= (int)children.size())
		return;

	children[i] = node;
}

TreeNode* TreeNode::getLeftmostChild() {
	TreeNode *node = NULL;
	int min_index = MAX_VALUE;
	for (int i = 0; i < children.size(); ++i) {
		if (children[i]->GetIndex() < min_index) {
			min_index = children[i]->GetIndex();
			node = children[i];
		}
	}
	return node;
}

TreeNode* TreeNode::getRightmostChild() {
	TreeNode *node = NULL;
	int max_index = 0;
	for (int i = 0; i < children.size(); ++i) {
		if (children[i]->GetIndex() > max_index) {
			max_index = children[i]->GetIndex();
			node = children[i];
		}
	}
	return node;
}

void TreeNode::addChild(TreeNode* node)
{
	int size = (int)children.size();
	children.push_back(node);
	node->SetParent(this);
}

TreeNode* TreeNode::getChildrenAt(int i)
{
	if(i >= (int)children.size())
		return NULL;

	return children[i];
}

bool ParseTreeNode::isParseLeaf()
{
	if(isChain(this))
		return true;
	else
		return false;
}

ParseTreeNode::ParseTreeNode(void)
{
	children.clear();
	parent = NULL;
    sibling = NULL;
    previousSibling = NULL;

	start_pos = 0;
	end_pos = 0;

	level = 0;

	isHead = false;
	headword = "";
	headtag = "";

	discourse_id = "";
	sense = "";
}

ParseTreeNode::~ParseTreeNode(void)
{
}

void ParseTreeNode::setStartPos(int i)
{
	start_pos = i;
}
int ParseTreeNode::getStartPos()
{
	return start_pos;
}
void ParseTreeNode::setEndPos(int i)
{
	end_pos = i;
}
int ParseTreeNode::getEndPos()
{
	return end_pos;
}

int ParseTreeNode::getLevel()
{
	return level;
}

void ParseTreeNode::setLevel(int l)
{
	level = l;
}

void ParseTreeNode::parseValue(string v)
{
	value = v;
	if(v.find(" ") != string::npos)
		return;
	//for leaf nodes
	if(v.find("#") != string::npos)
	{
		vector<string> fields;
		Utils::Split(v, '#', fields);
		if(fields.size() == 3)
		{
			word = fields[0];
			discourse_id = fields[1];
			sense = fields[2];
		}
	}
}

void ParseTreeNode::setValue(string v)
{
	value = v;
	if(v.find(" ") != string::npos)
		return;

	if(v.find("[") != string::npos) //internal nodes
	{
		int i = 0;
		string str = "";
		while(v[i] != '[') 
		{
			str += v[i];
			i++;
		}
		value = str;

		str = "";
		i++;
		while(v[i] != ']')
		{
			str += v[i];
			i++;
		}
		int index = str.find("/");
		headword = str.substr(0, index);
		headtag = str.substr(index+1);

		i++;
		if(i < v.size() && v[i]=='=')
			isHead = true;
	}
}

string ParseTreeNode::getValue()
{
	return value;
}

ParseTreeNode* ParseTreeNode::getSibling()
{
    return sibling;
}

void ParseTreeNode::setSibling(ParseTreeNode* node)
{
    sibling = node;
}

void ParseTreeNode::addChild(ParseTreeNode* node)
{
	int size = (int)children.size();
	if(size > 0)
	{
		ParseTreeNode *child = (ParseTreeNode*)children[size-1];
		child->setSibling(node);
		node->setPreviousSibling(child);
	}
	children.push_back(node);
}

void ParseTreeNode::setPreviousSibling(ParseTreeNode* node)
{
    previousSibling = node;
}

ParseTreeNode* ParseTreeNode::getPreviousSibling()
{
    return previousSibling;
}

ParseTreeNode* ParseTreeNode::getPreviousNeighbor()
{
    ParseTreeNode* neighbor = previousSibling;
	ParseTreeNode* curnode = this;
	while(neighbor == NULL)
	{
		if(curnode->getParent() == NULL)
			break;
		neighbor = curnode->getParent()->getPreviousSibling();
		curnode = curnode->getParent();
	}
	return neighbor;
}

ParseTreeNode* ParseTreeNode::getNeighbor()
{
	if(sibling == NULL)
		return NULL;

	ParseTreeNode* curnode = sibling;
	while(true)
	{
		if(curnode->isLeafPhrase())
			break;
		curnode = (ParseTreeNode*)curnode->getChildrenAt(0);
	}
	return curnode;
}

void ParseTreeNode::setParent(ParseTreeNode* node)
{
	parent = node;
}

ParseTreeNode* ParseTreeNode::getParent()
{
	return (ParseTreeNode *)parent;
}

bool ParseTreeNode::isSentence()
{
	if(Utils::SentenceTags.find(value) != Utils::SentenceTags.end())
		return true;
	else
		return false;
}

bool ParseTreeNode::isPunctuation()
{
	if(value == "," || value == "." || value == "``" || value == "''" || 
		value == ":" || value == ";" || value == "#" || value == "$" || 
		value == "-LRB-" || value == "-RRB-")
		return true;
	else
		return false;
}

bool ParseTreeNode::isWord()
{
	if(!isPhrase() && !isSentence())
		return true;
	else
		return false;
}

bool ParseTreeNode::isVerb()
{
	if(value != "VP" && value[0] == 'V')
		return true;
	else
		return false;
}

bool ParseTreeNode::isPhrase()
{
	if(Utils::PhraseTags.find(value) != Utils::PhraseTags.end())
		return true;
	else
		return false;
	/*if(value == "ADJP" || value == "ADVP" || value == "CONJP" || value == "PP" || value == "VP" ||
		value == "NP" || value == "WHADJP" || value == "WHAVP" || 
		value == "WHNP" || value == "WHPP" || value == "FRAG" ||
		value == "INTJ" || value == "LST" || value == "NAC" ||  value == "NX"
		|| value == "PRN" || value == "PRT" || value == "QP" || value == "RRC"
		|| value == "UCP" || value == "X")
		return true;
	else
		return false;*/
}

bool ParseTreeNode::isLeafVP()
{
	if(value != "VP")
		return false;
	
	for(int i=0; i<getChildrenSize(); i++)
	{
		if(((ParseTreeNode*)getChildrenAt(i))->getValue() == "VP")
			return false;
	}
	return true;
}

bool ParseTreeNode::isLeafPP()
{
	if(value != "PP")
		return false;
	
	for(int i=0; i<getChildrenSize(); i++)
	{
		if(((ParseTreeNode*)getChildrenAt(i))->getValue() == "PP")
			return false;
	}
	return true;
}

bool ParseTreeNode::isLeafNP()
{
	if(value != "NP")
		return false;
	
	for(int i=0; i<getChildrenSize(); i++)
	{
		if(((ParseTreeNode*)getChildrenAt(i))->getValue() == "NP")
			return false;
	}
	return true;
}

bool ParseTreeNode::isRootVP()
{
	if(value != "VP")
		return false;
	
	if(((ParseTreeNode*)parent)->getValue() == "VP")
		return false;

	return true;
}

bool ParseTreeNode::isInLeafPhrase()
{
	if(parent == NULL)
		return false;
	ParseTreeNode* cur = (ParseTreeNode *)parent;

	return cur->isLeafPhrase();
}

bool ParseTreeNode::isChain(ParseTreeNode* curNode)
{
	if(curNode->getChildrenSize() == 0)
		return true;

	if(curNode->getChildrenSize() > 1)
		return false;
	
	return isChain((ParseTreeNode*)curNode->getChildrenAt(0));
}

bool ParseTreeNode::isLeafUnit()
{
	for(int i=0; i<getChildrenSize(); i++) {
		if(getChildrenAt(i)->getChildrenSize() > 1) {
			return false;	
		}
	}
	return true;
}

bool ParseTreeNode::isLeafPhrase()
{
	if(isChain(this))
		return true;

	for(int i=0; i<getChildrenSize(); i++) {
		if(!(getChildrenAt(i)->getChildrenSize() == 1 && getChildrenAt(i)->getChildrenAt(0)->getChildrenSize() == 0))
			return false;
	}
	return true;
}

string ParseTreeNode::parseValue()
{
    if(isSentence())
	    return "hasSentence";
	else
		return value;
}

bool ParseTreeNode::inRange(int start, int end)
{
	if(start_pos >= start && end_pos <= end)
		return true;
	else
		return false;
}

bool ParseTreeNode::overlap(int start, int end)
{
	if((start_pos>=start && start_pos<=end)||(end_pos>=start && end_pos<=end))
		return true;
	else
		return false;
}

void ParseTreeNode::setHeadWord(string str)
{
	headword = str;
}
string ParseTreeNode::getHeadWord()
{
	return headword;
}
void ParseTreeNode::setHeadTag(string str)
{
	headtag = str;
}
string ParseTreeNode::getHeadTag()
{
	return headtag;
}
void ParseTreeNode::setIsHead(bool b)
{
	isHead = b;
}
bool ParseTreeNode::getIsHead()
{
	return isHead;
}

ParseTreeNode* ParseTreeNode::getClauseSubj()
{
	ParseTreeNode* curNode = this;
	if(curNode->getValue()[0] != 'S')
		return NULL;
	ParseTreeNode *child0 = (ParseTreeNode*)curNode->getChildrenAt(0);
	ParseTreeNode *child1 = (ParseTreeNode*)curNode->getChildrenAt(1);

	if(curNode->getChildrenSize() == 1 && child0->getValue()[0] == 'S') //SBAR->S
		curNode = child0;
	if(curNode->getChildrenSize() == 2 && child1->getValue()[0] == 'S')
		curNode = child1;

	for(int k=0; k<curNode->getChildrenSize(); k++)
	{
		if(((ParseTreeNode*)curNode->getChildrenAt(k))->getValue()[0] == 'N')
			return (ParseTreeNode*)curNode->getChildrenAt(k);
	}
	return NULL;
}
void ParseTreeNode::getVPArg1(ParseTreeNode*& argv1)
{
	argv1 = NULL;
	getVPArg1(this, argv1);
	if(argv1 == NULL)
	{
		argv1 = (ParseTreeNode*)this->getChildrenAt(this->getChildrenSize()-1);
	}
}

void ParseTreeNode::getVPArg1(ParseTreeNode* curnode, ParseTreeNode*& argv1)
{
	if(curnode->getValue() != "VP" && (curnode->isPunctuation() || curnode->isPhrase()))
	{
		argv1 = curnode;
		return;
	}

	for(int i=0; i<curnode->getChildrenSize(); i++)
	{
		getVPArg1((ParseTreeNode*)curnode->getChildrenAt(i), argv1);
		if(argv1 != NULL)
			break;
	}
}

void ParseTreeNode::getHeadWordIndex()
{
	headindex = 0;
	getHeadWordIndex(this, headword, headtag, headindex);
}

void ParseTreeNode::getHeadWordIndex(ParseTreeNode* curnode, string word, string pos, int& index)
{
	if(curnode->getChildrenSize() == 0 &&
			curnode->getValue() == word && curnode->getParent()->getValue() == pos)
	{
		index = curnode->getStartPos();
		return;
	}
	for(int i=0; i<curnode->getChildrenSize(); i++)
		getHeadWordIndex((ParseTreeNode*)curnode->getChildrenAt(i), word, pos, index);
}

void ParseTreeNode::getSubTreeFeatures(vector<string>& features)
{
	string fstr = "Subtree";
	getSubTree(this, fstr);
	features.push_back(fstr);
}

void ParseTreeNode::getSubTree(ParseTreeNode* curnode, string& fstr)
{
	fstr += "_"+value;
	
	if(curnode->isLeafPhrase())
		return;

	for(int i=0; i<curnode->getChildrenSize(); i++)
		getSubTree((ParseTreeNode*)curnode->getChildrenAt(i), fstr);
}

/**
* Returns true if <code>this</code> dominates the Tree passed in
* as an argument.  Object equality (==) rather than .equals() is used
* to determine domination.
* t.dominates(t) returns false.
*/
bool ParseTreeNode::dominates(ParseTreeNode *t) {
	vector<ParseTreeNode *> path;
	dominationPath(t, path);
	return (path.size() != 0);
}

/**
* Returns the path of nodes leading down to a dominated node,
* including <code>this</code> and the dominated node itself.
* Returns null if t is not dominated by <code>this</code>.  Object
* equality (==) is the relevant criterion.
* t.dominationPath(t) returns null.
*/
void ParseTreeNode::dominationPath(ParseTreeNode *t, vector<ParseTreeNode *> &path) {
    //Tree[] result = dominationPathHelper(t, 0);
    dominationPath(t, 0, path);
}

void ParseTreeNode::dominationPathHelper(ParseTreeNode *t, int depth, vector<ParseTreeNode*> &path) {
	for (int i = getChildrenSize() - 1; i >= 0; i--) {
	  ParseTreeNode *t1 = (ParseTreeNode *)children[i];
	  if (t1 == NULL) {
		return;
	  }
	  vector<ParseTreeNode *> result;
	  t1->dominationPath(t, depth + 1, result);
	  if (result.size() != 0) {
		result[depth] = this;
		return;
	  }
	}
}

void ParseTreeNode::dominationPath(ParseTreeNode *t, int depth, vector<ParseTreeNode*> &path) {
	if (this == t) {
	  path.resize(depth + 1);
	  path[depth] = this;
	  return;
	}
	return dominationPathHelper(t, depth, path);
}
