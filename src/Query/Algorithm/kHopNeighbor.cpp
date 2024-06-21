#include "PathQueryHandler.h"

using namespace std;

/**
	Compute and return the vertices reachable from vertex u by BFS
	at specified distances.

	@param uid vertex u's ID.
	@param directed if false, treat all edges in the graph as bidirectional.
	@param pred_set the set of edge labels allowed.
	@return the number of vertices reachable from u with k steps.
**/
 std::vector<int> PathQueryHandler::kHopNeighbor(int uid, bool directed, int k, const std::vector<int> &pred_set, int retNum) {
	if (uid < 0)
		return std::vector<int>();
	int s = uid;
	std::vector<int> cnt;
	std::vector<int> q[2];
	int N = getVertNum();
	std::vector<char> visited(N, 0);
	size_t qsz = 1, next_qsz = 0;
	int idx = 0, next_idx = 1;
	q[0].push_back(s);
	visited[s] = 1;
	int v, n, sz;
	while (k && qsz)
	{
		for (size_t i = 0; i < qsz; ++i)
		{
			v = q[idx][i];
			for (int d = directed; d <= 1; ++d)
			{
				for (int pred : pred_set)
				{
					if (d)
						sz = getOutSize(v, pred);
					else
						sz = getInSize(v, pred);
					for (int j = 0; j < sz; j++)
					{
						if (d)
							n = getOutVertID(v, pred, j);
						else
							n = getInVertID(v, pred, j);
						if (visited[n] == 0)
						{
							if (k == 1)
							{
								cnt.push_back(n);
								if (retNum >= 0 && cnt.size() >= retNum)
								{
									return cnt;
								}
							}
							else
							{
								if (q[next_idx].size() <= next_qsz)
									q[next_idx].push_back(n);
								else
									q[next_idx][next_qsz] = n;
								++next_qsz;
							}
							visited[n] = 1;
						}
					}
				}
			}
		}
		--k;
		idx = next_idx;
		next_idx = 1 - next_idx;
		qsz = next_qsz;
		next_qsz = 0;
	}
	return cnt;
}