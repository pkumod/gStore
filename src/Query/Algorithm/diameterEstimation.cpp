#include "PathQueryHandler.h"

using namespace std;

unsigned int PathQueryHandler::diameterEstimation(const std::vector<int> &pred_sets)
{
    std::cout << "diameterEstimation 允许出现的谓词:" << pred_sets.size() << std::endl;
    /**
     * @brief 采用两个队列的双向广度优先搜索方法，优先对邻居少的结点进行广度优先搜索
     * 
     */
    int ret = 0;
    int vertNum = getVertNum();
    int predNum = pred_sets.size();
    if (predNum == 0) {
        predNum = getEdgeNum();
    }
    std::set<int> vertSets; 
    for (int i = 0; i < predNum; i++) {
        int predVertNum = csr->id2vid[i].size();
        for (int j = 0; j < predVertNum; j++) {
            vertSets.insert(csr->id2vid[i][j]);
        }
    }
    for (int beginVertID = 0; beginVertID < vertSets.size(); beginVertID++) {
        // int beginPred = rand()%predNum;
        // int endPred = rand()%predNum;
        // while (beginPred == endPred) {
        //     endPred = rand()%predNum;
        // }
        for (int endVertID = beginVertID + 1; endVertID < vertSets.size(); endVertID++){
            int q1[vertNum + 1], q2[vertNum + 1];
            int front1,rear1,front2,rear2,i,flag;
            front1=rear1=0;
            front2=rear2=0;
            int cur, next;
            cur = beginVertID;
            next = endVertID;
            int vis[vertNum + 1] = {0};
            vis[cur] = 1;
            vis[next] = 2;
            q1[rear1] = cur;
            rear1 += 1;
            q2[rear2] = next;
            rear2 += 1;
            // 记录路径
            int step[vertNum + 1] = {0};
            while (front1 < rear1 && front2<rear2) {
                // 出列
                if (rear1-front1 < rear2-front2) {
                    cur = q1[front1++]; 
                    flag = 1;
                } else {
                    cur = q2[front2++];
                    flag = 2;
                }
                // 入列
                for (i = 0; i < predNum; i++) {
                    int predID = pred_sets[i];
                    int nextVertNum = getOutSize(cur, predID);
                    for (int j = 0; j < nextVertNum; j++) {
                        next = getOutVertID(cur, predID, j);
                        if (!vis[next]) {
                            // 当前结点未被扩展过
                            vis[next] = flag;
                            step[next] = step[cur] + 1;
                            if (flag == 1) {
                                q1[rear1++] = next;
                            } else if (flag == 2) {
                                q2[rear2++] = next;
                            }
                        } else if (vis[next] != vis[cur]) {
                            // 表示next结点以前按相反的方向访问过，正向和反向遇到了同一个结点
                            if (ret < step[cur] + step[next] + 1) {
                                ret = step[cur] + step[next] + 1;
                            }
                        }
                    }
                }
            } 
        } 
    }  
    return ret;
}