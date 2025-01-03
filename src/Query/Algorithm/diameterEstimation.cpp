#include "PathQueryHandler.h"

using namespace std;

unsigned int PathQueryHandler::diameterEstimation(const std::vector<int> &pred_sets)
{
    std::cout << "diameterEstimation 允许出现的谓词数量:" << pred_sets.size() << std::endl;
    unsigned int ret = 0;
    int vertex_num = csrHandler->getVertNum();
    int num_of_pred = pred_sets.size();
    std::set<int> vertex;
    for (int temp_u = 0; temp_u < vertex_num; temp_u++)
    {
        for (int i = 0; i < num_of_pred; ++i)
        {
            int x = pred_sets[i];
            int num = csrHandler->getOutSize(temp_u, x);
            if (num > 0)
            {
                vertex.insert(temp_u);
                break;
            }
        }
    }
    std::cout << "diameterEstimation vertex size:" << vertex.size()<< std::endl;
    for (auto temp_u : vertex)
    {
        std::pair<int, int> diameter = diameterEstimationByuid(temp_u, pred_sets);
        if (diameter.first > ret)
            ret = diameter.first;
    }
    return ret;
}

std::pair<int, int> PathQueryHandler::diameterEstimationByuid(int uid, const std::vector<int> &pred_set)
{
    std::pair<int, int> ret(0, -1);
    if (uid < 0)
		return ret;

    std::set<int> visited;
	std::queue<int> q_l;
	int num_of_pred = pred_set.size();
	std::map<int, int> dis_u;
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
				}
				else if (dis_u[t] > distance)
				{
					dis_u[t] = distance;
				}

				if (ret.first < dis_u[t])
				{
					ret.first = dis_u[t];
					ret.second = t;
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

    return ret;
}