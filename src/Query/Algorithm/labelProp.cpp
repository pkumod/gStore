#include "PathQueryHandler.h"

using namespace std;

/**
	The label detection community detection algorithm.

	@param directed if false, treat all edges in the graph as bidirectional.
	@param pred_set the set of edge labels allowed.
	@param maxIter the maximum number of iterations.
	@return a vector of vectors, each containing the IDs of the vertices in a community.
**/
vector<vector<int>> PathQueryHandler::labelProp(bool directed, const std::vector<int> &pred_set, int maxIter)
{
	class LPA
	{
	public:
		PathQueryHandler &g;
		bool directed;
		const vector<int> &pred_set;

		unsigned nodeNum;
		unsigned edgeNum;
		vector<int> coms;
		unsigned comNum;
		vector<int> labels;
		vector<int> maxLabels;
		double maxQ;

		LPA(PathQueryHandler &_g, bool _directed, const vector<int> &_pred_set)
			: g(_g), directed(_directed), pred_set(_pred_set), maxQ(-1)
		{
			nodeNum = g.getVertNum();
			edgeNum = g.getEdgeNum();
			comNum = nodeNum;
			coms.resize(nodeNum);
			labels.resize(nodeNum);
			for (unsigned i = 0; i < nodeNum; i++)
				coms[i] = i;
			for (unsigned i = 0; i < nodeNum; i++)
				labels[i] = i;
		}

		void propagate()
		{
			vector<int> cnt;
			vector<int> maxLbl;
			for (unsigned vid = 0; vid < nodeNum; vid++)
			{
				// async update
				cnt.assign(nodeNum, 0);
				maxLbl.clear();
				int maxCnt = 0, maxNum = 0;
				for (int pred : pred_set)
				{
					int inNum = g.getInSize(vid, pred);
					for (int i = 0; i < inNum; ++i)
					{
						int to = g.getInVertID(vid, pred, i);
						cnt[labels[to]]++;
						if (maxCnt < cnt[labels[to]])
							maxCnt = cnt[labels[to]];
					}
					if (directed)
						continue;
					int outNum = g.getOutSize(vid, pred);
					for (int i = 0; i < outNum; ++i)
					{
						int to = g.getOutVertID(vid, pred, i);
						cnt[labels[to]]++;
						if (maxCnt < cnt[labels[to]])
							maxCnt = cnt[labels[to]];
					}
				}
				for (unsigned lbl = 0; lbl < nodeNum; lbl++)
				{
					if (cnt[lbl] == maxCnt)
					{
						maxLbl.push_back(lbl);
						maxNum++;
					}
				}
				if (maxCnt)
					labels[vid] = maxLbl[rand() % maxNum];
			}
			set<int> diffLabel;
			for (unsigned vid = 0; vid < nodeNum; vid++)
				diffLabel.insert(labels[vid]);
			comNum = 0;
			for (auto ele : diffLabel)
				coms[comNum++] = ele;
			coms.resize(comNum);
		}

		int edgeSum(int com)
		{
			int res = 0;
			set<int> nodes;
			for (int vid = 0; vid < (int)nodeNum; vid++)
				if (labels[vid] == com)
					nodes.insert(vid);
			for (auto &node : nodes)
			{
				for (int pred : pred_set)
				{
					int outNum = g.getOutSize(node, pred);
					for (int i = 0; i < outNum; ++i)
					{
						int to = g.getOutVertID(node, pred, i);
						if (nodes.find(to) != nodes.end())
							res++;
					}
					if (directed)
						continue;
					int inNum = g.getInSize(node, pred);
					for (int i = 0; i < inNum; ++i)
					{
						int to = g.getInVertID(node, pred, i);
						if (nodes.find(to) != nodes.end())
							res++;
					}
				}
			}
			return res;
		}

		int degreeSum(int com)
		{
			int res = 0;
			for (int vid = 0; vid < (int)nodeNum; vid++)
			{
				if (labels[vid] != com)
					continue;
				for (int pred : pred_set)
				{
					int outNum = g.getOutSize(vid, pred);
					res += outNum;
					if (directed)
						continue;
					int inNum = g.getInSize(vid, pred);
					res += inNum;
				}
			}
			return res;
		}

		double modularity()
		{
			double Q = 0, dblM = 2.0 * edgeNum;
			for (auto com : coms)
			{
				int edge = edgeSum(com);
				int degree = degreeSum(com);
				Q += edge / dblM - (degree / dblM) * (degree / dblM);
			}
			if (maxQ < Q)
				maxLabels = labels;
			return Q;
		}

		vector<vector<int>> output()
		{
			vector<vector<int>> res(comNum);
			map<int, int> lbl2id;
			int idx = 0;
			for (auto com : coms)
				lbl2id[com] = idx++;
			for (unsigned vid = 0; vid < nodeNum; vid++)
				res[lbl2id[maxLabels[vid]]].push_back(vid);
			return res;
		}
	};
	const int seed = 114514;
	const double tol = 10;
	const double thres = 0.001;
	std::srand(seed);
	LPA lpa(*this, directed, pred_set);
	double oldQ = -1, Q;
	double oldNum = -1, comNum;
	int cnt = 0;
	for (int t = 1; t <= maxIter; ++t)
	{
		lpa.propagate();
		Q = lpa.modularity();
		comNum = lpa.comNum;
		if (comNum == oldNum && abs(Q - oldQ) < thres)
		{
			if (cnt >= tol)
				break;
			else
				cnt++;
		}
		else
			cnt = 0;
		oldQ = Q;
		oldNum = comNum;
	}
	return lpa.output();
}