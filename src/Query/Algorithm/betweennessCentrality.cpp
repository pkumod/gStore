#include "PathQueryHandler.h"

using namespace std;

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
			num = csrHandler->getOutSize(temp_u, x);
			for (int j = 0; j < num; ++j)
			{
				int t = csrHandler->getOutVertID(temp_u, x, j);
				nl.insert(t);
			}
			if (directed)
				continue;
			num = csrHandler->getInSize(temp_u, x);
			for (int j = 0; j < num; ++j)
			{
				int t = csrHandler->getInVertID(temp_u, x, j);
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
			int num = csrHandler->getOutSize(temp_u, x);
			for (int j = 0; j < num; ++j)
			{
				int t = csrHandler->getOutVertID(temp_u, x, j);
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
			num = csrHandler->getInSize(temp_u, x);
			for (int j = 0; j < num; ++j)
			{
				int t = csrHandler->getInVertID(temp_u, x, j);
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
					int num = csrHandler->getOutSize(temp_u, x);
					for (int j = 0; j < num; ++j)
					{
						if (csrHandler->getOutVertID(temp_u, x, j) == temp_v)
						{
							pred_l.insert(x);   
							break;
						}
					}
					if (directed)
						continue;
					num = csrHandler->getInSize(temp_u, x);
					for (int j = 0; j < num; ++j)
					{
						if (csrHandler->getInVertID(temp_u, x, j) == temp_v)
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

double PathQueryHandler::betweennessCentrality(int id, bool directed, const std::vector<int> &pred_sets)
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