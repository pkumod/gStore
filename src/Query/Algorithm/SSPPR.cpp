#include "PathQueryHandler.h"

using namespace std;

double PathQueryHandler::kth_ppr(unordered_map<int, double> &v2ppr, int retNum)
{
	static vector<double> temp_ppr;
	temp_ppr.clear();
	temp_ppr.resize(v2ppr.size());
	int i = 0;
	for (auto it = v2ppr.begin(); it != v2ppr.end(); ++it)
	{
		temp_ppr[i] = v2ppr[it->second];
		i++;
	}
	nth_element(temp_ppr.begin(), temp_ppr.begin() + retNum - 1, temp_ppr.end(),
				[](double x, double y)
				{ return x > y; });
	return temp_ppr[retNum - 1];
}

bool PathQueryHandler::if_stop(int retNum, double delta, double threshold, double epsilon, iMap<int> &topk_filter,
							   iMap<double> &upper_bounds, iMap<double> &lower_bounds, unordered_map<int, double> &v2ppr)
{
	if (kth_ppr(v2ppr, retNum) >= 2.0 * delta)
		return true;

	if (delta >= threshold)
		return false;

	const static double error = 1.0 + epsilon;
	const static double error_2 = 1.0 + epsilon;

	vector<pair<int, double>> topk_pprs;
	topk_pprs.clear();
	topk_pprs.resize(retNum);
	topk_filter.clean();

	static vector<pair<int, double>> temp_bounds;
	temp_bounds.clear();
	temp_bounds.resize(lower_bounds.occur.m_num);
	int nodeid;
	for (unsigned i = 0; i < lower_bounds.occur.m_num; i++)
	{
		nodeid = lower_bounds.occur[i];
		temp_bounds[i] = make_pair(nodeid, lower_bounds[nodeid]);
	}

	// sort topk nodes by lower bound
	partial_sort_copy(temp_bounds.begin(), temp_bounds.end(), topk_pprs.begin(), topk_pprs.end(),
					  [](pair<int, double> const &l, pair<int, double> const &r)
					  { return l.second > r.second; });

	// for topk nodes, upper-bound/low-bound <= 1+epsilon
	double ratio = 0.0;
	double largest_ratio = 0.0;
	for (auto &node : topk_pprs)
	{
		topk_filter.insert(node.first, 1);
		ratio = upper_bounds[node.first] / lower_bounds[node.first];
		if (ratio > largest_ratio)
			largest_ratio = ratio;
		if (ratio > error_2)
		{
			return false;
		}
	}

	// for remaining NO. retNum+1 to NO. n nodes, low-bound of retNum > the max upper-bound of remaining nodes
	double low_bound_k = topk_pprs[retNum - 1].second;
	if (low_bound_k <= delta)
		return false;

	for (unsigned i = 0; i < upper_bounds.occur.m_num; i++)
	{
		nodeid = upper_bounds.occur[i];
		if (topk_filter.exist(nodeid) || v2ppr[nodeid] <= 0)
			continue;

		double upper_temp = upper_bounds[nodeid];
		double lower_temp = lower_bounds[nodeid];
		if (upper_temp > low_bound_k * error)
		{
			if (upper_temp > (1 + epsilon) / (1 - epsilon) * lower_temp)
				continue;
			else
				return false;
		}
		else
			continue;
	}

	return true;
}

// retNum is the number of top nodes to return; k is the hop constraint -- don't mix them up!
void PathQueryHandler::SSPPR(int uid, int retNum, int k, const vector<int> &pred_set, vector<pair<int, double>> &topkV2ppr)
{
	topkV2ppr.clear();
	srand(time(NULL));

	unordered_map<int, double> v2ppr;
	pair<iMap<double>, iMap<double>> fwd_idx;
	iMap<double> ppr;
	iMap<int> topk_filter;
	iMap<double> upper_bounds;
	iMap<double> lower_bounds;

	// Data structures initialization
	fwd_idx.first.nil = -9;
	fwd_idx.first.initialize(getVertNum());
	fwd_idx.second.nil = -9;
	fwd_idx.second.initialize(getVertNum());
	upper_bounds.nil = -9;
	upper_bounds.init_keys(getVertNum());
	lower_bounds.nil = -9;
	lower_bounds.init_keys(getVertNum());
	ppr.nil = -9;
	ppr.initialize(getVertNum());
	topk_filter.nil = -9;
	topk_filter.initialize(getVertNum());

	// Params initialization
	int numPredEdges = getSetEdgeNum(pred_set);
	double ppr_decay_alpha = 0.77;
	double pfail = 1.0 / getVertNum() / getVertNum() / log(getVertNum()); // log(1/pfail) -> log(1*n/pfail)
	double delta = 1.0 / 4;
	double epsilon = 0.5;
	double rmax;
	double rmax_scale = 1.0;
	double omega;
	double alpha = 0.2;
	double min_delta = 1.0 / getVertNum();
	double threshold = (1.0 - ppr_decay_alpha) / pow(500, ppr_decay_alpha) / pow(getVertNum(), 1 - ppr_decay_alpha);
	double lowest_delta_rmax = epsilon * sqrt(min_delta / 3 / numPredEdges / log(2 / pfail));
	double rsum = 1.0;

	vector<pair<int, int>> forward_from;
	forward_from.clear();
	forward_from.reserve(getVertNum());
	forward_from.push_back(make_pair(uid, 0));

	fwd_idx.first.clean();	// reserve
	fwd_idx.second.clean(); // residual
	fwd_idx.second.insert(uid, rsum);

	double zero_ppr_upper_bound = 1.0;
	upper_bounds.reset_one_values();
	lower_bounds.reset_zero_values();

	// fora_query_topk_with_bound
	// for delta: try value from 1/4 to 1/n
	while (delta >= min_delta)
	{
		rmax = epsilon * sqrt(delta / 3 / numPredEdges / log(2 / pfail));
		rmax *= rmax_scale;
		omega = (2 + epsilon) * log(2 / pfail) / delta / epsilon / epsilon;

		if (getSetOutSize(uid, pred_set) == 0)
		{
			rsum = 0.0;
			fwd_idx.first.insert(uid, 1);
			compute_ppr_with_reserve(fwd_idx, v2ppr);
			break;
		}
		else
			forward_local_update_linear_topk(uid, rsum, rmax, lowest_delta_rmax, forward_from, fwd_idx, pred_set, alpha, k); // forward propagation, obtain reserve and residual

		compute_ppr_with_fwdidx_topk_with_bound(rsum, fwd_idx, v2ppr, delta, alpha, threshold,
												pred_set, pfail, zero_ppr_upper_bound, omega, upper_bounds, lower_bounds);
		if (if_stop(retNum, delta, threshold, epsilon, topk_filter, upper_bounds, lower_bounds, v2ppr) || delta <= min_delta)
			break;
		else
			delta = max(min_delta, delta / 2.0); // otherwise, reduce delta to delta/2
	}

	// Extract top-k results
	topkV2ppr.clear();
	topkV2ppr.resize(retNum);
	partial_sort_copy(v2ppr.begin(), v2ppr.end(), topkV2ppr.begin(), topkV2ppr.end(),
					  [](pair<int, double> const &l, pair<int, double> const &r)
					  { return l.second > r.second; });
	if (topkV2ppr.size() > 0)
	{
		size_t i = topkV2ppr.size() - 1;
		while (topkV2ppr[i].second == 0 && i != 0)
		{
			i--;
		}
		topkV2ppr.erase(topkV2ppr.begin() + i + 1, topkV2ppr.end()); // Get rid of ppr = 0 entries
	}
}