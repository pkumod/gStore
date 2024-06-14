#include "PathQueryHandler.h"

using namespace std;

/**
	Compute and return the PageRank of all vertices in the graph.

	@param directed if false, treat all edges in the graph as bidirectional.
	@param pred_set the set of edge labels allowed.
	@param alpha the damping parameter.
	@param maxIter the maximum number of iterations in power method eigenvalue solver.
	@param tol the error tolerance used to check convergence in power method solver.
	@return a vector that maps each vertex's ID to its PageRank value.
**/
void PathQueryHandler::PR(bool directed, const std::vector<int> &pred_set, int retNum, double alpha, int maxIter, double tol,
std::vector<std::pair<int, double>> &idx2val)
{
	// initialize
	int nodeNum = getVertNum();
	if (retNum > nodeNum)
		retNum = nodeNum;
	vector<int> neiNum(nodeNum);
	vector<double> oldPR(nodeNum), newPR(nodeNum);
	for (int vid = 0; vid < nodeNum; ++vid)
		for (int pred : pred_set)
		{
			neiNum[vid] += getOutSize(vid, pred);
			if (directed)
				continue;
			neiNum[vid] += getInSize(vid, pred);
		}
	double offset = (1 - alpha) / nodeNum;
	for (int vid = 0; vid < nodeNum; ++vid)
		oldPR[vid] = 1.0 / nodeNum, newPR[vid] = offset;
	// iterate
	bool stop = false;
	int cnt = 0;
	while (!stop && cnt++ <= maxIter) // PR(j) = d * sum(PR(i)/OUT(i)) + (1-d)
	{
		stop = true;
		// update
		for (int vid = 0; vid < nodeNum; ++vid)
		{
			if (!neiNum[vid]) // dangling nodes
			{
				double add = alpha * oldPR[vid] / nodeNum;
				for (int to = 0; to < nodeNum; ++to)
				{
					newPR[to] += add;
				}
			}
			else
			{
				double add = alpha * oldPR[vid] / neiNum[vid];
				for (int pred : pred_set)
				{
					int outNum = getOutSize(vid, pred);
					for (int i = 0; i < outNum; ++i)
					{
						int to = getOutVertID(vid, pred, i); // get the node
						newPR[to] += add;
					}
					if (directed)
						continue;
					int inNum = getInSize(vid, pred);
					for (int i = 0; i < inNum; ++i)
					{
						int to = getInVertID(vid, pred, i);
						newPR[to] += add;
					}
				}
			}
		}
		// compare
		for (int vid = 0; vid < nodeNum; ++vid)
		{
			if (abs(newPR[vid] - oldPR[vid]) > tol)
				stop = false;
			oldPR[vid] = newPR[vid];
			newPR[vid] = offset;
		}
	}
	idx2val.clear();
	for (int i = 0; i < nodeNum; i++)
		idx2val.emplace_back(i, oldPR[i]);
	// Extract top-k results
	if (retNum != -1 && retNum < nodeNum) {
		sort(idx2val.begin(), idx2val.end(), [](pair<size_t, double> const &l, pair<size_t, double> const &r)
			{ return l.second > r.second; });
		idx2val.erase(idx2val.begin() + retNum, idx2val.end());
	}
}