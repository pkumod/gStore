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
    for (int i : vertSets) {
        for (int j = 0; j < vertSets.size(); j++) {
            if (i == j) continue;
            int crossID = 0;
            int totalPathNum = shortestPathCount(i, j, true, pred_sets);
            if (totalPathNum == 0) continue;;
            int firstPathNum = shortestPathCount(i, id, true, pred_sets);
            int secondPathNum = shortestPathCount(id, j, true, pred_sets);
            // ret += (firstPathNum * secondPathNum) / totalPathNum;
            vector<int> minPath =  shortestPath(i, j, true, pred_sets);
            int minPathPred = (minPath.size() - 1) / 2;
            int firstPathPred = (shortestPath(i, id, true, pred_sets).size() - 1) / 2;
            int secondPathPred = (shortestPath(id, j, true, pred_sets).size() - 1) / 2;
            if (minPathPred == firstPathPred + secondPathPred) {
                crossID += firstPathNum * secondPathNum;
            }
            ret += crossID/totalPathNum;
        }
    }
    return ret;
}