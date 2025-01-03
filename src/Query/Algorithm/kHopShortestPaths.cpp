#include "PathQueryHandler.h"

using namespace std;

void PathQueryHandler::djstl_dfs(int uid, int vid, std::vector<int>& temp, std::map<int, std::vector<int>>& dis_l, std::vector<std::vector<int>>& path_l)
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
	temp.push_back(vid);
	for (int i = 0; i < dis_l[vid].size(); ++i)
    {
        djstl_dfs(uid, dis_l[vid][i], temp, dis_l, path_l);
    }
	temp.pop_back();
}

std::vector<std::vector<int>> PathQueryHandler::kHopShortestPaths(int uid, int vid, bool directed, const std::vector<int>& pred_set)
{
    if (uid == vid)
		return std::vector<std::vector<int>>();
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

	std::vector<int> temp;
    std::vector<std::vector<int>> path_l;
	djstl_dfs(uid, vid, temp, dis_pre, path_l);

    std::vector<std::vector<int>> ans_l;
    for (auto& path: path_l)
    {
        std::vector<std::vector<int>> ans_path;
        ans_path.push_back({uid});
        std::set<int> pred_l;
        for (int i=0; i<path.size(); i++)
        {
            if (path[i] == vid || i == path.size()-1)
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

            std::vector<std::vector<int>> pre_path = ans_path;
            int pos = 0;
            for (auto pred : pred_l)
            {
                if (pos == 0)
                {
                    for (int j=0; j < ans_path.size(); j++)
                    {
                        ans_path[j].push_back(pred);
                        ans_path[j].push_back(temp_v);
                    }
                }
                else
                {
                    std::vector<std::vector<int>> pred_path = pre_path;
                    for (int j=0; j < pred_path.size(); j++)
                    {
                        pred_path[j].push_back(pred);
                        pred_path[j].push_back(temp_v);
                        ans_path.push_back(pred_path[j]);
                    }
                }
                pos++;
            }
        }
        ans_l.insert(ans_l.end(), ans_path.begin(), ans_path.end());
    }
    // for (auto m : ans_l)
    // {
	// 	std::cout << "pre pre pre path:" << std::endl;
    //     for ( auto mm : m)
    //         std::cout << "pre pre pre:" << mm << std::endl;
    // }

	// std::cout << "uid:" << uid << "  vid:" << vid << std::endl;
	// for (auto&m : dis_u)
	// {
	// 	std::cout << "m:" << m.first << "  dis:" << m.second << std::endl;
	// }

    return ans_l;
}