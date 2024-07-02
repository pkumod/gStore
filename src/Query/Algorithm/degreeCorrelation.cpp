#include "PathQueryHandler.h"

using namespace std;

// double PathQueryHandler::degreeCorrelation(int uid, int k, const std::vector<int> &pred_sets)
// {
//     double ret = 0,e=getSetEdgeNum(pred_sets);
//     double averageD1 = 0,averageD2=0,s1=0,s2=0;
//     queue<int> q,de;
//     q.push(uid);
//     unordered_map<int,bool> vi;
//     unordered_map<int,int> degree;
//     vector<pair<float,float>> dpair;
//     unordered_set<int> Set;
//     auto getVDegree = [&](int id,int K)
//     {
//         int sum = 0;
//         for(auto pred:pred_sets)
//         {
//             int inSz = getInSize(id,pred),inNei;
//             int outSz = getOutSize(id,pred),outNei;
//             if(K==0)
//             {
//                 for(int i=0;i<inSz;i++)
//                 {
//                     inNei = getInVertID(id,pred,i);
//                     if(Set.find(inNei)!=Set.end())
//                         sum++;
//                 }
//                 for(int i=0;i<outSz;i++)
//                 {
//                     outNei = getOutVertID(id,pred,i);
//                     if(Set.find(outNei)!=Set.end())
//                         sum++;
//                 }
//             }
//             else
//             sum+=inSz+outSz;
//         }
//         degree[id] = sum;
//         return sum;
//     };
//     auto oneHop = [&](int id,int K)
//     {
//         unordered_set<int> oneHop_ret = unordered_set<int>();
//         for(auto pred:pred_sets)
//         {
//             int inSz = getInSize(id,pred),inNei;
//             int outSz = getOutSize(id,pred),outNei;

//             for(int i=0;i<inSz;i++)
//             {
//                 inNei = getInVertID(id,pred,i);
//                 oneHop_ret.insert(inNei);
//                 if(vi[inNei]==false&&(K!=0||(Set.find(inNei)!=Set.end())))
//                 {
//                     dpair.emplace_back(pair<double,double>(inNei,id));
//                 }

//             }
//             for(int i=0;i<outSz;i++)
//             {
//                 outNei = getOutVertID(id,pred,i);
//                 oneHop_ret.insert(outNei);
//                 if(vi[outNei]==false&&(K!=0||(Set.find(outNei)!=Set.end())))
//                 {
//                     dpair.emplace_back(pair<double,double>(id,outNei));
//                 }
//             }
//         }
//         return oneHop_ret;
//     };
//     Set.insert(uid);  
//     while(q.size())
//     {
//         int vid = q.front();
//         getVDegree(vid,k);
//         unordered_set<int> oneHopAdj=oneHop(vid,k);
//         vi[vid]=true;
//         q.pop();
//         if(k!=0)
//         for(auto v:oneHopAdj)
//         {
//             if(Set.find(v)==Set.end())
//             {              
//                 q.push(v);
//                 Set.insert(v);
//             }
//         }
//         if(k>0) k--;
//     }
//     for(auto v:dpair)
//     {
//         averageD1 += degree[v.first];
//         averageD2 += degree[v.second];
//     }
//     averageD1/=dpair.size();averageD2/=dpair.size();
//     for(auto v:dpair)
//     {        
//         s1+=pow(degree[v.first]-averageD1,2);s2+=pow(degree[v.second]-averageD2,2);
//         ret+=(degree[v.first]-averageD1)*(degree[v.second]-averageD2);
//     }
//     s1 = pow(s1,0.5);s2=pow(s2,0.5);
//     if(s1*s2==0)
//         ret=0;
//     else
//         ret /= (s1*s2);
//     return ret;
// }

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
	std::set<int> nei_node;
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
            num = getOutSize(temp_u, x);
            degree_num += num;
            for (int j = 0; j < num; ++j)
            {
                int t = getOutVertID(temp_u, x, j);
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
            num = getInSize(temp_u, x);
            degree_num += num;
            for (int j = 0; j < num; ++j)
            {
                int t = getInVertID(temp_u, x, j);
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
			nei_node.insert(temp_u);
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
			num = getOutSize(temp_u, x);
            for (int j = 0; j < num; ++j)
			{
				int t = getOutVertID(temp_u, x, j);
                double degree_t = degree[t];
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