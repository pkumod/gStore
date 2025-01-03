#include "PathQueryHandler.h"

using namespace std;

std::vector<int> PathQueryHandler::kHopAllNeighbors(int uid, bool direction, int k, const std::vector<int> &pred_set, int retNum)
{
    std::set<int> visited;
	std::queue<int> q_l;
	int num_of_pred = pred_set.size();
	std::map<int, int> dis_u;
	q_l.push(uid);
	dis_u[uid] = 0;
	std::set<int> nei_node;
	visited.insert(uid);

	while (!q_l.empty())
	{
		int temp_u = q_l.front();
		q_l.pop();
		std::set<int> nl;
		int distance = dis_u[temp_u] + 1;
		int num  = 0;
		for (int i = 0; i < num_of_pred; ++i)
		{
			int x = pred_set[i];
            num = csrHandler->getOutSize(temp_u, x);
            for (int j = 0; j < num; ++j)
            {
                int t = csrHandler->getOutVertID(temp_u, x, j);
                nl.insert(t);
                auto vit = dis_u.find(t);
                if (vit == dis_u.end())
                {
                    dis_u[t] = distance;
                }
                else if (dis_u[t] > distance)
                {
                    //Record minimum distance
                    dis_u[t] = distance;
                }
            }
			if (!direction)
			{
				num = csrHandler->getInSize(temp_u, x);
				for (int j = 0; j < num; ++j)
				{
					int t = csrHandler->getInVertID(temp_u, x, j);
					nl.insert(t);
					auto vit = dis_u.find(t);
					if (vit == dis_u.end())
					{
						dis_u[t] = dis_u[temp_u] + 1;
					}
					else if (dis_u[t] > dis_u[temp_u] + 1)
					{
						//Record minimum distance
						dis_u[t] = dis_u[temp_u] + 1;
					}
				}
			}
		}
		if (nei_node.size() >= retNum)
			break;
		// std::cout << "temp:" << temp_u << "   dis:"  << dis_u[temp_u] << "   size:"<< nl.size() << "  num_of_pred" << num_of_pred << "   skip_degree:" << skip_degree << "   degree:" << degree << std::endl;
		if (dis_u[temp_u] <= k)
		{
			nei_node.insert(temp_u);
			for (auto m : nl)
			{
				// Traversing the largest neighbor node is degree!!!
				if (visited.find(m) == visited.end() && dis_u[m] <= k)
				{
					// std::cout << "m:" << m << "   dis:" << dis_u[m]  << std::endl;
					visited.insert(m);
					q_l.push(m);
				}
			}
		}
	}

	std::vector<int> ret;
	for (auto&m : nei_node)
	{
		ret.push_back(m);
	}
    return ret;
}