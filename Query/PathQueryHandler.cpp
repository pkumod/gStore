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
	if (n != -1)
		return n;	// Only consider static graphs for now
	set<int> vertices;
	for (int j = 0; j < 2; j++)
	{
		for (int i = 0; i < csr[j].pre_num; i++)
			vertices.insert(csr[j].adjacency_list[i].begin(), csr[j].adjacency_list[i].end());
	}
	n = vertices.size();
	return n;
}

int PathQueryHandler::getEdgeNum()
{
	if (m != -1)
		return m;	// Only consider static graphs for now
	int ret = 0;
	for (int i = 0; i < csr[1].pre_num; i++)	// Same as summing that of csr[0]
		ret += csr[1].adjacency_list[i].size();
	m = ret;
	return m;
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
	if (vIndex == -1)	// This vertex does not participate in this pred's relations
		return 0;
	else if (vIndex == csr[1].offset_list[pred].size() - 1)
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
		getTotalInSize(vid, true);	// Load into cache

	if (pos < distinctInEdges[vid].size())
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
		for (int i = 0; i < csr[1].pre_num; i++)
			ret += getInSize(vid, i);
	}
	else
	{
		if (distinctInEdges.find(vid) == distinctInEdges.end())
		{
			if (distinctInEdges.size() == cacheMaxSize)
			{
				int replacement = rand() % cacheMaxSize;
				distinctInEdges.erase(next(distinctInEdges.begin(), replacement));
				// cout << "distinctInEdges replaced entry " << replacement << endl;
			}

			distinctInEdges[vid] = set<int>();
			for (int pred = 0; pred < csr[1].pre_num; pred++)
			{
				int vIndex = getInIndexByID(vid, pred);
				if (vIndex == -1)	// This vertex does not participate in this pred's relations
					continue;
				else if (vIndex == csr[1].offset_list[pred].size() - 1 \
					&& csr[1].adjacency_list[pred].size() > csr[1].offset_list[pred][vIndex])
					distinctInEdges[vid].insert(next(csr[1].adjacency_list[pred].begin(), csr[1].offset_list[pred][vIndex]), \
						csr[1].adjacency_list[pred].end());
				else if (csr[1].offset_list[pred][vIndex + 1] > csr[1].offset_list[pred][vIndex])
					distinctInEdges[vid].insert(next(csr[1].adjacency_list[pred].begin(), csr[1].offset_list[pred][vIndex]), \
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
	if (vIndex == -1)	// This vertex does not participate in this pred's relations
		return 0;
	else if (vIndex == csr[0].offset_list[pred].size() - 1)
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
		getTotalOutSize(vid, true);	// Load into cache

	if (pos < distinctOutEdges[vid].size())
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
		for (int i = 0; i < csr[1].pre_num; i++)
			ret += getOutSize(vid, i);
	}
	else
	{
		if (distinctOutEdges.find(vid) == distinctOutEdges.end())
		{
			if (distinctOutEdges.size() == cacheMaxSize)
			{
				int replacement = rand() % cacheMaxSize;
				distinctOutEdges.erase(next(distinctOutEdges.begin(), replacement));
				// cout << "distinctInEdges replaced entry " << replacement << endl;
			}

			distinctOutEdges[vid] = set<int>();
			for (int pred = 0; pred < csr[1].pre_num; pred++)
			{
				int vIndex = getOutIndexByID(vid, pred);
				if (vIndex == -1)	// This vertex does not participate in this pred's relations
					continue;
				else if (vIndex == csr[0].offset_list[pred].size() - 1 \
					&& csr[0].adjacency_list[pred].size() > csr[0].offset_list[pred][vIndex])
					distinctOutEdges[vid].insert(next(csr[0].adjacency_list[pred].begin(), csr[0].offset_list[pred][vIndex]), \
						csr[0].adjacency_list[pred].end());
				else if (csr[0].offset_list[pred][vIndex + 1] > csr[0].offset_list[pred][vIndex])
					distinctOutEdges[vid].insert(next(csr[0].adjacency_list[pred].begin(), csr[0].offset_list[pred][vIndex]), \
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

	int **indegree = new int*[numLabel];
	int **outdegree = new int*[numLabel];
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
void PathQueryHandler::dfs(map<int, vector<int> > &route, map<int, bool> &vis,
	int q, int v, const vector<int> pred_set, vector<int> &ans, bool &finished)
{		
	if(finished) return;
	int num_of_pred = pred_set.size();
	for(int i = 0; i < num_of_pred; ++i)
	{
		int num_out = getOutSize(q, pred_set[i]);
		for(int j = 0; j < num_out; ++j)
		{
			int temp = getOutVertID(q, pred_set[i], j);
			if(vis.find(temp) != vis.end() && vis[temp] == 1)	continue;
			vis[temp] = 1;
			route[temp] = route[q];
			route[temp].push_back(pred_set[i]);
			route[temp].push_back(temp);
			if(temp == v)
			{
				finished = 1;
				ans = route[temp];
				return;
			}
			if(finished) return;
			dfs(route, vis, temp, v, pred_set, ans, finished); //get the childnode dfs.
			if(finished) return;
			//vis[temp] = 0;
			//if(temp == v && flag == 1)	flag = 0; //need to recover the sign.
		}
		//if(directed) continue;
		//if directed == 0, need to consider the traverse edge.
		int num_in = getInSize(q, pred_set[i]);
		if(finished) return;
		for(int j = 0; j < num_in; ++j)
		{
			int temp = getInVertID(q, pred_set[i], j);
			if(vis.find(temp) != vis.end() && vis[temp] == 1)	continue;
			vis[temp] = 1;
			route[temp] = route[q];
			route[temp].push_back(pred_set[i]);
			route[temp].push_back(temp);
			if(temp == v)
			{
				finished = 1;
				ans = route[temp];
				return;
			}
			if(finished) return;
			dfs(route, vis, temp, v, pred_set, ans, finished); 
			if(finished) return;
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
	if(directed)
	{
		vector<int> ans1, ans2;
		ans1 = shortestPath(uid, vid, true, pred_set);
		ans2 = shortestPath(vid, uid, true, pred_set);
		if(ans1.size() == 0 || ans2.size() == 0) return ans;
		else
		{
			for(int i = 1; i < ans2.size() - 1; ++i)
				ans1.push_back(ans2[i]);
			ans1.push_back(uid);
			return ans1;
		}
	}
	else	//use BFS to get the smallest cycle. since every edge is undirected, just need to find the route from u to v.
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
		for(int i = lens - 2; i > 0; --i)
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

vector<int> PathQueryHandler::shortestPath0(int uid, int vid, bool directed, const vector<int> &pred_set)//cost more space and more time?
{
	//cout << "BFS1" << endl;
	map<int, vector<int> > route_u, route_v;
	route_u[uid].push_back(uid);
	route_v[vid].push_back(vid);
	queue<int> q_u, q_v;
	int num_of_pred = pred_set.size();
	q_u.push(uid);
	q_v.push(vid);
	
	bool flag = 0; // sign whether v has been found
	map<int, int>::iterator it;
	int meet_node = 0;
	while(flag == 0 && (!q_u.empty() || !q_v.empty()))
	{
		queue<int> new_q_u;
		while(!q_u.empty() && flag == 0)
		{
			int temp_u = q_u.front();
			q_u.pop();
			for(int i = 0; i < num_of_pred; ++i)
			{
				int x = pred_set[i];
				int num = getOutSize(temp_u, x);
				for(int j = 0; j < num; ++j)
				{
					int t = getOutVertID(temp_u, x, j); 
					
					if(route_v.find(t) != route_v.end())
					{
						flag = 1;
						meet_node = t;
						route_u[t] = route_u[temp_u];
						route_u[t].push_back(x);
						break;
					}									//get the meet_node

					if(route_u.find(t) != route_u.end()) continue;		//have already visited the node
					
					new_q_u.push(t);
					route_u[t] = route_u[temp_u];
					route_u[t].push_back(x);
					route_u[t].push_back(t);
				}
				if(flag) break;
				
				if(directed) continue;
				//undirected: need to visit the in-neighbours
				num = getInSize(temp_u, x);
				for(int j = 0; j < num; ++j)
				{
					int t = getInVertID(temp_u, x, j); 
					
					if(route_v.find(t) != route_v.end())
					{
						flag = 1;
						meet_node = t;
						route_u[t] = route_u[temp_u];
						route_u[t].push_back(x);
						break;
					}									//get the meet_node

					if(route_u.find(t) != route_u.end()) continue;		//have already visited the node
					
					new_q_u.push(t);
					route_u[t] = route_u[temp_u];
					route_u[t].push_back(x);
					route_u[t].push_back(t);
				}
				if(flag) break;
			}
		}
		q_u = new_q_u; //update the queue;

		queue<int> new_q_v;
		while(!q_v.empty() && flag == 0)
		{
			int temp_v = q_v.front();
			q_v.pop();
			for(int i = 0; i < num_of_pred; ++i)
			{
				int x = pred_set[i];
				int num = getInSize(temp_v, x);
				for(int j = 0; j < num; ++j)
				{
					int t = getInVertID(temp_v, x, j); 
					
					if(route_u.find(t) != route_u.end())
					{
						flag = 1;
						meet_node = t;
						route_v[t] = route_v[temp_v];
						route_v[t].push_back(x);
						break;
					}							

					if(route_v.find(t) != route_v.end()) continue;		
					
					new_q_v.push(t);
					route_v[t] = route_v[temp_v];
					route_v[t].push_back(x);
					route_v[t].push_back(t);
				}
				if(flag) break;
				
				if(directed) continue;

				num = getOutSize(temp_v, x);
				for(int j = 0; j < num; ++j)
				{
					int t = getOutVertID(temp_v, x, j); 
					
					if(route_u.find(t) != route_u.end())
					{
						flag = 1;
						meet_node = t;
						route_v[t] = route_v[temp_v];
						route_v[t].push_back(x);
						break;
					}							

					if(route_v.find(t) != route_v.end()) continue;		
					
					new_q_v.push(t);
					route_v[t] = route_v[temp_v];
					route_v[t].push_back(x);
					route_v[t].push_back(t);
				}
				if(flag) break;
			}
		}
		q_v = new_q_v;	
	}
	
	vector<int> ans;
	if(flag == 0) return ans; //no route from u to v.
	ans = route_u[meet_node];
	if(route_v.find(meet_node) != route_v.end())
	{
		int lens = route_v[meet_node].size();
		for(int i = 0; i < lens; ++i)
			ans.push_back(route_v[meet_node][lens - 1 - i]);
	}//make up the route with route_u and route_v.
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
vector<int> PathQueryHandler::shortestPath(int uid, int vid, bool directed, const vector<int> &pred_set) //cost less space and less time.
{
	//cout << "BFS2" << endl;
	map<int, int> dis_u, dis_v;//store the distance to u and v
	queue<int> q_u, q_v;
	int num_of_pred = pred_set.size();
	q_u.push(uid);
	q_v.push(vid);
	dis_u[uid] = 0;
	dis_v[vid] = 0;
	bool flag = 0;
	map<int, int>::iterator it;
	int meet_node = 0;
	while(flag == 0 && (!q_u.empty() || !q_v.empty()))
	{
		queue<int> new_q_u;
		
		while(!q_u.empty() && flag == 0)
		{
			int temp_u = q_u.front();
			
			q_u.pop();
			for(int i = 0; i < num_of_pred; ++i)
			{
				int x = pred_set[i];
				int num = getOutSize(temp_u, x);
				for(int j = 0; j < num; ++j)
				{
					int t = getOutVertID(temp_u, x, j); //get the node
					
					if(dis_v.find(t) != dis_v.end())
					{
						flag = 1;
						meet_node = t;
						dis_u[t] = dis_u[temp_u] + 1;
						break;
					}									//get the meet node

					if(dis_u.find(t) != dis_u.end()) continue;		//has visited before
					
					new_q_u.push(t);
					dis_u[t] = dis_u[temp_u] + 1;
				}
				if(flag) break;
				if(directed) continue;

				num = getInSize(temp_u, x);
				for(int j = 0; j < num; ++j)
				{
					int t = getInVertID(temp_u, x, j); //get the node
					
					if(dis_v.find(t) != dis_v.end())
					{
						flag = 1;
						meet_node = t;
						dis_u[t] = dis_u[temp_u] + 1;
						break;
					}									//get the meet node

					if(dis_u.find(t) != dis_u.end()) continue;		//has visited before
					
					new_q_u.push(t);
					dis_u[t] = dis_u[temp_u] + 1;
				}
				if(flag) break;
			}
		}
		q_u = new_q_u;
		
		queue<int> new_q_v;
		while(!q_v.empty() && flag == 0)
		{
			int temp_v = q_v.front();
			
			q_v.pop();
			for(int i = 0; i < num_of_pred; ++i)
			{
				int x = pred_set[i];
				int num = getInSize(temp_v, x);
				
				for(int j = 0; j < num; ++j)
				{
					int t = getInVertID(temp_v, x, j); 
					
					if(dis_u.find(t) != dis_u.end())
					{
						flag = 1;
						meet_node = t;
						dis_v[t] = dis_v[temp_v] + 1;
						break;
					}									

					if(dis_v.find(t) != dis_v.end()) continue;	// Already in	
					
					new_q_v.push(t);
					dis_v[t] = dis_v[temp_v] + 1;
					
				}
				if(flag) break;

				if(directed) continue;

				num = getOutSize(temp_v, x);
				for(int j = 0; j < num; ++j)
				{
					int t = getOutVertID(temp_v, x, j); 
					
					if(dis_u.find(t) != dis_u.end())
					{
						flag = 1;
						meet_node = t;
						dis_v[t] = dis_v[temp_v] + 1;
						break;
					}									

					if(dis_v.find(t) != dis_v.end()) continue;	// Already in	
					
					new_q_v.push(t);
					dis_v[t] = dis_v[temp_v] + 1;

				}
				if(flag) break;
			}
		}
		
		q_v = new_q_v;		
	}

	//get the route through the distance.
	vector<int> empty_ans;
	if(flag == 0) return empty_ans; // no route from u to v
	stack<int> s;
	s.push(meet_node);
	//cout << "*************you are wrong!!!**********" << endl;
	while(1)
	{
		int temp = s.top();
		if(temp == vid) break;
		int flag0 = 0;
		for(int i = 0; i < num_of_pred; ++i)
		{
			int x = pred_set[i];
			int num = getOutSize(temp, x);
			for(int j = 0; j < num; ++j)
			{
				int t = getOutVertID(temp, x, j); 
				if(dis_v.find(t) != dis_v.end() && dis_v[t] == dis_v[temp] - 1)
				{
					s.push(x);
					s.push(t);
					flag0 = 1;
					break;
				}
			}
			if(flag0) break;

			if(directed) continue;
			num = getInSize(temp, x);
			for(int j = 0; j < num; ++j)
			{
				int t = getInVertID(temp, x, j); 
				if(dis_v.find(t) != dis_v.end() && dis_v[t] == dis_v[temp] - 1)
				{
					s.push(x);
					s.push(t);
					flag0 = 1;
					break;
				}
			}
			if(flag0) break;
		}
	}//get the route from meet_node to v.
	
	stack<int> s_new;
	while(!s.empty())
	{
		int temp = s.top();
		s.pop();
		s_new.push(temp);
	}
	if(s_new.empty())
	{
		s_new.push(meet_node);
		//cout << "s_new empty" << endl;
	}
	while(1)
	{
		int temp = s_new.top();
		if(temp == uid) break;
		int flag0 = 0;
		for(int i = 0; i < num_of_pred; ++i)
		{
			int x = pred_set[i];
			int num = getInSize(temp, x);
			for(int j = 0; j < num; ++j)
			{
				int t = getInVertID(temp, x, j); 
				if(dis_u.find(t) != dis_u.end() && dis_u[t] == dis_u[temp] - 1)
				{
					s_new.push(x);
					s_new.push(t);
					flag0 = 1;
					break;
				}
			}
			if(flag0) break;

			if(directed) continue;

			num = getOutSize(temp, x);
			for(int j = 0; j < num; ++j)
			{
				int t = getOutVertID(temp, x, j); 
				if(dis_u.find(t) != dis_u.end() && dis_u[t] == dis_u[temp] - 1)
				{
					s_new.push(x);
					s_new.push(t);
					flag0 = 1;
					break;
				}
			}
			if(flag0) break;
		}
	}//get the route from u to meet_node.

	int lens = s_new.size();
	vector<int> ans(lens);
	for(int i = 0; i < lens; ++i)
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
		if(!directed)
		{
			uOutTotal += getInSize(uid, pred);
			vInTotal += getOutSize(vid, pred);
		}
	}
	if (uOutTotal == 0 || vInTotal == 0)
		return false;

	//partial push
    double  rmax_fwd = 0.001, rmax_bwd = 0.001;
    queue< pair<int,int> > candidatePush1;
    queue< pair<int,int> > candidatePush1_next;
    queue< pair<int,int> > candidatePush2;
    queue< pair<int,int> > candidatePush2_next;
    candidatePush1.push( pair<int,int>(uid,0) );
    candidatePush2.push( pair<int,int>(vid,0) );
    map<int, int> residue1, residue2, l1, l2, f1, f2;
    residue1[uid] = 1;
    residue2[vid] = 1;
    l1[uid] = 0;
    l2[vid] = 0;

    //full push
    queue< int > fwdQ;
    queue< int > fwdQ_next;
    queue< int > bwdQ;
    queue< int > bwdQ_next;
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
    while (fullL1 <= k && fullL2 <= k && \
        (!fwdQ.empty() || !fwdQ_next.empty()) && (!bwdQ.empty() || !bwdQ_next.empty()))
    {
        op_cnt = 0;
        while ((!candidatePush1.empty() || !candidatePush1_next.empty()) && op_cnt < push_bs \
            && rmax_fwd > init_rmax)
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
                	if( residue1[curNode] / getSetOutSize(curNode, pred_set) >= rmax_fwd )
	                {
	                    for (int pred : pred_set)
	                    {
	                    	int num_out = getOutSize(curNode, pred);
	                    	for (int i = 0; i < num_out; i++)
	                    	{
	                    		int adj = getOutVertID(curNode, pred, i);
		                        if (l1.find(adj) == l1.end())
		                        {
		                            l1[adj] = level+1;
		                            if (l2.find(adj) != l2.end() && l1[adj] + l2[adj] <= k)
		                                return true;
		                            if (f2.find(adj) != f2.end() && l1[adj] + f2[adj] <= k)
		                                return true;
		                            residue1[adj] = (1-alpha)*residue1[curNode]/getSetOutSize(curNode, pred_set);	// Should be equivalent to +=
		                            if(residue1[adj]/getSetOutSize(adj, pred_set) >= rmax_fwd )
		                                candidatePush1.push( pair<int,int>(adj,level+1) );
		                            else
		                                candidatePush1_next.push( pair<int,int>(adj,level+1) );
		                        }
	                    	}
	                    }
	                    residue1[curNode] = 0;
                	}
	                else
	                    candidatePush1_next.push( pair<int,int>(curNode,level) );
                }

                else
                {
	                if( residue1[curNode] / (getSetOutSize(curNode, pred_set) + getSetInSize(curNode, pred_set)) \
	                	>= rmax_fwd )
	                {
	                    for (int pred : pred_set)
	                    {
	                    	int num_out = getOutSize(curNode, pred);
	                    	for (int i = 0; i < num_out; i++)
	                    	{
	                    		int adj = getOutVertID(curNode, pred, i);
		                        if (l1.find(adj) == l1.end())
		                        {
		                            l1[adj] = level+1;
		                            if (l2.find(adj) != l2.end() && l1[adj] + l2[adj] <= k)
		                                return true;
		                            if (f2.find(adj) != f2.end() && l1[adj] + f2[adj] <= k)
		                                return true;
		                            residue1[adj] = (1-alpha)*residue1[curNode] / \
		                            	(getSetOutSize(curNode, pred_set) + getSetInSize(curNode, pred_set));	// Should be equivalent to +=
		                            if(residue1[adj] / (getSetOutSize(adj, pred_set) + getSetInSize(adj, pred_set)) >= rmax_fwd)
		                                candidatePush1.push( pair<int,int>(adj,level+1) );
		                            else
		                                candidatePush1_next.push( pair<int,int>(adj,level+1) );
		                        }
	                    	}

	                    	int num_in = getInSize(curNode, pred);
	                    	for (int i = 0; i < num_in; i++)
	                    	{
	                    		int adj = getInVertID(curNode, pred, i);
		                        if (l1.find(adj) == l1.end())
		                        {
		                            l1[adj] = level+1;
		                            if (l2.find(adj) != l2.end() && l1[adj] + l2[adj] <= k)
		                                return true;
		                            if (f2.find(adj) != f2.end() && l1[adj] + f2[adj] <= k)
		                                return true;
		                            residue1[adj] = (1-alpha)*residue1[curNode] / \
		                            	(getSetOutSize(curNode, pred_set) + getSetInSize(curNode, pred_set));	// Should be equivalent to +=
		                            if(residue1[adj]/(getSetOutSize(adj, pred_set) + getSetInSize(adj, pred_set)) >= rmax_fwd )
		                                candidatePush1.push( pair<int,int>(adj,level+1) );
		                            else
		                                candidatePush1_next.push( pair<int,int>(adj,level+1) );
		                        }
	                    	}
	                    }
	                    residue1[curNode] = 0;
	                }
	                else
	                    candidatePush1_next.push( pair<int,int>(curNode,level) );
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
            		if(f1.find(adj) == f1.end())
	                {
	                    f1[adj] = fullL1;
	                    if (l2.find(adj) != l2.end() && f1[adj] + l2[adj] <= k)
	                        return true;
	                    if (f2.find(adj) != f2.end() && f1[adj] + f2[adj] <= k)
	                        return true;
	                    fwdQ_next.push(adj);
	                }
            	}

            	if(directed) continue;

            	int num_in = getInSize(curNode, pred);
            	for (int i = 0; i < num_in; i++)
            	{
            		int adj = getOutVertID(curNode, pred, i);
            		if(f1.find(adj) == f1.end())
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
        while ((!candidatePush2.empty() || !candidatePush2_next.empty()) && op_cnt < push_bs \
            && rmax_bwd > init_rmax)
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
                	if( residue2[curNode] / getSetInSize(curNode, pred_set) >= rmax_bwd )
	                {
	                    for (int pred : pred_set)
	                    {
	                    	int num_in = getInSize(curNode, pred);
	                    	for (int i = 0; i < num_in; i++)
	                    	{
	                    		int adj = getInVertID(curNode, pred, i);
		                        if (l2.find(adj) == l2.end())
		                        {
		                            l2[adj] = level+1;
		                            if (l1.find(adj) != l1.end() && l1[adj] + l2[adj] <= k)
		                                return true;
		                            if (f1.find(adj) != f1.end() && f1[adj] + l2[adj] <= k)
		                                return true;
		                            residue2[adj] = (1-alpha)*residue1[curNode]/getSetInSize(curNode, pred_set);	// Should be equivalent to +=
		                            if(residue2[adj]/getSetInSize(adj, pred_set) >= rmax_fwd )
		                                candidatePush2.push( pair<int,int>(adj,level+1) );
		                            else
		                                candidatePush2_next.push( pair<int,int>(adj,level+1) );
		                        }
	                    	}
	                    }
	                    residue2[curNode] = 0;
	                }
	                else
	                    candidatePush2_next.push( pair<int,int>(curNode,level) );
                }
                else
                {
	                if( residue2[curNode] / (getSetInSize(curNode, pred_set) + getSetOutSize(curNode, pred_set)) \
	                	>= rmax_bwd )
	                {
	                    for (int pred : pred_set)
	                    {
	                    	int num_in = getInSize(curNode, pred);
	                    	for (int i = 0; i < num_in; i++)
	                    	{
	                    		int adj = getInVertID(curNode, pred, i);
		                        if (l2.find(adj) == l2.end())
		                        {
		                            l2[adj] = level+1;
		                            if (l1.find(adj) != l1.end() && l1[adj] + l2[adj] <= k)
		                                return true;
		                            if (f1.find(adj) != f1.end() && f1[adj] + l2[adj] <= k)
		                                return true;
		                            residue2[adj] = (1-alpha)*residue1[curNode] / \
		                            	(getSetInSize(curNode, pred_set) + getSetOutSize(curNode, pred_set));	// Should be equivalent to +=
		                            if(residue2[adj] / (getSetInSize(adj, pred_set) + getSetOutSize(adj, pred_set)) >= rmax_fwd )
		                                candidatePush2.push( pair<int,int>(adj,level+1) );
		                            else
		                                candidatePush2_next.push( pair<int,int>(adj,level+1) );
		                        }
	                    	}
	                    	
	                    	int num_out = getOutSize(curNode, pred);
	                    	for (int i = 0; i < num_out; i++)
	                    	{
	                    		int adj = getOutVertID(curNode, pred, i);
		                        if (l2.find(adj) == l2.end())
		                        {
		                            l2[adj] = level+1;
		                            if (l1.find(adj) != l1.end() && l1[adj] + l2[adj] <= k)
		                                return true;
		                            if (f1.find(adj) != f1.end() && f1[adj] + l2[adj] <= k)
		                                return true;
		                            residue2[adj] = (1-alpha)*residue1[curNode] / \
		                            	(getSetInSize(curNode, pred_set) + getSetOutSize(curNode, pred_set));	// Should be equivalent to +=
		                            if(residue2[adj] / (getSetInSize(adj, pred_set) + getSetOutSize(adj, pred_set)) >= rmax_fwd )
		                                candidatePush2.push( pair<int,int>(adj,level+1) );
		                            else
		                                candidatePush2_next.push( pair<int,int>(adj,level+1) );
		                        }
	                    	}
	                    }
	                    residue2[curNode] = 0;
	                }
	                else
	                    candidatePush2_next.push( pair<int,int>(curNode,level) );
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
            		if(f2.find(adj) == f2.end())
	                {
	                    f2[adj] = fullL2;
	                    if (l1.find(adj) != l1.end() && l1[adj] + f2[adj] <= k)
	                        return true;
	                    if (f1.find(adj) != f1.end() && f1[adj] + f2[adj] <= k)
	                        return true;
	                    fwdQ_next.push(adj);
	                }
            	}

            	if(directed) continue;

            	int num_out = getOutSize(curNode, pred);
            	for (int i = 0; i < num_out; i++)
            	{
            		int adj = getOutVertID(curNode, pred, i);
            		if(f2.find(adj) == f2.end())
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
		if(!directed)
		{
			uOutTotal += getInSize(uid, pred);
			vInTotal += getOutSize(vid, pred);
		}
	}
	if (uOutTotal == 0 || vInTotal == 0)
		return false;

	int level = 0;
	queue< int > fwdQ;
    queue< int > fwdQ_next;
    queue< int > bwdQ;
    queue< int > bwdQ_next;
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
	    		if(directed) continue;

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
	    				if (fSetMark.find(inNode) != \
	    					fSetMark.end() && fSetMark[inNode] + bSetMark[inNode] <= k)
	    					return true;
	    			}
	    		}

	    		if(directed) continue;
	    		int num_out = getOutSize(curNode, pred);
	    		for (int i = 0; i < num_out; i++)
	    		{
	    			int outNode = getOutVertID(curNode, pred, i);
	    			if (bSetMark.find(outNode) == bSetMark.end())
	    			{
	    				bwdQ_next.push(outNode);
	    				bSetMark[outNode] = level;
	    				if (fSetMark.find(outNode) != \
	    					fSetMark.end() && fSetMark[outNode] + bSetMark[outNode] <= k)
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
	cout<<"ret.size:"<<ret.size()<<",k:"<<k<<endl;
	if ((ret.size() - 1) / 2 > k)
		ret.clear();
	return ret;
}

// retNum is the number of top nodes to return; k is the hop constraint -- don't mix them up!
void PathQueryHandler::SSPPR(int uid, int retNum, int k, const vector<int> &pred_set, vector< pair<int ,double> > &topkV2ppr)
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
	double pfail = 1.0 / getVertNum() / getVertNum() / log(getVertNum());	// log(1/pfail) -> log(1*n/pfail)
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

    fwd_idx.first.clean();  //reserve
    fwd_idx.second.clean();  //residual
    fwd_idx.second.insert( uid, rsum );

    double zero_ppr_upper_bound = 1.0;
    upper_bounds.reset_one_values();
    lower_bounds.reset_zero_values();

    // fora_query_topk_with_bound
    // for delta: try value from 1/4 to 1/n
    int iteration = 0;
    while( delta >= min_delta ){
        rmax = epsilon * sqrt(delta / 3 / numPredEdges / log(2 / pfail));
        rmax *= rmax_scale;
	    omega = (2 + epsilon) * log(2 / pfail) / delta / epsilon / epsilon;
        
        if (getSetOutSize(uid, pred_set) == 0)
        {
            rsum = 0.0;
            fwd_idx.first.insert(uid, 1);
            compute_ppr_with_reserve(fwd_idx, v2ppr);
            return;
        }
        else
            forward_local_update_linear_topk( uid, rsum, rmax, lowest_delta_rmax, forward_from, fwd_idx, pred_set, alpha, k ); //forward propagation, obtain reserve and residual

        compute_ppr_with_fwdidx_topk_with_bound(rsum, fwd_idx, v2ppr, delta, alpha, threshold, \
        	pred_set, pfail, zero_ppr_upper_bound, omega, upper_bounds, lower_bounds);
        if(if_stop(retNum, delta, threshold, epsilon, topk_filter, upper_bounds, lower_bounds, v2ppr) \
        	|| delta <= min_delta)
            break;
        else
            delta = max( min_delta, delta / 2.0 );  // otherwise, reduce delta to delta/2
    }

    // Extract top-k results
    topkV2ppr.clear();
    topkV2ppr.resize(retNum);
    partial_sort_copy(v2ppr.begin(), v2ppr.end(), topkV2ppr.begin(), topkV2ppr.end(), 
        [](pair<int, double> const& l, pair<int, double> const& r){return l.second > r.second;});
    size_t i = topkV2ppr.size() - 1;
    while (topkV2ppr[i].second == 0)
    	i--;
    topkV2ppr.erase(topkV2ppr.begin() + i + 1, topkV2ppr.end());	// Get rid of ppr = 0 entries
}

void PathQueryHandler::compute_ppr_with_reserve(pair<iMap<double>, iMap<double>> &fwd_idx, unordered_map<int, double> &v2ppr)
{
    int node_id;
    double reserve;
    for(long i=0; i< fwd_idx.first.occur.m_num; i++){
        node_id = fwd_idx.first.occur[i];
        reserve = fwd_idx.first[ node_id ];
        if(reserve)
        	v2ppr[node_id] = reserve;
    }
}

void PathQueryHandler::forward_local_update_linear_topk(int s, double& rsum, double rmax, double lowest_rmax, vector<pair<int, int>>& forward_from, \
	pair<iMap<double>, iMap<double>> &fwd_idx, const vector<int> &pred_set, double alpha, int k)
{
    double myeps = rmax;

    vector<bool> in_forward(getVertNum());
    vector<bool> in_next_forward(getVertNum());

    std::fill(in_forward.begin(), in_forward.end(), false);
    std::fill(in_next_forward.begin(), in_next_forward.end(), false);

    vector<pair<int, int>> next_forward_from;
    next_forward_from.reserve(getVertNum());
    for(auto &v: forward_from)
        in_forward[v.first] = true;

    unsigned long i=0;
    while( i < forward_from.size() )
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
            if(!fwd_idx.first.exist(v))
                fwd_idx.first.insert( v, v_residue * alpha );
            else
                fwd_idx.first[v] += v_residue * alpha;

            rsum -= v_residue * alpha;
            if(out_neighbor == 0)
            {
                fwd_idx.second[s] += v_residue * (1-alpha);
                if(getSetOutSize(s, pred_set) > 0 && in_forward[s] != true && fwd_idx.second[s] / getSetOutSize(s, pred_set) >= myeps)
                {
                    // forward_from.push_back(make_pair(s, level + 1));
                    forward_from.push_back(make_pair(s, 0));
                    in_forward[s] = true;
                }
                else if(getSetOutSize(s, pred_set) >= 0 && in_next_forward[s] != true && fwd_idx.second[s] / getSetOutSize(s, pred_set) >= lowest_rmax)
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
            			exit(0);	// TODO: throw an exception
            		}

            		if(!fwd_idx.second.exist(next))
	                    fwd_idx.second.insert(next, avg_push_residual);
	                else
	                    fwd_idx.second[next] += avg_push_residual;

	                if( in_forward[next] != true && fwd_idx.second[next] / getSetOutSize(next, pred_set) >= myeps \
	                	&& (k == -1 || level < k))
	                {
	                    forward_from.push_back(make_pair(next, level + 1));
	                    in_forward[next] = true;
	                }
	                else
	                {
	                    if(in_next_forward[next] != true && fwd_idx.second[next] / getSetOutSize(next, pred_set) >= lowest_rmax \
	                    	&& (k == -1 || level < k))
	                    {
	                        next_forward_from.push_back(make_pair(next, level + 1));
	                        in_next_forward[next] = true;
	                    }
	                }
            	}
            }
        }
        else{
            if(in_next_forward[v] != true && fwd_idx.second[v] / getSetOutSize(v, pred_set) >= lowest_rmax){
                next_forward_from.push_back(make_pair(v, level));
                in_next_forward[v] = true;
            }
        }
    }
    
    forward_from = next_forward_from;
}

void PathQueryHandler::compute_ppr_with_fwdidx_topk_with_bound(double check_rsum, pair<iMap<double>, iMap<double>> &fwd_idx, \
	unordered_map<int, double> &v2ppr, double delta, double alpha, double threshold, \
	const vector<int> &pred_set, double pfail, double &zero_ppr_upper_bound, double omega, \
	iMap<double> &upper_bounds, iMap<double> &lower_bounds)
{
    compute_ppr_with_reserve(fwd_idx, v2ppr);

    if(check_rsum == 0.0)
        return;

    long num_random_walk = omega * check_rsum;
    long real_num_rand_walk = 0;

    for(long i=0; i < fwd_idx.second.occur.m_num; i++)
    {
        int source = fwd_idx.second.occur[i];
        double residual = fwd_idx.second[source];
        long num_s_rw = ceil(residual / check_rsum * num_random_walk);
        double a_s = residual / check_rsum * num_random_walk / num_s_rw;

        real_num_rand_walk += num_s_rw;

        double ppr_incre = a_s * check_rsum / num_random_walk;
        for(long j = 0; j < num_s_rw; j++){
            int des = random_walk(source, alpha, pred_set);
            if (v2ppr.find(des) == v2ppr.end())
            	v2ppr[des] = ppr_incre;
            else
                v2ppr[des] += ppr_incre;

        }
    }

    if(delta < threshold)
        set_ppr_bounds(fwd_idx, check_rsum, real_num_rand_walk, v2ppr, pfail, zero_ppr_upper_bound, \
        	upper_bounds, lower_bounds);
    else
        zero_ppr_upper_bound = calculate_lambda(check_rsum,  pfail, zero_ppr_upper_bound, real_num_rand_walk);
}

void PathQueryHandler::set_ppr_bounds(pair<iMap<double>, iMap<double>> &fwd_idx, double rsum, \
	long total_rw_num, unordered_map<int, double> &v2ppr, double pfail, double &zero_ppr_upper_bound, \
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
    zero_ppr_upper_bound = calculate_lambda( rsum,  pfail,  zero_ppr_upper_bound,  total_rw_num );
    
    for (auto it = v2ppr.begin(); it != v2ppr.end(); ++it)
    {
        nodeid = it->first;
        if(v2ppr[nodeid] <= 0)
            continue;
        double reserve=0.0;
        if(fwd_idx.first.exist(nodeid))
            reserve = fwd_idx.first[nodeid];
        double epsilon_a = 1.0;
        if( upper_bounds.exist(nodeid)  )
        {
            assert(upper_bounds[nodeid]>0.0);
            if(upper_bounds[nodeid] > reserve)
                epsilon_a = calculate_lambda( rsum, pfail, upper_bounds[nodeid] - reserve, total_rw_num);
            else
                epsilon_a = calculate_lambda( rsum, pfail, 1 - reserve, total_rw_num);
        }
        else
            epsilon_a = calculate_lambda( rsum, pfail, 1.0-reserve, total_rw_num);

        ub_eps_a = v2ppr[nodeid]+epsilon_a;
        lb_eps_a = v2ppr[nodeid]-epsilon_a;
        if(!(lb_eps_a > 0))
            lb_eps_a = 0;

        double epsilon_v = default_epsilon_v;
        if(fwd_idx.first.exist(nodeid) && fwd_idx.first[nodeid] > min_ppr)
        {
            if(lower_bounds.exist(nodeid))
                reserve = max(reserve, lower_bounds[nodeid]);
            epsilon_v = epsilon_v_div / sqrt(reserve);
        }
        else
        {
            if(lower_bounds[nodeid] > 0)
                epsilon_v = epsilon_v_div / sqrt(lower_bounds[nodeid]);
        }


        ub_eps_v = 1.0;
        lb_eps_v = 0.0;
        if(1.0 - epsilon_v > 0)
        {
            ub_eps_v = v2ppr[nodeid] / (1.0 - epsilon_v);
            lb_eps_v = v2ppr[nodeid] / (1.0 + epsilon_v);
        }

        up_bound = min( min(ub_eps_a, ub_eps_v), 1.0 );
        low_bound = max( max(lb_eps_a, lb_eps_v), reserve );
        if(up_bound>0)
        {
            if(!upper_bounds.exist(nodeid))
                upper_bounds.insert(nodeid, up_bound);
            else
                upper_bounds[nodeid] = up_bound;
        }
        
        if(low_bound>=0)
        {
            if(!lower_bounds.exist(nodeid))
                lower_bounds.insert(nodeid, low_bound);
            else
                lower_bounds[nodeid] = low_bound;
        }
    }
}

inline double PathQueryHandler::calculate_lambda(double rsum, double pfail, double upper_bound, long total_rw_num)
{
    return 1.0/3*log(2/pfail)*rsum/total_rw_num + 
    sqrt(4.0/9.0*log(2.0/pfail)*log(2.0/pfail)*rsum*rsum +
        8*total_rw_num*log(2.0/pfail)*rsum*upper_bound)
    /2.0/total_rw_num;
}

inline int PathQueryHandler::random_walk(int start, double alpha, const vector<int> &pred_set)
{
    int cur = start;
    unsigned long k;
    if(getSetOutSize(start, pred_set) == 0)
        return start;
    while (true)
    {
        if ((double)rand() / (double)RAND_MAX <= alpha)	// drand, return bool, bernoulli by alpha
            return cur;
        if (getSetOutSize(cur, pred_set))
        {
            k = rand() % getSetOutSize(cur, pred_set);	// lrand
        	unsigned long curr_idx = k;
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
            			exit(0);	// TODO: throw an exception
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
    nth_element(temp_ppr.begin(), temp_ppr.begin() + retNum - 1, temp_ppr.end(), \
    	[](double x, double y){return x > y;});
    return temp_ppr[retNum - 1];
}

bool PathQueryHandler::if_stop(int retNum, double delta, double threshold, double epsilon, iMap<int> &topk_filter, \
	iMap<double> &upper_bounds, iMap<double> &lower_bounds, unordered_map<int, double> &v2ppr)
{
    if(kth_ppr(v2ppr, retNum) >= 2.0 * delta)
        return true;

    if(delta>=threshold) return false;
    
    const static double error = 1.0 + epsilon;
    const static double error_2 = 1.0 + epsilon;

    vector< pair<int ,double> > topk_pprs;
    topk_pprs.clear();
    topk_pprs.resize(retNum);
    topk_filter.clean();

    static vector< pair<int, double> > temp_bounds;
    temp_bounds.clear();
    temp_bounds.resize(lower_bounds.occur.m_num);
    int nodeid;
    for(int i=0; i<lower_bounds.occur.m_num; i++){
        nodeid = lower_bounds.occur[i];
        temp_bounds[i] = make_pair( nodeid, lower_bounds[nodeid] );
    }

    //sort topk nodes by lower bound
    partial_sort_copy(temp_bounds.begin(), temp_bounds.end(), topk_pprs.begin(), topk_pprs.end(), 
        [](pair<int, double> const& l, pair<int, double> const& r){return l.second > r.second;});
    
    //for topk nodes, upper-bound/low-bound <= 1+epsilon
    double ratio=0.0;
    double largest_ratio=0.0;
    for(auto &node : topk_pprs){
        topk_filter.insert(node.first, 1);
        ratio = upper_bounds[node.first] / lower_bounds[node.first];
        if(ratio > largest_ratio)
            largest_ratio = ratio;
        if(ratio > error_2){
            return false;
        }
    }

    //for remaining NO. retNum+1 to NO. n nodes, low-bound of retNum > the max upper-bound of remaining nodes 
    double low_bound_k = topk_pprs[retNum - 1].second;
    if(low_bound_k <= delta)
        return false;

    for(int i=0; i<upper_bounds.occur.m_num; i++)
    {
        nodeid = upper_bounds.occur[i];
        if(topk_filter.exist(nodeid) || v2ppr[nodeid] <= 0)
            continue;

        double upper_temp = upper_bounds[nodeid];
        double lower_temp = lower_bounds[nodeid];
        if(upper_temp > low_bound_k * error)
        {
             if(upper_temp > (1 + epsilon) / (1 - epsilon) * lower_temp)
                 continue;
             else
                 return false;
        }
        else
            continue;
    }

    return true;
}

vector<pair<pair<int, int>, int>> kHopSubgraph(int uid, int vid, bool directed, int k, const std::vector<int> &pred_set)
{
	return vector<pair<pair<int, int>, int>>();
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
	Return all paths from u to v within k hops via edges labeled by labels
	in pred_set.

	@param uid the vertex u's ID.
	@param vid the vertex v's ID.
	@param directed if false, treat all edges in the graph as bidirectional.
	@param k the hop constraint.
	@param pred_set the set of edge labels allowed.
	@return a vector of all paths from u to v; the format of each path is the same as in shortestPath.
*/
vector<vector<int>> PathQueryHandler::kHopEnumeratePath(int uid, int vid, bool directed, int k, const std::vector<int> &pred_set)
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
	bc_dfs(uid, vid, directed, k, pred_set, s, bar, ret);
	cout << "ret.size() = " << ret.size() << endl;
	return ret;
}

int PathQueryHandler::bc_dfs(int uid, int vid, bool directed, int k, const std::vector<int> &pred_set, vector<pair<int, int>> &s, vector<int> &bar, vector<vector<int>> &paths)
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
					s.push_back(make_pair(to, pred));
					int next_f = bc_dfs(to, vid, directed, k, pred_set, s, bar, paths);
					if (next_f != k + 1 && f < next_f + 1)
						f = next_f + 1;
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
					s.push_back(make_pair(to, pred));
					int next_f = bc_dfs(to, vid, directed, k, pred_set, s, bar, paths);
					if (next_f != k + 1 && f < next_f + 1)
						f = next_f + 1;
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