#pragma once
#include "../../Database/CSR.h"
#include <iostream>
#include <memory>
#include <functional>

class CSRQueryHandler
{
    private:
	std::shared_ptr<CSR[]> csr;
	std::unordered_map<int, std::set<int> > distinctInEdges;
	std::unordered_map<int, std::set<int> > distinctOutEdges;
	int cacheMaxSize;
	int n, m;	// #vertices, #edges
public:
	CSRQueryHandler(std::shared_ptr<CSR[]>& _csr);
	
	~CSRQueryHandler();

	void inputGraph(std::string filename);	// Read in a graph FOR TESTING. Graph file format:
						// First line: #vertices #labels
						// Each following line: u v label
	void printCSR();	// Feel free to modify this for testing
	// void generateQueries(int queryType, bool directed, int numQueries, vector<pair<int, int>, int>& queries);

	// Labeled graph interface: edge (u, v, pred)
	int getPreNum();	// Get total number of predicate
	unsigned int getVertNum();	// Get total number of vertices
	unsigned int getEdgeNum();	// Get total number of edges
    int getSetEdgeNum(const std::vector<int> &pred_set);    // TODO: Get the number of edges labeled by preds in pred_set
	int getInIndexByID(int vid, int pred);	// Get vertice's index in csr[1] offset_list
	int getInSize(int vid, int pred);	// Get the number of in-neighbors of vert
						// linked by edges labeled by pred
	int getInVertID(int vid, int pred, int pos);	// Get the pos-th in-neighbor of vert
						// linked by an edge labeled by pred
	int getInVertID(int vid, int pos);	// Get in-neighbor regardless of pred;
						// corresponds to getTotalInSize with distinct
	int getSetInSize(int vid, const std::vector<int> &pred_set);	// Get number of in-neighbors by pred_set
	int getTotalInSize(int vid, bool distinct);	// Get the total number of in-neighbors of vert
						// distinct will eliminate repetitive occurrences of same in-neighbor with different label edges

	int getOutIndexByID(int vid, int pred);
	int getOutSize(int vid, int pred);
	int getOutVertID(int vid, int pred, int pos);
	int getOutVertID(int vid, int pos);
	int getSetOutSize(int vid, const std::vector<int> &pred_set);
	int getTotalOutSize(int vid, bool distinct);

	int getPredOffsetIndex(int pred, int vIndex) { return csr[1].offset_list[pred][vIndex]; }
	int getPredOffsetSize(int pred) { return csr[1].offset_list[pred].size(); }
	int getPredAdjacencyIndex(int pred, int vIndex) { return csr[1].adjacency_list[pred][vIndex]; }
	int getPredAdjacencySize(int pred) { return csr[1].adjacency_list[pred].size(); }
};