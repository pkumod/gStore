#include "PathQueryHandler.h"

using namespace std;

/**
	Compute and return the number of vertices reachable from vertex u by BFS
	at different distances.

	@param uid vertex u's ID.
	@param directed if false, treat all edges in the graph as bidirectional.
	@param pred_set the set of edge labels allowed.
	@return a vertex whose v[i] is the number of vertices reachable from u
	with i steps.
**/
vector<int> PathQueryHandler::bfsCount(int uid, bool directed, const std::vector<int> &pred_set)
{
	queue<int> q;
	q.push(uid);
	unordered_set<int> nodes;
	nodes.insert(uid);
	int currentlevel = 1; // vertices in the current level
	int nextlevel = 0;	  // vertices in the next level
	vector<int> ans;
	ans.push_back(1);
	while (!q.empty())
	{
		if (currentlevel == 0)
		{
			currentlevel = nextlevel;
			nextlevel = 0;
			ans.push_back(currentlevel);
		}
		int now = q.front();
		q.pop();
		currentlevel--;
		if (!directed)
		{
			for (int pred : pred_set)
			{
				int inNum = getInSize(now, pred);
				for (int i = 0; i < inNum; i++)
				{
					int inN = getInVertID(now, pred, i);
					if (!nodes.count(inN))
					{
						q.push(inN);
						nodes.insert(inN);
						nextlevel++;
					}
				}
			}
			for (int pred : pred_set)
			{
				int outNum = getOutSize(now, pred);
				for (int i = 0; i < outNum; i++)
				{
					int outN = getOutVertID(now, pred, i);
					if (!nodes.count(outN))
					{
						q.push(outN);
						nodes.insert(outN);
						nextlevel++;
					}
				}
			}
		}
		else
		{
			for (int pred : pred_set)
			{
				int outNum = getOutSize(now, pred);
				for (int i = 0; i < outNum; i++)
				{
					int outN = getOutVertID(now, pred, i);
					if (!nodes.count(outN))
					{
						q.push(outN);
						nodes.insert(outN);
						nextlevel++;
					}
				}
			}
		}
	}
	return ans;
}