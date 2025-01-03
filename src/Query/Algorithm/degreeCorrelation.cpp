#include "PathQueryHandler.h"

using namespace std;

 double PathQueryHandler::degreeCorrelation(int uid, int k, const std::vector<int> &pred_sets)
 {
    std::cout << "degreeCorrelation 允许出现的谓词数量:" << pred_sets.size() << std::endl;
	if (uid < 0 || pred_sets.empty())
	{
		std::cout << "degreeCorrelation  param error id:" <<  uid << "  pred_sets size::" << pred_sets.size() << std::endl;
		return 0;
	}

    unordered_map<int, int> degree;
    std::set<int> visited;
	std::queue<int> q_l;
	int num_of_pred = pred_sets.size();
	std::map<int, int> dis_u;
	q_l.push(uid);
	dis_u[uid] = 0;
	visited.insert(uid);
	while (!q_l.empty())
	{
		int temp_u = q_l.front();
		q_l.pop();
		std::set<int> nl;
		int distance = dis_u[temp_u] + 1;
		int num  = 0;
        int degree_num = 0;
		for (int i = 0; i < num_of_pred; ++i)
		{
			int x = pred_sets[i];
            num = csrHandler->getOutSize(temp_u, x);
            degree_num += num;
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
            }
            num = csrHandler->getInSize(temp_u, x);
            degree_num += num;
            for (int j = 0; j < num; ++j)
            {
                int t = csrHandler->getInVertID(temp_u, x, j);
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
            }
		}
        degree[temp_u] += degree_num;
		if (dis_u[temp_u] <= k)
		{
			for (auto m : nl)
			{
				if (visited.find(m) == visited.end() && dis_u[m] <= k)
				{
					visited.insert(m);
					q_l.push(m);
				}
			}
		}
	}

    std::cout << "degreeCorrelation id:" <<  uid << "  vertex size:" << visited.size() << std::endl;
    if (visited.size() <= 1)
        return 0;
    double averageD1 = 0;
    double averageD2 = 0;
    vector<pair<double,double>> dpair;
    for (int temp_u : visited)
    {
        double degree_u = degree[temp_u];
        int num = 0;
        for (int i = 0; i < num_of_pred; ++i)
		{
			int x = pred_sets[i];
			num = csrHandler->getOutSize(temp_u, x);
            for (int j = 0; j < num; ++j)
			{
				int t = csrHandler->getOutVertID(temp_u, x, j);
                double degree_t = degree[t];
                if (degree_t == 0)
                    continue;
                dpair.push_back(std::make_pair(degree_u, degree_t));
                averageD1 += (double)degree_u;
                averageD2 += (double)degree_t;
			}
		}
    }
    averageD1 /= double(dpair.size());
    averageD2 /= double(dpair.size());
    double s1 = 0;
    double s2 = 0;
    double ret = 0;
    for(auto v:dpair)
    {        
        s1+=pow(v.first-averageD1,2);s2+=pow(v.second-averageD2,2);
        ret+=(v.first-averageD1)*(v.second-averageD2);
    }
    s1 = pow(s1,0.5);s2=pow(s2,0.5);
    if(s1*s2==0)
        ret=0;
    else
        ret /= (s1*s2);
    return ret;
 }