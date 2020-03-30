// TODO: Convert all IDs to unsigned to incorporate more data?

#include "../Util/Util.h"
#include "../Database/CSR.h"

#ifndef _QUERY_PATH_H
#define _QUERY_PATH_H

class PathQueryHandler
{
private:
	CSR *csr;
public:
	PathQueryHandler(CSR *_csr);

	void inputGraph(std::string filename);	// Read in a graph FOR TESTING. Graph file format:
						// First line: #vertices #labels
						// Each following line: u v label
	void printCSR();	// Feel free to modify this for testing

	// Labeled graph interface: edge (u, v, pred)

	// int getVertNum();	// Get total number of vertices
	int getEdgeNum();	// Get total number of edges
	int getInIndexByID(int vid, int pred);	// Get vertice's index in csr[1] offset_list
	int getInSize(int vid, int pred);	// Get the number of in-neighbors of vert
						// linked by edges labeled by pred
	int getInVertID(int vid, int pred, int pos);	// Get the pos-th in-neighbor of vert
						// linked by an edge labeled by pred
	int getTotalInSize(int vid);	// Get the total number of in-neighbors of vert

	int getOutIndexByID(int vid, int pred);
	int getOutSize(int vid, int pred);
	int getOutVertID(int vid, int pred, int pos);
	int getTotalOutSize(int vid);
	void dfs(std::map<int, std::vector<int> > &route, std::map<int, bool> &vis, int q, int v, const std::vector<int> pred_set, std::vector<int> &ans, bool &finished);

	// Path query evaluation functions
	
	std::vector<int> simpleCycle(int uid, int vid, bool directed, const std::vector<int> &pred_set);
	std::vector<int> cycle(int uid, int vid, bool directed, const std::vector<int> &pred_set);
	std::vector<int> shortestPath(int uid, int vid, const std::vector<int> &pred_set);
	std::vector<int> shortestPath0(int uid, int vid, const std::vector<int> &pred_set);
	bool kHopReachable(int uid, int vid, int k, const std::vector<int> &pred_set);
};

#endif //_QUERY_PATH_H