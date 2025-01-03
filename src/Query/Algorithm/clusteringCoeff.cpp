#include "PathQueryHandler.h"

using namespace std;

/**
	Compute and return the (global) clustering coefficient of the graph.

	@param directed if false, treat all edges in the graph as bidirectional.
	@param pred_set the set of edge labels allowed.
	@return the (global) clustering coefficient of the graph.
**/
double PathQueryHandler::clusteringCoeff(bool directed, const std::vector<int> &pred_set)
{
	cout << "global clusteringCoeff" << endl;
	if (pred_set.empty())
		return -1;
	int n = csrHandler->getVertNum();
	int nclt = 0; // number of closed triplet
	int nopt = 0; // number of open triplet
	double gcc;

	for (int curnode = 0; curnode < n; curnode++)
	{
		unordered_set<int> neiSet;
		for (int pred : pred_set)
		{
			int inSize = csrHandler->getInSize(curnode, pred);
			for (int j = 0; j < inSize; j++)
			{
				neiSet.insert(csrHandler->getInVertID(curnode, pred, j));
			}
			int outSize = csrHandler->getOutSize(curnode, pred);
			for (int j = 0; j < outSize; j++)
			{
				neiSet.insert(csrHandler->getOutVertID(curnode, pred, j));
			}
		}

		for (int neiNode : neiSet)
		{
			unordered_set<int> neighbors;
			for (int pred : pred_set)
			{
				int outOutSize = csrHandler->getOutSize(neiNode, pred);
				for (int k = 0; k < outOutSize; k++)
				{
					neighbors.insert(csrHandler->getOutVertID(neiNode, pred, k));
				}
				int outInSize = csrHandler->getInSize(neiNode, pred);
				for (int k = 0; k < outInSize; k++)
				{
					neighbors.insert(csrHandler->getInVertID(neiNode, pred, k));
				}
			}
			for (int outOutNode : neighbors)
			{
				if (neiSet.find(outOutNode) != neiSet.end())
				{
					nclt++;
				}
				else
				{
					if (outOutNode == curnode)
						continue;
					nopt++;
				}
			}
		}
	}
	nclt /= 2;
	nopt /= 2;
	if (nclt + nopt == 0)
	{
		gcc = 0;
	}
	else
	{
		gcc = (double)nclt / (nclt + nopt);
	}
	return gcc;
}

/**
	Compute and return the (local) clustering coefficient of vertex u.

	@param uid vertex u's ID.
	@param directed if false, treat all edges in the graph as bidirectional.
	@param pred_set the set of edge labels allowed.
	@return the (local) clustering coefficient of vertex u.
**/
double PathQueryHandler::clusteringCoeff(int uid, bool directed, const std::vector<int> &pred_set)
{
	cout << "local clusteringCoeff" << endl;
	if (pred_set.empty())
		return -1;
	double lcc;
	int dins = 0, maxns = 0;
	if (directed)
	{
		unordered_set<int> inSet, neighbors;
		for (int pred : pred_set)
		{
			int inSize = csrHandler->getInSize(uid, pred);
			for (int i = 0; i < inSize; i++)
			{
				inSet.insert(csrHandler->getInVertID(uid, pred, i));
				neighbors.insert(csrHandler->getInVertID(uid, pred, i));
			}
		}

		for (int pred : pred_set)
		{
			int outSize = csrHandler->getOutSize(uid, pred);
			for (int j = 0; j < outSize; j++)
			{
				int outNode = csrHandler->getOutVertID(uid, pred, j);
				if (!neighbors.count(outNode))
					neighbors.insert(outNode);
				for (int pd : pred_set) // all out neighbors
				{
					int outOutSize = csrHandler->getOutSize(outNode, pd);
					for (int k = 0; k < outOutSize; k++)
					{
						int outOutNode = csrHandler->getOutVertID(outNode, pd, k);
						if (inSet.count(outOutNode))
							dins++; // directed neighbors size
					}
				}
			}
		}
		int ns = neighbors.size(); // indirected neighbors size
		maxns = ns * (ns - 1);
		if (maxns == 0)
		{
			lcc = 0;
		}
		else
		{
			lcc = (double)dins / maxns;
		}
	}
	else
	{
		unordered_set<int> neiSet;
		for (int pred : pred_set)
		{
			int inSize = csrHandler->getInSize(uid, pred);
			for (int i = 0; i < inSize; i++)
			{
				neiSet.insert(csrHandler->getInVertID(uid, pred, i));
			}
			int outSize = csrHandler->getOutSize(uid, pred);
			for (int j = 0; j < outSize; j++)
			{
				neiSet.insert(csrHandler->getOutVertID(uid, pred, j));
			}
		}

		for (int neiNode : neiSet)
		{
			unordered_set<int> nbrs;
			for (int pred : pred_set)
			{
				int outOutSize = csrHandler->getOutSize(neiNode, pred);
				for (int k = 0; k < outOutSize; k++)
				{
					nbrs.insert(csrHandler->getOutVertID(neiNode, pred, k));
				}
				int outInSize = csrHandler->getInSize(neiNode, pred);
				for (int k = 0; k < outInSize; k++)
				{
					nbrs.insert(csrHandler->getInVertID(neiNode, pred, k));
				}
			}
			for (int outOutNode : nbrs)
			{
				if (neiSet.find(outOutNode) != neiSet.end())
					dins++;
			}
		}
		dins /= 2;				// directed neighbors size (de-duplication)
		int ns = neiSet.size(); // indirected neighbors size
		maxns = ns * (ns - 1) / 2;
		if (maxns == 0)
		{
			lcc = 0;
		}
		else
		{
			lcc = (double)dins / maxns;
		}
	}
	return lcc;
}