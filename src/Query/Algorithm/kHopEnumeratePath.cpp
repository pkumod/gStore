#include "PathQueryHandler.h"

using namespace std;

int PathQueryHandler::bc_dfs(int uid, int vid, int &retBudget, bool directed, int k, const std::vector<int> &pred_set, vector<pair<int, int>> &s, vector<int> &bar, vector<vector<int>> &paths)
{
	int f = k + 1;

	if (uid == vid)
	{
		cout << "stack: ";
		paths.emplace_back();
		for (size_t i = 0; i < s.size(); i++)
		{
			auto pr = s[i];
			cout << '(' << pr.first << ',' << pr.second << ") ";
			if (i != 0)
				paths.back().emplace_back(pr.second);
			paths.back().emplace_back(pr.first);
		}
		cout << endl;
		cout << "path: ";
		for (auto ele : paths.back())
			cout << ele << ' ';
		cout << endl;
		s.pop_back();
		if (retBudget > 0)
			retBudget--;
		return 0;
	}
	else if ((int)s.size() - 1 < k)
	{
		int inNum, outNum, to;
		for (int pred : pred_set)
		{
			outNum = getOutSize(uid, pred);
			for (int i = 0; i < outNum; ++i)
			{
				to = getOutVertID(uid, pred, i);
				bool stacked = false;
				for (auto pr : s)
					if (pr.first == to)
					{
						stacked = true;
						break;
					}
				if (stacked)
					continue;

				if ((int)s.size() + bar[to] <= k)
				{
					if (retBudget != 0)
					{
						s.push_back(make_pair(to, pred));
						int next_f = bc_dfs(to, vid, retBudget, directed, k, pred_set, s, bar, paths);
						if (next_f != k + 1 && f < next_f + 1)
							f = next_f + 1;
					}
				}

			}
			if (directed)
				continue;
			inNum = getInSize(uid, pred);
			for (int i = 0; i < inNum; ++i)
			{
				to = getInVertID(uid, pred, i);
				bool stacked = false;
				for (auto pr : s)
					if (pr.first == to)
					{
						stacked = true;
						break;
					}
				if (stacked)
					continue;

				if ((int)s.size() + bar[to] <= k)
				{
					if (retBudget != 0)
					{
						s.push_back(make_pair(to, -pred - 1));
						int next_f = bc_dfs(to, vid, retBudget, directed, k, pred_set, s, bar, paths);
						if (next_f != k + 1 && f < next_f + 1)
							f = next_f + 1;
					}
				}
			}
		}
	}

	// The following pruning not suitable for labeled multigraphs
	// if (f == k + 1)
	// 	bar[uid] = k - s.size() + 2;
	// else
	// 	updateBarrier(uid, directed, pred_set, bar, f);

	s.pop_back();
	return f;
}

/**
	Return all paths from u to v within k hops via edges labeled by labels
	in pred_set.

	@param uid the vertex u's ID.
	@param vid the vertex v's ID.
	@param retNum the maximum number of paths to return (if -1, then return all paths).
	@param directed if false, treat all edges in the graph as bidirectional.
	@param k the hop constraint.
	@param pred_set the set of edge labels allowed.
	@return a vector of all paths from u to v; the format of each path is the same as in shortestPath.
*/
vector<vector<int>> PathQueryHandler::kHopEnumeratePath(int uid, int vid, int retNum, bool directed, int k, const std::vector<int> &pred_set)
{
	vector<vector<int>> ret;
	vector<pair<int, int>> s;
	vector<int> bar(getVertNum(), 0);
	// Use reverse kBFS from destination to refine bar
	queue<int> q;
	q.push(vid);
	while (!q.empty())
	{
		int cur = q.front();
		q.pop();
		if (bar[cur] >= k)
			continue;
		for (int pred : pred_set)
		{
			int inNum = getInSize(cur, pred);
			for (int i = 0; i < inNum; ++i)
			{
				int to = getInVertID(cur, pred, i);
				if (bar[to] == 0 && to != vid)
				{
					bar[to] = bar[cur] + 1;
					q.push(to);
				}
			}
			if (directed)
				continue;
			int outNum = getOutSize(cur, pred);
			for (int i = 0; i < outNum; ++i)
			{
				int to = getOutVertID(cur, pred, i);
				if (bar[to] == 0 && to != vid)
				{
					bar[to] = bar[cur] + 1;
					if (to == 2)
						cout << "bar[to] = " << bar[to] << endl;
					q.push(to);
				}
			}
		}
	}

	s.push_back(make_pair(uid, -1));	// (vertex_id, pred_id that precedes it)
	int retBudget = retNum;
	bc_dfs(uid, vid, retBudget, directed, k, pred_set, s, bar, ret);
	cout << "ret.size() = " << ret.size() << endl;
	return ret;
}