#include "PathQueryHandler.h"

using namespace std;

PathQueryHandler::PathQueryHandler(CSR *_csr)
{
	if (_csr)
		csr = _csr;
	else
		csr = new CSR[2];
	cacheMaxSize = 10000;
	n = -1;
	m = -1;
	srand(time(NULL));
}

PathQueryHandler::~PathQueryHandler()
{
}

int PathQueryHandler::getVertNum()
{
	// if (n != -1)
	// 	return n; // Only consider static graphs for now
	// set<int> vertices;
	// for (int j = 0; j < 2; j++)
	// {
	// 	for (unsigned i = 0; i < csr[j].pre_num; i++)
	// 		vertices.insert(csr[j].adjacency_list[i].begin(), csr[j].adjacency_list[i].end());
	// }
	// n = vertices.size();
	// return n;

	// save vertNum to CSR.n when database loadCSR 
	return csr[1].n;
}

int PathQueryHandler::getEdgeNum()
{
	// if (m != -1)
	// 	return m; // Only consider static graphs for now
	// int ret = 0;
	// for (unsigned i = 0; i < csr[1].pre_num; i++) // Same as summing that of csr[0]
	// 	ret += csr[1].adjacency_list[i].size();
	// m = ret;
	// return m;

	// save edgeNum to CSR.m when database loadCSR 
	return csr[1].m;
}

int PathQueryHandler::getSetEdgeNum(const vector<int> &pred_set)
{
	int ret = 0;
	for (int pred : pred_set)
		ret += csr[1].adjacency_list[pred].size();
	return ret;
}

int PathQueryHandler::getInIndexByID(int vid, int pred)
{
	if (csr[1].vid2id[pred].find(vid) != csr[1].vid2id[pred].end())
		return csr[1].vid2id[pred][vid];
	else
		return -1;
}

int PathQueryHandler::getInSize(int vid, int pred)
{
	int vIndex = getInIndexByID(vid, pred);
	if (vIndex == -1) // This vertex does not participate in this pred's relations
		return 0;
	else if (vIndex == static_cast<int>(csr[1].offset_list[pred].size()) - 1)
		return csr[1].adjacency_list[pred].size() - csr[1].offset_list[pred][vIndex];
	else
		return csr[1].offset_list[pred][vIndex + 1] - csr[1].offset_list[pred][vIndex];
}

int PathQueryHandler::getInVertID(int vid, int pred, int pos)
{
	if (pos >= getInSize(vid, pred))
		return -1;
	int offset = csr[1].offset_list[pred][getInIndexByID(vid, pred)];
	return csr[1].adjacency_list[pred][offset + pos];
}

int PathQueryHandler::getInVertID(int vid, int pos)
{
	if (distinctInEdges.find(vid) == distinctInEdges.end())
		getTotalInSize(vid, true); // Load into cache

	if (pos < static_cast<int>(distinctInEdges[vid].size()))
		return *next(distinctInEdges[vid].begin(), pos);
	else
		return -1;
}

int PathQueryHandler::getSetInSize(int vid, const std::vector<int> &pred_set)
{
	int ret = 0;
	for (int pred : pred_set)
		ret += getInSize(vid, pred);
	return ret;
}

int PathQueryHandler::getTotalInSize(int vid, bool distinct)
{
	int ret = 0;
	if (!distinct)
	{
		for (int i = 0; i < static_cast<int>(csr[1].pre_num); i++)
			ret += getInSize(vid, i);
	}
	else
	{
		if (distinctInEdges.find(vid) == distinctInEdges.end())
		{
			if (static_cast<int>(distinctInEdges.size()) == cacheMaxSize)
			{
				int replacement = rand() % cacheMaxSize;
				distinctInEdges.erase(next(distinctInEdges.begin(), replacement));
				// cout << "distinctInEdges replaced entry " << replacement << endl;
			}

			distinctInEdges[vid] = set<int>();
			for (int pred = 0; pred < static_cast<int>(csr[1].pre_num); pred++)
			{
				int vIndex = getInIndexByID(vid, pred);
				if (vIndex == -1) // This vertex does not participate in this pred's relations
					continue;
				else if (vIndex == static_cast<int>(csr[1].offset_list[pred].size()) - 1 && csr[1].adjacency_list[pred].size() > csr[1].offset_list[pred][vIndex])
					distinctInEdges[vid].insert(next(csr[1].adjacency_list[pred].begin(), csr[1].offset_list[pred][vIndex]),
												csr[1].adjacency_list[pred].end());
				else if (csr[1].offset_list[pred][vIndex + 1] > csr[1].offset_list[pred][vIndex])
					distinctInEdges[vid].insert(next(csr[1].adjacency_list[pred].begin(), csr[1].offset_list[pred][vIndex]),
												next(csr[1].adjacency_list[pred].begin(), csr[1].offset_list[pred][vIndex + 1]));
			}
		}
		ret = distinctInEdges[vid].size();
	}
	return ret;
}

int PathQueryHandler::getOutIndexByID(int vid, int pred)
{
	if (csr[0].vid2id[pred].find(vid) != csr[0].vid2id[pred].end())
		return csr[0].vid2id[pred][vid];
	else
		return -1;
}

int PathQueryHandler::getOutSize(int vid, int pred)
{
	int vIndex = getOutIndexByID(vid, pred);
	if (vIndex == -1) // This vertex does not participate in this pred's relations
		return 0;
	else if (vIndex == static_cast<int>(csr[0].offset_list[pred].size()) - 1)
		return csr[0].adjacency_list[pred].size() - csr[0].offset_list[pred][vIndex];
	else
		return csr[0].offset_list[pred][vIndex + 1] - csr[0].offset_list[pred][vIndex];
}

int PathQueryHandler::getOutVertID(int vid, int pred, int pos)
{
	if (pos >= getOutSize(vid, pred))
		return -1;
	int offset = csr[0].offset_list[pred][getOutIndexByID(vid, pred)];
	return csr[0].adjacency_list[pred][offset + pos];
}

int PathQueryHandler::getOutVertID(int vid, int pos)
{
	if (distinctOutEdges.find(vid) == distinctOutEdges.end())
		getTotalOutSize(vid, true); // Load into cache

	if (pos < static_cast<int>(distinctOutEdges[vid].size()))
		return *next(distinctOutEdges[vid].begin(), pos);
	else
		return -1;
}

int PathQueryHandler::getSetOutSize(int vid, const std::vector<int> &pred_set)
{
	int ret = 0;
	for (int pred : pred_set)
		ret += getOutSize(vid, pred);
	return ret;
}

int PathQueryHandler::getTotalOutSize(int vid, bool distinct)
{
	int ret = 0;
	if (!distinct)
	{
		for (int i = 0; i < static_cast<int>(csr[1].pre_num); i++)
			ret += getOutSize(vid, i);
	}
	else
	{
		if (distinctOutEdges.find(vid) == distinctOutEdges.end())
		{
			if (static_cast<int>(distinctOutEdges.size()) == cacheMaxSize)
			{
				int replacement = rand() % cacheMaxSize;
				distinctOutEdges.erase(next(distinctOutEdges.begin(), replacement));
				// cout << "distinctInEdges replaced entry " << replacement << endl;
			}

			distinctOutEdges[vid] = set<int>();
			for (int pred = 0; pred < static_cast<int>(csr[1].pre_num); pred++)
			{
				int vIndex = getOutIndexByID(vid, pred);
				if (vIndex == -1) // This vertex does not participate in this pred's relations
					continue;
				else if (vIndex == static_cast<int>(csr[0].offset_list[pred].size()) - 1 && csr[0].adjacency_list[pred].size() > csr[0].offset_list[pred][vIndex])
					distinctOutEdges[vid].insert(next(csr[0].adjacency_list[pred].begin(), csr[0].offset_list[pred][vIndex]),
												 csr[0].adjacency_list[pred].end());
				else if (csr[0].offset_list[pred][vIndex + 1] > csr[0].offset_list[pred][vIndex])
					distinctOutEdges[vid].insert(next(csr[0].adjacency_list[pred].begin(), csr[0].offset_list[pred][vIndex]),
												 next(csr[0].adjacency_list[pred].begin(), csr[0].offset_list[pred][vIndex + 1]));
			}
		}
		ret = distinctOutEdges[vid].size();
	}
	return ret;
}

void PathQueryHandler::inputGraph(string filename)
{
	// ifstream infile(filename.c_str());
	ifstream infile(filename);

	int n, numLabel;
	infile >> n >> numLabel;
	csr[0].init(numLabel);
	csr[1].init(numLabel);

	int **indegree = new int *[numLabel];
	int **outdegree = new int *[numLabel];
	for (int i = 0; i < numLabel; i++)
	{
		indegree[i] = new int[n];
		memset(indegree[i], 0, n * sizeof(int));
		outdegree[i] = new int[n];
		memset(outdegree[i], 0, n * sizeof(int));
	}

	int from, to, label;
	while (infile >> from >> to >> label)
	{
		outdegree[label][from]++;
		indegree[label][to]++;
	}
	int ***inAdjList = new int **[numLabel];
	int ***outAdjList = new int **[numLabel];
	for (int i = 0; i < numLabel; i++)
	{
		inAdjList[i] = new int *[n];
		outAdjList[i] = new int *[n];
		for (int j = 0; j < n; j++)
		{
			inAdjList[i][j] = new int[indegree[i][j]];
			outAdjList[i][j] = new int[outdegree[i][j]];
		}
	}
	int **pointer_in = new int *[numLabel];
	int **pointer_out = new int *[numLabel];
	for (int i = 0; i < numLabel; i++)
	{
		pointer_in[i] = new int[n];
		memset(pointer_in[i], 0, n * sizeof(int));
		pointer_out[i] = new int[n];
		memset(pointer_out[i], 0, n * sizeof(int));
	}

	infile.clear();
	infile.seekg(0);
	infile >> n >> numLabel;
	while (infile >> from >> to >> label)
	{
		outAdjList[label][from][pointer_out[label][from]] = to;
		pointer_out[label][from]++;
		inAdjList[label][to][pointer_in[label][to]] = from;
		pointer_in[label][to]++;
	}
	infile.close();

	int *pointer_outAdj = new int[numLabel];
	int *pointer_inAdj = new int[numLabel];
	memset(pointer_outAdj, 0, numLabel * sizeof(int));
	memset(pointer_inAdj, 0, numLabel * sizeof(int));
	for (int i = 0; i < numLabel; i++)
	{
		for (int j = 0; j < n; j++)
		{
			csr[0].id2vid[i].push_back(j);
			csr[0].vid2id[i][j] = j;
			csr[0].offset_list[i].push_back(pointer_outAdj[i]);
			for (int k = 0; k < outdegree[i][j]; k++)
			{
				csr[0].adjacency_list[i].push_back(outAdjList[i][j][k]);
				pointer_outAdj[i]++;
			}

			csr[1].id2vid[i].push_back(j);
			csr[1].vid2id[i][j] = j;
			csr[1].offset_list[i].push_back(pointer_inAdj[i]);
			for (int k = 0; k < indegree[i][j]; k++)
			{
				csr[1].adjacency_list[i].push_back(inAdjList[i][j][k]);
				pointer_inAdj[i]++;
			}
		}
	}

	for (int i = 0; i < numLabel; i++)
	{
		delete[] indegree[i];
		delete[] outdegree[i];
	}
	delete[] indegree;
	delete[] outdegree;
	for (int i = 0; i < numLabel; i++)
	{
		for (int j = 0; j < n; j++)
		{
			delete[] inAdjList[i][j];
			delete[] outAdjList[i][j];
		}
		delete[] inAdjList[i];
		delete[] outAdjList[i];
	}
	delete[] inAdjList;
	delete[] outAdjList;
	for (int i = 0; i < numLabel; i++)
	{
		delete[] pointer_in[i];
		delete[] pointer_out[i];
	}
	delete[] pointer_in;
	delete[] pointer_out;
	delete[] pointer_outAdj;
	delete[] pointer_inAdj;
}

void PathQueryHandler::printCSR()
{
	cout << "----------OUT----------" << endl;
	csr[0].print();
	cout << endl;
	cout << "----------IN----------" << endl;
	csr[1].print();
}

/**
	@param u the vertex u's id.
	@param v the vertex v's id.
	@param flag whether has passed the vertex v.
	@param q the node now dfs starts from.
	@param vis the nodes have been visited during this dfs.
	@param route route[i] stores the route from u to i.
	@param directed if false, treat all edges in the graph as bidirectional.
	@param pred_set the set of edge labels allowed.
	@param finished whether have found the simple circle: if true, return at once.
	@param ans store the final circle with u and v: if none, ans is empty.

	In fact, it is more like a recall than a DFS.
**/
void PathQueryHandler::dfs(map<int, vector<int>> &route, map<int, bool> &vis,
						   int q, int v, const vector<int> pred_set, vector<int> &ans, bool &finished)
{
	if (finished)
		return;
	int num_of_pred = pred_set.size();
	for (int i = 0; i < num_of_pred; ++i)
	{
		int num_out = getOutSize(q, pred_set[i]);
		for (int j = 0; j < num_out; ++j)
		{
			int temp = getOutVertID(q, pred_set[i], j);
			if (vis.find(temp) != vis.end() && vis[temp] == 1)
				continue;
			vis[temp] = 1;
			route[temp] = route[q];
			route[temp].push_back(pred_set[i]);
			route[temp].push_back(temp);
			if (temp == v)
			{
				finished = 1;
				ans = route[temp];
				return;
			}
			if (finished)
				return;
			dfs(route, vis, temp, v, pred_set, ans, finished); // get the childnode dfs.
			if (finished)
				return;
			// vis[temp] = 0;
			// if(temp == v && flag == 1)	flag = 0; //need to recover the sign.
		}
		// if(directed) continue;
		// if directed == 0, need to consider the traverse edge.
		int num_in = getInSize(q, pred_set[i]);
		if (finished)
			return;
		for (int j = 0; j < num_in; ++j)
		{
			int temp = getInVertID(q, pred_set[i], j);
			if (vis.find(temp) != vis.end() && vis[temp] == 1)
				continue;
			vis[temp] = 1;
			route[temp] = route[q];
			route[temp].push_back(pred_set[i]);
			route[temp].push_back(temp);
			if (temp == v)
			{
				finished = 1;
				ans = route[temp];
				return;
			}
			if (finished)
				return;
			dfs(route, vis, temp, v, pred_set, ans, finished);
			if (finished)
				return;
		}
	}
}

/**
	If there exists one or more SIMPLE CYCLES with all edge labels in pred_set
	and containing both the vertices u and v, return one of them; otherwise
	return an empty vector.

	@param uid the vertex u's ID.
	@param vid the vertex v's ID.
	@param directed if false, treat all edges in the graph as bidirectional.
	@param pred_set the set of edge labels allowed.
	@return a vector of vertex IDs representing the SIMPLE CYCLE, shaped like
	[v0, pred0, v1, pred1, ..., vk, predk], where v0 = u, and predi labels the
	edge from vi to vi+1 (predk labels the edge from vk to v0); empty if no
	such SIMPLE CYCLE exists.
*/
/*vector<int> PathQueryHandler::simpleCycle(int uid, int vid, bool directed,
	const vector<int> &pred_set)
{
	vector<int> ans;
	map<int, vector<int> > route;
	map<int, bool> vis;
	vis[uid] = 1;
	route[uid].push_back(uid);
	bool flag = false;
	if(uid == vid) flag = true;
	bool finished = false;
	dfs(route, vis, uid, flag, uid, vid, pred_set, ans, finished, directed);
	return ans;
}*/

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

bool PathQueryHandler::kHopReachableTest(int uid, int vid, bool directed, int k, const std::vector<int> &pred_set)
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

	int level = 0;
	queue<int> fwdQ;
	queue<int> fwdQ_next;
	queue<int> bwdQ;
	queue<int> bwdQ_next;
	fwdQ.push(uid);
	bwdQ.push(vid);
	map<int, int> fSetMark;
	map<int, int> bSetMark;
	fSetMark[uid] = 0;
	bSetMark[vid] = 0;

	while (level < k)
	{
		level++;
		while (!fwdQ.empty())
		{
			int curNode = fwdQ.front();
			fwdQ.pop();
			for (int pred : pred_set)
			{
				int num_out = getOutSize(curNode, pred);
				for (int i = 0; i < num_out; i++)
				{
					int outNode = getOutVertID(curNode, pred, i);
					if (fSetMark.find(outNode) == fSetMark.end())
					{
						fwdQ_next.push(outNode);
						fSetMark[outNode] = level;
					}
				}
				if (directed)
					continue;

				int num_in = getInSize(curNode, pred);
				for (int i = 0; i < num_in; i++)
				{
					int inNode = getInVertID(curNode, pred, i);
					if (fSetMark.find(inNode) == fSetMark.end())
					{
						fwdQ_next.push(inNode);
						fSetMark[inNode] = level;
					}
				}
			}
		}
		while (!bwdQ.empty())
		{
			int curNode = bwdQ.front();
			bwdQ.pop();
			for (int pred : pred_set)
			{
				int num_in = getInSize(curNode, pred);
				for (int i = 0; i < num_in; i++)
				{
					int inNode = getInVertID(curNode, pred, i);
					if (bSetMark.find(inNode) == bSetMark.end())
					{
						bwdQ_next.push(inNode);
						bSetMark[inNode] = level;
						if (fSetMark.find(inNode) !=
								fSetMark.end() &&
							fSetMark[inNode] + bSetMark[inNode] <= k)
							return true;
					}
				}

				if (directed)
					continue;
				int num_out = getOutSize(curNode, pred);
				for (int i = 0; i < num_out; i++)
				{
					int outNode = getOutVertID(curNode, pred, i);
					if (bSetMark.find(outNode) == bSetMark.end())
					{
						bwdQ_next.push(outNode);
						bSetMark[outNode] = level;
						if (fSetMark.find(outNode) !=
								fSetMark.end() &&
							fSetMark[outNode] + bSetMark[outNode] <= k)
							return true;
					}
				}
			}
		}
		if (fwdQ_next.empty() || bwdQ_next.empty())
			return false;
		swap(fwdQ_next, fwdQ);
		swap(bwdQ_next, bwdQ);
	}

	return false;
}

vector<int> PathQueryHandler::kHopReachablePath(int uid, int vid, bool directed, int k, const std::vector<int> &pred_set)
{
	vector<int> ret = shortestPath(uid, vid, directed, pred_set);
	cout << "ret.size:" << ret.size() << ",k:" << k << endl;
	if ((static_cast<int>(ret.size()) - 1) / 2 > k)
		ret.clear();
	return ret;
}

/**
 * @brief Supports LDBC SNB IC14 (v0.x and v1.x, old).
 * Assumes both uid and vid are of the type Person.
 * 
 * @param uid Person1
 * @param vid Person2
 * @return a vector of all paths and their weights as per IC14's requirements
 */
vector<pair<vector<int>, double>> PathQueryHandler::IC14(int uid, int vid, int knowsPred, int hasCreatorPred,
int typePred, int replyPred, int postId, int commentId) {
	vector<pair<vector<int>, double>> ret;
	if (uid == vid)
		return ret;
	vector<int> pred_set(1, knowsPred);
	vector<int> sp = shortestPath(uid, vid, true, pred_set);
	if (sp.empty())
		return ret;
	size_t spSz = sp.size();
	size_t spLen = (spSz - 1) / 2;
	vector<vector<int>> allSp = kHopEnumeratePath(uid, vid, -1, true, spLen, pred_set);

	// Collate all vertex pairs appearing in any shortest path
	unordered_map<int, unordered_map<int, double>> edgeWeight;
	for (const auto &oneSp : allSp) {
		for (size_t i = 0; i < spSz - 2; i += 2)
			edgeWeight[oneSp[i]][oneSp[i + 2]] = 0;
	}
	
	// Calculate weights
	vector<int> leftPost, leftComm, rightPost, rightComm;
	unordered_set<int> leftCommReplyOf, rightCommReplyOf;
	for (auto &e : edgeWeight) {
		getPostComments(e.first, hasCreatorPred, typePred, postId, commentId, leftPost, leftComm);
		getCommReplyOf(replyPred, leftComm, leftCommReplyOf);
		for (auto &dstW : e.second) {
			getPostComments(dstW.first, hasCreatorPred, typePred, postId, commentId, rightPost, rightComm);
			getCommReplyOf(replyPred, rightComm, rightCommReplyOf);
			
			size_t comm2post = 0, comm2comm = 0;
			for (int rp : rightPost) {
				if (leftCommReplyOf.find(rp) != leftCommReplyOf.end())
					comm2post++;
			}
			for (int rc : rightComm) {
				if (leftCommReplyOf.find(rc) != leftCommReplyOf.end())
					comm2comm++;
			}
			for (int lp : leftPost) {
				if (rightCommReplyOf.find(lp) != rightCommReplyOf.end())
					comm2post++;
			}
			for (int lc : leftComm) {
				if (leftCommReplyOf.find(lc) != leftCommReplyOf.end())
					comm2comm++;
			}
			dstW.second = double(comm2post) + double(comm2comm) * .5;
		}
	}

	// Report weights
	for (auto &oneSp : allSp) {
		double totalWeight = 0;
		for (size_t i = 0; i < spSz - 2; i += 2)
			totalWeight += edgeWeight[oneSp[i]][oneSp[i + 2]];
		ret.emplace_back(std::move(oneSp), totalWeight);
	}
	return ret;
}

void PathQueryHandler::getPostComments(int vid, int hasCreatorPred, int typePred, int postId, int commentId,
std::vector<int> &postVec, std::vector<int> &commentVec) {
	postVec.clear();
	commentVec.clear();
	int inSz = getInSize(vid, hasCreatorPred);
	for (int i = 0; i < inSz; i++) {
		int cur = getInVertID(vid, hasCreatorPred, i);
		if (getOutSize(cur, typePred) != 1)
			continue;
		int curType = getOutVertID(cur, typePred, 0);
		if (curType == postId)
			postVec.emplace_back(cur);
		else if (curType == commentId)
			commentVec.emplace_back(cur);
	}
}

void PathQueryHandler::getCommReplyOf(int replyPred, const std::vector<int> &commentVec, std::unordered_set<int> &commReplyOf) {
	commReplyOf.clear();
	for (int comm : commentVec) {
		int commOutSz = getOutSize(comm, replyPred);
		for (size_t i = 0; i < commOutSz; i++)
			commReplyOf.emplace(getOutVertID(comm, replyPred, i));
	}
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

void PathQueryHandler::updateBarrier(int uid, bool directed, const vector<int> &pred_set, std::vector<int> &bar, int l)
{
	if (bar[uid] > l)
	{
		bar[uid] = l;
		for (int pred : pred_set)
		{
			int inNum = getInSize(uid, pred);
			for (int i = 0; i < inNum; ++i)
				updateBarrier(getInVertID(uid, pred, i), directed, pred_set, bar, l + 1);
			if (directed)
				continue;
			int outNum = getOutSize(uid, pred);
			for (int i = 0; i < outNum; ++i)
				updateBarrier(getOutVertID(uid, pred, i), directed, pred_set, bar, l + 1);
		}
	}
}

/**
	Compute and return the number of triangles in the graph, only considering the
	edges in the path with labels in pred_set.

	@param directed if false, treat all edges in the graph as bidirectional.
					if true, only count cycle type triangle (eg: a->b->c->a)
	@param pred_set the set of edge labels allowed.
	@return the number of triangles in the graph.
**/
long long PathQueryHandler::triangleCounting(bool directed, const std::vector<int> &pred_set)
{
	if (pred_set.empty())
		return -1;

	long long numTriangle = 0;
	// loop each vertex and count triangle
	int vertex_num = getVertNum();
	for (int vid = 0; vid < vertex_num; ++vid)
	{
		// count triangle for vid

		// directed: get the in-neighbor set of the current node
		// directed: neighbor
		unordered_set<int> inSet;
		for (int pred : pred_set)
		{
			int inSize = getInSize(vid, pred);
			for (int j = 0; j < inSize; j++)
				inSet.insert(getInVertID(vid, pred, j));
			if (directed == 0)
			{
				int outSize = getOutSize(vid, pred);
				for (int j = 0; j < outSize; j++)
					inSet.insert(getOutVertID(vid, pred, j));
			}
		}

		// directed:
		// for each out-neighbor, intersect its out-neighbors with the current node's in-neighbors
		// the size of the intersection is the # of triangles
		if (directed)
		{
			for (int pred : pred_set)
			{
				int outSize = getOutSize(vid, pred);
				for (int j = 0; j < outSize; j++)
				{
					int outNode = getOutVertID(vid, pred, j);
					for (int pd : pred_set)
					{
						int outOutSize = getOutSize(outNode, pd);
						for (int k = 0; k < outOutSize; k++)
						{
							int outOutNode = getOutVertID(outNode, pd, k);
							if (inSet.find(outOutNode) != inSet.end())
								numTriangle++;
						}
					}
				}
			}
		}
		// undirected:
		// for each neighbor, intersect its neighbors(except current node) with the current node's neighbors
		else
		{
			for (int outNode : inSet)
			{
				unordered_set<int> nbrs;
				for (int pred : pred_set)
				{
					int outOutSize = getOutSize(outNode, pred);
					for (int k = 0; k < outOutSize; k++)
					{
						nbrs.insert(getOutVertID(outNode, pred, k));
					}
					int outInSize = getInSize(outNode, pred);
					for (int k = 0; k < outInSize; k++)
					{
						nbrs.insert(getInVertID(outNode, pred, k));
					}
				}
				for (int outOutNode : nbrs)
				{
					if (inSet.find(outOutNode) != inSet.end())
						numTriangle++;
				}
			}
		}
	}
	//}

	int dup_num = 6;
	if (directed)
		dup_num = 3;
	if (numTriangle % dup_num != 0)
	{
		cout << "ERROR!!!" << numTriangle << " is not triple of " << dup_num << "!" << endl;
		return -1;
	}
	cout << numTriangle / dup_num << endl;
	return numTriangle / dup_num;
}

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

/**
	Compute and return the PageRank of all vertices in the graph.

	@param directed if false, treat all edges in the graph as bidirectional.
	@param pred_set the set of edge labels allowed.
	@param alpha the damping parameter.
	@param maxIter the maximum number of iterations in power method eigenvalue solver.
	@param tol the error tolerance used to check convergence in power method solver.
	@return a vector that maps each vertex's ID to its PageRank value.
**/
void PathQueryHandler::PR(bool directed, const std::vector<int> &pred_set, int retNum, double alpha, int maxIter, double tol,
std::vector<std::pair<int, double>> &idx2val)
{
	// initialize
	int nodeNum = getVertNum();
	if (retNum > nodeNum)
		retNum = nodeNum;
	vector<int> neiNum(nodeNum);
	vector<double> oldPR(nodeNum), newPR(nodeNum);
	for (int vid = 0; vid < nodeNum; ++vid)
		for (int pred : pred_set)
		{
			neiNum[vid] += getOutSize(vid, pred);
			if (directed)
				continue;
			neiNum[vid] += getInSize(vid, pred);
		}
	double offset = (1 - alpha) / nodeNum;
	for (int vid = 0; vid < nodeNum; ++vid)
		oldPR[vid] = 1.0 / nodeNum, newPR[vid] = offset;
	// iterate
	bool stop = false;
	int cnt = 0;
	while (!stop && cnt++ <= maxIter) // PR(j) = d * sum(PR(i)/OUT(i)) + (1-d)
	{
		stop = true;
		// update
		for (int vid = 0; vid < nodeNum; ++vid)
		{
			if (!neiNum[vid]) // dangling nodes
			{
				double add = alpha * oldPR[vid] / nodeNum;
				for (int to = 0; to < nodeNum; ++to)
				{
					newPR[to] += add;
				}
			}
			else
			{
				double add = alpha * oldPR[vid] / neiNum[vid];
				for (int pred : pred_set)
				{
					int outNum = getOutSize(vid, pred);
					for (int i = 0; i < outNum; ++i)
					{
						int to = getOutVertID(vid, pred, i); // get the node
						newPR[to] += add;
					}
					if (directed)
						continue;
					int inNum = getInSize(vid, pred);
					for (int i = 0; i < inNum; ++i)
					{
						int to = getInVertID(vid, pred, i);
						newPR[to] += add;
					}
				}
			}
		}
		// compare
		for (int vid = 0; vid < nodeNum; ++vid)
		{
			if (abs(newPR[vid] - oldPR[vid]) > tol)
				stop = false;
			oldPR[vid] = newPR[vid];
			newPR[vid] = offset;
		}
	}
	idx2val.clear();
	for (int i = 0; i < nodeNum; i++)
		idx2val.emplace_back(i, oldPR[i]);
	// Extract top-k results
	if (retNum != -1 && retNum < nodeNum) {
		sort(idx2val.begin(), idx2val.end(), [](pair<size_t, double> const &l, pair<size_t, double> const &r)
			{ return l.second > r.second; });
		idx2val.erase(idx2val.begin() + retNum, idx2val.end());
	}
}

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

/**
	Compute and return the single-source shortest path lengths from vertex u.
	Note: edges are unweighted; do not return unreachable vertices.

	@param uid vertex u's ID.
	@param directed if false, treat all edges in the graph as bidirectional.
	@param pred_set the set of edge labels allowed.
	@return a map from each vertex's ID to the length of u's shortest path to it.
**/
unordered_map<int, int> PathQueryHandler::SSSPLen(int uid, bool directed, const std::vector<int> &pred_set)
{
	unordered_map<int, int> res;
	set<int> vis;
	queue<int> Q;
	Q.push(uid);
	vis.insert(uid);
	res[uid] = 0;
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
					res[to] = res[vid] + 1;
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
					res[to] = res[vid] + 1;
					Q.push(to);
				}
			}
		}
	}
	return res;
}

/**
	The label detection community detection algorithm.

	@param directed if false, treat all edges in the graph as bidirectional.
	@param pred_set the set of edge labels allowed.
	@param maxIter the maximum number of iterations.
	@return a vector of vectors, each containing the IDs of the vertices in a community.
**/
vector<vector<int>> PathQueryHandler::labelProp(bool directed, const std::vector<int> &pred_set, int maxIter)
{
	class LPA
	{
	public:
		PathQueryHandler &g;
		bool directed;
		const vector<int> &pred_set;

		unsigned nodeNum;
		unsigned edgeNum;
		vector<int> coms;
		unsigned comNum;
		vector<int> labels;
		vector<int> maxLabels;
		double maxQ;

		LPA(PathQueryHandler &_g, bool _directed, const vector<int> &_pred_set)
			: g(_g), directed(_directed), pred_set(_pred_set), maxQ(-1)
		{
			nodeNum = g.getVertNum();
			edgeNum = g.getEdgeNum();
			comNum = nodeNum;
			coms.resize(nodeNum);
			labels.resize(nodeNum);
			for (unsigned i = 0; i < nodeNum; i++)
				coms[i] = i;
			for (unsigned i = 0; i < nodeNum; i++)
				labels[i] = i;
		}

		void propagate()
		{
			vector<int> cnt;
			vector<int> maxLbl;
			for (unsigned vid = 0; vid < nodeNum; vid++)
			{
				// async update
				cnt.assign(nodeNum, 0);
				maxLbl.clear();
				int maxCnt = 0, maxNum = 0;
				for (int pred : pred_set)
				{
					int inNum = g.getInSize(vid, pred);
					for (int i = 0; i < inNum; ++i)
					{
						int to = g.getInVertID(vid, pred, i);
						cnt[labels[to]]++;
						if (maxCnt < cnt[labels[to]])
							maxCnt = cnt[labels[to]];
					}
					if (directed)
						continue;
					int outNum = g.getOutSize(vid, pred);
					for (int i = 0; i < outNum; ++i)
					{
						int to = g.getOutVertID(vid, pred, i);
						cnt[labels[to]]++;
						if (maxCnt < cnt[labels[to]])
							maxCnt = cnt[labels[to]];
					}
				}
				for (unsigned lbl = 0; lbl < nodeNum; lbl++)
				{
					if (cnt[lbl] == maxCnt)
					{
						maxLbl.push_back(lbl);
						maxNum++;
					}
				}
				if (maxCnt)
					labels[vid] = maxLbl[rand() % maxNum];
			}
			set<int> diffLabel;
			for (unsigned vid = 0; vid < nodeNum; vid++)
				diffLabel.insert(labels[vid]);
			comNum = 0;
			for (auto ele : diffLabel)
				coms[comNum++] = ele;
			coms.resize(comNum);
		}

		int edgeSum(int com)
		{
			int res = 0;
			set<int> nodes;
			for (int vid = 0; vid < (int)nodeNum; vid++)
				if (labels[vid] == com)
					nodes.insert(vid);
			for (auto &node : nodes)
			{
				for (int pred : pred_set)
				{
					int outNum = g.getOutSize(node, pred);
					for (int i = 0; i < outNum; ++i)
					{
						int to = g.getOutVertID(node, pred, i);
						if (nodes.find(to) != nodes.end())
							res++;
					}
					if (directed)
						continue;
					int inNum = g.getInSize(node, pred);
					for (int i = 0; i < inNum; ++i)
					{
						int to = g.getInVertID(node, pred, i);
						if (nodes.find(to) != nodes.end())
							res++;
					}
				}
			}
			return res;
		}

		int degreeSum(int com)
		{
			int res = 0;
			for (int vid = 0; vid < (int)nodeNum; vid++)
			{
				if (labels[vid] != com)
					continue;
				for (int pred : pred_set)
				{
					int outNum = g.getOutSize(vid, pred);
					res += outNum;
					if (directed)
						continue;
					int inNum = g.getInSize(vid, pred);
					res += inNum;
				}
			}
			return res;
		}

		double modularity()
		{
			double Q = 0, dblM = 2.0 * edgeNum;
			for (auto com : coms)
			{
				int edge = edgeSum(com);
				int degree = degreeSum(com);
				Q += edge / dblM - (degree / dblM) * (degree / dblM);
			}
			if (maxQ < Q)
				maxLabels = labels;
			return Q;
		}

		vector<vector<int>> output()
		{
			vector<vector<int>> res(comNum);
			map<int, int> lbl2id;
			int idx = 0;
			for (auto com : coms)
				lbl2id[com] = idx++;
			for (unsigned vid = 0; vid < nodeNum; vid++)
				res[lbl2id[maxLabels[vid]]].push_back(vid);
			return res;
		}
	};
	const int seed = 114514;
	const double tol = 10;
	const double thres = 0.001;
	std::srand(seed);
	LPA lpa(*this, directed, pred_set);
	double oldQ = -1, Q;
	double oldNum = -1, comNum;
	int cnt = 0;
	for (int t = 1; t <= maxIter; ++t)
	{
		lpa.propagate();
		Q = lpa.modularity();
		comNum = lpa.comNum;
		if (comNum == oldNum && abs(Q - oldQ) < thres)
		{
			if (cnt >= tol)
				break;
			else
				cnt++;
		}
		else
			cnt = 0;
		oldQ = Q;
		oldNum = comNum;
	}
	return lpa.output();
}

/**
	Compute and return the weakly connected components of the graph (when undirected,
	equivalent to the connected components).

	@param directed if false, treat all edges in the graph as bidirectional.
	@param pred_set the set of edge labels allowed.
	@return a vector of vectors, each containing the IDs of the vertices in a WCC.
**/
vector<vector<int>> PathQueryHandler::WCC(const std::vector<int> &pred_set)
{
	/*mark wcc*/
	unordered_map<int, int> vid2wccid;
	int wccid = 0;
	vector<int> wccsz;

	// loop each vertex for bfs start vertex

	// vector<unsigned> *id2vid = csr->id2vid;
	// unsigned pdnum = csr->pre_num;
	// for(unsigned pd=0; pd<pdnum; ++pd){
	//	for(int vertexid : id2vid[pd]){
	int vertex_num = getVertNum();
	for (int vertexid = 0; vertexid < vertex_num; ++vertexid)
	{
		if (vid2wccid.count(vertexid) == 0)
		{
			cout << "start vertexid: " << vertexid << endl;

			unordered_set<int> visited; // visited in current wcc
			// mark when pushing into queue: to avoid one same vertex being added twice in the same level
			queue<int> q;
			int uid = vertexid; // current vertex
			q.push(uid);
			visited.insert(uid);
			vid2wccid[uid] = wccid;
			while (q.empty() == 0)
			{
				uid = q.front();
				q.pop();

				// directed or not, all neighbors
				for (int pred : pred_set)
				{
					int sz = getInSize(uid, pred), vid;
					for (int j = 0; j < sz; j++)
					{
						vid = getInVertID(uid, pred, j);
						if (visited.count(vid) == 0)
						{
							q.push(vid);
							visited.insert(vid);
							vid2wccid[vid] = wccid;
						}
					}
					sz = getOutSize(uid, pred);
					for (int j = 0; j < sz; j++)
					{
						vid = getOutVertID(uid, pred, j);
						if (visited.count(vid) == 0)
						{
							q.push(vid);
							visited.insert(vid);
							vid2wccid[vid] = wccid;
						}
					}
				}
			}
			++wccid;
			cout << "current found WCC count: " << wccid << endl;

			cout << "new wcc (sz:" << visited.size() << "): ";
			for (auto v : visited)
			{
				cout << v << " ";
			}
			cout << endl;

			wccsz.push_back(visited.size());
		}
	}
	//}

	/*vid2wccid to result*/
	// already know the exact size of each wcc: utilize this to skip the "double alloc and copy" growing stage of STL vector
	vector<vector<int>> result(wccid); // call default constructor for each wcc vector, each 3 pointer, no further memory alloc
	for (int i = 0; i < wccid; ++i)
	{
		// reserve: Increase the capacity of the vector (the total number of elements that the vector can hold WITHOUT requiring reallocation)
		result[i].reserve(wccsz[i]);
	}
	for (auto pr : vid2wccid)
	{
		result[pr.second].push_back(pr.first);
	}

	// will call move constructor when construction return val
	return move(result);
}

/**
	Compute and return the (local) clustering coefficient of vertex u.

	@param uid vertex u's ID.
	@param directed if false, treat all edges in the graph as bidirectional.
	@param pred_set the set of edge labels allowed.
	@return the (local) clustering coefficient of vertex u.
**/
double PathQueryHandler::clusteringCoeff(int uid, bool directed, const std::vector<int> &pred_set)
{
	cout << "local clusteringCoeff" << endl;
	if (pred_set.empty())
		return -1;
	double lcc;
	int dins = 0, maxns = 0;
	if (directed)
	{
		unordered_set<int> inSet, neighbors;
		for (int pred : pred_set)
		{
			int inSize = getInSize(uid, pred);
			for (int i = 0; i < inSize; i++)
			{
				inSet.insert(getInVertID(uid, pred, i));
				neighbors.insert(getInVertID(uid, pred, i));
			}
		}

		for (int pred : pred_set)
		{
			int outSize = getOutSize(uid, pred);
			for (int j = 0; j < outSize; j++)
			{
				int outNode = getOutVertID(uid, pred, j);
				if (!neighbors.count(outNode))
					neighbors.insert(outNode);
				for (int pd : pred_set) // all out neighbors
				{
					int outOutSize = getOutSize(outNode, pd);
					for (int k = 0; k < outOutSize; k++)
					{
						int outOutNode = getOutVertID(outNode, pd, k);
						if (inSet.count(outOutNode))
							dins++; // directed neighbors size
					}
				}
			}
		}
		int ns = neighbors.size(); // indirected neighbors size
		maxns = ns * (ns - 1);
		if (maxns == 0)
		{
			lcc = 0;
		}
		else
		{
			lcc = (double)dins / maxns;
		}
	}
	else
	{
		unordered_set<int> neiSet;
		for (int pred : pred_set)
		{
			int inSize = getInSize(uid, pred);
			for (int i = 0; i < inSize; i++)
			{
				neiSet.insert(getInVertID(uid, pred, i));
			}
			int outSize = getOutSize(uid, pred);
			for (int j = 0; j < outSize; j++)
			{
				neiSet.insert(getOutVertID(uid, pred, j));
			}
		}

		for (int neiNode : neiSet)
		{
			unordered_set<int> nbrs;
			for (int pred : pred_set)
			{
				int outOutSize = getOutSize(neiNode, pred);
				for (int k = 0; k < outOutSize; k++)
				{
					nbrs.insert(getOutVertID(neiNode, pred, k));
				}
				int outInSize = getInSize(neiNode, pred);
				for (int k = 0; k < outInSize; k++)
				{
					nbrs.insert(getInVertID(neiNode, pred, k));
				}
			}
			for (int outOutNode : nbrs)
			{
				if (neiSet.find(outOutNode) != neiSet.end())
					dins++;
			}
		}
		dins /= 2;				// directed neighbors size (de-duplication)
		int ns = neiSet.size(); // indirected neighbors size
		maxns = ns * (ns - 1) / 2;
		if (maxns == 0)
		{
			lcc = 0;
		}
		else
		{
			lcc = (double)dins / maxns;
		}
	}
	return lcc;
}

/**
	Compute and return the (global) clustering coefficient of the graph.

	@param directed if false, treat all edges in the graph as bidirectional.
	@param pred_set the set of edge labels allowed.
	@return the (global) clustering coefficient of the graph.
**/
double PathQueryHandler::clusteringCoeff(bool directed, const std::vector<int> &pred_set)
{
	cout << "global clusteringCoeff" << endl;
	if (pred_set.empty())
		return -1;
	int n = getVertNum();
	int nclt = 0; // number of closed triplet
	int nopt = 0; // number of open triplet
	double gcc;

	for (int curnode = 0; curnode < n; curnode++)
	{
		unordered_set<int> neiSet;
		for (int pred : pred_set)
		{
			int inSize = getInSize(curnode, pred);
			for (int j = 0; j < inSize; j++)
			{
				neiSet.insert(getInVertID(curnode, pred, j));
			}
			int outSize = getOutSize(curnode, pred);
			for (int j = 0; j < outSize; j++)
			{
				neiSet.insert(getOutVertID(curnode, pred, j));
			}
		}

		for (int neiNode : neiSet)
		{
			unordered_set<int> neighbors;
			for (int pred : pred_set)
			{
				int outOutSize = getOutSize(neiNode, pred);
				for (int k = 0; k < outOutSize; k++)
				{
					neighbors.insert(getOutVertID(neiNode, pred, k));
				}
				int outInSize = getInSize(neiNode, pred);
				for (int k = 0; k < outInSize; k++)
				{
					neighbors.insert(getInVertID(neiNode, pred, k));
				}
			}
			for (int outOutNode : neighbors)
			{
				if (neiSet.find(outOutNode) != neiSet.end())
				{
					nclt++;
				}
				else
				{
					if (outOutNode == curnode)
						continue;
					nopt++;
				}
			}
		}
	}
	nclt /= 2;
	nopt /= 2;
	if (nclt + nopt == 0)
	{
		gcc = 0;
	}
	else
	{
		gcc = (double)nclt / (nclt + nopt);
	}
	return gcc;
}

/**
	Compute and return the maximum clique containing the input vertices (assuming
	the graph is undirected).

	@param uid_ls the vector of input vertices.
	@param pred_set the set of edge labels allowed.
	@return the vertices in the maximal clique containing the input vertices.
**/
namespace MaxKPX
{
    using ept = long; // edge pointer type

    class LinearHeap
    {
    public:
        int n;       // number vertices
        int key_cap; // the maximum allowed key value

        int min_key; // possible min key
        int max_key; // possible max key

        int *key_s; // key of vertices

        int *head_s; // head of doubly-linked list for a specific weight

        int *pre_s;  // pre for doubly-linked list
        int *next_s; // next for doubly-linked list

        LinearHeap(int _n, int _key_cap)
        {
            n = _n;
            key_cap = _key_cap;

            min_key = max_key = key_cap;

            head_s = key_s = pre_s = next_s = nullptr;
        }

        ~LinearHeap()
        {
            if (head_s != nullptr)
            {
                delete[] head_s;
                head_s = nullptr;
            }
            if (pre_s != nullptr)
            {
                delete[] pre_s;
                pre_s = nullptr;
            }
            if (next_s != nullptr)
            {
                delete[] next_s;
                next_s = nullptr;
            }
            if (key_s != nullptr)
            {
                delete[] key_s;
                key_s = nullptr;
            }
        }

        void init(int _n, int _key_cap, int *_id_s, int *_key_s)
        {
            if (key_s == nullptr)
                key_s = new int[n];
            if (pre_s == nullptr)
                pre_s = new int[n];
            if (next_s == nullptr)
                next_s = new int[n];
            if (head_s == nullptr)
                head_s = new int[key_cap + 1];

            // assert(_key_cap <= key_cap);
            min_key = max_key = _key_cap;
            for (int i = 0; i <= _key_cap; i++)
                head_s[i] = n;

            for (int i = 0; i < _n; i++)
            {
                int id = _id_s[i];
                int key = _key_s[id];
                // assert(id < n); assert(key <= _key_cap);

                key_s[id] = key;
                pre_s[id] = n;
                next_s[id] = head_s[key];
                if (head_s[key] != n)
                    pre_s[head_s[key]] = id;
                head_s[key] = id;

                if (key < min_key)
                    min_key = key;
            }
        }

        int get_key(int id) { return key_s[id]; }

        void get_ids(int *vs, int &vs_size)
        {
            for (int i = min_key; i <= max_key; i++)
            {
                for (int id = head_s[i]; id != n; id = next_s[id])
                {
                    vs[vs_size++] = id;
                }
            }
        }

        bool get_min(int &id, int &key)
        { // return true if success, return false otherwise
            while (min_key <= max_key && head_s[min_key] == n)
                ++min_key;
            if (min_key > max_key)
                return false;

            id = head_s[min_key];
            key = min_key;

            // assert(key_s[id] == key);

            return true;
        }

        bool pop_min(int &id, int &key)
        { // return true if success, return false otherwise
            while (min_key <= max_key && head_s[min_key] == n)
                ++min_key;
            if (min_key > max_key)
                return false;

            id = head_s[min_key];
            key = min_key;

            key_s[id] = key_cap + 1;
            // assert(key_s[id] == key);

            head_s[min_key] = next_s[id];
            if (head_s[min_key] != n)
                pre_s[head_s[min_key]] = n;
            return true;
        }

        int decrement(int id, int dec)
        {
            // assert(key_s[id] >= dec);
            if (key_s[id] > key_cap)
                return 0;

            if (pre_s[id] == n)
            {
                // assert(head_s[key_s[id]] == id);
                head_s[key_s[id]] = next_s[id];
                if (next_s[id] != n)
                    pre_s[next_s[id]] = n;
            }
            else
            {
                int pid = pre_s[id];
                next_s[pid] = next_s[id];
                if (next_s[id] != n)
                    pre_s[next_s[id]] = pid;
            }

            int &key = key_s[id];
            key -= dec;
            pre_s[id] = n;
            next_s[id] = head_s[key];
            if (head_s[key] != n)
                pre_s[head_s[key]] = id;
            head_s[key] = id;

            if (key < min_key)
                min_key = key;
            return key;
        }

        void del(int id)
        {
            if (key_s[id] > key_cap)
                return;

            if (pre_s[id] == n)
            {
                // assert(head_s[key_s[id]] == id);
                head_s[key_s[id]] = next_s[id];
                if (next_s[id] != n)
                    pre_s[next_s[id]] = n;
            }
            else
            {
                int pid = pre_s[id];
                next_s[pid] = next_s[id];
                if (next_s[id] != n)
                    pre_s[next_s[id]] = pid;
            }
        }
    };

    class Searcher
    {
    public:
        int n;

        char *matrix;
        int *commNeis;

        int *neiInG;
        int *neiInP;

        int K;
        int *bestSol;
        int bestSz;

        int *neighbors;
        int *nonneighbors;

        std::vector<int> addList;
        int addListSz = 0;
        std::vector<std::pair<int, int>> removed;
        int removeSz = 0;

        int *PC;     // union of P and C, where P is at the front
        int *PC_rid; // reverse ID for PC
        std::queue<int> Qv;
        std::queue<std::pair<int, int>> Qe;
        int *lvlID;

        int P_end;
        int C_end;
        int lvl;

        std::vector<std::pair<int, int>> vp;

        bool enable = true;
        bool enableInput = true;

    public:
        Searcher()
        {
            n = 0;
            matrix = NULL;
            commNeis = NULL;

            neiInG = neiInP = NULL;

            bestSol = NULL;
            K = bestSz = 0;

            neighbors = nonneighbors = NULL;

            PC = PC_rid = NULL;
            lvlID = NULL;
            removeSz = 0;
        }

        ~Searcher()
        {
            if (matrix != NULL)
            {
                delete[] matrix;
                matrix = NULL;
            }
            if (commNeis != NULL)
            {
                delete[] commNeis;
                commNeis = NULL;
            }
            if (neiInG != NULL)
            {
                delete[] neiInG;
                neiInG = NULL;
            }
            if (neiInP != NULL)
            {
                delete[] neiInP;
                neiInP = NULL;
            }
            if (bestSol != NULL)
            {
                delete[] bestSol;
                bestSol = NULL;
            }
            if (PC != NULL)
            {
                delete[] PC;
                PC = NULL;
            }
            if (PC_rid != NULL)
            {
                delete[] PC_rid;
                PC_rid = NULL;
            }
            if (neighbors != NULL)
            {
                delete[] neighbors;
                neighbors = NULL;
            }
            if (nonneighbors != NULL)
            {
                delete[] nonneighbors;
                nonneighbors = NULL;
            }
            if (lvlID != NULL)
            {
                delete[] lvlID;
                lvlID = NULL;
            }
        }

        void load(int _n, const std::vector<std::pair<int, int>> &vp)
        {
            n = _n;
            matrix = new char[n * n];
            commNeis = new int[n * n];

            neiInG = new int[n];
            neiInP = new int[n];
            bestSol = new int[n];
            PC = new int[n];
            PC_rid = new int[n];
            neighbors = new int[n];
            nonneighbors = new int[n];
            lvlID = new int[n];
            memset(commNeis, 0, sizeof(int) * n * n);
            memset(matrix, 0, sizeof(char) * n * n);
            for (int i = 0; i < n; i++)
                neiInG[i] = 0;
            for (int i = 0; i < vp.size(); i++)
            {
                assert(vp[i].first >= 0 && vp[i].first < n && vp[i].second >= 0 && vp[i].second < n);
                int a = vp[i].first, b = vp[i].second;
                neiInG[a]++;
                neiInG[b]++;
                matrix[a * n + b] = matrix[b * n + a] = 1;
            }
        }

        void run(int K_, std::vector<int> &res)
        {
            K = K_;
            bestSz = res.size();
            P_end = C_end = lvl = 0;
            init();
            if (PC_rid[0] < C_end)
            {
                if (!moveC2P_R(0))
                {
                    if (C_end >= bestSz)
                    {
                        enable = enableInput;
                        lvl++, reduce(0, 0), lvl--;
                        enable = true;
                    }
                }
            }
            if (bestSz > res.size())
            {
                res.clear();
                for (int i = 0; i < bestSz; i++)
                    res.push_back(bestSol[i]);
            }
        }

        void runWHOLE(int K_, std::vector<int> &res)
        {
            K = K_;
            bestSz = res.size();
            P_end = C_end = lvl = 0;
            init();
            enable = enableInput;
            lvl++, reduce(0, 0), lvl--;
            enable = true;
            if (bestSz > res.size())
            {
                res.clear();
                for (int i = 0; i < bestSz; i++)
                    res.push_back(bestSol[i]);
            }
        }

    private:
        void init()
        {
            // the following computes a degeneracy ordering
            while (!Qv.empty())
                Qv.pop();
            while (!Qe.empty())
                Qe.pop();
            int *seq = neighbors;
            int *core = nonneighbors;
            int *vis = PC;
            memset(vis, 0, sizeof(int) * n);
            int max_core = 0, UB = 0, idx = n;
            for (int i = 0; i < n; i++)
            {
                int u, min_degree = n;
                for (int j = 0; j < n; j++)
                    if (!vis[j] && neiInG[j] < min_degree)
                    {
                        u = j;
                        min_degree = neiInG[j];
                    }
                if (min_degree > max_core)
                    max_core = min_degree;
                core[u] = max_core;
                seq[i] = u;
                vis[u] = 1;

                int t_UB = core[u] + K;
                if (n - i < t_UB)
                    t_UB = n - i;
                if (t_UB > UB)
                    UB = t_UB;

                if (idx == n && min_degree + K >= n - i)
                    idx = i;

                for (int j = 0; j < n; j++)
                    if (!vis[j] && matrix[u * n + j])
                        --neiInG[j];
            }

            if (n - idx > bestSz)
            {
                bestSz = n - idx;
                for (int i = idx; i < n; i++)
                    bestSol[i - idx] = seq[i];
                // printf("Degen find a solution of size %u\n", bestSz);
            }

            memset(neiInP, 0, sizeof(int) * n);
            C_end = 0;
            for (int i = 0; i < n; i++)
                PC_rid[i] = n;
            for (int i = 0; i < n; i++)
                if (core[i] + K > bestSz)
                {
                    PC[C_end] = i;
                    PC_rid[i] = C_end;
                    ++C_end;
                }

            if (PC_rid[0] == n)
            {
                C_end = 0;
                return;
            }

            for (int i = 0; i < C_end; i++)
            {
                int u = PC[i];
                neiInG[u] = 0;
                for (int j = 0; j < C_end; j++)
                    if (matrix[u * n + PC[j]])
                        ++neiInG[u];
            }

            for (int i = 0; i < C_end; i++)
            {
                int neighbors_n = 0;
                char *t_matrix = matrix + PC[i] * n;
                for (int j = 0; j < C_end; j++)
                    if (t_matrix[PC[j]])
                        neighbors[neighbors_n++] = PC[j];
                for (int j = 0; j < neighbors_n; j++)
                    for (int k = j + 1; k < neighbors_n; k++)
                    {
                        ++commNeis[neighbors[j] * n + neighbors[k]];
                        ++commNeis[neighbors[k] * n + neighbors[j]];
                    }
            }

            memset(lvlID, 0, sizeof(int) * n);
            for (int i = 0; i < C_end; i++)
                lvlID[PC[i]] = n;

            for (int i = 0; i < C_end; i++)
                for (int j = i + 1; j < C_end; j++)
                {
                    if (matrix[PC[i] * n + PC[j]] && upper_bound_based_prune(0, PC[i], PC[j]))
                    {
                        Qe.push(std::make_pair(PC[i], PC[j]));
                    }
                }

            removeSz = 0;
            if (CTCP())
                C_end = 0;
        }

        bool checkNei(int u)
        {
            int notSat = 0;
            char *t_matrix = matrix + u * n;
            for (int i = P_end; i < C_end; ++i)
            {
                int ele = PC[i];
                if (u != ele && !t_matrix[ele] && K + neiInG[ele] < C_end)
                {
                    if (notSat)
                        return false;
                    notSat++;
                }
            }
            return true;
        }

        bool checkTriangle(int u)
        {
            std::vector<int> NotNei;
            char *t_matrix = matrix + u * n;
            for (int i = P_end; i < C_end; ++i)
            {
                int ele = PC[i];
                if (u != ele && !t_matrix[ele])
                    NotNei.emplace_back(ele);
            }
            return !matrix[NotNei[0] * n + NotNei[1]];
        }

        void branch(int &begIdx, int &endIdx)
        {
            endIdx = begIdx;
            int minnei = 0x3f3f3f3f;
            int pivot;
            char *t_matrix = matrix + 0 * n;
            for (int i = P_end; i < C_end; i++)
            {
                int v = PC[i];
                if (!t_matrix[v] && COND(v))
                { // HOP2 first
                    if (addListSz == endIdx)
                    {
                        addList.push_back(v);
                        ++endIdx;
                        ++addListSz;
                    }
                    else
                        addList[endIdx++] = v;
                    return;
                }
                if (neiInG[v] < minnei)
                {
                    minnei = neiInG[v];
                    pivot = v;
                }
            }
            t_matrix = matrix + pivot * n;
            for (int i = P_end; i < C_end; i++)
                if (!t_matrix[PC[i]])
                {
                    if (addListSz == endIdx)
                    {
                        addList.push_back(PC[i]);
                        ++endIdx;
                        ++addListSz;
                    }
                    else
                        addList[endIdx++] = PC[i];
                }
            std::sort(addList.data() + begIdx, addList.data() + endIdx, [&](int a, int b)
                      { return neiInG[a] > neiInG[b]; });
        }

        void reduce(int begIdx, int endIdx)
        {
            if (C_end <= bestSz)
            {
                return;
            }
            if (P_end == C_end)
            {
                bestSz = P_end;
                for (int i = 0; i < bestSz; i++)
                    bestSol[i] = PC[i];
                return;
            }
            bool isKPlex = true;
            for (int i = 0; i < C_end; i++)
                if (neiInG[PC[i]] + K < C_end)
                {
                    isKPlex = false;
                    break;
                }
            if (isKPlex)
            {
                bestSz = C_end;
                for (int i = 0; i < bestSz; i++)
                    bestSol[i] = PC[i];
                return;
            }
            else if (C_end == bestSz + 1)
            {
                return;
            }

            // reduction for high degree vertices
            int pivot = n;
            for (int i = P_end; i < C_end; i++)
            {
                int v = PC[i];
                const int NotNei = C_end - neiInG[v];
                if (NotNei <= 2)
                {
                    pivot = v;
                    break;
                }
                else if (neiInP[v] == P_end)
                {
                    if (NotNei == 3 && checkTriangle(v))
                    {
                        pivot = v;
                        break;
                    }
                    else if (NotNei <= K + 1 && checkNei(v))
                    {
                        pivot = v;
                        break;
                    }
                }
            }

            // reduction for the whole P
            if (pivot == n)
            {
                int cursor = bound();
                if (cursor >= C_end)
                {
                    return;
                }
                if (cursor + 1 == C_end)
                    pivot = PC[cursor];
            }

            // reduction for high degree vertices
            if (pivot < n)
            {
                int oldRemoveSz = removeSz;
                if (!moveC2P_R(pivot))
                    lvl++, reduce(begIdx, endIdx), lvl--;
                unR(oldRemoveSz);
                moveP2C();
                return;
            }

            // no more un-reduced lazy branches
            if (begIdx >= endIdx || PC_rid[addList[endIdx - 1]] >= C_end || PC_rid[addList[endIdx - 1]] < P_end)
                branch(begIdx, endIdx); // branch in lazy way for better reduction of generated branches

            pivot = addList[--endIdx];

            int oldBestSz = bestSz;
            int oldRemoveSz = removeSz;
            // the first branch includes u into S

            if (!moveC2P_R(pivot))
                lvl++, reduce(begIdx, endIdx), lvl--;
            unR(oldRemoveSz);

            while (!Qe.empty())
                Qe.pop();
            // the second branch exclude u from S

            if (removed.size() == removeSz)
            {
                removed.push_back(std::make_pair(-1, pivot));
                ++removeSz;
            }
            else
                removed[removeSz++] = std::make_pair(-1, pivot);
            bool pruned = moveP2X_R();
            if (!pruned && bestSz > oldBestSz)
                pruned = updateR();
            if (!pruned && !CTCP())
                lvl++, reduce(endIdx, endIdx), lvl--;
            unR(oldRemoveSz);
        }

        bool moveC2P_R(int u)
        {
            swap_pos(P_end, PC_rid[u]);
            ++P_end;
            char *t_matrix = matrix + u * n;

            int neighbors_n = 0, nonneighbors_n = 0;
            for (int i = 0; i < C_end; i++)
                if (i != P_end - 1)
                {
                    if (t_matrix[PC[i]])
                        neighbors[neighbors_n++] = PC[i];
                    else
                        nonneighbors[nonneighbors_n++] = PC[i];
                }

            for (int i = 0; i < neighbors_n; i++)
                ++neiInP[neighbors[i]];

            if (neiInP[u] + K == P_end)
            { // only neighbors of u in R can be candidates --- RR2
                int i = 0;
                while (i < nonneighbors_n && PC_rid[nonneighbors[i]] < P_end)
                    ++i;
                for (; i < nonneighbors_n; i++)
                { // remove non-neighbors from R
                    assert(lvlID[nonneighbors[i]] > lvl);
                    lvlID[nonneighbors[i]] = lvl;
                    Qv.push(nonneighbors[i]);
                }
            }
            else
            { // only non-neighbors of u may change their allowance --- RR1
                int i = 0;
                while (i < nonneighbors_n && PC_rid[nonneighbors[i]] < P_end)
                    ++i;
                for (; i < nonneighbors_n; i++)
                    if (P_end - neiInP[nonneighbors[i]] >= K)
                    {
                        assert(lvlID[nonneighbors[i]] > lvl);
                        lvlID[nonneighbors[i]] = lvl;
                        Qv.push(nonneighbors[i]);
                    }
            }

            // RR2
            for (int i = 0; i < nonneighbors_n && PC_rid[nonneighbors[i]] < P_end; i++)
                if (neiInP[nonneighbors[i]] + K == P_end)
                {
                    char *tt_matrix = matrix + nonneighbors[i] * n;
                    for (int j = P_end; j < C_end; j++)
                        if (lvlID[PC[j]] > lvl && !tt_matrix[PC[j]])
                        {
                            lvlID[PC[j]] = lvl;
                            Qv.push(PC[j]);
                        }
                }
            if (enable)
            {
                // RR4
                for (int i = 0; i < nonneighbors_n; i++)
                {
                    int v = nonneighbors[i];
                    assert(!t_matrix[v]);
                    if (PC_rid[v] < P_end || lvlID[v] == lvl || t_matrix[v])
                        continue;
                    if (upper_bound_based_prune(P_end, u, v))
                    {
                        lvlID[v] = lvl;
                        Qv.push(v);
                    }
                }

                // update cn(.,.)
                for (int i = 0; i < neighbors_n; i++)
                { // process common neighbors of u
                    for (int j = i + 1; j < neighbors_n; j++)
                    {
#ifndef NDEBUG
                        if (!commNeis[neighbors[i] * n + neighbors[j]])
                        {
                            printf("cn[neighbors[i]*n + neighbors[j]]: %u %u\n", commNeis[neighbors[i] * n + neighbors[j]], commNeis[neighbors[j] * n + neighbors[i]]);
                        }
#endif
                        assert(commNeis[neighbors[i] * n + neighbors[j]]);
                        --commNeis[neighbors[i] * n + neighbors[j]];
                        --commNeis[neighbors[j] * n + neighbors[i]];
                    }
                }
                int new_n = 0;
                for (int i = 0; i < nonneighbors_n; i++)
                    if (lvlID[nonneighbors[i]] > lvl)
                        nonneighbors[new_n++] = nonneighbors[i];
                nonneighbors_n = new_n;
                for (int i = 1; i < nonneighbors_n; i++)
                { // process common non-neighbors of u
                    int w = nonneighbors[i];
                    for (int j = 0; j < i; j++)
                    {
                        int v = nonneighbors[j];
                        if (!upper_bound_based_prune(P_end, v, w))
                            continue;
                        if (PC_rid[w] < P_end)
                            return true; // v, w \in S --- UB2
                        else if (PC_rid[v] >= P_end)
                        { // v, w, \in R --- RR5
                            if (matrix[v * n + w])
                                Qe.push(std::make_pair(v, w));
                        }
                        else
                        { // RR4
                            assert(lvlID[w] > lvl);
                            lvlID[w] = lvl;
                            Qv.push(w);
                            break;
                        }
                    }
                }
            }
            return CTCP();
        }

        bool CTCP()
        {
            if (!enable)
            {
                while (!Qe.empty())
                    Qe.pop();
            }
            while (!Qv.empty() || !Qe.empty())
            {
                while (!Qv.empty())
                {
                    int u = Qv.front();
                    Qv.pop(); // remove u
                    assert(PC[PC_rid[u]] == u);
                    assert(PC_rid[u] >= P_end && PC_rid[u] < C_end);
                    --C_end;
                    swap_pos(PC_rid[u], C_end);
                    bool terminate = false;
                    int neighbors_n = 0;
                    char *t_matrix = matrix + u * n;
                    for (int i = 0; i < C_end; i++)
                        if (t_matrix[PC[i]])
                        {
                            int w = PC[i];
                            neighbors[neighbors_n++] = w;
                            --neiInG[w];
                            if (neiInG[w] + K <= bestSz)
                            {
                                if (i < P_end)
                                    terminate = true; // UB1
                                else if (lvlID[w] > lvl)
                                { // RR3
                                    lvlID[w] = lvl;
                                    Qv.push(w);
                                }
                            }
                        }
                    // UB1
                    if (terminate)
                    {
                        for (int i = 0; i < neighbors_n; i++)
                            ++neiInG[neighbors[i]];
                        lvlID[u] = n;
                        ++C_end;
                        return true;
                    }
                    else
                    {
                        if (removed.size() == removeSz)
                        {
                            removed.push_back(std::make_pair(-1, u));
                            ++removeSz;
                        }
                        else
                            removed[removeSz++] = std::make_pair(-1, u);
                    }

                    if (enable)
                    {
                        for (int i = 1; i < neighbors_n; i++)
                        {
                            int w = neighbors[i];
                            for (int j = 0; j < i; j++)
                            {
                                int v = neighbors[j];
                                assert(commNeis[v * n + w]);
#ifndef NDEBUG
                                int common_neighbors = 0;
                                for (int k = P_end; k <= C_end; k++)
                                    if (matrix[PC[k] * n + v] && matrix[PC[k] * n + w])
                                        ++common_neighbors;
                                assert(commNeis[v * n + w] == common_neighbors);
                                assert(commNeis[w * n + v] == common_neighbors);
#endif
                                --commNeis[v * n + w];
                                --commNeis[w * n + v];
#ifndef NDEBUG
                                common_neighbors = 0;
                                for (int k = P_end; k < C_end; k++)
                                    if (matrix[PC[k] * n + v] && matrix[PC[k] * n + w])
                                        ++common_neighbors;
                                assert(commNeis[v * n + w] == common_neighbors);
                                assert(commNeis[w * n + v] == common_neighbors);
#endif

                                if (!upper_bound_based_prune(P_end, v, w))
                                    continue;

                                if (PC_rid[w] < P_end)
                                    terminate = true; // v, w \in S --- UB2
                                else if (PC_rid[v] >= P_end)
                                { // v, w, \in R --- RR5
                                    if (matrix[v * n + w])
                                        Qe.push(std::make_pair(v, w));
                                }
                                else if (lvlID[w] > lvl)
                                { // RR4
                                    lvlID[w] = lvl;
                                    Qv.push(w);
                                }
                            }
                        }
                        if (terminate)
                        {
                            return true;
                        }
                    }
                }

                if (Qe.empty())
                    break;

                if (enable)
                {
                    int v = Qe.front().first, w = Qe.front().second;
                    Qe.pop();
                    if (lvlID[v] <= lvl || lvlID[w] <= lvl || !matrix[v * n + w])
                        continue;
                    assert(PC_rid[v] >= P_end && PC_rid[v] < C_end && PC_rid[w] >= P_end && PC_rid[w] < C_end);

                    if (neiInG[v] + K <= bestSz + 1)
                    {
                        lvlID[v] = lvl;
                        Qv.push(v);
                    }
                    if (neiInG[w] + K <= bestSz + 1)
                    {
                        lvlID[w] = lvl;
                        Qv.push(w);
                    }
                    if (!Qv.empty())
                        continue;
                    assert(matrix[v * n + w]);
                    matrix[v * n + w] = matrix[w * n + v] = 0;
                    --neiInG[v];
                    --neiInG[w];

                    if (removed.size() == removeSz)
                    {
                        removed.push_back(std::make_pair(v, w));
                        ++removeSz;
                    }
                    else
                        removed[removeSz++] = std::make_pair(v, w);

                    char *t_matrix = matrix + v * n;
                    for (int i = 0; i < C_end; i++)
                        if (t_matrix[PC[i]])
                        {
                            --commNeis[w * n + PC[i]];
                            --commNeis[PC[i] * n + w];
                            if (!upper_bound_based_prune(P_end, w, PC[i]))
                                continue;
                            if (i < P_end)
                            {
                                if (lvlID[w] > lvl)
                                {
                                    lvlID[w] = lvl;
                                    Qv.push(w);
                                }
                            }
                            else if (matrix[w * n + PC[i]])
                                Qe.push(std::make_pair(w, PC[i]));
                        }
                    t_matrix = matrix + w * n;
                    for (int i = 0; i < C_end; i++)
                        if (t_matrix[PC[i]])
                        {
                            --commNeis[v * n + PC[i]];
                            --commNeis[PC[i] * n + v];
                            if (!upper_bound_based_prune(P_end, v, PC[i]))
                                continue;
                            if (i < P_end)
                            {
                                if (lvlID[v] > lvl)
                                {
                                    lvlID[v] = lvl;
                                    Qv.push(v);
                                }
                            }
                            else if (matrix[v * n + PC[i]])
                                Qe.push(std::make_pair(v, PC[i]));
                        }
                }
            }
            return false;
        }

        void unR(int oldRemoveSz)
        {
            while (!Qv.empty())
            {
                int u = Qv.front();
                Qv.pop();
                assert(lvlID[u] == lvl && PC_rid[u] < C_end);
                lvlID[u] = n;
            }
            while (removeSz > oldRemoveSz)
            {
                int v = removed[--removeSz].first, w = removed[removeSz].second;
                if (v == -1)
                {
                    int u = w; // remove u
                    assert(PC[PC_rid[u]] == u && PC_rid[u] == C_end);
                    C_end++;
                    lvlID[u] = n;
                    int neighbors_n = 0;
                    char *t_matrix = matrix + u * n;
                    for (int i = 0; i < C_end; i++)
                        if (t_matrix[PC[i]])
                        {
                            int w = PC[i];
                            neighbors[neighbors_n++] = w;
                            ++neiInG[w];
                        }
                    if (enable)
                    {
                        for (int i = 1; i < neighbors_n; i++)
                        {
                            int w = neighbors[i];
                            for (int j = 0; j < i; j++)
                            {
                                int v = neighbors[j];
                                ++commNeis[v * n + w];
                                ++commNeis[w * n + v];
                            }
                        }
                    }
                }
                else
                {
                    assert(PC_rid[v] >= P_end && PC_rid[v] < C_end && PC_rid[w] >= P_end && PC_rid[w] < C_end);
                    assert(lvlID[v] == n && lvlID[w] == n && !matrix[v * n + w]);
                    matrix[v * n + w] = matrix[w * n + v] = 1;
                    ++neiInG[v];
                    ++neiInG[w];

                    char *t_matrix = matrix + v * n;
                    for (int i = 0; i < C_end; i++)
                        if (t_matrix[PC[i]])
                        {
                            ++commNeis[w * n + PC[i]];
                            ++commNeis[PC[i] * n + w];
                        }
                    t_matrix = matrix + w * n;
                    for (int i = 0; i < C_end; i++)
                        if (t_matrix[PC[i]])
                        {
                            ++commNeis[v * n + PC[i]];
                            ++commNeis[PC[i] * n + v];
                        }
                }
            }
        }

        void moveP2C()
        {
            assert(P_end);
            int u = PC[--P_end];
            int neighbors_n = 0;
            char *t_matrix = matrix + u * n;
            for (int i = 0; i < C_end; i++)
                if (t_matrix[PC[i]])
                    neighbors[neighbors_n++] = PC[i];
            for (int i = 0; i < neighbors_n; i++)
                --neiInP[neighbors[i]];

            if (enable)
            {
                for (int i = 0; i < neighbors_n; i++)
                {
                    int v = neighbors[i];
                    for (int j = i + 1; j < neighbors_n; j++)
                    {
                        int w = neighbors[j];
                        ++commNeis[v * n + w];
                        ++commNeis[w * n + v];
                    }
                }
            }
        }

        bool moveP2X_R()
        {
            assert(P_end);
            int u = PC[P_end - 1];
            --P_end;
            --C_end;
            swap_pos(P_end, C_end);
            lvlID[u] = lvl;

            bool ret = false;
            int neighbors_n = 0;
            char *t_matrix = matrix + u * n;
            for (int i = 0; i < C_end; i++)
                if (t_matrix[PC[i]])
                    neighbors[neighbors_n++] = PC[i];
            for (int i = 0; i < neighbors_n; i++)
            {
                --neiInP[neighbors[i]];
                --neiInG[neighbors[i]];
                if (neiInG[neighbors[i]] + K <= bestSz)
                {
                    if (PC_rid[neighbors[i]] < P_end)
                        ret = true;
                    else
                    {
                        assert(lvlID[neighbors[i]] > lvl);
                        lvlID[neighbors[i]] = lvl;
                        Qv.push(neighbors[i]);
                    }
                }
            }
            if (ret)
                return true;

            if (enable)
            {
                for (int i = 1; i < neighbors_n; i++)
                    if (lvlID[neighbors[i]] > lvl)
                    {
                        int w = neighbors[i];
                        for (int j = 0; j < i; j++)
                        {
                            int v = neighbors[j];
                            if (!upper_bound_based_prune(P_end, v, w))
                                continue;

                            if (PC_rid[w] < P_end)
                                return true; // v, w \in S
                            else if (PC_rid[v] >= P_end)
                            { // v, w, \in R
                                if (matrix[v * n + w])
                                    Qe.push(std::make_pair(v, w));
                            }
                            else
                            {
                                assert(lvlID[w] > lvl);
                                lvlID[w] = lvl;
                                Qv.push(w);
                                break;
                            }
                        }
                    }
            }
            return false;
        }

        bool updateR()
        {
            for (int i = 0; i < P_end; i++)
                if (neiInG[PC[i]] + K <= bestSz)
                    return true;

            if (enable)
            {
                for (int i = 0; i < P_end; i++)
                    for (int j = i + 1; j < P_end; j++)
                    {
                        if (upper_bound_based_prune(P_end, PC[i], PC[j]))
                            return true;
                    }
            }

            for (int i = P_end; i < C_end; i++)
                if (lvlID[PC[i]] > lvl)
                {
                    if (P_end - neiInP[PC[i]] >= K || neiInG[PC[i]] + K <= bestSz)
                    {
                        assert(lvlID[PC[i]] > lvl);
                        lvlID[PC[i]] = lvl;
                        Qv.push(PC[i]);
                        continue;
                    }
                    char *t_matrix = matrix + PC[i] * n;
                    for (int j = 0; j < P_end; j++)
                    {
                        if ((P_end - neiInP[PC[j]] == K && !t_matrix[PC[j]]) || (enable && upper_bound_based_prune(P_end, PC[i], PC[j])))
                        {
                            assert(lvlID[PC[i]] > lvl);
                            lvlID[PC[i]] = lvl;
                            Qv.push(PC[i]);
                            break;
                        }
                    }
                }

            if (enable)
            {
                for (int i = P_end; i < C_end; i++)
                    if (lvlID[PC[i]] > lvl)
                    {
                        for (int j = i + 1; j < C_end; j++)
                            if (lvlID[PC[i]] < lvl && matrix[PC[i] * n + PC[j]])
                            {
                                if (upper_bound_based_prune(P_end, PC[i], PC[j]))
                                    Qe.push(std::make_pair(PC[i], PC[j]));
                            }
                    }
            }
            return false;
        }

        int bound()
        {
            vp.clear();
            for (int i = 0; i < P_end; i++)
                vp.push_back(std::make_pair(K - (P_end - neiInP[PC[i]]), PC[i]));
            // for(ui i = 0;i < P_end;i ++) vp.push_back(std::make_pair(-(neiInG[PC[i]]-neiInP[PC[i]]), PC[i]));
            sort(vp.begin(), vp.end());
            int UB = P_end, cursor = P_end;
            for (int i = 0; i < (int)vp.size(); i++)
            {
                int u = vp[i].second;
                if (vp[i].first == 0)
                    continue;
                int count = 0;
                char *t_matrix = matrix + u * n;
                for (int j = cursor; j < C_end; j++)
                    if (!t_matrix[PC[j]])
                    {
                        if (j != cursor + count)
                            swap_pos(j, cursor + count);
                        ++count;
                    }
                int t_ub = count;
                if (vp[i].first < t_ub)
                    t_ub = vp[i].first;
                if (UB + t_ub <= bestSz)
                {
                    UB += t_ub;
                    cursor += count;
                }
                else
                {
                    return cursor + (bestSz - UB);
                }
            }
            cursor += (bestSz - UB);
            if (cursor > C_end)
                cursor = C_end;
            return cursor;
        }

        bool upper_bound_based_prune(int P_end, int u, int v)
        {
            // ui ub = P_end + 2*K - (P_end - degree_in_S[u]) - (P_end - degree_in_S[v]) + cn[u*n + v];
            int ub = 2 * K + neiInP[u] - P_end + neiInP[v] + commNeis[u * n + v];
            if (PC_rid[u] >= P_end)
            {
                --ub; // P_end ++
                if (matrix[u * n + v])
                    ++ub; // neiInP[v] ++
            }
            if (PC_rid[v] >= P_end)
            {
                --ub;
                if (matrix[v * n + u])
                    ++ub;
            }
            return ub <= bestSz;
        }

        void swap_pos(int i, int j)
        {
            std::swap(PC[i], PC[j]);
            PC_rid[PC[i]] = i;
            PC_rid[PC[j]] = j;
        }

        bool COND(int v)
        {
            return (neiInG[v] + K <= bestSz + 1) || (neiInP[v] + K == P_end + 1) || (neiInP[0] + K == P_end + 1);
        }
    };

    class Graph
    {
    public:
		PathQueryHandler &handler;
		const std::vector<int> &pred_set;
        int K;            // the value of k in k-plex
        int n;            // number of nodes of the graph
        ept m;           // number of edges of the graph

        ept *pstart; // offset of neighbors of nodes
        ept *pend;   // used in search
        ept *pend_buf;
        int *edges; // adjacent ids of edges

        std::vector<int> kplex;

        int *s_degree;
        ept *s_pstart;
        ept *s_pend;
        int *s_edges;
        int *s_peel_sequence;
        int *s_core;
        char *s_vis;
        LinearHeap *s_heap;

        int *s_edgelist_pointer;
        int *s_tri_cnt;
        int *s_edge_list;
        int *s_active_edgelist;
        char *s_deleted;

        Graph(PathQueryHandler &_handler, const std::vector<int> &_pred_set, const int _K)
		:handler(_handler),pred_set(_pred_set),K(_K)
        {
            n = m = 0;
            pstart = nullptr;
            pend = pend_buf = nullptr;
            edges = nullptr;
            kplex.clear();
            s_degree = s_edges = NULL;
            s_pstart = s_pend = NULL;
            s_peel_sequence = s_core = NULL;
            s_vis = NULL;
            s_heap = NULL;
            s_edgelist_pointer = NULL;
            s_tri_cnt = s_edge_list = NULL;
            s_active_edgelist = NULL;
            s_deleted = NULL;
        }

        ~Graph()
        {
            if (pstart != nullptr)
            {
                delete[] pstart;
                pstart = nullptr;
            }
            if (pend != nullptr)
            {
                delete[] pend;
                pend = nullptr;
            }
            if (pend_buf != NULL)
            {
                delete[] pend_buf;
                pend_buf = NULL;
            }
            if (edges != nullptr)
            {
                delete[] edges;
                edges = nullptr;
            }
            if (s_degree != NULL)
            {
                delete[] s_degree;
                s_degree = NULL;
            }
            if (s_pstart != NULL)
            {
                delete[] s_pstart;
                s_pstart = NULL;
            }
            if (s_pend != NULL)
            {
                delete[] s_pend;
                s_pend = NULL;
            }
            if (s_edges != NULL)
            {
                delete[] s_edges;
                s_edges = NULL;
            }
            if (s_peel_sequence != NULL)
            {
                delete[] s_peel_sequence;
                s_peel_sequence = NULL;
            }
            if (s_core != NULL)
            {
                delete[] s_core;
                s_core = NULL;
            }
            if (s_vis != NULL)
            {
                delete[] s_vis;
                s_vis = NULL;
            }
            if (s_heap != NULL)
            {
                delete s_heap;
                s_heap = NULL;
            }
            if (s_edgelist_pointer != NULL)
            {
                delete[] s_edgelist_pointer;
                s_edgelist_pointer = NULL;
            }
            if (s_active_edgelist != NULL)
            {
                delete[] s_active_edgelist;
                s_active_edgelist = NULL;
            }
            if (s_deleted != NULL)
            {
                delete[] s_deleted;
                s_deleted = NULL;
            }
        }

        void read()
        {
			n = handler.getVertNum();
			m = handler.getSetEdgeNum(pred_set);

            // printf("\tn = %d;  m = %d (undirected)\n", n, m);

            if (pstart == nullptr)
                pstart = new ept[n + 1];
            if (edges == nullptr)
                edges = new int[2*m];

            pstart[0] = 0;
            for (int uid = 0; uid < n; uid++)
            {
				int sum=0;
				for (auto pred : pred_set)
				{
					int outNum = handler.getOutSize(uid, pred);
					for (int i = 0; i < outNum; ++i)
					{
						int to = handler.getOutVertID(uid, pred, i);
						edges[pstart[uid]+sum]=to;
						sum++;
					}
					int inNum = handler.getInSize(uid, pred);
					for (int i = 0; i < inNum; ++i)
					{
						int to = handler.getInVertID(uid, pred, i);
						edges[pstart[uid]+sum]=to;
						sum++;
					}
				}
				// remove self loops and parallel edges
				int *buff = edges + pstart[uid];
				sort(buff, buff + sum);
				int idx = 0;
				for (int j = 0; j < sum; j++)
				{
					if (buff[j] >= n)
						printf("vertex id %u wrong\n", buff[j]);
					if (buff[j] == uid || (j > 0 && buff[j] == buff[j - 1]))
						continue;
					buff[idx++] = buff[j];
				}
                pstart[uid + 1] = pstart[uid] + idx;
            }
        }

        void search()
        {
            kplex.resize(2 * K - 2);
            int max_degree = 0;
            for (int i = 0; i < n; i++)
            {
                if (pstart[i + 1] - pstart[i] > max_degree)
                    max_degree = pstart[i + 1] - pstart[i];
            }
            heuristic_kplex_max_degree(10);
            int oldn = n;
            int *seq = new int[n];
            int *core = new int[n];
            int *degree = new int[n];
            char *vis = new char[n];

            LinearHeap *heap = new LinearHeap(n, n - 1);

            int UB = degen(n, seq, core, pstart, edges, degree, vis, heap, true);

            delete heap;
            delete[] vis;
            delete[] degree;

            if (kplex.size() < UB)
            {
                int old_size = kplex.size();
                int *out_mapping = new int[n];
                int *rid = new int[n];

                shrink_graph(n, m, seq, core, out_mapping, NULL, rid, pstart, edges, true);
                // delete[] core; core = NULL;

                int *degree = new int[n];
                for (int i = 0; i < n; i++)
                    degree[i] = pstart[i + 1] - pstart[i];

                LinearHeap *linear_heap = new LinearHeap(n, n - 1);
                linear_heap->init(n, n - 1, seq, degree);

                assert(pend == nullptr);
                pend = new ept[n];

                int *edgelist_pointer = new int[m];
                oriented_triangle_counting(n, m, seq, pstart, pend, edges, edgelist_pointer, rid); // edgelist_pointer currently stores triangle_counts

                // delete[] seq; seq = NULL;

                pend_buf = new ept[n];
                int *edge_list = new int[m];
                int *tri_cnt = new int[m / 2];
                reorganize_oriented_graph(n, tri_cnt, edge_list, pstart, pend, pend_buf, edges, edgelist_pointer, rid);

                for (int i = 0; i < n; i++)
                    pend[i] = pstart[i + 1];

                int *active_edgelist = new int[m >> 1];
                int active_edgelist_n = m >> 1;
                for (int i = 0; i < (m >> 1); i++)
                    active_edgelist[i] = i;

                int *Qe = new int[m >> 1];
                char *deleted = new char[m >> 1];
                memset(deleted, 0, sizeof(char) * (m >> 1));
                char *exists = new char[n];
                memset(exists, 0, sizeof(char) * n);

                int *Qv = new int[n];
                int Qv_n = 0;
                Searcher *kplex_solver = new Searcher();

                if (kplex.size() > 2 * K - 2)
                {
                    m -= 2 * peeling(n, linear_heap, Qv, Qv_n, kplex.size() + 1 - K, Qe, true, kplex.size() + 1 - 2 * K, tri_cnt, active_edgelist, active_edgelist_n, edge_list, edgelist_pointer, deleted, degree, pstart, pend, edges, exists);
                    // printf("*** After core-truss co-pruning: n = %d, m = %d, density = %.4lf\n", n - Qv_n, m / 2, 1.0 * m / (n - Qv_n) / (n - Qv_n - 1));
                }

                int max_n = n - Qv_n;
                s_degree = new int[max_n];
                s_pstart = new ept[max_n + 1];
                s_pend = new ept[max_n];
                s_edges = new int[m];
                s_peel_sequence = new int[max_n];
                s_core = new int[max_n];
                s_vis = new char[max_n];
                s_heap = new LinearHeap(max_n, max_n - 1);
                s_edgelist_pointer = new int[m];
                s_tri_cnt = new int[m / 2];
                s_edge_list = new int[m];
                s_active_edgelist = new int[m / 2];
                s_deleted = new char[m / 2];

                vector<pair<int, int>> vp;
                vp.reserve(m / 2);
                int *t_degree = new int[n];

                int max_n_prune = 0, max_n_search = 0, prune_cnt = 0, search_cnt = 0;
                double min_density_prune = 1, min_density_search = 1, total_density_prune = 0, total_density_search = 0;
                int last_m = 0;

                for (int i = 0; i < n && m && kplex.size() < UB; i++)
                {
                    int u, key;
                    linear_heap->pop_min(u, key);
                    // if(key != 0) printf("u = %u, key = %u\n", u, key);
                    if (key < kplex.size() + 1 - K)
                    {
                        if (degree[u] != 0)
                        { // degree[u] == 0 means u is deleted. it could be the case that degree[u] == 0, but key[u] > 0, as key[u] is not fully updated in linear_heap
                            Qv[0] = u;
                            Qv_n = 1;
                            if (kplex.size() + 1 > 2 * K)
                                m -= 2 * peeling(n, linear_heap, Qv, Qv_n, kplex.size() + 1 - K, Qe, false, kplex.size() + 1 - 2 * K, tri_cnt, active_edgelist, active_edgelist_n, edge_list, edgelist_pointer, deleted, degree, pstart, pend, edges, exists);
                            else
                                m -= 2 * peeling(n, linear_heap, Qv, Qv_n, kplex.size() + 1 - K, Qe, false, 0, tri_cnt, active_edgelist, active_edgelist_n, edge_list, edgelist_pointer, deleted, degree, pstart, pend, edges, exists);
                        }
                        continue;
                    }
                    if (m == 0)
                        break;
                    assert(degree[u] == key);

                    int *ids = Qv;
                    int ids_n = 0;
                    bool mflag = false;

                    if (K < 3 || max_n == oldn)
                        kplex_solver->enableInput = false;
                    if (K >= 10)
                        kplex_solver->enableInput = true;

                    // if(last_m<0.8*m) {
                    if (true)
                    {
                        int pre_size;
                        do
                        {
                            pre_size = kplex.size();
                            extract_subgraph_and_prune(u, ids, ids_n, rid, vp, Qe, t_degree, exists, pend, deleted, edgelist_pointer);
                            if (ids_n)
                            {
                                double density = (double(vp.size() * 2)) / ids_n / (ids_n - 1);
                                total_density_prune += density;
                                ++prune_cnt;
                                if (density < min_density_prune)
                                    min_density_prune = density;
                                if (ids_n > max_n_prune)
                                    max_n_prune = ids_n;
                            }
                            if (ids_n > kplex.size() && vp.size() * 2 < m)
                                subgraph_prune(ids, ids_n, vp, rid, Qv, Qe, exists);
                            // Qv_n=0;
                            // if(kplex.size() != pre_size && kplex.size()> 2*K-2) m -= 2*peeling(n, linear_heap, Qv, Qv_n, kplex.size()+1-K, Qe, true, kplex.size()+1-2*K, tri_cnt, active_edgelist, active_edgelist_n, edge_list, edgelist_pointer, deleted, degree, pstart, pend, edges, exists);
                        } while (kplex.size() != pre_size);
                    }
                    else
                    {
                        extract_graph(n, m, degree, ids, ids_n, rid, vp, exists, pstart, pend, edges, deleted, edgelist_pointer);
                        double density = (double(vp.size() * 2)) / ids_n / (ids_n - 1);
                        total_density_prune += density;
                        ++prune_cnt;
                        if (density < min_density_prune)
                            min_density_prune = density;
                        if (ids_n > max_n_prune)
                            max_n_prune = ids_n;
                        mflag = true;
                    }
                    last_m = vp.size() * 2;
                    int pre_size = kplex.size();
                    if (ids_n > kplex.size())
                    {
                        double density = (double(vp.size() * 2)) / ids_n / (ids_n - 1);
                        total_density_search += density;
                        ++search_cnt;
                        if (density < min_density_search)
                            min_density_search = density;
                        if (ids_n > max_n_search)
                            max_n_search = ids_n;
                        kplex_solver->load(ids_n, vp);
                        if (mflag)
                        {
                            kplex_solver->runWHOLE(K, kplex);
                            break;
                        }
                        else
                            kplex_solver->run(K, kplex);
                    }
                    Qv[0] = u;
                    Qv_n = 1;
                    if (kplex.size() != pre_size && kplex.size() > 2 * K - 2)
                    {
                        for (int j = 0; j < kplex.size(); j++)
                            kplex[j] = ids[kplex[j]];
                        m -= 2 * peeling(n, linear_heap, Qv, Qv_n, kplex.size() + 1 - K, Qe, true, kplex.size() + 1 - 2 * K, tri_cnt, active_edgelist, active_edgelist_n, edge_list, edgelist_pointer, deleted, degree, pstart, pend, edges, exists);
                    }
                    else if (kplex.size() > 2 * K - 2)
                        m -= 2 * peeling(n, linear_heap, Qv, Qv_n, kplex.size() + 1 - K, Qe, false, kplex.size() + 1 - 2 * K, tri_cnt, active_edgelist, active_edgelist_n, edge_list, edgelist_pointer, deleted, degree, pstart, pend, edges, exists);
                    else
                        m -= 2 * peeling(n, linear_heap, Qv, Qv_n, kplex.size() + 1 - K, Qe, false, 0, tri_cnt, active_edgelist, active_edgelist_n, edge_list, edgelist_pointer, deleted, degree, pstart, pend, edges, exists);
                }

                if (prune_cnt == 0)
                    ++prune_cnt;
                if (search_cnt == 0)
                    ++search_cnt;
                // printf("prune_cnt: %u, max_n: %u, min_density: %.4lf, avg_density: %.4lf\n", prune_cnt, max_n_prune, min_density_prune, total_density_prune / prune_cnt);
                // printf("search_cnt: %u, max_n: %u, min_density: %.4lf, avg_density: %.4lf\n", search_cnt, max_n_search, min_density_search, total_density_search / search_cnt);

                if (kplex.size() > old_size)
                {
                    for (int i = 0; i < kplex.size(); i++)
                    {
                        kplex[i] = out_mapping[kplex[i]];
                    }
                }

                delete kplex_solver;
                delete linear_heap;
                delete[] t_degree;
                delete[] exists;
                delete[] out_mapping;
                delete[] rid;
                delete[] degree;
                delete[] edgelist_pointer;
                delete[] tri_cnt;
                delete[] active_edgelist;
                delete[] Qe;
                delete[] Qv;
                delete[] deleted;
            }
            delete[] core;
            delete[] seq;
        }

        vector<int> peel(int alpha, int beta)
        {
            int oldn = n;
            int *seq = new int[n];
            int *core = new int[n];
            int *degree = new int[n];
            char *vis = new char[n];

            LinearHeap *heap = new LinearHeap(n, n - 1);
            kplex.resize(alpha);
			degen(n, seq, core, pstart, edges, degree, vis, heap, true);
			kplex.resize(alpha);

            delete heap;
            delete[] vis;

			int *out_mapping = new int[n];
			int *rid = new int[n];
			
			assert(n>0 && m>0);
			shrink_graph(n, m, seq, core, out_mapping, NULL, rid, pstart, edges, true);
			// delete[] core; core = NULL;

			degree = new int[n];
			for (int i = 0; i < n; i++)
				degree[i] = pstart[i + 1] - pstart[i];

			LinearHeap *linear_heap = new LinearHeap(n, n - 1);
			linear_heap->init(n, n - 1, seq, degree);

			assert(pend == nullptr);
			pend = new ept[n];

			int *edgelist_pointer = new int[m];
			oriented_triangle_counting(n, m, seq, pstart, pend, edges, edgelist_pointer, rid); // edgelist_pointer currently stores triangle_counts

			// delete[] seq; seq = NULL;

			pend_buf = new ept[n];
			int *edge_list = new int[m];
			int *tri_cnt = new int[m / 2];
			reorganize_oriented_graph(n, tri_cnt, edge_list, pstart, pend, pend_buf, edges, edgelist_pointer, rid);

			for (int i = 0; i < n; i++)
				pend[i] = pstart[i + 1];

			int *active_edgelist = new int[m >> 1];
			int active_edgelist_n = m >> 1;
			for (int i = 0; i < (m >> 1); i++)
				active_edgelist[i] = i;

			int *Qe = new int[m >> 1];
			char *deleted = new char[m >> 1];
			memset(deleted, 0, sizeof(char) * (m >> 1));
			char *exists = new char[n];
			memset(exists, 0, sizeof(char) * n);

			int *Qv = new int[n];
			int Qv_n = 0;

			m -= 2 * peeling(n, linear_heap, Qv, Qv_n, alpha, Qe, true, beta, tri_cnt, active_edgelist, active_edgelist_n, edge_list, edgelist_pointer, deleted, degree, pstart, pend, edges, exists);

			int max_n = n - Qv_n;

			vector<int> res;
			for (int i = 0; i < n; i++)
			{
				int u, key;
				linear_heap->pop_min(u, key);
				if(degree[u]) res.push_back(out_mapping[u]);
			}

			delete linear_heap;
			delete[] exists;
			delete[] out_mapping;
			delete[] rid;
			delete[] degree;
			delete[] edgelist_pointer;
			delete[] tri_cnt;
			delete[] active_edgelist;
			delete[] Qe;
			delete[] Qv;
			delete[] deleted;
            delete[] core;
            delete[] seq;

			return res;
        }

        void subgraph_prune(int *ids, int &_n, vector<pair<int, int>> &edge_list, int *rid, int *Qv, int *Qe, char *exists)
        {
            int s_n;
            ept s_m;
            load_graph_from_edgelist(_n, edge_list, s_n, s_m, s_degree, s_pstart, s_edges);
            degen(s_n, s_peel_sequence, s_core, s_pstart, s_edges, s_degree, s_vis, s_heap, false);
            shrink_graph(s_n, s_m, s_peel_sequence, s_core, ids, ids, rid, s_pstart, s_edges, false);

            if (s_n > 0 && kplex.size() + 1 > 2 * K)
            {
                // if(false) {
                // printf("before n = %u, m = %lu\n", s_n, s_m);
                oriented_triangle_counting(s_n, s_m, s_peel_sequence, s_pstart, s_pend, s_edges, s_edgelist_pointer, rid);
                reorganize_oriented_graph(s_n, s_tri_cnt, s_edge_list, s_pstart, s_pend, pend_buf, s_edges, s_edgelist_pointer, rid);
                for (int i = 0; i < s_n; i++)
                {
                    s_pend[i] = s_pstart[i + 1];
                    s_degree[i] = s_pend[i] - s_pstart[i];
                }
                for (ept i = 0; i < (s_m >> 1); i++)
                    s_active_edgelist[i] = i;
                memset(s_deleted, 0, sizeof(char) * (s_m >> 1));

                int s_active_edgelist_n = s_m >> 1;
                int Qv_n = 0;
                s_m -= 2 * peeling(0, NULL, Qv, Qv_n, kplex.size() + 1 - K, Qe, true, kplex.size() + 1 - 2 * K, s_tri_cnt, s_active_edgelist, s_active_edgelist_n, s_edge_list, s_edgelist_pointer, s_deleted, s_degree, s_pstart, s_pend, s_edges, exists);
                for (int i = 0; i < Qv_n; i++)
                    if (Qv[i] == 0)
                    {
                        _n = 0;
                        return;
                    }
                extract_subgraph(0, Qv, s_n, rid, edge_list, exists, s_pstart, s_pend, s_edges, s_deleted, s_edgelist_pointer);
                for (int i = 0; i < s_n; i++)
                    Qv[i] = ids[Qv[i]];
                for (int i = 0; i < s_n; i++)
                    ids[i] = Qv[i];
                _n = s_n;
                assert(edge_list.size() * 2 == s_m);
                // printf("*after n = %u, m = %lu\n", s_n, s_m);
            }
            else
            {
                _n = s_n;
                edge_list.clear();
                for (int i = 0; i < s_n; i++)
                    for (ept j = s_pstart[i]; j < s_pstart[i + 1]; j++)
                        if (s_edges[j] > i)
                        {
                            edge_list.push_back(make_pair(s_edges[j], i));
                        }
                assert(edge_list.size() * 2 == s_m);
            }
        }

        void load_graph_from_edgelist(int _n, const vector<pair<int, int>> &edge_list, int &n, ept &m, int *degree, ept *pstart, int *edges)
        {
            n = _n;
            m = (int)edge_list.size() * 2;
            for (int i = 0; i < n; i++)
                degree[i] = 0;
            for (ept i = 0; i < m / 2; i++)
            {
                assert(edge_list[i].first >= 0 && edge_list[i].first < n && edge_list[i].second >= 0 && edge_list[i].second < n);
                degree[edge_list[i].first]++;
                degree[edge_list[i].second]++;
            }

            pstart[0] = 0;
            for (int i = 0; i < n; i++)
                pstart[i + 1] = pstart[i] + degree[i];
            for (ept i = 0; i < m / 2; i++)
            {
                int a = edge_list[i].first, b = edge_list[i].second;
                edges[pstart[a]++] = b;
                edges[pstart[b]++] = a;
            }
            for (int i = 0; i < n; i++)
                pstart[i] -= degree[i];
        }

        void extract_graph(int n, int m, int *degree, int *ids, int &ids_n, int *rid, vector<pair<int, int>> &vp, char *exists, ept *pstart, ept *pend, int *edges, char *deleted, int *edgelist_pointer)
        {
            ids_n = 0;
            vp.clear();
            for (int i = 0; i < n; ++i)
            {
                if (degree[i])
                {
                    ids[ids_n] = i;
                    rid[i] = ids_n++;
                }
            }
            for (int i = 0; i < ids_n; i++)
            {
                int u = ids[i];
                for (ept j = pstart[u]; j < pend[u]; j++)
                    if (!deleted[edgelist_pointer[j]] && u < edges[j])
                    {
                        vp.push_back(make_pair(rid[u], rid[edges[j]]));
                    }
            }
        }

        void extract_subgraph(int u, int *ids, int &ids_n, int *rid, vector<pair<int, int>> &vp, char *exists, ept *pstart, ept *pend, int *edges, char *deleted, int *edgelist_pointer)
        {
            ids_n = 0;
            vp.clear();
            ids[ids_n++] = u;
            exists[u] = 1;
            rid[u] = 0;
            int u_n = pstart[u];
            for (ept i = pstart[u]; i < pend[u]; i++)
                if (!deleted[edgelist_pointer[i]])
                {
                    edges[u_n] = edges[i];
                    edgelist_pointer[u_n++] = edgelist_pointer[i];
                    int v = edges[i];
                    rid[v] = ids_n;
                    ids[ids_n++] = v;
                    exists[v] = 1;
                }
            pend[u] = u_n;
            int old_size = ids_n;
            for (int i = 1; i < old_size; i++)
            {
                u = ids[i];
                u_n = pstart[u];
                for (ept j = pstart[u]; j < pend[u]; j++)
                    if (!deleted[edgelist_pointer[j]])
                    {
                        edges[u_n] = edges[j];
                        edgelist_pointer[u_n++] = edgelist_pointer[j];
                        int v = edges[j];
                        if (exists[v])
                            continue;
                        rid[v] = ids_n;
                        ids[ids_n++] = v;
                        exists[v] = 1;
                    }
                pend[u] = u_n;
            }
            for (int i = 0; i < old_size; i++)
            {
                u = ids[i];
                for (ept j = pstart[u]; j < pend[u]; j++)
                    if (edges[j] > u)
                    {
                        vp.push_back(make_pair(rid[u], rid[edges[j]]));
                    }
            }
            for (int i = old_size; i < ids_n; i++)
            {
                u = ids[i];
                u_n = pstart[u];
                for (ept j = pstart[u]; j < pend[u]; j++)
                    if (!deleted[edgelist_pointer[j]])
                    {
                        edges[u_n] = edges[j];
                        edgelist_pointer[u_n++] = edgelist_pointer[j];
                        if (edges[j] > u && exists[edges[j]])
                            vp.push_back(make_pair(rid[u], rid[edges[j]]));
                    }
                pend[u] = u_n;
            }
            for (int i = 0; i < ids_n; i++)
                exists[ids[i]] = 0;
        }

        void extract_subgraph_and_prune(int u, int *ids, int &ids_n, int *rid, vector<pair<int, int>> &vp, int *Q, int *degree, char *exists, ept *pend, char *deleted, int *edgelist_pointer)
        {
            vp.clear();
            ids_n = 0;
            ids[ids_n++] = u;
            exists[u] = 1;
            int u_n = pstart[u];
            for (ept i = pstart[u]; i < pend[u]; i++)
                if (!deleted[edgelist_pointer[i]])
                {
                    edges[u_n] = edges[i];
                    edgelist_pointer[u_n++] = edgelist_pointer[i];
                    int v = edges[i];
                    ids[ids_n++] = v;
                    exists[v] = 2;
                }
            pend[u] = u_n;

            int Q_n = 0;
            for (int i = 1; i < ids_n; i++)
            {
                u = ids[i];
                u_n = pstart[u];
                degree[u] = 0;
                for (ept j = pstart[u]; j < pend[u]; j++)
                    if (!deleted[edgelist_pointer[j]])
                    {
                        edges[u_n] = edges[j];
                        edgelist_pointer[u_n++] = edgelist_pointer[j];
                        if (exists[edges[j]] == 2)
                            ++degree[u];
                    }
                pend[u] = u_n;
                if (degree[u] + 2 * K <= kplex.size())
                    Q[Q_n++] = u;
            }
            for (int i = 0; i < Q_n; i++)
            {
                u = Q[i];
                exists[u] = 10;
                for (ept j = pstart[u]; j < pend[u]; j++)
                    if (exists[edges[j]] == 2)
                    {
                        if ((degree[edges[j]]--) + 2 * K == kplex.size() + 1)
                        {
                            assert(Q_n < m / 2);
                            Q[Q_n++] = edges[j];
                        }
                    }
            }
            assert(Q_n <= ids_n);
            if (ids_n - 1 - Q_n + K <= kplex.size())
            {
                for (int i = 0; i < ids_n; i++)
                    exists[ids[i]] = 0;
                ids_n = 0;
                return;
            }

            int nr_size = ids_n;
            for (int i = 1; i < nr_size; i++)
                if (exists[ids[i]] == 2)
                {
                    u = ids[i];
                    for (ept j = pstart[u]; j < pend[u]; j++)
                    {
                        if (!exists[edges[j]])
                        {
                            ids[ids_n++] = edges[j];
                            exists[edges[j]] = 3;
                            degree[edges[j]] = 1;
                        }
                        else if (exists[edges[j]] == 3)
                            ++degree[edges[j]];
                    }
                }

#ifndef NDEBUG
                // printf("Entire list: ");
                // for(ui i = 0;i < nr_size;i ++) printf(" %u", ids[i]);
                // printf("\n");
#endif

            int new_size = 1;
            for (int i = 1; i < nr_size; i++)
            {
                if (exists[ids[i]] == 10)
                    exists[ids[i]] = 0;
                else
                    ids[new_size++] = ids[i];
            }
            assert(new_size + Q_n == nr_size);
            int old_nr_size = nr_size;
            nr_size = new_size;
            for (int i = old_nr_size; i < ids_n; i++)
            {
                if (degree[ids[i]] + 2 * K <= kplex.size() + 2)
                    exists[ids[i]] = 0;
                else
                    ids[new_size++] = ids[i];
            }
            ids_n = new_size;

            // for(ui i = 0;i < ids_n;i ++) printf(" %u", ids[i]);
            // printf("\n");

            for (int i = 0; i < ids_n; i++)
            {
                assert(exists[ids[i]]);
                rid[ids[i]] = i;
            }

            for (int i = 0; i < nr_size; i++)
            {
                u = ids[i];
                for (ept j = pstart[u]; j < pend[u]; j++)
                    if (exists[edges[j]] && edges[j] > u)
                    {
                        assert(!deleted[edgelist_pointer[j]]);
                        vp.push_back(make_pair(rid[u], rid[edges[j]]));
                    }
            }
            for (int i = nr_size; i < ids_n; i++)
            {
                u = ids[i];
                u_n = pstart[u];
                for (ept j = pstart[u]; j < pend[u]; j++)
                    if (!deleted[edgelist_pointer[j]])
                    {
                        edges[u_n] = edges[j];
                        edgelist_pointer[u_n++] = edgelist_pointer[j];
                        if (edges[j] > u && exists[edges[j]])
                            vp.push_back(make_pair(rid[u], rid[edges[j]]));
                    }
                pend[u] = u_n;
            }
            for (int i = 0; i < ids_n; i++)
                exists[ids[i]] = 0;
        }

        // max-degree-based heuristic k-plex computation
        void heuristic_kplex_max_degree(int processed_threshold)
        {
            int *head = new int[n];
            int *next = new int[n];
            int *degree = new int[n];

            int *vis = new int[n];
            memset(vis, 0, sizeof(int) * n);

            int max_degree = 0;
            for (int i = 0; i < n; i++)
                head[i] = n;
            for (int i = 0; i < n; i++)
            {
                degree[i] = pstart[i + 1] - pstart[i];
                if (degree[i] > max_degree)
                    max_degree = degree[i];
                next[i] = head[degree[i]];
                head[degree[i]] = i;
            }

            for (int processed_vertices = 0; max_degree + K >= kplex.size() && processed_vertices < processed_threshold; processed_vertices++)
            {
                int u = n;
                while (max_degree >= 0 && max_degree + K >= kplex.size() && u == n)
                {
                    for (int v = head[max_degree]; v != n;)
                    {
                        int tmp = next[v];
                        if (degree[v] == max_degree)
                        {
                            u = v;
                            head[max_degree] = tmp;
                            break;
                        }
                        else if (degree[v] + K >= kplex.size())
                        {
                            next[v] = head[degree[v]];
                            head[degree[v]] = v;
                        }
                        v = tmp;
                    }
                    if (u == n)
                    {
                        head[max_degree] = n;
                        --max_degree;
                    }
                }
                if (u == n)
                    break;

                vis[u] = 1;
                for (int k = pstart[u]; k < pstart[u + 1]; k++)
                    if (!vis[edges[k]])
                        --degree[edges[k]];

                vector<int> vs;
                for (int j = pstart[u]; j < pstart[u + 1]; j++)
                    if (!vis[edges[j]])
                        vs.push_back(edges[j]);

                vector<int> vs_deg(vs.size());
                for (int j = 0; j < vs.size(); j++)
                    vis[vs[j]] = 2;
                for (int j = 0; j < vs.size(); j++)
                {
                    int v = vs[j], d = 0;
                    for (int k = pstart[v]; k < pstart[v + 1]; k++)
                    {
                        if (vis[edges[k]] == 2)
                            ++d;
                    }
                    vs_deg[j] = d;
                }
                for (int j = 0; j < vs.size(); j++)
                    vis[vs[j]] = 0;

                vector<int> res;
                res.push_back(u);
                int vs_size = vs.size();
                while (vs_size > 0 && res.size() + vs_size + K - 1 > kplex.size())
                {
                    // while(vs_size > 0) {
                    int idx = 0;
                    for (int j = 1; j < vs_size; j++)
                    {
                        if (vs_deg[j] > vs_deg[idx])
                            idx = j;
                        else if (vs_deg[j] == vs_deg[idx] && degree[vs[j]] > degree[vs[idx]])
                            idx = j;
                    }
                    u = vs[idx];

                    int new_size = 0;
                    for (int j = pstart[u]; j < pstart[u + 1]; j++)
                        if (!vis[edges[j]])
                            vis[edges[j]] = 2;
                    for (int j = 0; j < vs_size; j++)
                        if (vis[vs[j]])
                        {
                            if (j != new_size)
                                swap(vs[new_size], vs[j]);
                            vs_deg[new_size] = vs_deg[j];
                            ++new_size;
                        }
                    for (int j = pstart[u]; j < pstart[u + 1]; j++)
                        if (vis[edges[j]] == 2)
                            vis[edges[j]] = 0;

                    res.push_back(u);
                    for (int k = 0; k < new_size; k++)
                        vis[vs[k]] = k + 2;
                    for (int j = new_size; j < vs_size; j++)
                    {
                        int v = vs[j];
                        for (int k = pstart[v]; k < pstart[v + 1]; k++)
                        {
                            if (vis[edges[k]] >= 2)
                                --vs_deg[vis[edges[k]] - 2];
                        }
                    }
                    for (int k = 0; k < new_size; k++)
                        vis[vs[k]] = 0;

                    vs_size = new_size;
                }

                // TO DO: extend res to be a maximal k-plex

                if (res.size() > kplex.size())
                    kplex = res;
            }

            delete[] vis;
            delete[] head;
            delete[] next;
            delete[] degree;
        }

        // degeneracy-based k-plex
        // return an upper bound of the maximum k-plex size
        int degen(int n, int *peel_sequence, int *core, ept *pstart, int *edges, int *degree, char *vis, LinearHeap *heap, bool output)
        {

            int threshold = (kplex.size() + 1 > K ? kplex.size() + 1 - K : 0);

            for (int i = 0; i < n; i++)
                degree[i] = pstart[i + 1] - pstart[i];

            int queue_n = 0, new_size = 0;
            for (int i = 0; i < n; i++)
                if (degree[i] < threshold)
                    peel_sequence[queue_n++] = i;
            for (int i = 0; i < queue_n; i++)
            {
                int u = peel_sequence[i];
                degree[u] = 0;
                for (ept j = pstart[u]; j < pstart[u + 1]; j++)
                    if (degree[edges[j]] > 0)
                    {
                        if ((degree[edges[j]]--) == threshold)
                            peel_sequence[queue_n++] = edges[j];
                    }
            }
            int UB = n;
            if (queue_n == n)
                UB = kplex.size();

            memset(vis, 0, sizeof(char) * n);
            for (int i = 0; i < n; i++)
            {
                if (degree[i] >= threshold)
                    peel_sequence[queue_n + (new_size++)] = i;
                else
                {
                    vis[i] = 1;
                    core[i] = 0;
                }
            }
            assert(queue_n + new_size == n);

            if (new_size != 0)
            {
                heap->init(new_size, new_size - 1, peel_sequence + queue_n, degree);
                int max_core = 0;
                int idx = n;
                UB = 0;
                for (int i = 0; i < new_size; i++)
                {
                    int u, key;
                    heap->pop_min(u, key);
                    if (key > max_core)
                        max_core = key;
                    core[u] = max_core;
                    peel_sequence[queue_n + i] = u;

                    int t_UB = core[u] + K;
                    if (new_size - i < t_UB)
                        t_UB = new_size - i;
                    if (t_UB > UB)
                        UB = t_UB;

                    if (idx == n && key + K >= new_size - i)
                        idx = i;
                    vis[u] = 1;

                    for (ept j = pstart[u]; j < pstart[u + 1]; j++)
                        if (vis[edges[j]] == 0)
                        {
                            heap->decrement(edges[j], 1);
                        }
                }

                if (new_size - idx > kplex.size())
                {
                    kplex.clear();
                    for (int i = idx; i < new_size; i++)
                        kplex.push_back(peel_sequence[queue_n + i]);
                }
            }
            return UB;
        }

        // in_mapping and out_mapping can be the same array
        // note that core is not maintained, and is assumed to not be used anymore
        void shrink_graph(int &n, ept &m, int *peel_sequence, int *core, int *out_mapping, int *in_mapping, int *rid, ept *pstart, int *edges, bool output)
        {
            int cnt = 0;
            for (int i = 0; i < n; i++)
                if (core[i] + K > kplex.size())
                {
                    rid[i] = cnt;
                    if (in_mapping == NULL)
                        out_mapping[cnt] = i;
                    else
                        out_mapping[cnt] = in_mapping[i];
                    ++cnt;
                }

            if (true)
            {
                cnt = 0;
                ept pos = 0;
                for (int i = 0; i < n; i++)
                    if (core[i] + K > kplex.size())
                    {
                        ept t_start = pstart[i];
                        pstart[cnt] = pos;
                        for (ept j = t_start; j < pstart[i + 1]; j++)
                            if (core[edges[j]] + K > kplex.size())
                            {
                                edges[pos++] = rid[edges[j]];
                            }
                        ++cnt;
                    }
                pstart[cnt] = pos;

                // printf("%u %u %u %u\n", n, cnt, core[peel_sequence[n-cnt-1]], core[peel_sequence[n-cnt]]);
                // assert(core[peel_sequence[n - cnt - 1]] == 0 || core[peel_sequence[n - cnt - 1]] + K <= kplex.size());
                // assert(cnt == 0 || core[peel_sequence[n - cnt]] + K > kplex.size());
                for (int i = 0; i < cnt; i++)
                {
                    peel_sequence[i] = rid[peel_sequence[n - cnt + i]];
                    // core[i] = core[out_mapping[i]];
                }

                n = cnt;
                m = pos;
            }

            // if (output)
            //     printf("*** After core shrink: n = %d, m = %d (undirected)\n", n, m / 2);
        }

        // orient graph and triangle counting
        void oriented_triangle_counting(int n, int m, int *peel_sequence, ept *pstart, ept *pend, int *edges, int *tri_cnt, int *adj)
        {
            int *rid = adj;
            for (int i = 0; i < n; i++)
                rid[peel_sequence[i]] = i;
            for (int i = 0; i < n; i++)
            {
                ept &end = pend[i] = pstart[i];
                for (ept j = pstart[i]; j < pstart[i + 1]; j++)
                    if (rid[edges[j]] > rid[i])
                        edges[end++] = edges[j];
            }

#ifndef NDEBUG
            long long sum = 0;
            for (int i = 0; i < n; i++)
                sum += pend[i] - pstart[i];
            printf("%lld %lld\n", sum, m);
            assert(sum * 2 == m);
#endif

            memset(adj, 0, sizeof(int) * n);
            long long cnt = 0;
            memset(tri_cnt, 0, sizeof(int) * m);
            for (int u = 0; u < n; u++)
            {
                for (ept j = pstart[u]; j < pend[u]; j++)
                    adj[edges[j]] = j + 1;

                for (ept j = pstart[u]; j < pend[u]; j++)
                {
                    int v = edges[j];
                    for (ept k = pstart[v]; k < pend[v]; k++)
                        if (adj[edges[k]])
                        {
                            ++tri_cnt[j];
                            ++tri_cnt[k];
                            ++tri_cnt[adj[edges[k]] - 1];
                            ++cnt;
                        }
                }

                for (ept j = pstart[u]; j < pend[u]; j++)
                    adj[edges[j]] = 0;
            }
        }

        // reorganize the adjacency lists
        // and sort each adjacency list to be in increasing order
        void reorganize_oriented_graph(int n, int *tri_cnt, int *edge_list, ept *pstart, ept *pend, ept *pend2, int *edges, int *edgelist_pointer, int *buf)
        {
            for (int i = 0; i < n; i++)
                pend2[i] = pend[i];
            ept pos = 0;
            for (int i = 0; i < n; i++)
            {
                for (ept j = pstart[i]; j < pend[i]; j++)
                {
                    tri_cnt[pos >> 1] = edgelist_pointer[j];
                    edge_list[pos++] = i;
                    edge_list[pos++] = edges[j];

                    ept &k = pend2[edges[j]];
                    edgelist_pointer[k] = edgelist_pointer[j] = (pos >> 1) - 1;
                    edges[k++] = i;
                }
            }

#ifndef NDEBUG
            for (int i = 0; i < n; i++)
                assert(pend2[i] == pstart[i + 1]);
#endif

            for (int i = 0; i < n; i++)
            {
                pend2[i] = pend[i];
                pend[i] = pstart[i];
            }
            for (int i = 0; i < n; i++)
            {
                for (ept j = pend2[i]; j < pstart[i + 1]; j++)
                {
                    ept &k = pend[edges[j]];
                    edgelist_pointer[k] = edgelist_pointer[j];
                    edges[k++] = i;
                }
            }

            ept *ids = pend2;
            for (int i = 0; i < n; i++)
            {
                if (pend[i] == pstart[i] || pend[i] == pstart[i + 1])
                    continue;
                ept j = pstart[i], k = pend[i], pos = 0;
                while (j < pend[i] && k < pstart[i + 1])
                {
                    if (edges[j] < edges[k])
                    {
                        ids[pos] = edges[j];
                        buf[pos++] = edgelist_pointer[j++];
                    }
                    else
                    {
                        ids[pos] = edges[k];
                        buf[pos++] = edgelist_pointer[k++];
                    }
                }
                while (j < pend[i])
                {
                    ids[pos] = edges[j];
                    buf[pos++] = edgelist_pointer[j++];
                }
                while (k < pstart[i + 1])
                {
                    ids[pos] = edges[k];
                    buf[pos++] = edgelist_pointer[k++];
                }
                for (ept j = 0; j < pos; j++)
                {
                    edges[pstart[i] + j] = ids[j];
                    edgelist_pointer[pstart[i] + j] = buf[j];
                }
            }
        }

        char find(int u, int w, ept &b, ept e, char *deleted, ept &idx, int *edgelist_pointer, int *edges)
        {
            if (b >= e)
                return 0;

            while (b + 1 < e)
            {
                idx = b + (e - b) / 2;
                if (edges[idx] > w)
                    e = idx;
                else
                    b = idx;
            }

            if (edges[b] == w)
            {
                idx = edgelist_pointer[b];
                if (!deleted[idx])
                    return 1;
            }

            return 0;
        }

        // return the number of peeled edges
        ept peeling(int critical_vertex, LinearHeap *linear_heap, int *Qv, int &Qv_n, int d_threshold, int *Qe, bool initialize_Qe, int t_threshold, int *tri_cnt, int *active_edgelist, int &active_edgelist_n, int *edge_list, int *edgelist_pointer, char *deleted, int *degree, ept *pstart, ept *pend, int *edges, char *exists)
        {
            ept Qe_n = 0;
#ifndef NO_TRUSS_PRUNE
            if (initialize_Qe)
            {
                ept active_edgelist_newn = 0;
                for (ept j = 0; j < active_edgelist_n; j++)
                    if (!deleted[active_edgelist[j]])
                    {
                        if (tri_cnt[active_edgelist[j]] < t_threshold)
                            Qe[Qe_n++] = active_edgelist[j];
                        else
                            active_edgelist[active_edgelist_newn++] = active_edgelist[j];
                    }
                active_edgelist_n = active_edgelist_newn;
            }
#endif

            // printf("%lu\n", Qe_n);

            ept deleted_edges_n = 0;
            int Qv_idx = 0;
            while (Qv_idx < Qv_n || Qe_n)
            {
                if (Qe_n == 0)
                {
                    // printf("hit\n");
                    int u = Qv[Qv_idx++]; // delete u from the graph due to have a degree < d_threshold
                    ept u_n = pstart[u];
                    for (ept k = pstart[u]; k < pend[u]; k++)
                        if (!deleted[edgelist_pointer[k]])
                        {
                            edges[u_n] = edges[k];
                            edgelist_pointer[u_n++] = edgelist_pointer[k];
                            exists[edges[k]] = 1;
                        }
                    pend[u] = u_n;

                    for (ept k = pstart[u]; k < pend[u]; k++)
                        deleted[edgelist_pointer[k]] = 1;
                    deleted_edges_n += pend[u] - pstart[u];
                    degree[u] = 0;
                    if (linear_heap != NULL)
                        linear_heap->del(u);
                    // printf("Removed %u\n", u);

                    for (ept k = pstart[u]; k < pend[u]; k++)
                    {
                        int v = edges[k];
                        ept v_n = pstart[v];
                        for (ept x = pstart[v]; x < pend[v]; x++)
                            if (!deleted[edgelist_pointer[x]])
                            {
                                edges[v_n] = edges[x];
                                edgelist_pointer[v_n++] = edgelist_pointer[x];
                                if (edges[x] > v && exists[edges[x]])
                                {
                                    if ((tri_cnt[edgelist_pointer[x]]--) == t_threshold)
                                        Qe[Qe_n++] = edgelist_pointer[x];
                                }
                            }
                        pend[v] = v_n;

                        if ((degree[v]--) == d_threshold)
                        {
                            Qv[Qv_n++] = v;
                            if (v == critical_vertex)
                            {
                                for (ept k = pstart[u]; k < pend[u]; k++)
                                    exists[edges[k]] = 0;
                                return 0;
                            }
                        }
                        if (linear_heap != NULL)
                            linear_heap->decrement(v, 1);
                    }

                    for (ept k = pstart[u]; k < pend[u]; k++)
                        exists[edges[k]] = 0;
                }
#ifdef NO_TRUSS_PRUNE
                Qe_n = 0;
#endif
                for (ept j = 0; j < Qe_n; j++)
                {
                    ept idx = Qe[j];
                    int u = edge_list[idx << 1], v = edge_list[(idx << 1) + 1];
                    int tri_n = tri_cnt[idx];
                    // printf("remove %u %u\n", u, v);
                    deleted[idx] = 1;
                    if ((degree[u]--) == d_threshold)
                    {
                        Qv[Qv_n++] = u;
                        if (u == critical_vertex)
                            return 0;
                    }
                    if ((degree[v]--) == d_threshold)
                    {
                        Qv[Qv_n++] = v;
                        if (v == critical_vertex)
                            return 0;
                    }
                    // printf("before\n");
                    if (linear_heap != NULL)
                    {
                        linear_heap->decrement(u, 1);
                        linear_heap->decrement(v, 1);
                    }
                    // printf("after\n");
                    deleted_edges_n++;

                    if (degree[u] < degree[v])
                        swap(u, v);
                    // printf("here\n");

                    if (degree[u] > degree[v] * 2)
                    { // binary search
                        // if(false) {
                        ept v_n = pstart[v], start = pstart[u];
                        for (ept k = pstart[v]; k < pend[v]; k++)
                            if (!deleted[edgelist_pointer[k]])
                            {
                                edges[v_n] = edges[k];
                                edgelist_pointer[v_n++] = edgelist_pointer[k];

                                if (tri_n && find(u, edges[k], start, pend[u], deleted, idx, edgelist_pointer, edges))
                                {
                                    --tri_n;
                                    if ((tri_cnt[idx]--) == t_threshold)
                                        Qe[Qe_n++] = idx;
                                    if ((tri_cnt[edgelist_pointer[k]]--) == t_threshold)
                                        Qe[Qe_n++] = edgelist_pointer[k];
                                }
                            }
                        pend[v] = v_n;
                        assert(tri_n == 0);
                    }
                    else
                    { // sorted_merge
                        ept ii = pstart[u], jj = pstart[v];
                        ept u_n = pstart[u], v_n = pstart[v];

                        while (true)
                        {
                            while (ii < pend[u] && deleted[edgelist_pointer[ii]])
                                ++ii;
                            while (jj < pend[v] && deleted[edgelist_pointer[jj]])
                                ++jj;
                            if (ii >= pend[u] || jj >= pend[v])
                                break;

                            if (edges[ii] == edges[jj])
                            {
                                edges[u_n] = edges[ii];
                                edgelist_pointer[u_n++] = edgelist_pointer[ii];
                                edges[v_n] = edges[jj];
                                edgelist_pointer[v_n++] = edgelist_pointer[jj];

                                if ((tri_cnt[edgelist_pointer[ii]]--) == t_threshold)
                                    Qe[Qe_n++] = edgelist_pointer[ii];
                                if ((tri_cnt[edgelist_pointer[jj]]--) == t_threshold)
                                    Qe[Qe_n++] = edgelist_pointer[jj];

                                ++ii;
                                ++jj;
                            }
                            else if (edges[ii] < edges[jj])
                            {
                                edges[u_n] = edges[ii];
                                edgelist_pointer[u_n++] = edgelist_pointer[ii];
                                ++ii;
                            }
                            else
                            {
                                edges[v_n] = edges[jj];
                                edgelist_pointer[v_n++] = edgelist_pointer[jj];
                                ++jj;
                            }
                        }
                        while (ii < pend[u])
                        {
                            if (!deleted[edgelist_pointer[ii]])
                            {
                                edges[u_n] = edges[ii];
                                edgelist_pointer[u_n++] = edgelist_pointer[ii];
                            }
                            ++ii;
                        }
                        while (jj < pend[v])
                        {
                            if (!deleted[edgelist_pointer[jj]])
                            {
                                edges[v_n] = edges[jj];
                                edgelist_pointer[v_n++] = edgelist_pointer[jj];
                            }
                            ++jj;
                        }
                        pend[u] = u_n;
                        pend[v] = v_n;
                    }
                }
                Qe_n = 0;
            }
            return deleted_edges_n;
        }
    };

} // namespace MaxKPX
vector<int> PathQueryHandler::maximumKplex(const std::vector<int> &pred_set, int k)
{
	MaxKPX::Graph graph(*this, pred_set, k);
	graph.read(); graph.search();
	return graph.kplex;
}

vector<int> PathQueryHandler::coreTruss(const std::vector<int> &pred_set, int alpha, int beta) {
	MaxKPX::Graph graph(*this, pred_set, 1);
	graph.read(); 
	return graph.peel(alpha,beta);
}


// retNum is the number of top nodes to return; k is the hop constraint -- don't mix them up!
void PathQueryHandler::SSPPR(int uid, int retNum, int k, const vector<int> &pred_set, vector<pair<int, double>> &topkV2ppr)
{
	topkV2ppr.clear();
	srand(time(NULL));

	unordered_map<int, double> v2ppr;
	pair<iMap<double>, iMap<double>> fwd_idx;
	iMap<double> ppr;
	iMap<int> topk_filter;
	iMap<double> upper_bounds;
	iMap<double> lower_bounds;

	// Data structures initialization
	fwd_idx.first.nil = -9;
	fwd_idx.first.initialize(getVertNum());
	fwd_idx.second.nil = -9;
	fwd_idx.second.initialize(getVertNum());
	upper_bounds.nil = -9;
	upper_bounds.init_keys(getVertNum());
	lower_bounds.nil = -9;
	lower_bounds.init_keys(getVertNum());
	ppr.nil = -9;
	ppr.initialize(getVertNum());
	topk_filter.nil = -9;
	topk_filter.initialize(getVertNum());

	// Params initialization
	int numPredEdges = getSetEdgeNum(pred_set);
	double ppr_decay_alpha = 0.77;
	double pfail = 1.0 / getVertNum() / getVertNum() / log(getVertNum()); // log(1/pfail) -> log(1*n/pfail)
	double delta = 1.0 / 4;
	double epsilon = 0.5;
	double rmax;
	double rmax_scale = 1.0;
	double omega;
	double alpha = 0.2;
	double min_delta = 1.0 / getVertNum();
	double threshold = (1.0 - ppr_decay_alpha) / pow(500, ppr_decay_alpha) / pow(getVertNum(), 1 - ppr_decay_alpha);
	double lowest_delta_rmax = epsilon * sqrt(min_delta / 3 / numPredEdges / log(2 / pfail));
	double rsum = 1.0;

	vector<pair<int, int>> forward_from;
	forward_from.clear();
	forward_from.reserve(getVertNum());
	forward_from.push_back(make_pair(uid, 0));

	fwd_idx.first.clean();	// reserve
	fwd_idx.second.clean(); // residual
	fwd_idx.second.insert(uid, rsum);

	double zero_ppr_upper_bound = 1.0;
	upper_bounds.reset_one_values();
	lower_bounds.reset_zero_values();

	// fora_query_topk_with_bound
	// for delta: try value from 1/4 to 1/n
	while (delta >= min_delta)
	{
		rmax = epsilon * sqrt(delta / 3 / numPredEdges / log(2 / pfail));
		rmax *= rmax_scale;
		omega = (2 + epsilon) * log(2 / pfail) / delta / epsilon / epsilon;

		if (getSetOutSize(uid, pred_set) == 0)
		{
			rsum = 0.0;
			fwd_idx.first.insert(uid, 1);
			compute_ppr_with_reserve(fwd_idx, v2ppr);
			break;
		}
		else
			forward_local_update_linear_topk(uid, rsum, rmax, lowest_delta_rmax, forward_from, fwd_idx, pred_set, alpha, k); // forward propagation, obtain reserve and residual

		compute_ppr_with_fwdidx_topk_with_bound(rsum, fwd_idx, v2ppr, delta, alpha, threshold,
												pred_set, pfail, zero_ppr_upper_bound, omega, upper_bounds, lower_bounds);
		if (if_stop(retNum, delta, threshold, epsilon, topk_filter, upper_bounds, lower_bounds, v2ppr) || delta <= min_delta)
			break;
		else
			delta = max(min_delta, delta / 2.0); // otherwise, reduce delta to delta/2
	}

	// Extract top-k results
	topkV2ppr.clear();
	topkV2ppr.resize(retNum);
	partial_sort_copy(v2ppr.begin(), v2ppr.end(), topkV2ppr.begin(), topkV2ppr.end(),
					  [](pair<int, double> const &l, pair<int, double> const &r)
					  { return l.second > r.second; });
	size_t i = topkV2ppr.size() - 1;
	while (topkV2ppr[i].second == 0)
		i--;
	topkV2ppr.erase(topkV2ppr.begin() + i + 1, topkV2ppr.end()); // Get rid of ppr = 0 entries
}

void PathQueryHandler::compute_ppr_with_reserve(pair<iMap<double>, iMap<double>> &fwd_idx, unordered_map<int, double> &v2ppr)
{
	int node_id;
	double reserve;
	for (long i = 0; i < fwd_idx.first.occur.m_num; i++)
	{
		node_id = fwd_idx.first.occur[i];
		reserve = fwd_idx.first[node_id];
		if (reserve)
			v2ppr[node_id] = reserve;
	}
}

void PathQueryHandler::forward_local_update_linear_topk(int s, double &rsum, double rmax, double lowest_rmax, vector<pair<int, int>> &forward_from,
														pair<iMap<double>, iMap<double>> &fwd_idx, const vector<int> &pred_set, double alpha, int k)
{
	double myeps = rmax;

	vector<bool> in_forward(getVertNum());
	vector<bool> in_next_forward(getVertNum());

	std::fill(in_forward.begin(), in_forward.end(), false);
	std::fill(in_next_forward.begin(), in_next_forward.end(), false);

	vector<pair<int, int>> next_forward_from;
	next_forward_from.reserve(getVertNum());
	for (auto &v : forward_from)
		in_forward[v.first] = true;

	unsigned long i = 0;
	while (i < forward_from.size())
	{
		int v = forward_from[i].first;
		int level = forward_from[i].second;
		i++;
		// if (k != -1 && level >= k)
		// 	continue;
		in_forward[v] = false;
		if (fwd_idx.second[v] / getSetOutSize(v, pred_set) >= myeps)
		{
			int out_neighbor = getSetOutSize(v, pred_set);
			double v_residue = fwd_idx.second[v];
			fwd_idx.second[v] = 0;
			if (!fwd_idx.first.exist(v))
				fwd_idx.first.insert(v, v_residue * alpha);
			else
				fwd_idx.first[v] += v_residue * alpha;

			rsum -= v_residue * alpha;
			if (out_neighbor == 0)
			{
				fwd_idx.second[s] += v_residue * (1 - alpha);
				if (getSetOutSize(s, pred_set) > 0 && in_forward[s] != true && fwd_idx.second[s] / getSetOutSize(s, pred_set) >= myeps)
				{
					// forward_from.push_back(make_pair(s, level + 1));
					forward_from.push_back(make_pair(s, 0));
					in_forward[s] = true;
				}
				else if (getSetOutSize(s, pred_set) >= 0 && in_next_forward[s] != true && fwd_idx.second[s] / getSetOutSize(s, pred_set) >= lowest_rmax)
				{
					// next_forward_from.push_back(make_pair(s, level + 1));
					next_forward_from.push_back(make_pair(s, 0));
					in_next_forward[s] = true;
				}
				continue;
			}
			double avg_push_residual = ((1 - alpha) * v_residue) / out_neighbor;
			// int out_neighbor_test = 0;
			// cout << "out_neighbor = " << out_neighbor << endl;
			for (int pred : pred_set)
			{
				int out_neighbor_pred = getOutSize(v, pred);
				// out_neighbor_test += out_neighbor_pred;
				for (int i = 0; i < out_neighbor_pred; i++)
				{
					int next = getOutVertID(v, pred, i);
					if (next == -1)
					{
						cout << "ERROR!!!!!!" << endl;
						exit(0); // TODO: throw an exception
					}

					if (!fwd_idx.second.exist(next))
						fwd_idx.second.insert(next, avg_push_residual);
					else
						fwd_idx.second[next] += avg_push_residual;

					if (in_forward[next] != true && fwd_idx.second[next] / getSetOutSize(next, pred_set) >= myeps && (k == -1 || level < k))
					{
						forward_from.push_back(make_pair(next, level + 1));
						in_forward[next] = true;
					}
					else
					{
						if (in_next_forward[next] != true && fwd_idx.second[next] / getSetOutSize(next, pred_set) >= lowest_rmax && (k == -1 || level < k))
						{
							next_forward_from.push_back(make_pair(next, level + 1));
							in_next_forward[next] = true;
						}
					}
				}
			}
		}
		else
		{
			if (in_next_forward[v] != true && fwd_idx.second[v] / getSetOutSize(v, pred_set) >= lowest_rmax)
			{
				next_forward_from.push_back(make_pair(v, level));
				in_next_forward[v] = true;
			}
		}
	}

	forward_from = next_forward_from;
}

void PathQueryHandler::compute_ppr_with_fwdidx_topk_with_bound(double check_rsum, pair<iMap<double>, iMap<double>> &fwd_idx,
															   unordered_map<int, double> &v2ppr, double delta, double alpha, double threshold,
															   const vector<int> &pred_set, double pfail, double &zero_ppr_upper_bound, double omega,
															   iMap<double> &upper_bounds, iMap<double> &lower_bounds)
{
	compute_ppr_with_reserve(fwd_idx, v2ppr);

	if (check_rsum == 0.0)
		return;

	long num_random_walk = omega * check_rsum;
	long real_num_rand_walk = 0;

	for (long i = 0; i < fwd_idx.second.occur.m_num; i++)
	{
		int source = fwd_idx.second.occur[i];
		double residual = fwd_idx.second[source];
		long num_s_rw = ceil(residual / check_rsum * num_random_walk);
		double a_s = residual / check_rsum * num_random_walk / num_s_rw;

		real_num_rand_walk += num_s_rw;

		double ppr_incre = a_s * check_rsum / num_random_walk;
		for (long j = 0; j < num_s_rw; j++)
		{
			int des = random_walk(source, alpha, pred_set);
			if (v2ppr.find(des) == v2ppr.end())
				v2ppr[des] = ppr_incre;
			else
				v2ppr[des] += ppr_incre;
		}
	}

	if (delta < threshold)
		set_ppr_bounds(fwd_idx, check_rsum, real_num_rand_walk, v2ppr, pfail, zero_ppr_upper_bound,
					   upper_bounds, lower_bounds);
	else
		zero_ppr_upper_bound = calculate_lambda(check_rsum, pfail, zero_ppr_upper_bound, real_num_rand_walk);
}

void PathQueryHandler::set_ppr_bounds(pair<iMap<double>, iMap<double>> &fwd_idx, double rsum,
									  long total_rw_num, unordered_map<int, double> &v2ppr, double pfail, double &zero_ppr_upper_bound,
									  iMap<double> &upper_bounds, iMap<double> &lower_bounds)
{
	double min_ppr = 1.0 / getVertNum();
	double sqrt_min_ppr = sqrt(1.0 / getVertNum());

	double epsilon_v_div = sqrt(2.67 * rsum * log(2.0 / pfail) / total_rw_num);
	double default_epsilon_v = epsilon_v_div / sqrt_min_ppr;

	int nodeid;
	double ub_eps_a;
	double lb_eps_a;
	double ub_eps_v;
	double lb_eps_v;
	double up_bound;
	double low_bound;
	zero_ppr_upper_bound = calculate_lambda(rsum, pfail, zero_ppr_upper_bound, total_rw_num);

	for (auto it = v2ppr.begin(); it != v2ppr.end(); ++it)
	{
		nodeid = it->first;
		if (v2ppr[nodeid] <= 0)
			continue;
		double reserve = 0.0;
		if (fwd_idx.first.exist(nodeid))
			reserve = fwd_idx.first[nodeid];
		double epsilon_a = 1.0;
		if (upper_bounds.exist(nodeid))
		{
			assert(upper_bounds[nodeid] > 0.0);
			if (upper_bounds[nodeid] > reserve)
				epsilon_a = calculate_lambda(rsum, pfail, upper_bounds[nodeid] - reserve, total_rw_num);
			else
				epsilon_a = calculate_lambda(rsum, pfail, 1 - reserve, total_rw_num);
		}
		else
			epsilon_a = calculate_lambda(rsum, pfail, 1.0 - reserve, total_rw_num);

		ub_eps_a = v2ppr[nodeid] + epsilon_a;
		lb_eps_a = v2ppr[nodeid] - epsilon_a;
		if (!(lb_eps_a > 0))
			lb_eps_a = 0;

		double epsilon_v = default_epsilon_v;
		if (fwd_idx.first.exist(nodeid) && fwd_idx.first[nodeid] > min_ppr)
		{
			if (lower_bounds.exist(nodeid))
				reserve = max(reserve, lower_bounds[nodeid]);
			epsilon_v = epsilon_v_div / sqrt(reserve);
		}
		else
		{
			if (lower_bounds[nodeid] > 0)
				epsilon_v = epsilon_v_div / sqrt(lower_bounds[nodeid]);
		}

		ub_eps_v = 1.0;
		lb_eps_v = 0.0;
		if (1.0 - epsilon_v > 0)
		{
			ub_eps_v = v2ppr[nodeid] / (1.0 - epsilon_v);
			lb_eps_v = v2ppr[nodeid] / (1.0 + epsilon_v);
		}

		up_bound = min(min(ub_eps_a, ub_eps_v), 1.0);
		low_bound = max(max(lb_eps_a, lb_eps_v), reserve);
		if (up_bound > 0)
		{
			if (!upper_bounds.exist(nodeid))
				upper_bounds.insert(nodeid, up_bound);
			else
				upper_bounds[nodeid] = up_bound;
		}

		if (low_bound >= 0)
		{
			if (!lower_bounds.exist(nodeid))
				lower_bounds.insert(nodeid, low_bound);
			else
				lower_bounds[nodeid] = low_bound;
		}
	}
}

inline double PathQueryHandler::calculate_lambda(double rsum, double pfail, double upper_bound, long total_rw_num)
{
	return 1.0 / 3 * log(2 / pfail) * rsum / total_rw_num +
		   sqrt(4.0 / 9.0 * log(2.0 / pfail) * log(2.0 / pfail) * rsum * rsum +
				8 * total_rw_num * log(2.0 / pfail) * rsum * upper_bound) /
			   2.0 / total_rw_num;
}

inline int PathQueryHandler::random_walk(int start, double alpha, const vector<int> &pred_set)
{
	int cur = start;
	int k;
	if (getSetOutSize(start, pred_set) == 0)
		return start;
	while (true)
	{
		if ((double)rand() / (double)RAND_MAX <= alpha) // drand, return bool, bernoulli by alpha
			return cur;
		if (getSetOutSize(cur, pred_set))
		{
			k = rand() % getSetOutSize(cur, pred_set); // lrand
			int curr_idx = k;
			for (int pred : pred_set)
			{
				int curr_out = getOutSize(cur, pred);
				if (curr_out <= curr_idx)
					curr_idx -= curr_out;
				else
				{
					cur = getOutVertID(cur, pred, curr_idx);
					if (cur == -1)
					{
						cout << "ERROR1!!!!!!" << endl;
						exit(0); // TODO: throw an exception
					}
					break;
				}
			}
		}
		else
			cur = start;
	}
}

double PathQueryHandler::kth_ppr(unordered_map<int, double> &v2ppr, int retNum)
{
	static vector<double> temp_ppr;
	temp_ppr.clear();
	temp_ppr.resize(v2ppr.size());
	int i = 0;
	for (auto it = v2ppr.begin(); it != v2ppr.end(); ++it)
	{
		temp_ppr[i] = v2ppr[it->second];
		i++;
	}
	nth_element(temp_ppr.begin(), temp_ppr.begin() + retNum - 1, temp_ppr.end(),
				[](double x, double y)
				{ return x > y; });
	return temp_ppr[retNum - 1];
}

bool PathQueryHandler::if_stop(int retNum, double delta, double threshold, double epsilon, iMap<int> &topk_filter,
							   iMap<double> &upper_bounds, iMap<double> &lower_bounds, unordered_map<int, double> &v2ppr)
{
	if (kth_ppr(v2ppr, retNum) >= 2.0 * delta)
		return true;

	if (delta >= threshold)
		return false;

	const static double error = 1.0 + epsilon;
	const static double error_2 = 1.0 + epsilon;

	vector<pair<int, double>> topk_pprs;
	topk_pprs.clear();
	topk_pprs.resize(retNum);
	topk_filter.clean();

	static vector<pair<int, double>> temp_bounds;
	temp_bounds.clear();
	temp_bounds.resize(lower_bounds.occur.m_num);
	int nodeid;
	for (unsigned i = 0; i < lower_bounds.occur.m_num; i++)
	{
		nodeid = lower_bounds.occur[i];
		temp_bounds[i] = make_pair(nodeid, lower_bounds[nodeid]);
	}

	// sort topk nodes by lower bound
	partial_sort_copy(temp_bounds.begin(), temp_bounds.end(), topk_pprs.begin(), topk_pprs.end(),
					  [](pair<int, double> const &l, pair<int, double> const &r)
					  { return l.second > r.second; });

	// for topk nodes, upper-bound/low-bound <= 1+epsilon
	double ratio = 0.0;
	double largest_ratio = 0.0;
	for (auto &node : topk_pprs)
	{
		topk_filter.insert(node.first, 1);
		ratio = upper_bounds[node.first] / lower_bounds[node.first];
		if (ratio > largest_ratio)
			largest_ratio = ratio;
		if (ratio > error_2)
		{
			return false;
		}
	}

	// for remaining NO. retNum+1 to NO. n nodes, low-bound of retNum > the max upper-bound of remaining nodes
	double low_bound_k = topk_pprs[retNum - 1].second;
	if (low_bound_k <= delta)
		return false;

	for (unsigned i = 0; i < upper_bounds.occur.m_num; i++)
	{
		nodeid = upper_bounds.occur[i];
		if (topk_filter.exist(nodeid) || v2ppr[nodeid] <= 0)
			continue;

		double upper_temp = upper_bounds[nodeid];
		double lower_temp = lower_bounds[nodeid];
		if (upper_temp > low_bound_k * error)
		{
			if (upper_temp > (1 + epsilon) / (1 - epsilon) * lower_temp)
				continue;
			else
				return false;
		}
		else
			continue;
	}

	return true;
}

/**
	Generate an equal number of positive and negative queries for a given query type,
	and storing them in the queries vector.
	Note: Treat pred_set as full (allow all possible predicates) for now.
	Note: To deal with the problem that all vertex IDs cannot be accessed at once for
	graphs with multiple predicates, you may employ a two-level strategy when picking
	srcID and dstID: first randomly pick a predicate, then randomly pick a vertexID in
	that predicate's id2vid list.

	@param queryType the type of queries (0 for cycle, 1 for shortestPath).
	@param directed if false, treat all edges in the graph as bidirectional.
	@param numQueries the number of positive (negative) queries to generate.
	@param queries vector to store the generated queries. Each element will be ((srcID, dstID), result).
	Result for cycle will be 0 for non-existence of cycles and 1 for existence. Result for shortestPath
	will be the length of the shortest path; negative queries for this category will be UNREACHABLE pairs,
	where result will be -1.
*/
// void PathQueryHandler::generateQueries(int queryType, bool directed, int numQueries, vector<pair<int, int>, int>& queries)
// {

// }

/**
 * transfer vector to string, split with ','
 *
 * @param path_set
 * @return std::string
 */
std::string PathQueryHandler::getPathString(std::vector<int> &path_set)
{
	stringstream ss;
	int lens = path_set.size();
	for (int i = 0; i < lens; ++i)
	{
		if (i != 0)
		{
			ss << ",";
		}
		ss << path_set[i];
	}
	return ss.str();
}

vector<int> PathQueryHandler::BFS(int uid, bool directed, const vector<int> &pred_set, bool forward)
{
	vector<int> ret;
	unordered_set<int> ret_set;
	ret.push_back(uid);
	ret_set.insert(uid);
	size_t curr = 0;
	while (curr < ret.size())
	{
		if (!directed)
		{
			for (int pred : pred_set)
			{
				int inSz = getInSize(ret[curr], pred), inNei;
				for (int i = 0; i < inSz; i++)
				{
					inNei = getInVertID(ret[curr], pred, i);
					if (ret_set.find(inNei) == ret_set.end())
					{
						ret.push_back(inNei);
						ret_set.insert(inNei);
					}
				}
				int outSz = getOutSize(ret[curr], pred), outNei;
				for (int i = 0; i < outSz; i++)
				{
					outNei = getOutVertID(ret[curr], pred, i);
					if (ret_set.find(outNei) == ret_set.end())
					{
						ret.push_back(outNei);
						ret_set.insert(outNei);
					}
				}
			}
		}
		else if (forward)
		{
			for (int pred : pred_set)
			{
				int outSz = getOutSize(ret[curr], pred), outNei;
				for (int i = 0; i < outSz; i++)
				{
					outNei = getOutVertID(ret[curr], pred, i);
					if (ret_set.find(outNei) == ret_set.end())
					{
						ret.push_back(outNei);
						ret_set.insert(outNei);
					}
				}
			}
		}
		else
		{
			for (int pred : pred_set)
			{
				int inSz = getInSize(ret[curr], pred), inNei;
				for (int i = 0; i < inSz; i++)
				{
					inNei = getInVertID(ret[curr], pred, i);
					if (ret_set.find(inNei) == ret_set.end())
					{
						ret.push_back(inNei);
						ret_set.insert(inNei);
					}
				}
			}
		}
		curr++;
	}

	return ret;
}

/**
	Compute and return the number of vertices reachable from vertex u by BFS
	at specified distances.

	@param uid vertex u's ID.
	@param directed if false, treat all edges in the graph as bidirectional.
	@param pred_set the set of edge labels allowed.
	@return the number of vertices reachable from u with k steps.
**/
int PathQueryHandler::kHopCount(int uid, bool directed, int k, const std::vector<int> &pred_set) {
	 std::vector<int> neighbor = kHopNeighbor(uid, directed, k, pred_set, -1);
	 return neighbor.size();
}
/**
	Compute and return the vertices reachable from vertex u by BFS
	at specified distances.

	@param uid vertex u's ID.
	@param directed if false, treat all edges in the graph as bidirectional.
	@param pred_set the set of edge labels allowed.
	@return the number of vertices reachable from u with k steps.
**/
 std::vector<int> PathQueryHandler::kHopNeighbor(int uid, bool directed, int k, const std::vector<int> &pred_set, int retNum) {
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


/**
	lovain

	@param phase1_loop_num 第一阶段最大迭代轮数(>=1) .
	@param min_modularity_increase 模块度增益阈值(0~1) .
	@param pred_set 谓词集合（无该属性的边不参与计算）.
	@param directed 图是否存在方向（为false时认为是无向图）
	@return 社区数.
*/
void PathQueryHandler::louvain(int phase1_loop_num,float min_modularity_increase,std::vector<int> &pred_set,bool directed, std::pair<size_t, std::map<int, std::set<int> > > &result){
	//所有节点id
  	set<int> vids;
	//当前存在的节点/超节点,以及存在于哪个社区
	map<int,int> super_nodes;
	//超节点对应的真实vid节点
	map<int,set<int> > super2vid;
	//当前的社区
	map<int,set<int> > community;
	//所有从节点i指其他节点的权重（包括自己）（出边）
	map<int,map<int,double> > weights;
	//首先，遍历满足谓词集合的点和边，初始化上面的变量
	for (int pred : pred_set){
		vids.insert(csr[1].adjacency_list[pred].begin(), csr[1].adjacency_list[pred].end());
	}
	for(int vid:vids){
		super_nodes[vid]=vid;
		super2vid[vid]=set<int> ();
		super2vid[vid].insert(vid);
		community[vid]=set<int> ();
		community[vid].insert(vid);
		weights[vid]=map<int,double>();
	}
	//有向图所有边的权值和，即公式中的2m
	double m=0;
	// 初始化k_i_in
	for(int pred:pred_set){
		for(int vid:vids){
			int vIndex = getInIndexByID(vid, pred);
			if(vIndex!=-1){
				int begin=csr[1].offset_list[pred][vIndex];
				int end=0;
				// 如果是offset_list中的最后一个
				if(vIndex == csr[1].offset_list[pred].size() - 1){
					end=csr[1].adjacency_list[pred].size();
				}else{
					end=csr[1].offset_list[pred][vIndex+1];
				}
				for(int i=begin;i<end;i++){
					int out_vid=csr[1].adjacency_list[pred][i];
					if(weights[out_vid].find(vid)==weights[out_vid].end()){
						weights[out_vid][vid]=1;
					}else{
						weights[out_vid][vid]+=1;
					}
					m++;
					// 如果是双向的
					if(!directed){
						if(weights[vid].find(out_vid)==weights[vid].end()){
						weights[vid][out_vid]=1;
					}else{
						weights[vid][out_vid]+=1;
					}
					m++;
					}
				}
			}
		}
	}

	bool finished=false;
	while (!finished)
	{
		//第一阶段
		bool phase1_finished=true;
		//社区C的内部的所有边的权值和
		map<int,double> s_in;
		//所有指向区域C中的节点的边的权值和
		map<int,double> s_tot;
		//计算s_in,s_out
		for(auto weight:weights){
			int out_id=weight.first;
			//出发点所在社区
			int out_community=super_nodes[out_id];
			for(auto in_node:weight.second){
				int in_id=in_node.first;
				//入点所在社区
				int in_community=super_nodes[in_id];
				if(in_community==out_community){
					if(s_in.find(in_community)==s_in.end()){
						s_in[in_community]=in_node.second;
					}else{
						s_in[in_community]+=in_node.second;
					}
					//s_tot
					if(s_tot.find(in_community)==s_tot.end()){
						s_tot[in_community]=in_node.second;
					}else{
						s_tot[in_community]=in_node.second;
					}
				}else{
					if(s_tot.find(in_community)==s_tot.end()){
						s_tot[in_community]=in_node.second;
					}else{
						s_tot[in_community]=in_node.second;
					}
				}
			}
		}
		
		for(int phase1_loop=0;phase1_loop<phase1_loop_num;phase1_loop++){
			phase1_finished=true;
			//遍历超节点
			//super_node.first-vid super_node.second-community
			for(auto super_node:super_nodes){
				//目前最大的deta_q
				double max_deta_q=min_modularity_increase;
				//所有从节点i指向区域C的边的权值和
				double max_k_i_in=0;
				//所有从区域C指向节点i的边的权值和
				double max_k_i_out=0;
				//指向节点i的所有边的权值和
				double k_i=0;
				//最大deta_q所在社区
				int max_community=-1;
				//计算k_i
				for(auto weight:weights){
					if(weight.second.find(super_node.first)!=weight.second.end()){
						k_i+=weight.second[super_node.first];
					}
				}
				//com.first-community com.second-vids
				for(auto com:community){
					//不是super_node所在的社区
					if(com.first!=super_node.second){
						double k_i_in=0;
						double k_i_out=0;
						for(auto node:com.second){
							//社区中存在指向i的点
							if(weights[node].find(super_node.first)!=weights[node].end()){
								k_i_out+=weights[node][super_node.first];
							}
							//节点i指向社区中的点
							if(weights[super_node.first].find(node)!=weights[super_node.first].end()){
								k_i_in+=weights[super_node.first][node];
							}
						}
						double deta_q=(k_i_in-s_tot[com.first]*k_i*2/m)/m;
						if(deta_q>max_deta_q){
							max_deta_q=deta_q;
							max_k_i_in=k_i_in;
							max_k_i_out=k_i_out;
							max_community=com.first;
						}
					}
				}

				//如果存在可以并入的社区
				if(max_community!=-1){
					phase1_finished=false;
					//旧社区id
					int old_community=super_node.second;
					//更新super_nodes community s_in s_tot
					//i自环
					int i_to_i=0;
					if(weights[super_node.first].find(super_node.first)!=weights[super_node.first].end()){
						i_to_i=weights[super_node.first][super_node.first];
					}
					community[old_community].erase(super_node.first);
					//如果转移之后旧社区中还存在点，就需要更新旧社区的community s_in s_tot
					if(community[old_community].size()!=0){
						//分别是i指向旧社区中点的，旧社区中点指向i的，不包括i自环
						int i_to_old=0;
						int old_to_i=0;
						for(auto node:community[old_community]){
							if(weights[super_node.first].find(node)!=weights[super_node.first].end()){
								i_to_old+=weights[super_node.first][node];
							}
							if(weights[node].find(super_node.first)!=weights[node].end()){
								old_to_i+=weights[node][super_node.first];
							}
						}
						s_in[old_community]=s_in[old_community]-i_to_old-old_to_i-i_to_i;
						s_tot[old_community]=s_tot[old_community]-k_i;
					}else{
						s_in.erase(old_community);
						s_tot.erase(old_community);
						community.erase(old_community);

					}
					//分别是i指向新社区中点的，新社区中点指向i的，不包括i自环
					int i_to_new=0;
					int new_to_i=0;
					for(auto node:community[max_community]){
						if(weights[super_node.first].find(node)!=weights[super_node.first].end()){
							i_to_new+=weights[super_node.first][node];
						}
						if(weights[node].find(super_node.first)!=weights[node].end()){
							new_to_i+=weights[node][super_node.first];
						}
					}
					s_in[max_community]=s_in[max_community]+i_to_new+new_to_i+i_to_i;
					s_tot[max_community]=s_tot[max_community]+k_i;
					community[max_community].insert(super_node.first);
					super_nodes[super_node.first]=max_community;
				}
			}
			if(phase1_finished)break;
		}
		//判断是否结束
		finished=true;
		//如果社区中存在超过一个的点
		for(auto com:community){
			if(com.second.size()>1){
				finished=false;
				break;
			}
		}
		if(finished)break;
		//第二阶段,更新super_nodes，super2vid，community，weights
		map<int,int> new_super_nodes;
		map<int,set<int> > new_community;
		map<int,map<int,double> > new_weights;
		map<int,set<int> > new_super2vid;
		for(auto com:community){
			new_super_nodes[com.first]=com.first;
			new_community[com.first]=set<int> ();
			new_community[com.first].insert(com.first);
			new_weights[com.first]=map<int,double> ();
			new_super2vid[com.first]=set<int> ();
			//超节点的自环
			new_weights[com.first][com.first]=s_in[com.first];
			//更新super2vid
			for(auto node:com.second){
				new_super2vid[com.first].insert(super2vid[node].begin(),super2vid[node].end());
			}
		}
		for(auto weight:weights){
			int out_id=weight.first;
			int out_community=super_nodes[out_id];
			for(auto in_node:weight.second){
				int in_id=in_node.first;
				int in_community=super_nodes[in_id];
				if(in_community!=out_community){
					if(new_weights[out_community].find(in_community)==new_weights[out_community].end()){
						new_weights[out_community][in_community]=weights[out_id][in_id];
					}else{
						new_weights[out_community][in_community]+=weights[out_id][in_id];
					}
				}
			}
		}
		super_nodes=new_super_nodes;
		community=new_community;
		weights=new_weights;
		super2vid=new_super2vid;
	}
	
	result.first=super2vid.size();
	result.second=super2vid;
}
