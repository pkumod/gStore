// TODO: Convert all IDs to unsigned to incorporate more data?

#include "../Util/Util.h"
#include "../Database/CSR.h"

#ifndef _QUERY_PATH_H
#define _QUERY_PATH_H

class PathQueryHandler
{
private:
	CSR *csr;
	std::unordered_map<int, std::set<int> > distinctInEdges;
	std::unordered_map<int, std::set<int> > distinctOutEdges;
	int cacheMaxSize;
	int n, m;	// #vertices, #edges
public:
	PathQueryHandler(CSR *_csr);

	void inputGraph(std::string filename);	// Read in a graph FOR TESTING. Graph file format:
						// First line: #vertices #labels
						// Each following line: u v label
	void printCSR();	// Feel free to modify this for testing
	// void generateQueries(int queryType, bool directed, int numQueries, vector<pair<int, int>, int>& queries);

	// Labeled graph interface: edge (u, v, pred)

	int getVertNum();	// Get total number of vertices
	int getEdgeNum();	// Get total number of edges
    int getSetEdgeNum(const std::vector<int> &pred_set);    // TODO: Get the number of edges labeled by preds in pred_set
	int getInIndexByID(int vid, int pred);	// Get vertice's index in csr[1] offset_list
	int getInSize(int vid, int pred);	// Get the number of in-neighbors of vert
						// linked by edges labeled by pred
	int getInVertID(int vid, int pred, int pos);	// Get the pos-th in-neighbor of vert
						// linked by an edge labeled by pred
	int getInVertID(int vid, int pos);	// Get in-neighbor regardless of pred;
						// corresponds to getTotalInSize with distinct
	int getSetInSize(int vid, const std::vector<int> &pred_set);	// Get number of in-neighbors by pred_set
	int getTotalInSize(int vid, bool distinct);	// Get the total number of in-neighbors of vert
						// distinct will eliminate repetitive occurrences of same in-neighbor with different label edges

	int getOutIndexByID(int vid, int pred);
	int getOutSize(int vid, int pred);
	int getOutVertID(int vid, int pred, int pos);
	int getOutVertID(int vid, int pos);
	int getSetOutSize(int vid, const std::vector<int> &pred_set);
	int getTotalOutSize(int vid, bool distinct);
	void dfs(std::map<int, std::vector<int> > &route, std::map<int, bool> &vis, int q, int v, const std::vector<int> pred_set, std::vector<int> &ans, bool &finished);

	// Path query evaluation functions
	
	std::vector<int> simpleCycle(int uid, int vid, bool directed, const std::vector<int> &pred_set);
	std::vector<int> cycle(int uid, int vid, bool directed, const std::vector<int> &pred_set);
	std::vector<int> shortestPath(int uid, int vid, bool directed, const std::vector<int> &pred_set);
	std::vector<int> shortestPath0(int uid, int vid, bool directed, const std::vector<int> &pred_set);
	bool kHopReachable(int uid, int vid, bool directed, int k, const std::vector<int> &pred_set);
	bool kHopReachableTest(int uid, int vid, bool directed, int k, const std::vector<int> &pred_set);
	std::vector<int> kHopReachablePath(int uid, int vid, bool directed, int k, const std::vector<int> &pred_set);

	void SSPPR(int uid, int retNum, int k, const std::vector<int> &pred_set, std::vector< std::pair<int ,double> > &topkV2ppr);

    std::vector<std::pair<std::pair<int, int>, int>> kHopSubgraph(int uid, int vid, bool directed, int k, const std::vector<int> &pred_set);
    
private:
    // Helper functions for SSPPR
    void compute_ppr_with_reserve(std::pair<iMap<double>, iMap<double>> &fwd_idx, std::unordered_map<int, double> &v2ppr);
    void forward_local_update_linear_topk(int s, double& rsum, double rmax, double lowest_rmax, \
        std::vector<std::pair<int, int>>& forward_from, std::pair<iMap<double>, iMap<double>> &fwd_idx, \
        const std::vector<int> &pred_set, double alpha, int k);
    void compute_ppr_with_fwdidx_topk_with_bound(double check_rsum, std::pair<iMap<double>, iMap<double>> &fwd_idx, \
        std::unordered_map<int, double> &v2ppr, double delta, double alpha, double threshold, \
        const std::vector<int> &pred_set, double pfail, double &zero_ppr_upper_bound, double omega, \
        iMap<double> &upper_bounds, iMap<double> &lower_bounds);
    void set_ppr_bounds(std::pair<iMap<double>, iMap<double>> &fwd_idx, double rsum, long total_rw_num, \
        std::unordered_map<int, double> &v2ppr, double pfail, double &zero_ppr_upper_bound, \
        iMap<double> &upper_bounds, iMap<double> &lower_bounds);
    inline double calculate_lambda(double rsum, double pfail, double upper_bound, long total_rw_num);
    inline int random_walk(int start, double alpha, const std::vector<int> &pred_set);
    double kth_ppr(std::unordered_map<int, double> &v2ppr, int retNum);
    bool if_stop(int retNum, double delta, double threshold, double epsilon, iMap<int> &topk_filter, \
        iMap<double> &upper_bounds, iMap<double> &lower_bounds, std::unordered_map<int, double> &v2ppr);
};

#endif //_QUERY_PATH_H