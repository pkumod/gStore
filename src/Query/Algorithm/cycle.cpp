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

	use the map route_u[q] to store the route from u to q.
	use the map route_v[q] to store the route from v to q.
**/
vector<int> PathQueryHandler::shortestPath0(int uid, int vid, bool directed, const vector<int> &pred_set) // cost more space and more time?
{
	// cout << "BFS1" << endl;
	map<int, vector<int>> route_u, route_v;
	route_u[uid].push_back(uid);
	route_v[vid].push_back(vid);
	queue<int> q_u, q_v;
	int num_of_pred = pred_set.size();
	q_u.push(uid);
	q_v.push(vid);

	bool flag = 0; // sign whether v has been found
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
					int t = getOutVertID(temp_u, x, j);

					if (route_v.find(t) != route_v.end())
					{
						flag = 1;
						meet_node = t;
						route_u[t] = route_u[temp_u];
						route_u[t].push_back(x);
						break;
					} // get the meet_node

					if (route_u.find(t) != route_u.end())
						continue; // have already visited the node

					new_q_u.push(t);
					route_u[t] = route_u[temp_u];
					route_u[t].push_back(x);
					route_u[t].push_back(t);
				}
				if (flag)
					break;

				if (directed)
					continue;
				// undirected: need to visit the in-neighbours
				num = getInSize(temp_u, x);
				for (int j = 0; j < num; ++j)
				{
					int t = getInVertID(temp_u, x, j);

					if (route_v.find(t) != route_v.end())
					{
						flag = 1;
						meet_node = t;
						route_u[t] = route_u[temp_u];
						route_u[t].push_back(x);
						break;
					} // get the meet_node

					if (route_u.find(t) != route_u.end())
						continue; // have already visited the node

					new_q_u.push(t);
					route_u[t] = route_u[temp_u];
					route_u[t].push_back(x);
					route_u[t].push_back(t);
				}
				if (flag)
					break;
			}
		}
		q_u = new_q_u; // update the queue;

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

					if (route_u.find(t) != route_u.end())
					{
						flag = 1;
						meet_node = t;
						route_v[t] = route_v[temp_v];
						route_v[t].push_back(x);
						break;
					}

					if (route_v.find(t) != route_v.end())
						continue;

					new_q_v.push(t);
					route_v[t] = route_v[temp_v];
					route_v[t].push_back(x);
					route_v[t].push_back(t);
				}
				if (flag)
					break;

				if (directed)
					continue;

				num = getOutSize(temp_v, x);
				for (int j = 0; j < num; ++j)
				{
					int t = getOutVertID(temp_v, x, j);

					if (route_u.find(t) != route_u.end())
					{
						flag = 1;
						meet_node = t;
						route_v[t] = route_v[temp_v];
						route_v[t].push_back(x);
						break;
					}

					if (route_v.find(t) != route_v.end())
						continue;

					new_q_v.push(t);
					route_v[t] = route_v[temp_v];
					route_v[t].push_back(x);
					route_v[t].push_back(t);
				}
				if (flag)
					break;
			}
		}
		q_v = new_q_v;
	}

	vector<int> ans;
	if (flag == 0)
		return ans; // no route from u to v.
	ans = route_u[meet_node];
	if (route_v.find(meet_node) != route_v.end())
	{
		int lens = route_v[meet_node].size();
		for (int i = 0; i < lens; ++i)
			ans.push_back(route_v[meet_node][lens - 1 - i]);
	} // make up the route with route_u and route_v.
	return ans;
}

/**
	If there exists one or more CYCLES with all edge labels in pred_set
	and containing both the vertices u and v, return one of them; otherwise
	return an empty vector.

	@param uid the vertex u's ID.
	@param vid the vertex v's ID.
	@param directed if false, treat all edges in the graph as bidirectional.
	@param pred_set the set of edge labels allowed.
	@return a vector of vertex IDs representing the CYCLE, shaped like
	[v0, pred0, v1, pred1, ..., vk, predk, v0], where v0 = u, and predi labels the
	edge from vi to vi+1 (predk labels the edge from vk to v0); empty if no
	such CYCLE exists.
*/
std::vector<int> PathQueryHandler::cycle(int uid, int vid, bool directed,
										 const std::vector<int> &pred_set)
{
	vector<int> ans;
	if (directed)
	{
		vector<int> ans1, ans2;
		ans1 = shortestPath(uid, vid, true, pred_set);
		ans2 = shortestPath(vid, uid, true, pred_set);
		if (ans1.size() == 0 || ans2.size() == 0)
			return ans;
		else
		{
			for (int i = 1; i < static_cast<int>(ans2.size()) - 1; ++i)
				ans1.push_back(ans2[i]);
			ans1.push_back(uid);
			return ans1;
		}
	}
	else // use BFS to get the smallest cycle. since every edge is undirected, just need to find the route from u to v.
	{
		// int num_of_pred = pred_set.size();
		// queue<int> q;
		// q.push(uid);
		// map<int, int> dis;
		// dis[uid] = 0;
		// bool finished = 0;
		// while(!q.empty() && !finished)
		// {
		// 	int temp = q.front();
		// 	q.pop();
		// 	for(int i = 0; i < num_of_pred; ++i)
		// 	{
		// 		int x = pred_set[i];
		// 		int num_out = getOutSize(temp, x);
		// 		for(int j = 0; j < num_out; ++j)
		// 		{
		// 			int t = getOutVertID(temp, x, j); //get the node
		// 			if(dis.find(t) != dis.end())
		// 				continue;
		// 			q.push(t);
		// 			dis[t] = dis[temp] + 1;
		// 			if(t == vid)
		// 			{
		// 				finished = 1;
		// 				break;
		// 			}
		// 		}
		// 		if(finished) break;
		// 		int num_in = getInSize(temp, x);
		// 		for(int j = 0; j < num_in; ++j)
		// 		{
		// 			int t = getInVertID(temp, x, j);
		// 			if(dis.find(t) != dis.end())
		// 				continue;
		// 			q.push(t);
		// 			dis[t] = dis[temp] + 1;
		// 			if(t == vid)
		// 			{
		// 				finished = 1;
		// 				break;
		// 			}
		// 		}
		// 		if(finished) break;
		// 	}

		// }
		// if(!finished) return ans;
		// stack<int> s;
		// s.push(vid);
		// while(1)
		// {
		// 	int temp = s.top();
		// 	if(temp == uid) break;
		// 	int flag0 = 0;
		// 	for(int i = 0; i < num_of_pred; ++i)
		// 	{
		// 		int x = pred_set[i];
		// 		int num_in = getInSize(temp, x);
		// 		for(int j = 0; j < num_in; ++j)
		// 		{
		// 			int t = getInVertID(temp, x, j);
		// 			if(dis.find(t) != dis.end() && dis[t] == dis[temp] - 1)
		// 			{
		// 				s.push(x);
		// 				s.push(t);
		// 				flag0 = 1;
		// 				break;
		// 			}
		// 		}
		// 		if(flag0) break;
		// 		int num_out = getOutSize(temp, x);
		// 		for(int j = 0; j < num_out; ++j)
		// 		{
		// 			int t = getOutVertID(temp, x, j);
		// 			if(dis.find(t) != dis.end() && dis[t] == dis[temp] - 1)
		// 			{
		// 				s.push(x);
		// 				s.push(t);
		// 				flag0 = 1;
		// 				break;
		// 			}
		// 		}
		// 		if(flag0) break;
		// 	}
		// }
		// while(!s.empty())
		// {
		// 	ans.push_back(s.top());
		// 	s.pop();
		// }
		ans = shortestPath0(uid, vid, false, pred_set);
		int lens = ans.size();
		if (lens == 0)
			return ans;
		for (int i = lens - 2; i > 0; --i)
			ans.push_back(ans[i]);
		ans.push_back(uid);
		return ans;
	}
}