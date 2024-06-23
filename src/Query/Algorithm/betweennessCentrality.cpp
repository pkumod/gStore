#include "PathQueryHandler.h"

using namespace std;

double PathQueryHandler::betweennessCentrality(int id, const std::vector<int> &pred_sets)
{
    double ret = 0;
    std::cout << "betweennessCentrality 节点v的介数中心度:" << id << std::endl;
    std::cout << "betweennessCentrality 允许出现的谓词:" << pred_sets.size() << std::endl;
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
        for (int j = i + 1; j < vertSets.size(); j++) {
            int totalPathNum = shortestPathCount(i, j, true, pred_sets);
            int firstPathNum = shortestPathCount(i, id, true, pred_sets);
            int secondPathNum = shortestPathCount(id, j, true, pred_sets);
            ret += (firstPathNum * secondPathNum) / totalPathNum;
        }
    }
    return ret;
}