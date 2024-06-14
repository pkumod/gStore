#include "PathQueryHandler.h"

using namespace std;

/**
	Compute and return the number of triangles in the graph, only considering the
	edges in the path with labels in pred_set.

	@param directed if false, treat all edges in the graph as bidirectional.
					if true, only count cycle type triangle (eg: a->b->c->a)
	@param pred_set the set of edge labels allowed.
	@return the number of triangles in the graph.
**/
long long PathQueryHandler::triangleCounting(bool directed, const std::vector<int> &pred_set)
{
	if (pred_set.empty())
		return -1;

	long long numTriangle = 0;
	// loop each vertex and count triangle
	int vertex_num = getVertNum();
	for (int vid = 0; vid < vertex_num; ++vid)
	{
		// count triangle for vid

		// directed: get the in-neighbor set of the current node
		// directed: neighbor
		unordered_set<int> inSet;
		for (int pred : pred_set)
		{
			int inSize = getInSize(vid, pred);
			for (int j = 0; j < inSize; j++)
				inSet.insert(getInVertID(vid, pred, j));
			if (directed == 0)
			{
				int outSize = getOutSize(vid, pred);
				for (int j = 0; j < outSize; j++)
					inSet.insert(getOutVertID(vid, pred, j));
			}
		}

		// directed:
		// for each out-neighbor, intersect its out-neighbors with the current node's in-neighbors
		// the size of the intersection is the # of triangles
		if (directed)
		{
			for (int pred : pred_set)
			{
				int outSize = getOutSize(vid, pred);
				for (int j = 0; j < outSize; j++)
				{
					int outNode = getOutVertID(vid, pred, j);
					for (int pd : pred_set)
					{
						int outOutSize = getOutSize(outNode, pd);
						for (int k = 0; k < outOutSize; k++)
						{
							int outOutNode = getOutVertID(outNode, pd, k);
							if (inSet.find(outOutNode) != inSet.end())
								numTriangle++;
						}
					}
				}
			}
		}
		// undirected:
		// for each neighbor, intersect its neighbors(except current node) with the current node's neighbors
		else
		{
			for (int outNode : inSet)
			{
				unordered_set<int> nbrs;
				for (int pred : pred_set)
				{
					int outOutSize = getOutSize(outNode, pred);
					for (int k = 0; k < outOutSize; k++)
					{
						nbrs.insert(getOutVertID(outNode, pred, k));
					}
					int outInSize = getInSize(outNode, pred);
					for (int k = 0; k < outInSize; k++)
					{
						nbrs.insert(getInVertID(outNode, pred, k));
					}
				}
				for (int outOutNode : nbrs)
				{
					if (inSet.find(outOutNode) != inSet.end())
						numTriangle++;
				}
			}
		}
	}
	//}

	int dup_num = 6;
	if (directed)
		dup_num = 3;
	if (numTriangle % dup_num != 0)
	{
		cout << "ERROR!!!" << numTriangle << " is not triple of " << dup_num << "!" << endl;
		return -1;
	}
	cout << numTriangle / dup_num << endl;
	return numTriangle / dup_num;
}