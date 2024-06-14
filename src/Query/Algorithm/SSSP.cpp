#include "PathQueryHandler.h"

using namespace std;

/**
	Compute and return the single-source shortest paths from vertex u.
	Note: edges are unweighted; do not return unreachable vertices.

	@param uid vertex u's ID.
	@param directed if false, treat all edges in the graph as bidirectional.
	@param pred_set the set of edge labels allowed.
	@return a map from each vertex's ID to one of u's shortest path to it.
**/
unordered_map<int, vector<int>> PathQueryHandler::SSSP(int uid, bool directed, const std::vector<int> &pred_set)
{
	unordered_map<int, vector<int>> res;
	set<int> vis;
	queue<int> Q;
	Q.push(uid);
	vis.insert(uid);
	res[uid].push_back(uid);
	while (Q.size())
	{
		int vid = Q.front();
		Q.pop();
		for (int pred : pred_set)
		{
			int outNum = getOutSize(vid, pred);
			for (int i = 0; i < outNum; ++i)
			{
				int to = getOutVertID(vid, pred, i); // get the node]
				if (!vis.count(to))
				{
					vis.insert(to);
					res[to] = res[vid];
					res[to].push_back(pred);
					res[to].push_back(to);
					Q.push(to);
				}
			}
			if (directed)
				continue;
			int inNum = getInSize(vid, pred);
			for (int i = 0; i < inNum; ++i)
			{
				int to = getInVertID(vid, pred, i);
				if (!vis.count(to))
				{
					vis.insert(to);
					res[to] = res[vid];
					res[to].push_back(pred);
					res[to].push_back(to);
					Q.push(to);
				}
			}
		}
	}
	return res;
}