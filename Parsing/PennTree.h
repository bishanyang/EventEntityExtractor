#pragma once
#include <stack>
#include <queue>
#include <set>
#include "../Utils.h"
#include "TreeNode.h"
#include "GraphNode.h"

using namespace std;

class phraseElem
{
public:
	ParseTreeNode* node;
	int start;
	int end;
};

class PennTree
{
public:
	PennTree();
	~PennTree();

private:
	ParseTreeNode* root;

private:
	string ToString(ParseTreeNode* currentNode, int level);
    void deleteNode(ParseTreeNode *node);
    void getChildrenSize(ParseTreeNode *currentNode, double &total, double &numerator);
	
public:
	vector<Span> units;

	vector<ParseTreeNode*> leaves;
	vector<ParseTreeNode*> clauses;
	vector<Span> chunks; //chunk view
	vector<Connective> connectives;

public:
	Span getLeafSegment(int i);

	string GetSegmentParse(int start, int end);
	bool isSentenceNode(ParseTreeNode *node);
	int  GetSegmentGroupBoundary(int i);
	void MergeUnitSegments(vector<Span> unit_spans, vector<Span> &merge_spans);
	void GetSegmentFeatures(int start, int end, vector<string> &features);
	ParseTreeNode *GetMultiUnitParent(int s);
	bool GetMultiUnitSegments(int start, int end, vector<Span> &spans);
	string GetSyntacticPath(int start, int end);

	void getChunks();

	void getClauses();
	void getClauses(ParseTreeNode* curNode, vector<ParseTreeNode*>& clauses);

	// get segment that contains i
	void getSegmentUnit(int i, Span &en);

	void getSegmentation();
	void getSegmentation(ParseTreeNode* curNode, vector<Span>& phrases);

	void setDependencyGraph(DependencyGraph* graph);

	ParseTreeNode* getRoot();
	bool isBuilt();
	void clear();
	
    // build tree
	bool ReadTree(string text);
	void ReadDiscourseTree(string text);
	void getConnectives();

    string ToString();
	
    int getNumberOfNodes();
	int getNumberOfNodes(ParseTreeNode *currentNode);

	void getNumberOfLeaves(ParseTreeNode* currentNode, int& num);

	void getLeaves(ParseTreeNode* currentNode, vector<ParseTreeNode*>& leaves);
	void getLeaves();

	void genPos(ParseTreeNode* currentNode, int& offset);
	void genPos();

	void breadthFirstTravel();

	void getWidth(ParseTreeNode* currentNode, int& num);
	int getWidth();

	string getTreePath(ParseTreeNode* s, ParseTreeNode* t);
	string getTreePath(Span e1, Span e2);
	string getChunkPath(Span e1, Span e2);

	ParseTreeNode* getCommonRoot(int start, int end);
	void getSegmentInfo(ParseTreeNode* currentNode, int start, int end, vector<ParseTreeNode*>& segnodes);
	void getSegmentInfo(int start, int end, vector<ParseTreeNode*>& segnodes);

	string getSubTree(int start, int end);
	void getSubTree(ParseTreeNode* curnode, int start, int end, string& path);

	void getStructuralFeatures(ParseTreeNode* curnode, int start, int end, vector<string>& features);

	bool inClause(Span e1, Span e2);

	bool addCandidate(map<int, set<int> >& candidates, int start, int end);
	void genSegmentCandidates(map<int, set<int> >& candidates);

	string GetParseTag(int start, int end);

	//int GetHeadIndex(int start, int end);

	void genHeadIndex();
	void genHeadIndex(ParseTreeNode* currentNode);
};
