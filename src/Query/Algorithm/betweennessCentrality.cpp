#include "PathQueryHandler.h"

using namespace std;

double PathQueryHandler::betweennessCentrality(int id, const std::vector<int> &pred_sets)
{
    double ret = 0;
    std::cout << "betweennessCentrality 节点v的介数中心度:" << id << std::endl;
    std::cout << "betweennessCentrality 允许出现的谓词:" << pred_sets.size() << std::endl;
    return ret;
}