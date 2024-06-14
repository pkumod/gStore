#include "PathQueryHandler.h"

using namespace std;

/**
	Compute and return the closeness centrality of a vertex, only
	considering the edges in the path with labels in pred_set.
	closenessCentrality(u) = n' / \\Sigma_{v \\neq u}d_{uv},
	where n' = the number of reachable vertices from u,
	d_{uv} = the distance from u to v (only consider v reachable from u)

	@param uid vertex u's ID.
	@param directed if false, treat all edges in the graph as bidirectional.
	@param pred_set the set of edge labels allowed.
	@return the closeness centrality of vertex u.
**/
double PathQueryHandler::closenessCentrality(int uid, bool directed, const std::vector<int> &pred_set)
{
	map<int, int> dis;
	dis[uid] = 0;
	queue<int> Q;
	Q.push(uid);
	int sum = 0;
	int cnt = 0;
	while (Q.size())
	{
		int ele = Q.front();
		Q.pop();
		cnt++;
		for (int pred : pred_set)
		{
			int outNum = getOutSize(ele, pred);
			for (int i = 0; i < outNum; ++i)
			{
				int to = getOutVertID(ele, pred, i); // get the node
				if (dis.find(to) != dis.end())
					continue;
				dis[to] = dis[ele] + 1;
				sum += dis[to];
				Q.push(to);
			}
			if (directed)
				continue;
			int inNum = getInSize(ele, pred);
			for (int i = 0; i < inNum; ++i)
			{
				int to = getInVertID(ele, pred, i); // get the node
				if (dis.find(to) != dis.end())
					continue;
				dis[to] = dis[ele] + 1;
				sum += dis[to];
				Q.push(to);
			}
		}
	}
	return 1.0 * (cnt - 1) / sum;
}