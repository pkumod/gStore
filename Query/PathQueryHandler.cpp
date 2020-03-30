#include "PathQueryHandler.h"

using namespace std;

PathQueryHandler::PathQueryHandler(CSR *_csr)
{
	csr = _csr;
}

// int PathQueryHandler::getVertNum()
// {

// }

int PathQueryHandler::getEdgeNum()
{
	int ret = 0;
	for (int i = 0; i < csr[1].pre_num; i++)
		ret += csr[1].adjacency_list[i].size();
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

int PathQueryHandler::getTotalInSize(int vid)
{
	int ret = 0;
	for (int i = 0; i < csr[1].pre_num; i++)
		ret += getInSize(vid, i);
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

int PathQueryHandler::getTotalOutSize(int vid)
{
	int ret = 0;
	for (int i = 0; i < csr[1].pre_num; i++)
		ret += getOutSize(vid, i);
	return ret;
}

void PathQueryHandler::inputGraph(string filename)
{
	ifstream infile(filename.c_str());

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
		ans1 = shortestPath(uid, vid, pred_set);
		ans2 = shortestPath(vid, uid, pred_set);
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
		int num_of_pred = pred_set.size();
		queue<int> q;
		q.push(uid);
		map<int, int> dis;
		dis[uid] = 0;
		bool finished = 0;
		while(!q.empty() && !finished)
		{
			int temp = q.front();
			q.pop();
			for(int i = 0; i < num_of_pred; ++i)
			{
				int x = pred_set[i];
				int num_out = getOutSize(temp, x);
				for(int j = 0; j < num_out; ++j)
				{
					int t = getOutVertID(temp, x, j); //get the node
					if(dis.find(t) != dis.end())
						continue;						
					q.push(t);
					dis[t] = dis[temp] + 1;
					if(t == vid)
					{
						finished = 1;
						break;
					}
				}
				if(finished) break;
				int num_in = getInSize(temp, x);
				for(int j = 0; j < num_in; ++j)
				{
					int t = getInVertID(temp, x, j);
					if(dis.find(t) != dis.end())
						continue;
					q.push(t);
					dis[t] = dis[temp] + 1;
					if(t == vid)
					{
						finished = 1;
						break;
					}
				}
				if(finished) break;
			}

		}
		if(!finished) return ans;
		stack<int> s;
		s.push(vid);
		while(1)
		{
			int temp = s.top();
			if(temp == uid) break;
			int flag0 = 0;
			for(int i = 0; i < num_of_pred; ++i)
			{
				int x = pred_set[i];
				int num_in = getInSize(temp, x);
				for(int j = 0; j < num_in; ++j)
				{
					int t = getInVertID(temp, x, j); 
					if(dis.find(t) != dis.end() && dis[t] == dis[temp] - 1)
					{
						s.push(x);
						s.push(t);
						flag0 = 1;
						break;
					}
				}
				if(flag0) break;
				int num_out = getOutSize(temp, x);
				for(int j = 0; j < num_out; ++j)
				{
					int t = getOutVertID(temp, x, j); 
					if(dis.find(t) != dis.end() && dis[t] == dis[temp] - 1)
					{
						s.push(x);
						s.push(t);
						flag0 = 1;
						break;
					}
				}
				if(flag0) break;
			}
		}
		while(!s.empty())
		{
			ans.push_back(s.top());
			s.pop();
		}
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
	@param pred_set the set of edge labels allowed.
	@return a vector of vertex IDs representing the shortest path, shaped like
	[v0, pred0, v1, pred1, ..., vk, predk, vk+1], where v0 = u, vk+1 = v, 
	and predi labels the edge from vi to vi+1.

	use the map route_u[q] to store the route from u to q.
	use the map route_v[q] to store the route from v to q.
**/

vector<int> PathQueryHandler::shortestPath0(int uid, int vid, const vector<int> &pred_set)//cost more space and more time?
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
	@param pred_set the set of edge labels allowed.
	@return a vector of vertex IDs representing the shortest path, shaped like
	[v0, pred0, v1, pred1, ..., vk, predk, vk+1], where v0 = u, vk+1 = v, 
	and predi labels the edge from vi to vi+1.

	cost more time.
	use dis_u, dis_v to record the distance from u,v to any node.
	use bbfs to get the meet node.
	get the route through searching the distance and meet node.
*/
vector<int> PathQueryHandler::shortestPath(int uid, int vid, const vector<int> &pred_set) //cost less space and less time.
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
	@param k the hop constraint.
	@param pred_set the set of edge labels allowed.
	@return true if v is reachable from u within k hops, false otherwise.
*/
bool PathQueryHandler::kHopReachable(int uid, int vid, int k, const std::vector<int> &pred_set)
{
	if (uid == vid)
		return true;
	int uOutTotal = 0, vInTotal = 0;
	for (int pred : pred_set)
	{
		uOutTotal += getOutSize(uid, pred);
		vInTotal += getInSize(vid, pred);
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
    		}
    	}
    	if (fwdQ_next.empty() || bwdQ_next.empty())
    		return false;
    	swap(fwdQ_next, fwdQ);
    	swap(bwdQ_next, bwdQ);
    }

    return false;
}
// bool PathQueryHandler::kHopReachable(int uid, int vid, int k, const std::vector<int> &pred_set)
// {
// 	if(uid == vid)
// 		return true;
// 	int uOutTotal = 0, vInTotal = 0;
// 	for (int pred : pred_set)
// 	{
// 		uOutTotal += getOutSize(uid, pred);
// 		vInTotal += getInSize(vid, pred);
// 	}
// 	if (uOutTotal == 0 || vInTotal == 0)
// 		return false;
	
//     // partial push
//     double c = 1;	// Parameter to set
//     double  rmax = 100.0 / sqrt((double)(getVertNum() * getEdgeNum())) / c;
//     double  init_rmax = 0.001;
//     queue< pair<int,int> > candidatePush1;
//     queue< pair<int,int> > candidatePush1_next;
//     queue< pair<int,int> > candidatePush2;
//     queue< pair<int,int> > candidatePush2_next;
//     candidatePush1.push( pair<int,int>(uid,0) );
//     candidatePush2.push( pair<int,int>(vid,0) );
//     unordered_map<int, double> residue1;
//     unordered_map<int, double> residue2;
//     residue1[sourceNode] = 1;
//     residue2[targetNode] = 1;
//     unordered_map<int, int> l1;
//     unordered_map<int, int> l2;
//     l1[sourceNode] = 0;
//     l2[targetNode] = 0;
//     int visitedm1 = 0;
//     int visitedm2 = 0;

//     // full push
//     queue< int > fwdQ;
//     queue< int > fwdQ_next;
//     queue< int > bwdQ;
//     queue< int > bwdQ_next;
//     int fullL1 = 0;
//     int fullL2 = 0;
//     fwdQ.push(sourceNode);
//     bwdQ.push(targetNode);
//     unordered_map<int, int> f1;
//     unordered_map<int, int> f2;
//     f1[sourceNode] = 0;
//     f2[targetNode] = 0;
//     int fullm1 = 0;
//     int fullm2 = 0;
//     int m1_next = outDeg[sourceNode];
//     int m2_next = inDeg[targetNode];

//     while( init_rmax > r_max )
//     {
//         while(!candidatePush1.empty())
//         {
//             int curNode = candidatePush1.front().first;
//             int level = candidatePush1.front().second;
//             candidatePush1.pop();
//             if( residue1[curNode] / getTotalOutSize(curNode) >= init_rmax )
//             {
//                 for (int pred : pred_set)
//                 {
//                 	for (int i = 0; i < getOutSize(curNode, pred); i++)
//                 	{
//                 		int adj = getOutVertID(curNode, pred, i);
//                 		visitedm1++;
//                 		auto success = l1.emplace(adj, level + 1);
//                 		if (success.second)
//                 		{
//                 			if (l2.find(adj) != l2.end() && l1[adj] + l2[adj] <= k)
//                 				return true;
//                 			if (f2.find(adj) != f2.end() && l1[adj] + f2[adj] <= k)
//                 				return true;
//                 			residue1[adj] += (1 - alpha) * residue1[curNode] / getTotalOutSize(curNode);
//                 			if (residue1[adj] / getTotalOutSize(adj) >= init_rmax)
// 	                            candidatePush1.push( pair<int,int>(adj,level+1) );
// 	                        else
// 	                            candidatePush1_next.push( pair<int,int>(adj,level+1) );
//                 		}
//                 	}
//                 }
//             }
//             else
//                 candidatePush1_next.push( pair<int,int>(curNode, level) );
//         }

//         if(candidatePush1_next.empty())
//         	return false;

//         while(!candidatePush2.empty())
//         {
//             int curNode = candidatePush2.front().first;
//             int level = candidatePush2.front().second;
//             candidatePush2.pop();
//             if( residue2[curNode] / getTotalInSize(curNode) >= init_rmax )
//             {
//                 for (int pred : pred_set)
//                 {
//                 	for (int i = 0; i < getInSize(curNode, pred); i++)
//                 	{
//                 		int adj = getInVertID(curNode, pred, i);
//                 		visitedm2++;
//                 		auto success = l2.emplace(adj, level + 1);
//                 		if (success.second)
//                 		{
//                 			if (l1.find(adj) != l1.end() && l1[adj] + l2[adj] <= k)
//                 				return true;
//                 			if (f1.find(adj) != f1.end() && f1[adj] + f2[adj] <= k)
//                 				return true;
//                 			residue2[adj] += (1 - alpha) * residue2[curNode] / getTotalOutSize(curNode);
//                 			if (residue2[adj] / getTotalOutSize(adj) >= init_rmax)
// 	                            candidatePush2.push( pair<int,int>(adj,level+1) );
// 	                        else
// 	                            candidatePush2_next.push( pair<int,int>(adj,level+1) );
//                 		}
//                 	}
//                 }
//             }
//             else
//                 candidatePush2_next.push( pair<int,int>(curNode, level) );
//         }

//         if(candidatePush2_next.empty())
//         	return false;

		
//         while( fullm1+m1_next < visitedm1 ){
//             fullL1++;
//             m1_next = 0;
//             while(!fwdQ.empty()){
//                 int curNode = fwdQ.front();
//                 fwdQ.pop();
//                 for( int adj : fwdG[curNode] ){
//                     fullm1++;
//                     if(f1[adj] == -1){
//                         f1[adj] = fullL1;
//                         if(l2[adj]!=-1) return (f1[adj]+l2[adj]);
//                         if(f2[adj]!=-1) return (f1[adj]+f2[adj]);
//                         // stopL = min(fullL1+pre1[adj],stopL);
//                         m1_next += outDeg[adj];
//                         fwdQ_next.push(adj);
//                     }
//                 }  
//             }
//             if( fwdQ_next.empty() ) return -1;
//             // if( fullL1+fullL2 >= stopL ){
//             //     // cout<<"success: "<<stopL<<endl;
//             //     return -1;
//             // } 
//             swap(fwdQ_next , fwdQ);

//         }

//         while( fullm2+m2_next < visitedm2 ){
//             fullL2++;
//             m2_next = 0;
//             while(!bwdQ.empty()){
//                 int curNode = bwdQ.front();
//                 bwdQ.pop();
//                 for( int adj : bwdG[curNode] ){
//                     fullm2++;
//                     if(f2[adj] == -1){
//                         f2[adj] = fullL2;
//                         if(l1[adj]!=-1) return (f2[adj]+l1[adj]);
//                         if(f1[adj]!=-1) return (f1[adj]+f2[adj]);
//                         // stopL = min(fullL2+pre2[adj],stopL);
//                         m2_next += inDeg[adj];
//                         bwdQ_next.push(adj);
//                     }
//                 }  
//             }
//             if( bwdQ_next.empty() ) return -1;
//             // if( fullL1+fullL2 >= stopL ){
//             //     // cout<<"success: "<<stopL<<endl;
//             //     return -1;
//             // } 
//             swap(bwdQ_next , bwdQ);

//         }

//         swap(candidatePush1,candidatePush1_next);
//         swap(candidatePush2,candidatePush2_next);

//         init_rmax /= 2;
//     }
//     return -1;
// }
