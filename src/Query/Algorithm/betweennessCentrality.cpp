#include "PathQueryHandler.h"

using namespace std;

double PathQueryHandler::betweennessCentrality(int id, bool directed, const std::vector<int> &pred_sets)
{
    double ret = 0;
    std::cout << "betweennessCentrality 节点v的介数中心度:" << id << std::endl;
    std::cout << "betweennessCentrality 方向:" << directed << std::endl;
    /**
     * @brief 实现介度中心度算法
     * 
     */
    int predNum = pred_sets.size();
    if (predNum == 0) {
        predNum = getEdgeNum();
    }
    // 将所有与允许出现的谓词相关的顶点收集起来
    std::set<int> vertSets; 
    for (int i = 0; i < predNum; i++) {
        int predVertNum = csr->id2vid[i].size();
        for (int j = 0; j < predVertNum; j++) {
            if (csr->id2vid[i][j] == id) continue;
            vertSets.insert(csr->id2vid[i][j]);
        }
    }
    int vertArray[vertSets.size()]; 
    int k = 0;
    for (auto ii : vertSets) {
        vertArray[k] = ii;
        k++;
    }
    // cout << "测试"<<endl;
    // for (k = 0; k < vertSets.size(); k++) {
    //     cout << vertArray[k] << " "; 
    // }
    for (int i = 0; i < vertSets.size(); i++) {
        int j = 0;
        if (!directed) {
            // 无向
            j = i + 1;
        }
        for (; j < vertSets.size(); j++) {
            if (i == j) continue;
            int crossID = 0;
            int totalPathNum = shortPathCountForBetweennessCentrality(vertArray[i], vertArray[j], directed, pred_sets);
            if (totalPathNum == 0) continue;;
            int firstPathNum = shortPathCountForBetweennessCentrality(vertArray[i], id, directed, pred_sets);
            int secondPathNum = shortPathCountForBetweennessCentrality(id, vertArray[j], directed, pred_sets);
            // ret += (firstPathNum * secondPathNum) / totalPathNum;
            vector<int> minPath =  shortestPath(vertArray[i], vertArray[j], directed, pred_sets);
            int minPathPred = (minPath.size() - 1) / 2;
            int firstPathPred = (shortestPath(vertArray[i], id, directed, pred_sets).size() - 1) / 2;
            int secondPathPred = (shortestPath(id, vertArray[j], directed, pred_sets).size() - 1) / 2;
            if (minPathPred == (firstPathPred + secondPathPred)) {
                crossID = firstPathNum * secondPathNum;
            }
            // cout << "起点：" << i << " 终点:" << j << endl;
            // cout << totalPathNum << " " << firstPathNum << " " << secondPathNum << endl;
            // cout << minPathPred << " " << firstPathPred << " " << secondPathPred << endl;
            // cout << "总路径：" << totalPathNum << "有效路径：" << crossID << endl; 
            ret += (crossID * 1.0) / (totalPathNum * 1.0);
        }
    }
    return ret;
}

int PathQueryHandler::shortPathCountForBetweennessCentrality(int uid, int vid, bool directed, const std::vector<int> &pred_set) {
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
				for (int pred : pred_set)
				{
                    set<int> attendVertSet;
                    for (int d = directed; d <= 1; ++d)
                    {
                        if (d)
						    sz = getOutSize(v, pred);
						else
						    sz = getInSize(v, pred);
                        
						for (int j = 0; j < sz; j++)
						{
							if (d) {
                                n = getOutVertID(v, pred, j);
                                // 如果该结点已经出现过，则直接跳过当前结点
                                if (attendVertSet.find(n) != attendVertSet.end()) {
                                    continue;
                                }
                            }
							else {
                                n = getInVertID(v, pred, j);
                                // v-pred->到的所有结点记录下来
                                attendVertSet.insert(n);
                            }
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
				for (int pred : pred_set)
				{
                    set<int> attendVertSet;
                    for (int d = directed; d <= 1; ++d) 
                    {
                        if (d)
							sz = getInSize(v, pred);
						else
							sz = getOutSize(v, pred);
						for (int j = 0; j < sz; j++)
						{
							if (d) {
                                n = getInVertID(v, pred, j);
                                if (attendVertSet.find(n) != attendVertSet.end()) continue;
                            }
							else {
                                n = getOutVertID(v, pred, j);
                                attendVertSet.insert(n);
                            }
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

std::set<int> PathQueryHandler::getAllNeighborsById(int uid, bool directed, const std::vector<int> &pred_set)
{
	std::set<int> visited;
	std::queue<int> q_l;
	int num_of_pred = pred_set.size();
	q_l.push(uid);
	visited.insert(uid);

	while (!q_l.empty())
	{
		int temp_u = q_l.front();
		q_l.pop();
		std::set<int> nl;
		int num  = 0;
		for (int i = 0; i < num_of_pred; ++i)
		{
			int x = pred_set[i];
			num = getOutSize(temp_u, x);
			for (int j = 0; j < num; ++j)
			{
				int t = getOutVertID(temp_u, x, j);
				nl.insert(t);
			}
			if (directed)
				continue;
			num = getInSize(temp_u, x);
			for (int j = 0; j < num; ++j)
			{
				int t = getInVertID(temp_u, x, j);
				nl.insert(t);
			}
		}
		for (auto m : nl)
		{
			if (visited.find(m) == visited.end())
			{
				visited.insert(m);
				q_l.push(m);
			}
		}
	}
	visited.erase(uid);
	return visited;
}

void PathQueryHandler::betweennessCentrality_dfs(int uid, int vid, std::vector<int>& temp, std::map<int, std::vector<int>>& dis_l, std::vector<std::vector<int>>& path_l, int passid, bool& pass)
{
	if (uid == vid)
	{
		temp.push_back(vid);
        std::vector<int> path;
		for (int i = temp.size()-1; i >= 0; i--)
        {
            path.push_back(temp[i]);
        }
        path_l.push_back(path);
		temp.pop_back();
		return;
	}
	if (vid == passid)
		pass = true;
	temp.push_back(vid);
	for (int i = 0; i < dis_l[vid].size(); ++i)
    {
        betweennessCentrality_dfs(uid, dis_l[vid][i], temp, dis_l, path_l, passid, pass);
    }
	temp.pop_back();
}

double PathQueryHandler::shortestPathScalePassId(int uid, int passid, bool directed, const std::vector<int> &pred_set)
{
	if (uid == passid)
		return 0;

	std::set<int> visited;
	std::queue<int> q_l;
	int num_of_pred = pred_set.size();
	std::map<int, int> dis_u;
	std::map<int, std::vector<int>> dis_pre;
	q_l.push(uid);
	dis_u[uid] = 0;
	visited.insert(uid);

	while (!q_l.empty())
	{
		int temp_u = q_l.front();
		q_l.pop();
		int distance = dis_u[temp_u] + 1;
		std::set<int> nl;
		for (int i = 0; i < num_of_pred; ++i)
		{
			int x = pred_set[i];
			int num = getOutSize(temp_u, x);
			for (int j = 0; j < num; ++j)
			{
				int t = getOutVertID(temp_u, x, j);
				// if vertex_set node not find, give up this node
				nl.insert(t);
				auto vit = dis_u.find(t);
				if (vit == dis_u.end())
				{
					dis_u[t] = distance;
					dis_pre[t].push_back(temp_u);
				}
				else if (dis_u[t] > distance)
				{
					dis_u[t] = distance;
					dis_pre[t].clear();
					dis_pre[t].push_back(temp_u);
				}
				else if (dis_u[t] == distance)
				{
					auto it = std::find(dis_pre[t].begin(), dis_pre[t].end(), temp_u);
					if (it == dis_pre[t].end())
						dis_pre[t].push_back(temp_u);	
				}
			}

			if (directed)
				continue;
			num = getInSize(temp_u, x);
			for (int j = 0; j < num; ++j)
			{
				int t = getInVertID(temp_u, x, j);
				// if vertex_set node not find, give up this node
			   	nl.insert(t);
				auto vit = dis_u.find(t);
				if (vit == dis_u.end())
				{
					dis_u[t] = distance;
					dis_pre[t].push_back(temp_u);
				}
				else if (dis_u[t] > distance)
				{
					dis_u[t] = distance;
					dis_pre[t].clear();
					dis_pre[t].push_back(temp_u);
				}
				else if (dis_u[t] == distance)
				{
					auto it = std::find(dis_pre[t].begin(), dis_pre[t].end(), temp_u);
					if (it == dis_pre[t].end())
						dis_pre[t].push_back(temp_u);
				}
			}
		}
		for (auto m : nl)
		{
			if (visited.find(m) == visited.end())
			{
				visited.insert(m);
				q_l.push(m);
			}
		}
	}

	double ret = 0;
	for (auto m : dis_u)
	{
		int temp_uid = m.first;
		int distance = m.second;
		if (distance <= 1 || temp_uid == uid || temp_uid == passid)
			continue;
		std::vector<int> temp;
		std::vector<std::vector<int>> path_l;
		bool pass = false;
		betweennessCentrality_dfs(uid, temp_uid, temp, dis_pre, path_l, passid, pass);
		if (!pass)
			continue;
		int pass_num = 0;
		int total_num = 0;
		for (auto& path: path_l)
		{
			int path_num = 1;
			std::set<int> pred_l;
			bool is_pass = false;
			for (int i=0; i<path.size(); i++)
			{
				if (path[i] == temp_uid || i == path.size()-1)
					break;
				pred_l.clear();
				int temp_u = path[i];
				int temp_v = path[i+1];
				for (int i = 0; i < num_of_pred; ++i)
				{
					int x = pred_set[i];
					int num = getOutSize(temp_u, x);
					for (int j = 0; j < num; ++j)
					{
						if (getOutVertID(temp_u, x, j) == temp_v)
						{
							pred_l.insert(x);   
							break;
						}
					}
					if (directed)
						continue;
					num = getInSize(temp_u, x);
					for (int j = 0; j < num; ++j)
					{
						if (getInVertID(temp_u, x, j) == temp_v)
						{
							pred_l.insert(x);   
							break;
						}
					}
				}

				if (temp_v == passid)
					is_pass = true;

				path_num *= pred_l.size();
			}
			if (is_pass)
				pass_num += path_num;
			total_num += path_num;
		}

		if (pass_num == 0 || total_num == 0)
			continue;;

		// std::cout << "shortestPathScalePassId:" << uid  << "  ret:" << pass_num << "/" << total_num << std::endl;	
		ret += (double)pass_num / (double)total_num;
	}

    return ret;
}

double PathQueryHandler::betweennessCentrality2(int id, bool directed, const std::vector<int> &pred_sets)
{
	std::cout << "betweennessCentrality 允许出现的谓词数量:" << pred_sets.size() << std::endl;
	if (id < 0 || pred_sets.empty())
	{
		std::cout << "betweennessCentrality  param error id:" <<  id << "  pred_sets size::" << pred_sets.size() << std::endl;
		return 0;
	}
	double ret = 0;
	std::set<int> vertex = getAllNeighborsById(id, false, pred_sets);
	if (vertex.size() <= 1)
		return 0;
	std::cout << "betweennessCentrality id:" <<  id << "  vertex size:" << vertex.size() << std::endl;

	for (int uid : vertex)
	{
		if (uid < 0 || uid == id)
			continue;
		// std::cout << "betweennessCentrality uid:" << uid << std::endl;
		ret += shortestPathScalePassId(uid, id, directed, pred_sets);
	}

	if (!directed)
		ret /= (double)2;

	return ret;
}