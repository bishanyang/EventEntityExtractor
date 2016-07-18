#include "GraphNode.h"
#include <assert.h>

GraphNode::GraphNode(int p)
{
	pos = p;
}

GraphNode::~GraphNode(void)
{
	for (int i = 0; i < edges.size(); ++i) {
		delete edges[i];
		edges[i] = NULL;
	}
}

void GraphNode::clear()
{
	pos = -1;
}

GraphEdge::GraphEdge(GraphNode* l, GraphNode* r, string v)
{
	lv = l;
	rv = r;
	value = v;
}

void GraphNode::addEdge(GraphEdge* edge)
{
	edges.push_back(edge);
}

DependencyGraph::~DependencyGraph()
{
	for(int i=0; i<nodes.size(); i++) {
		delete nodes[i];
		nodes[i] = NULL;
	}
}

DependencyGraph::DependencyGraph()
{
	vsize = 0;
	esize = 0;
}

void DependencyGraph::clear()
{
	vsize = 0;
	esize = 0;

	nodes.clear();
}

void DependencyGraph::getDepArgs(int wid1, string dep, vector<int>& wid2)
{
	vector<string> deps;
	Utils::Split(dep, ',', deps);

	vector<int> starts;
	vector<int> args;
	args.push_back(wid1);

	int d = 0;
	while(d<deps.size()) //length of the path
	{
		starts.swap(args);
		args.clear();
		for(int i=0; i<starts.size(); i++)
		{
			GraphNode* node = nodes[starts[i]];
			for(int j=0; j<node->getEdgeNum(); j++)
			{
				string var = node->getEdge(j)->value;
				if(var.substr(0,deps[d].size()) == deps[d]) {//found relation
					int direction = atoi(var.substr(deps[d].size()+1).c_str());
					if (direction > 0) // only consider positive direction!!!
						args.push_back(node->getEdge(j)->rv->pos);
				}
			}
		}
		d++;
	}
	for(int i=0; i<args.size(); i++) {
		wid2.push_back(args[i]);
	}
}

bool DependencyGraph::buildGraph(vector<pair<int, int> > &dep_pairs, vector<string> &dep_relations,
		int graphsize)
{
	//initialize
	clear();
	int size = graphsize;
	nodes.resize(size);
	for(int i=0; i<size; i++)
		nodes[i] = new GraphNode(i);

	vsize = size;
	esize = 0;

	for(int i = 0; i < dep_pairs.size(); ++i) {
		int a0 = dep_pairs[i].first; //parent
		int a1 = dep_pairs[i].second; //child
		string dep = dep_relations[i];

		string estr = dep+"_+1";
		GraphEdge* edge1 = new GraphEdge(nodes[a0], nodes[a1], estr);
		nodes[a0]->addEdge(edge1);

		estr = dep+"_-1";
		GraphEdge* edge2 = new GraphEdge(nodes[a1], nodes[a0], estr);
		nodes[a1]->addEdge(edge2);
	}

	return true;
}

void DependencyGraph::getEdges(int cur, vector<string>& edges)
{
	GraphNode* node = nodes[cur];
	for(int i=0; i<node->getEdgeNum(); i++) {
		edges.push_back("Edge_"+node->getEdge(i)->value);
	}
}

void DependencyGraph::getNeighbors(int cur, vector<string>& neighbors)
{
	GraphNode* node = nodes[cur];
	for(int i=0; i<node->getEdgeNum(); i++)
	{
		string str = node->getEdge(i)->value;
		str += "/" + Utils::int2string(node->getEdge(i)->rv->pos);
		neighbors.push_back(str);
	}
}

void DependencyGraph::getLeftBoundEdges(int start, vector<string>& edges)
{
	GraphNode* node = nodes[start];
	for(int i=0; i<node->getEdgeNum(); i++)
	{
		int to = node->getEdge(i)->rv->pos;
		if(to < start)
		{
			string fstr = "Left_Edge_"+node->getEdge(i)->value;
			edges.push_back(fstr);
		}
	}
}

void DependencyGraph::getRightBoundEdges(int end, vector<string>& edges)
{
	GraphNode* node = nodes[end];
	for(int i=0; i<node->getEdgeNum(); i++)
	{
		int to = node->getEdge(i)->rv->pos;
		if(to > end)
		{
			string fstr = "Right_Edge_"+node->getEdge(i)->value;
			edges.push_back(fstr);
		}
	}
}

void DependencyGraph::getBoundEdges(int start, int end, vector<string>& edges)
{
	for(int k=start; k<=end; k++)
	{
		GraphNode* node = nodes[k];
		for(int i=0; i<node->getEdgeNum(); i++)
		{
			int to = node->getEdge(i)->rv->pos;
			if(to>=start && to<=end)
			{
				string fstr = "In_"+node->getEdge(i)->value;
				edges.push_back(fstr);
			}
			else if(to < start)
			{
				string fstr = "Left_"+node->getEdge(i)->value;
				edges.push_back(fstr);
			}
			else if(to > end)
			{
				string fstr = "Right_"+node->getEdge(i)->value;
				edges.push_back(fstr);
			}
		}
	}
}

void DependencyGraph::getPath(int s, int t, vector<string>& paths) 
{
	vector<bool> visited(nodes.size());
	string text = "";
	paths.clear();
	fill(visited.begin(), visited.end(), false);
	getPath(nodes[s], t, visited, text, paths);
}

void DependencyGraph::getPath(GraphNode* node, int t, vector<bool>& visited, string& text, vector<string>& paths) 
{	
	visited[node->pos] = true;
	text += Utils::int2string(node->pos)+",";

	if(node->pos == t) {
		paths.push_back(text);
		return;
	}

	for(int i=0; i<node->getEdgeNum(); i++) {
		GraphNode* to = node->getEdge(i)->rv;
		if(!visited[to->pos]) {
			text += node->getEdge(i)->value+",";

			getPath(to, t, visited, text, paths);

			//clear text for "to"
			int index = (int)text.size()-2;
			while(index >= 0 && text[index] != ',') index--;
			index--;
			while(index >= 0 && text[index] != ',') index--;

			text = text.substr(0, index+1);
		}
	}
	visited[node->pos] = false;
}

string DependencyGraph::getShortestPath(Span e1, Span e2)
{
	map<string, int> pathmap;
	for(int s=e1.start; s<=e1.end; s++) {
		for(int t=e2.start; t<=e2.end; t++) {
			vector<string> paths;
			getPath(s, t, paths);
			for(int i=0; i<paths.size(); i++) {
				if(pathmap.find(paths[i]) == pathmap.end()) {
					vector<string> v;
					Utils::Split(paths[i], ',', v);
					pathmap[paths[i]] = (int)v.size(); //path to length
				}
			}
		}
	}

	if(pathmap.size() == 0)
		return "";

	vector<pair<string, int> > vec = Utils::sortMap(pathmap);
	return vec[0].first;
}

bool DependencyGraph::isConnected(int s, int t, vector<int> visitor)
{
	int i = 0;
	for(i = s; i <= t; i++)
	{
		if(visitor[i] == 0 && nodes[i]->getEdgeNum() == 0)
			break;
	}
	if(i<=t)
		return false;
	else
		return true;
}

vector<int> DependencyGraph::getCommonRoot(int s, int t)
{
	vector<int> comroot;
	if(s == t)
	{
		comroot.push_back(s);
		return comroot;
	}

	for(int i=s; i<=t; i++)
	{
		vector<int> visitor;
		visitor.resize(nodes.size());
		fill(visitor.begin(), visitor.end(), 0);
		//start from i, traverse the graph
		subGraphDFS(nodes[i], s, t, visitor);
		if(isConnected(s,t,visitor))
			comroot.push_back(i);
	}
	return comroot;
}

void DependencyGraph::subGraphDFS(GraphNode* node, int s, int t, vector<int>& visitor)
{
	if(node->pos < s || node->pos > t)
		return;
	visitor[node->pos] += 1;
	for(int i=0; i<node->getEdgeNum(); i++)
	{
		GraphNode* to = node->getEdge(i)->rv;
		if(visitor[to->pos] == 0)
			subGraphDFS(to, s, t, visitor);
	}
}

bool DependencyTree::SameRoot(int start, int end) {
	if (start == end) return true;
	// is start is the root
	bool root = false;
	for (int i = start; i <= end; ++i) {
		// i is the parent, check
		int j = 0;
		for (j = start; j <= end; ++j) {
			if (j == i) continue;
			if (nodes[j]->parent != nodes[i]) break;
		}
		if (j > end) {
			root = true;
			break;
		}
	}
	return root;
}

void DependencyTree::ShortestPath(int start, int end, vector<int> &visited) {

}

string DependencyTree::SubTree(int start, int end, vector<string> &words) {
	string str = "";
	for (int i = start; i <= end; ++i) {
		str += GetSurroundingRelations(i, words);
	}
	return str;
}

string DependencyTree::GetSurroundingRelations(int i, vector<string> &words) {
	string str = words[i];
	if (nodes[i]->parent) {
		str += " <-(" + nodes[i]->parent_dep + ") "+words[nodes[i]->parent->node_id]+"\n";
	}
	for (int j = 0; j < nodes[i]->children.size(); ++j) {
		str += words[i] + " -> (" + ((DepTreeNode*)nodes[i]->children[j])->parent_dep
				+ ") " + words[nodes[i]->children[j]->node_id] + "\n";
	}
	return str;
}

bool DependencyTree::BuildTree(vector<pair<int, int> > &dep_pairs,
		vector<string> &dep_relations, int treesize) {
	//initialize
	nodes.clear();
	size = treesize;
	nodes.resize(size);
	for(int i=0; i<size; i++) {
		nodes[i] = new DepTreeNode();
		nodes[i]->SetIndex(i);
	}

	//build nodes
	for(int i = 0; i < dep_pairs.size(); ++i) {
		int a0 = dep_pairs[i].first; //parent
		int a1 = dep_pairs[i].second; //child
		string dep = dep_relations[i];
		nodes[a0]->addChild(nodes[a1]);
		nodes[a1]->parent_dep = dep;
	}

	// find roots
	vector<DepTreeNode*> roots;
	for (int i = 0; i < nodes.size(); ++i) {
		if (nodes[i]->IsSingleton()) continue;
		if (nodes[i]->parent == NULL) {
			roots.push_back(nodes[i]);
		}
	}

	// Check if they have more than one root
	if (roots.size() > 1) {
		//cout<<"More than one root!!!"<<endl;
		return false;
	}

	if (roots.size() == 0) {
		if (size != 1) {
			return false;
		}
		roots.push_back(nodes[0]);
	}

	root = roots[0];
	for (int i = 0; i < nodes.size(); ++i) {
		if (nodes[i] != root && nodes[i]->IsSingleton()) {
			root->addChild(nodes[i]);
			nodes[i]->parent_dep = "root";
		}
	}

	// Check loops
	vector<bool> visited(size, false);
	fill (visited.begin(), visited.end(), false);
	if (CheckLoop(root, visited)) {
		return false;
	}

	// Create a dummy root to connects all the roots
/*	root = new DepTreeNode();
	root->SetIndex(size);
	// connects all the roots to root
	for (int i = 0; i < roots.size(); ++i) {
		root->addChild(roots[i]);
		roots[i]->parent_dep = "root";
	}
*/
	return true;
}

bool DependencyTree::CheckLoop(DepTreeNode *node, vector<bool> &visited) {
	if (visited[node->node_id]) return true;
	visited[node->node_id] = true;
	for (int i = 0; i < node->children.size(); ++i) {
		bool flag = CheckLoop((DepTreeNode*)node->children[i], visited);
		if (flag) return true;
	}
	return false;
}

void DependencyTree::ClearObservation() {
	for (int i = 0; i < nodes.size(); ++i) {
		nodes[i]->observed = false;
	}
}
