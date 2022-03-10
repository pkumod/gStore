/*
 * @Author: wangjian
 * @Date: 2022-02-14 11:17:16
 * @LastEditTime: 2022-02-28 10:01:39
 * @LastEditors: Please set LastEditors
 * @Description: CSR API
 * @FilePath: /gstore/Database/CSRUtil.h
 */
#include "CSR.h"

#ifndef _DATABASE_CSR_UTIL_H
#define _DATABASE_CSR_UTIL_H

class CSRUtil
{
private:
	CSR *csr;
	// 部分邻接列表的缓存
    std::unordered_map<int, std::set<int> > distinctInEdges;
	std::unordered_map<int, std::set<int> > distinctOutEdges;
	//允许缓存结点的最大数目，一旦超过这个数目，就会随机选择一个已载入的结点替换出去
	int cacheMaxSize;
    // n节点总数，m边总数
	int n, m;
public:
    CSRUtil(CSR *_csr);
    ~CSRUtil();
	CSR* getCSR();
	 // Get total number of vertices
    int getVertNum();
    // Get total number of edges
	int getEdgeNum();
    // Get the number of edges labeled by preds in pred_set
    int getSetEdgeNum(const std::vector<int> &pred_set);
    // Get vertice's index in csr[1] offset_list
	int getInIndexByID(int vid, int pred);
    // Get the number of in-neighbors of vert
	int getInSize(int vid, int pred);	
	// Get the pos-th in-neighbor of vert
	int getInVertID(int vid, int pred, int pos);
	// Get in-neighbor regardless of pred;
	int getInVertID(int vid, int pos);	
	// Get number of in-neighbors by pred_set
	int getSetInSize(int vid, const std::vector<int> &pred_set);
    // Get the total number of in-neighbors of vert
	int getTotalInSize(int vid, bool distinct);
	 // Get vertice's index in csr[0] offset_list
	int getOutIndexByID(int vid, int pred);
	 // Get the number of out-neighbors of vert
	int getOutSize(int vid, int pred);
	// Get the pos-th out-neighbor of vert
	int getOutVertID(int vid, int pred, int pos);
	// Get out-neighbor regardless of pred;
	int getOutVertID(int vid, int pos);
	// Get number of out-neighbors by pred_set
	int getSetOutSize(int vid, const std::vector<int> &pred_set);
	// Get the total number of out-neighbors of vert
	int getTotalOutSize(int vid, bool distinct);
	// Get paths string 
	std::string getPathString(std::vector<int> &path_set);
	std::string getPathString(std::vector<std::string> &path_set);
};

#endif