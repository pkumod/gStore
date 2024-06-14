#include "PathQueryHandler.h"

using namespace std;

/**
	Compute and return the all shortest path between vertices u and v, with the
	constraint that all edges in the path are labeled by labels in pred_set.

	@param uid the vertex u's ID .
	@param vid the vertex v's ID .
	@param directed if false, treat all edges in the graph as bidirectional.
	@param pred_set the set of edge labels allowed.
	@return the total shortest path.
*/
int PathQueryHandler::shortestPathCount(int uid, int vid, bool directed, const std::vector<int> &pred_set)
{
	int N = getVertNum();
	std::vector<int> q[2], tq[2];
	std::vector<int> scnt(N, 0), tcnt(N, 0); // scnt[v]: s--v, number of all shortest paths
	std::vector<int> sd(N, 0), td(N, 0);	 // sd[v]: s--v, shortest path len
	std::vector<int> joint;
	// std::vector<int> nbr_dedup(N, -1); // used for nbr dedup // no need: multi-edge contribute to different path
	int s = uid, t = vid;

	size_t qsz = 1, next_qsz = 0;
	int idx = 0, next_idx = 1;
	if (q[0].empty())
		q[0].push_back(s);
	else
		q[0][0] = s;
	scnt[s] = 1;
	size_t tqsz = 1, tnext_qsz = 0;
	int tidx = 0, tnext_idx = 1;
	if (tq[0].empty())
		tq[0].push_back(t);
	else
		tq[0][0] = t;
	tcnt[t] = 1;
	int v, n, vcnt, vd, sz;
	bool meet = 0;
	while (meet == 0 && qsz && tqsz)
	{
		if (qsz < tqsz)
		{
			for (size_t i = 0; i < qsz; ++i)
			{
				v = q[idx][i];
				vcnt = scnt[v];
				vd = sd[v];
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
							// if (nbr_dedup[n] != v)
							// {
							// 	nbr_dedup[n] = v;
							if (scnt[n] == 0)
							{
								if (tcnt[n])
								{
									meet = 1;
									joint.push_back(n);
								}
								if (meet == 0)
								{
									if (q[next_idx].size() <= next_qsz)
										q[next_idx].push_back(n);
									else
										q[next_idx][next_qsz] = n;
									++next_qsz;
								}
								scnt[n] = vcnt;
								sd[n] = vd + 1;
							}
							// n has been visited by some other vertices in the same level as v
							else if (sd[n] == vd + 1)
							{
								scnt[n] += vcnt;
							}
							// }
						}
					}
				}
			}
			idx = next_idx;
			next_idx = 1 - next_idx;
			qsz = next_qsz;
			next_qsz = 0;
		}
		else
		{
			for (size_t i = 0; i < tqsz; ++i)
			{
				v = tq[tidx][i];
				vcnt = tcnt[v];
				vd = td[v];
				for (int d = directed; d <= 1; ++d)
				{
					for (int pred : pred_set)
					{
						if (d)
							sz = getInSize(v, pred);
						else
							sz = getOutSize(v, pred);
						for (int j = 0; j < sz; j++)
						{
							if (d)
								n = getInVertID(v, pred, j);
							else
								n = getOutVertID(v, pred, j);
							// if (nbr_dedup[n] != v)
							// {
							// 	nbr_dedup[n] = v;
							if (tcnt[n] == 0)
							{
								if (scnt[n])
								{
									meet = 1;
									joint.push_back(n);
								}
								if (meet == 0)
								{
									if (tq[tnext_idx].size() <= tnext_qsz)
										tq[tnext_idx].push_back(n);
									else
										tq[tnext_idx][tnext_qsz] = n;
									++tnext_qsz;
								}
								tcnt[n] = vcnt;
								td[n] = vd + 1;
							}
							// n has been visited by some other vertices in the same level as v
							else if (td[n] == vd + 1)
							{
								tcnt[n] += vcnt;
							}
							// }
						}
					}
				}
			}
			tidx = tnext_idx;
			tnext_idx = 1 - tnext_idx;
			tqsz = tnext_qsz;
			tnext_qsz = 0;
		}
	}
	if (meet == 0) // no path between s and t
		return 0;
	int path_num = 0;
	for (auto jt : joint)
		path_num += scnt[jt] * tcnt[jt];
	return path_num;
}