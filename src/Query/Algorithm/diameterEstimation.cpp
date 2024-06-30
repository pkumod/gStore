#include "PathQueryHandler.h"

using namespace std;

// unsigned int PathQueryHandler::diameterEstimation(const std::vector<int> &pred_sets)
// {
//     std::cout << "diameterEstimation 允许出现的谓词:" << pred_sets.size() << std::endl;
//     /**
//      * @brief 采用两个队列的双向广度优先搜索方法，优先对邻居少的结点进行广度优先搜索
//      * 
//      */
//     int ret = 0;
//     int vertNum = getVertNum();
//     int predNum = pred_sets.size();
//     if (predNum == 0) {
//         predNum = getEdgeNum();
//     }
//     std::set<int> vertSets; 
//     for (int i = 0; i < predNum; i++) {
//         int predVertNum = csr->id2vid[i].size();
//         for (int j = 0; j < predVertNum; j++) {
//             vertSets.insert(csr->id2vid[i][j]);
//         }
//     }
//     for (int beginVertID = 0; beginVertID < vertSets.size(); beginVertID++) {
//         // int beginPred = rand()%predNum;
//         // int endPred = rand()%predNum;
//         // while (beginPred == endPred) {
//         //     endPred = rand()%predNum;
//         // }
//         for (int endVertID = beginVertID + 1; endVertID < vertSets.size(); endVertID++){
//             // int q1[vertNum + 1], q2[vertNum + 1];
//             std::vector<int> q1(vertNum + 1), q2(vertNum + 1);
//             int front1,rear1,front2,rear2,i,flag;
//             front1=rear1=0;
//             front2=rear2=0;
//             int cur, next;
//             cur = beginVertID;
//             next = endVertID;
//             std::vector<int> vis(vertNum + 1, 0);
//             vis[cur] = 1;
//             vis[next] = 2;
//             q1[rear1] = cur;
//             rear1 += 1;
//             q2[rear2] = next;
//             rear2 += 1;
//             // 记录路径
//             std::vector<int> step(vertNum + 1, 0);
//             while (front1 < rear1 && front2<rear2) {
//                 // 出列
//                 if (rear1-front1 < rear2-front2) {
//                     cur = q1[front1++]; 
//                     flag = 1;
//                 } else {
//                     cur = q2[front2++];
//                     flag = 2;
//                 }
//                 // 入列
//                 for (i = 0; i < predNum; i++) {
//                     int predID = pred_sets[i];
//                     int nextVertNum = getOutSize(cur, predID);
//                     for (int j = 0; j < nextVertNum; j++) {
//                         next = getOutVertID(cur, predID, j);
//                         if (!vis[next]) {
//                             // 当前结点未被扩展过
//                             vis[next] = flag;
//                             step[next] = step[cur] + 1;
//                             if (flag == 1) {
//                                 q1[rear1++] = next;
//                             } else if (flag == 2) {
//                                 q2[rear2++] = next;
//                             }
//                         } else if (vis[next] != vis[cur]) {
//                             // 表示next结点以前按相反的方向访问过，正向和反向遇到了同一个结点
//                             if (ret < step[cur] + step[next] + 1) {
//                                 ret = step[cur] + step[next] + 1;
//                             }
//                         }
//                     }
//                 }
//             } 
//         } 
//     }  
//     return ret;
// }

unsigned int PathQueryHandler::diameterEstimation2(const std::vector<int> &pred_sets)
{
    std::cout << "diameterEstimation 允许出现的谓词数量:" << pred_sets.size() << std::endl;
    unsigned int ret = 0;
    int vertex_num = getVertNum();
    int num_of_pred = pred_sets.size();
    std::set<int> vertex;
    for (int temp_u = 0; temp_u < vertex_num; temp_u++)
    {
        for (int i = 0; i < num_of_pred; ++i)
        {
            int x = pred_sets[i];
            int num = getOutSize(temp_u, x);
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
	std::map<int, std::vector<int>> dis_pre;
	q_l.push(uid);
	dis_u[uid] = 0;
	visited.insert(uid);
    std::set<int> visited_s;

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
				nl.insert(t);
				auto vit = dis_u.find(t);
				if (vit == dis_u.end())
				{
					dis_u[t] = distance;
				}
				else if (dis_u[t] < distance && t != uid)
				{
					dis_u[t] = distance;
				}
                //不重复记录已访问的点，避免环路
                if (dis_u[t] > ret.first && visited_s.find(t) == visited_s.end())
                {
                    ret.first = dis_u[t];
                    ret.second= t;
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
        visited_s.insert(temp_u);
	}

    return ret;
}