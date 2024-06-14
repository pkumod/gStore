#include "PathQueryHandler.h"

using namespace std;

/**
	Determine if v is reachable from u within k hops via edges labeled by labels
	in pred_set.

	@param uid the vertex u's ID.
	@param vid the vertex v's ID.
	@param directed if false, treat all edges in the graph as bidirectional.
	@param k the hop constraint.
	@param pred_set the set of edge labels allowed.
	@return true if v is reachable from u within k hops, false otherwise.
*/
bool PathQueryHandler::kHopReachable(int uid, int vid, bool directed, int k, const std::vector<int> &pred_set)
{
	if (uid == vid)
		return true;
	int uOutTotal = 0, vInTotal = 0;
	for (int pred : pred_set)
	{
		uOutTotal += getOutSize(uid, pred);
		vInTotal += getInSize(vid, pred);
		if (!directed)
		{
			uOutTotal += getInSize(uid, pred);
			vInTotal += getOutSize(vid, pred);
		}
	}
	if (uOutTotal == 0 || vInTotal == 0)
		return false;

	// partial push
	double rmax_fwd = 0.001, rmax_bwd = 0.001;
	queue<pair<int, int>> candidatePush1;
	queue<pair<int, int>> candidatePush1_next;
	queue<pair<int, int>> candidatePush2;
	queue<pair<int, int>> candidatePush2_next;
	candidatePush1.push(pair<int, int>(uid, 0));
	candidatePush2.push(pair<int, int>(vid, 0));
	map<int, int> residue1, residue2, l1, l2, f1, f2;
	residue1[uid] = 1;
	residue2[vid] = 1;
	l1[uid] = 0;
	l2[vid] = 0;

	// full push
	queue<int> fwdQ;
	queue<int> fwdQ_next;
	queue<int> bwdQ;
	queue<int> bwdQ_next;
	int fullL1 = 0;
	int fullL2 = 0;
	fwdQ.push(uid);
	bwdQ.push(vid);
	f1[uid] = 0;
	f2[vid] = 0;

	double theta = 1, c_fora = 0.1, alpha = 0.2;
	double init_rmax = 100.0 / pow(getEdgeNum(), 3.0 / 4.0) / c_fora;
	int batchSize = 10;
	int push_bs = theta * batchSize, bfs_bs = batchSize;
	int op_cnt;
	fullL1 = 1;
	fullL2 = 1;
	while (fullL1 <= k && fullL2 <= k &&
		   (!fwdQ.empty() || !fwdQ_next.empty()) && (!bwdQ.empty() || !bwdQ_next.empty()))
	{
		op_cnt = 0;
		while ((!candidatePush1.empty() || !candidatePush1_next.empty()) && op_cnt < push_bs && rmax_fwd > init_rmax)
		{
			if (candidatePush1.empty())
			{
				swap(candidatePush1, candidatePush1_next);
				rmax_fwd /= 2;
				continue;
			}
			int curNode = candidatePush1.front().first;
			int level = candidatePush1.front().second;
			candidatePush1.pop();
			op_cnt++;
			if (level <= k)
			{
				if (!directed)
				{
					if (residue1[curNode] / getSetOutSize(curNode, pred_set) >= rmax_fwd)
					{
						for (int pred : pred_set)
						{
							int num_out = getOutSize(curNode, pred);
							for (int i = 0; i < num_out; i++)
							{
								int adj = getOutVertID(curNode, pred, i);
								if (l1.find(adj) == l1.end())
								{
									l1[adj] = level + 1;
									if (l2.find(adj) != l2.end() && l1[adj] + l2[adj] <= k)
										return true;
									if (f2.find(adj) != f2.end() && l1[adj] + f2[adj] <= k)
										return true;
									residue1[adj] = (1 - alpha) * residue1[curNode] / getSetOutSize(curNode, pred_set); // Should be equivalent to +=
									if (residue1[adj] / getSetOutSize(adj, pred_set) >= rmax_fwd)
										candidatePush1.push(pair<int, int>(adj, level + 1));
									else
										candidatePush1_next.push(pair<int, int>(adj, level + 1));
								}
							}
						}
						residue1[curNode] = 0;
					}
					else
						candidatePush1_next.push(pair<int, int>(curNode, level));
				}

				else
				{
					if (residue1[curNode] / (getSetOutSize(curNode, pred_set) + getSetInSize(curNode, pred_set)) >= rmax_fwd)
					{
						for (int pred : pred_set)
						{
							int num_out = getOutSize(curNode, pred);
							for (int i = 0; i < num_out; i++)
							{
								int adj = getOutVertID(curNode, pred, i);
								if (l1.find(adj) == l1.end())
								{
									l1[adj] = level + 1;
									if (l2.find(adj) != l2.end() && l1[adj] + l2[adj] <= k)
										return true;
									if (f2.find(adj) != f2.end() && l1[adj] + f2[adj] <= k)
										return true;
									residue1[adj] = (1 - alpha) * residue1[curNode] /
													(getSetOutSize(curNode, pred_set) + getSetInSize(curNode, pred_set)); // Should be equivalent to +=
									if (residue1[adj] / (getSetOutSize(adj, pred_set) + getSetInSize(adj, pred_set)) >= rmax_fwd)
										candidatePush1.push(pair<int, int>(adj, level + 1));
									else
										candidatePush1_next.push(pair<int, int>(adj, level + 1));
								}
							}

							int num_in = getInSize(curNode, pred);
							for (int i = 0; i < num_in; i++)
							{
								int adj = getInVertID(curNode, pred, i);
								if (l1.find(adj) == l1.end())
								{
									l1[adj] = level + 1;
									if (l2.find(adj) != l2.end() && l1[adj] + l2[adj] <= k)
										return true;
									if (f2.find(adj) != f2.end() && l1[adj] + f2[adj] <= k)
										return true;
									residue1[adj] = (1 - alpha) * residue1[curNode] /
													(getSetOutSize(curNode, pred_set) + getSetInSize(curNode, pred_set)); // Should be equivalent to +=
									if (residue1[adj] / (getSetOutSize(adj, pred_set) + getSetInSize(adj, pred_set)) >= rmax_fwd)
										candidatePush1.push(pair<int, int>(adj, level + 1));
									else
										candidatePush1_next.push(pair<int, int>(adj, level + 1));
								}
							}
						}
						residue1[curNode] = 0;
					}
					else
						candidatePush1_next.push(pair<int, int>(curNode, level));
				}
			}
		}

		op_cnt = 0;
		// clock_gettime(CLOCK_MONOTONIC, &start_at);
		while ((!fwdQ.empty() || !fwdQ_next.empty()) && op_cnt < bfs_bs && fullL1 <= k)
		{
			if (fwdQ.empty())
			{
				swap(fwdQ, fwdQ_next);
				fullL1++;
				continue;
			}
			int curNode = fwdQ.front();
			fwdQ.pop();
			op_cnt++;
			for (int pred : pred_set)
			{
				int num_out = getOutSize(curNode, pred);
				for (int i = 0; i < num_out; i++)
				{
					int adj = getOutVertID(curNode, pred, i);
					if (f1.find(adj) == f1.end())
					{
						f1[adj] = fullL1;
						if (l2.find(adj) != l2.end() && f1[adj] + l2[adj] <= k)
							return true;
						if (f2.find(adj) != f2.end() && f1[adj] + f2[adj] <= k)
							return true;
						fwdQ_next.push(adj);
					}
				}

				if (directed)
					continue;

				int num_in = getInSize(curNode, pred);
				for (int i = 0; i < num_in; i++)
				{
					int adj = getOutVertID(curNode, pred, i);
					if (f1.find(adj) == f1.end())
					{
						f1[adj] = fullL1;
						if (l2.find(adj) != l2.end() && f1[adj] + l2[adj] <= k)
							return true;
						if (f2.find(adj) != f2.end() && f1[adj] + f2[adj] <= k)
							return true;
						fwdQ_next.push(adj);
					}
				}
			}
		}
		// clock_gettime(CLOCK_MONOTONIC, &end_at);
		// elapsed = timeDiff(start_at, end_at);
		// printf("fullL1 = %d, time = %lf ms\n", fullL1, elapsed);

		op_cnt = 0;
		while ((!candidatePush2.empty() || !candidatePush2_next.empty()) && op_cnt < push_bs && rmax_bwd > init_rmax)
		{
			if (candidatePush2.empty())
			{
				swap(candidatePush2, candidatePush2_next);
				rmax_bwd /= 2;
				continue;
			}
			int curNode = candidatePush2.front().first;
			int level = candidatePush2.front().second;
			candidatePush2.pop();
			op_cnt++;
			if (level <= k)
			{
				if (!directed)
				{
					if (residue2[curNode] / getSetInSize(curNode, pred_set) >= rmax_bwd)
					{
						for (int pred : pred_set)
						{
							int num_in = getInSize(curNode, pred);
							for (int i = 0; i < num_in; i++)
							{
								int adj = getInVertID(curNode, pred, i);
								if (l2.find(adj) == l2.end())
								{
									l2[adj] = level + 1;
									if (l1.find(adj) != l1.end() && l1[adj] + l2[adj] <= k)
										return true;
									if (f1.find(adj) != f1.end() && f1[adj] + l2[adj] <= k)
										return true;
									residue2[adj] = (1 - alpha) * residue1[curNode] / getSetInSize(curNode, pred_set); // Should be equivalent to +=
									if (residue2[adj] / getSetInSize(adj, pred_set) >= rmax_fwd)
										candidatePush2.push(pair<int, int>(adj, level + 1));
									else
										candidatePush2_next.push(pair<int, int>(adj, level + 1));
								}
							}
						}
						residue2[curNode] = 0;
					}
					else
						candidatePush2_next.push(pair<int, int>(curNode, level));
				}
				else
				{
					if (residue2[curNode] / (getSetInSize(curNode, pred_set) + getSetOutSize(curNode, pred_set)) >= rmax_bwd)
					{
						for (int pred : pred_set)
						{
							int num_in = getInSize(curNode, pred);
							for (int i = 0; i < num_in; i++)
							{
								int adj = getInVertID(curNode, pred, i);
								if (l2.find(adj) == l2.end())
								{
									l2[adj] = level + 1;
									if (l1.find(adj) != l1.end() && l1[adj] + l2[adj] <= k)
										return true;
									if (f1.find(adj) != f1.end() && f1[adj] + l2[adj] <= k)
										return true;
									residue2[adj] = (1 - alpha) * residue1[curNode] /
													(getSetInSize(curNode, pred_set) + getSetOutSize(curNode, pred_set)); // Should be equivalent to +=
									if (residue2[adj] / (getSetInSize(adj, pred_set) + getSetOutSize(adj, pred_set)) >= rmax_fwd)
										candidatePush2.push(pair<int, int>(adj, level + 1));
									else
										candidatePush2_next.push(pair<int, int>(adj, level + 1));
								}
							}

							int num_out = getOutSize(curNode, pred);
							for (int i = 0; i < num_out; i++)
							{
								int adj = getOutVertID(curNode, pred, i);
								if (l2.find(adj) == l2.end())
								{
									l2[adj] = level + 1;
									if (l1.find(adj) != l1.end() && l1[adj] + l2[adj] <= k)
										return true;
									if (f1.find(adj) != f1.end() && f1[adj] + l2[adj] <= k)
										return true;
									residue2[adj] = (1 - alpha) * residue1[curNode] /
													(getSetInSize(curNode, pred_set) + getSetOutSize(curNode, pred_set)); // Should be equivalent to +=
									if (residue2[adj] / (getSetInSize(adj, pred_set) + getSetOutSize(adj, pred_set)) >= rmax_fwd)
										candidatePush2.push(pair<int, int>(adj, level + 1));
									else
										candidatePush2_next.push(pair<int, int>(adj, level + 1));
								}
							}
						}
						residue2[curNode] = 0;
					}
					else
						candidatePush2_next.push(pair<int, int>(curNode, level));
				}
			}
		}

		op_cnt = 0;
		// clock_gettime(CLOCK_MONOTONIC, &start_at);
		while ((!bwdQ.empty() || !bwdQ_next.empty()) && op_cnt < bfs_bs && fullL2 <= k)
		{
			if (bwdQ.empty())
			{
				swap(bwdQ, bwdQ_next);
				fullL2++;
				continue;
			}
			int curNode = bwdQ.front();
			bwdQ.pop();
			op_cnt++;
			for (int pred : pred_set)
			{
				int num_in = getInSize(curNode, pred);
				for (int i = 0; i < num_in; i++)
				{
					int adj = getOutVertID(curNode, pred, i);
					if (f2.find(adj) == f2.end())
					{
						f2[adj] = fullL2;
						if (l1.find(adj) != l1.end() && l1[adj] + f2[adj] <= k)
							return true;
						if (f1.find(adj) != f1.end() && f1[adj] + f2[adj] <= k)
							return true;
						fwdQ_next.push(adj);
					}
				}

				if (directed)
					continue;

				int num_out = getOutSize(curNode, pred);
				for (int i = 0; i < num_out; i++)
				{
					int adj = getOutVertID(curNode, pred, i);
					if (f2.find(adj) == f2.end())
					{
						f2[adj] = fullL2;
						if (l1.find(adj) != l1.end() && l1[adj] + f2[adj] <= k)
							return true;
						if (f1.find(adj) != f1.end() && f1[adj] + f2[adj] <= k)
							return true;
						fwdQ_next.push(adj);
					}
				}
			}
		}
	}

	return false;
}