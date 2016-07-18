#pragma once
#include "../Utils.h"
#include <queue>
#include <stack>
#include "TreeNode.h"

class GraphEdge;

class GraphNode
{
public:
	GraphNode(int p);
	~GraphNode(void);

public:
	int pos; //word position
	vector<GraphEdge*> edges;

public: 
	void addEdge(GraphEdge* edge);
	void setEdgeCapacity();
	int getEdgeNum() {return (int)edges.size();}
	
	bool isIsolated() {if(getEdgeNum()==0) return true; else return false;}

	GraphEdge* getEdge(int i) {return edges[i];}

	void clear();
};

class GraphEdge
{
public:
	GraphEdge(GraphNode* l, GraphNode* r, string v);

public:
	GraphNode* lv;
	GraphNode* rv;
	string value;
};


class DependencyGraph
{
public:
	DependencyGraph();
	~DependencyGraph();
public:
	int vsize; //number of nodes
	int esize; //number of edges

	vector<GraphNode*> nodes; //matrix

	void clear();

	void getPath(GraphNode* node, int t, vector<bool>& visited, string& text, vector<string>& paths);
	void getPath(int s, int t, vector<string>& paths);
	void getDepArgs(int wid1, string dep, vector<int>& wid2);

	void getNeighbors(int cur, vector<string>& neighbors);
	void getEdges(int cur, vector<string>& edges);
	void getLeftBoundEdges(int start, vector<string>& edges);
	void getRightBoundEdges(int end, vector<string>& edges);
	void getBoundEdges(int start, int end, vector<string>& edges);
	
	bool buildGraph(vector<pair<int, int> > &dep_pairs, vector<string> &dep_relations, int graphsize);

	string getShortestPath(Span e1, Span e2);

	bool isConnected(int s, int t, vector<int> visitor);
	vector<int> getCommonRoot(int s, int t);
	void subGraphDFS(GraphNode* node, int s, int t, vector<int>& visitor);
};

class DependencyTree
{
public:
	DependencyTree() : root(0), size(0) {
	}
	~DependencyTree() {
		for(int i=0; i<nodes.size(); i++) {
			delete nodes[i];
		}
	}
public:
	vector<DepTreeNode*> nodes; //matrix
	DepTreeNode *root;
	int size; //number of nodes

	bool CheckLoop(DepTreeNode *node, vector<bool> &visited);
	void ClearObservation();

	void getPath(GraphNode* node, int t, vector<bool>& visited, string& text, vector<string>& paths);
	void getPath(int s, int t, vector<string>& paths);
	void getDepArgs(int wid1, string dep, vector<int>& wid2);
	string getShortestPath(Span e1, Span e2);

	bool isConnected(int s, int t, vector<int> visitor);

	bool BuildTree(vector<pair<int, int> > &dep_pairs, vector<string> &deprelations, int treesize);
	bool SameRoot(int start, int end);

	void ShortestPath(int start, int end, vector<int> &visited);
	string SubTree(int start, int end, vector<string> &words);
	string GetSurroundingRelations(int i, vector<string> &words);
};

