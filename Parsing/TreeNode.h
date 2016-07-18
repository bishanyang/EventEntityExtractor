#pragma once
#include <vector>
#include <string>
#include "../Utils.h"
#include <time.h>

using namespace std;

class Connective: public Span
{
public:
	string id;
	string sense;
	string str;
};

class TreeNode {
public:
	int node_id;            // change to int, easier to map
	TreeNode* parent;
	vector<TreeNode*> children;
public:
	TreeNode();
	virtual ~TreeNode();
	void SetIndex(int i) { node_id = i; }
	int GetIndex() { return node_id; }
	void SetParent(TreeNode *node) { parent = node; }
	TreeNode *GetParent() { return parent; }
	int getChildrenSize();
	void setChildrenAt(int i, TreeNode* node);
	void addChild(TreeNode* node);
	TreeNode* getChildrenAt(int i);

	TreeNode* getLeftmostChild();
	TreeNode* getRightmostChild();

	bool IsLeaf() {
		return (children.size() == 0);
	}

	bool IsSingleton() {
		if (parent == NULL && children.size() == 0) return true;
		else return false;
	}

	TreeNode* GetRoot() {
		TreeNode *node = this;
		while(node->parent != NULL) {
			node = node->parent;
		}
		return node;
	}
};

class DepTreeNode : public TreeNode {
public:
	string parent_dep;
	bool observed;
public:
	DepTreeNode() : TreeNode() {
		observed = false;
	}
};

class ParseTreeNode : public TreeNode
{
public:
	ParseTreeNode(void);
	ParseTreeNode(int id);
	~ParseTreeNode(void);
private:
	int node_id;            // change to int, easier to map
	int level;
	
	int start_pos; //word position
	int end_pos;

	int type; //0:root, 1:clause, 2:phrase, 3:word 

    string value;

    ParseTreeNode* sibling;
    ParseTreeNode* previousSibling;
public:
	string headword;
	string headtag;
	int headindex;
	bool isHead;

	//discourse info
	string discourse_id;
	string sense;
	string word;
public: 
	bool isLeafUnit();
	ParseTreeNode *getParseParent() {
		return (ParseTreeNode*)parent;
	}

	bool isParseLeaf();
	void parseValue(string str);
	bool isChain(ParseTreeNode* curNode);

	ParseTreeNode* getClauseSubj();
	void getVPArg1(ParseTreeNode*& argv1);
	void getVPArg1(ParseTreeNode* curnode, ParseTreeNode*& argv1);

	int getID();
	int getLevel();
	
	void setStartPos(int i);
	int getStartPos();
	void setEndPos(int i);
	int getEndPos();

	void setHeadWord(string str);
	string getHeadWord();
	void setHeadTag(string str);
	string getHeadTag();
	void setIsHead(bool b);
	bool getIsHead();

	void setValue(string v);
	string getValue();
	void setParent(ParseTreeNode* node);
	ParseTreeNode* getParent();
    void setSibling(ParseTreeNode* node);
	ParseTreeNode* getSibling();
    void setPreviousSibling(ParseTreeNode* node);
    ParseTreeNode* getPreviousSibling();
	void setLevel(int l);
	void setChildrenAt(int i, ParseTreeNode* node);
	void addChild(ParseTreeNode* node);

	bool isPunctuation();
	bool isSentence();
	bool isPhrase();
	bool isWord();
	bool isLeafVP();
	bool isVerb();
	bool isRootVP();
	bool isLeafPP();
	bool isLeafNP();

	bool overlap(int start, int end);
	bool inRange(int start, int end);
	string parseValue();

	bool isInLeafPhrase();
	bool isLeafPhrase();

	void getHeadWordIndex();
	void getHeadWordIndex(ParseTreeNode* curnode, string word, string pos, int& index);

	void getSubTreeFeatures(vector<string>& features);
	void getSubTree(ParseTreeNode* curnode, string& fstr);

	ParseTreeNode* getPreviousNeighbor();
	ParseTreeNode* getNeighbor();

	bool dominates(ParseTreeNode *t);
	void dominationPath(ParseTreeNode *t, vector<ParseTreeNode *> &path);
	void dominationPathHelper(ParseTreeNode *t, int depth, vector<ParseTreeNode*> &path);
	void dominationPath(ParseTreeNode *t, int depth, vector<ParseTreeNode*> &path);
};
