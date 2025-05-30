#include "PathQueryHandler.h"

using namespace std;

double PathQueryHandler::confidenceDegree(int uid, bool directed, const std::vector<int> &pred_set)
{
    map<unsigned int, unsigned int> dis;
    dis[uid] = 0;
    queue<unsigned int> Q;
    Q.push(uid);
    unsigned int sum = 0;
    unsigned int cnt = 0;
    unsigned int pre_num = pred_set.size();
    while (Q.size())
    {
        unsigned int ele = Q.front();
        Q.pop();
        cnt++;
        for (unsigned pre = 0; pre < pre_num; pre++)
        {
            int outNum = csrHandler->getOutSize(ele, pre);
            for (int i = 0; i < outNum; ++i)
            {
                int to = csrHandler->getOutVertID(ele, pre, i); // get the node
                if (dis.find(to) != dis.end())
                    continue;
                dis[to] = dis[ele] + 1;
                sum += dis[to];
                Q.push(to);
            }
            if (directed)
                continue;
            int inNum = csrHandler->getInSize(ele, pre);
            for (int i = 0; i < inNum; ++i)
            {
                int to = csrHandler->getInVertID(ele, pre, i); // get the node
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