#include "PathQueryHandler.h"

using namespace std;

/**
	Compute and return the shortest path between vertices u and v, with the
	constraint that all edges in the path are labeled by labels in pred_set.

	@param uid the vertex u's ID.
	@param vid the vertex v's ID.
	@param directed if false, treat all edges in the graph as bidirectional.
	@param pred_set the set of edge labels allowed.
	@return a vector of vertex IDs representing the shortest path, shaped like
	[v0, pred0, v1, pred1, ..., vk, predk, vk+1], where v0 = u, vk+1 = v,
	and predi labels the edge from vi to vi+1.

	cost more time.
	use dis_u, dis_v to record the distance from u,v to any node.
	use bbfs to get the meet node.
	get the route through searching the distance and meet node.
*/
vector<int> PathQueryHandler::shortestPath(int uid, int vid, bool directed, const vector<int> &pred_set) // cost less space and less time.
{
	// cout << "BFS2" << endl;
	map<int, int> dis_u, dis_v; // store the distance to u and v
	queue<int> q_u, q_v;
	int num_of_pred = pred_set.size();
	q_u.push(uid);
	q_v.push(vid);
	dis_u[uid] = 0;
	dis_v[vid] = 0;
	bool flag = 0;
	map<int, int>::iterator it;
	int meet_node = 0;
	while (flag == 0 && (!q_u.empty() || !q_v.empty()))
	{
		queue<int> new_q_u;

		while (!q_u.empty() && flag == 0)
		{
			int temp_u = q_u.front();

			q_u.pop();
			for (int i = 0; i < num_of_pred; ++i)
			{
				int x = pred_set[i];
				int num = getOutSize(temp_u, x);
				for (int j = 0; j < num; ++j)
				{
					int t = getOutVertID(temp_u, x, j); // get the node

					if (dis_v.find(t) != dis_v.end())
					{
						flag = 1;
						meet_node = t;
						dis_u[t] = dis_u[temp_u] + 1;
						break;
					} // get the meet node

					if (dis_u.find(t) != dis_u.end())
						continue; // has visited before

					new_q_u.push(t);
					dis_u[t] = dis_u[temp_u] + 1;
				}
				if (flag)
					break;
				if (directed)
					continue;

				num = getInSize(temp_u, x);
				for (int j = 0; j < num; ++j)
				{
					int t = getInVertID(temp_u, x, j); // get the node

					if (dis_v.find(t) != dis_v.end())
					{
						flag = 1;
						meet_node = t;
						dis_u[t] = dis_u[temp_u] + 1;
						break;
					} // get the meet node

					if (dis_u.find(t) != dis_u.end())
						continue; // has visited before

					new_q_u.push(t);
					dis_u[t] = dis_u[temp_u] + 1;
				}
				if (flag)
					break;
			}
		}
		q_u = new_q_u;

		queue<int> new_q_v;
		while (!q_v.empty() && flag == 0)
		{
			int temp_v = q_v.front();

			q_v.pop();
			for (int i = 0; i < num_of_pred; ++i)
			{
				int x = pred_set[i];
				int num = getInSize(temp_v, x);

				for (int j = 0; j < num; ++j)
				{
					int t = getInVertID(temp_v, x, j);

					if (dis_u.find(t) != dis_u.end())
					{
						flag = 1;
						meet_node = t;
						dis_v[t] = dis_v[temp_v] + 1;
						break;
					}

					if (dis_v.find(t) != dis_v.end())
						continue; // Already in

					new_q_v.push(t);
					dis_v[t] = dis_v[temp_v] + 1;
				}
				if (flag)
					break;

				if (directed)
					continue;

				num = getOutSize(temp_v, x);
				for (int j = 0; j < num; ++j)
				{
					int t = getOutVertID(temp_v, x, j);

					if (dis_u.find(t) != dis_u.end())
					{
						flag = 1;
						meet_node = t;
						dis_v[t] = dis_v[temp_v] + 1;
						break;
					}

					if (dis_v.find(t) != dis_v.end())
						continue; // Already in

					new_q_v.push(t);
					dis_v[t] = dis_v[temp_v] + 1;
				}
				if (flag)
					break;
			}
		}

		q_v = new_q_v;
	}

	// get the route through the distance.
	vector<int> empty_ans;
	if (flag == 0)
		return empty_ans; // no route from u to v
	stack<int> s;
	s.push(meet_node);
	// cout << "*************you are wrong!!!**********" << endl;
	while (1)
	{
		int temp = s.top();
		if (temp == vid)
			break;
		int flag0 = 0;
		for (int i = 0; i < num_of_pred; ++i)
		{
			int x = pred_set[i];
			int num = getOutSize(temp, x);
			for (int j = 0; j < num; ++j)
			{
				int t = getOutVertID(temp, x, j);
				if (dis_v.find(t) != dis_v.end() && dis_v[t] == dis_v[temp] - 1)
				{
					s.push(x);
					s.push(t);
					flag0 = 1;
					break;
				}
			}
			if (flag0)
				break;

			if (directed)
				continue;
			num = getInSize(temp, x);
			for (int j = 0; j < num; ++j)
			{
				int t = getInVertID(temp, x, j);
				if (dis_v.find(t) != dis_v.end() && dis_v[t] == dis_v[temp] - 1)
				{
					s.push(-x - 1);
					s.push(t);
					flag0 = 1;
					break;
				}
			}
			if (flag0)
				break;
		}
	} // get the route from meet_node to v.

	stack<int> s_new;
	while (!s.empty())
	{
		int temp = s.top();
		s.pop();
		s_new.push(temp);
	}
	if (s_new.empty())
	{
		s_new.push(meet_node);
		// cout << "s_new empty" << endl;
	}
	while (1)
	{
		int temp = s_new.top();
		if (temp == uid)
			break;
		int flag0 = 0;
		for (int i = 0; i < num_of_pred; ++i)
		{
			int x = pred_set[i];
			int num = getInSize(temp, x);
			for (int j = 0; j < num; ++j)
			{
				int t = getInVertID(temp, x, j);
				if (dis_u.find(t) != dis_u.end() && dis_u[t] == dis_u[temp] - 1)
				{
					s_new.push(x);
					s_new.push(t);
					flag0 = 1;
					break;
				}
			}
			if (flag0)
				break;

			if (directed)
				continue;

			num = getOutSize(temp, x);
			for (int j = 0; j < num; ++j)
			{
				int t = getOutVertID(temp, x, j);
				if (dis_u.find(t) != dis_u.end() && dis_u[t] == dis_u[temp] - 1)
				{
					s_new.push(-x - 1);
					s_new.push(t);
					flag0 = 1;
					break;
				}
			}
			if (flag0)
				break;
		}
	} // get the route from u to meet_node.

	int lens = s_new.size();
	vector<int> ans(lens);
	for (int i = 0; i < lens; ++i)
	{
		ans[i] = s_new.top();
		s_new.pop();
	}
	return ans;
}