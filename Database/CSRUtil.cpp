/*
 * @Author: wangjian
 * @Date: 2022-02-14 16:29:27
 * @LastEditTime: 2022-02-16 16:22:05
 * @LastEditors: Please set LastEditors
 * @Description:  CSR API
 * @FilePath: /gstore/Database/CSRUtil.cpp
 */
#include "CSRUtil.h"

using namespace std;

CSRUtil::CSRUtil(CSR *_csr) {
	if (_csr)
		csr = _csr;
	else
		csr = new CSR[2];
    cacheMaxSize = 10000;
	n = -1;
	m = -1;
	srand(time(NULL));
}

CSRUtil::~CSRUtil() 
{

}

/**
 * 获取存储图的Compressed Sparse Row数据结构
 * csr[0]存储出边信息，csr[1]存储入边信息，其每个成员变量都以谓词编号为下标，不同谓词所关联的边分开存储
 * CSR类主要的成员变量包括：
 * --pre_num: 即谓词总数
 * --id2vid, vid2id: offset_list中下标与节点ID（或反之）之间的转换
 * --offset_list: 记录每个结点的邻接列表从总的邻接列表adjacency_list中的哪个下标开始
 * --adjacency_list: 总的邻接列表
 * 
 * @return CSR* 
 */
CSR* CSRUtil::getCSR() 
{
	return csr;
}

/**
 * 获取图中结点的总数
 * 第一次调用会对图中结点总数进行统计，需较长时间。后续的调用将在常数时间内完成。
 * @return int 
 */
int CSRUtil::getVertNum() 
{
    if (n != -1)
	    return n;	// Only consider static graphs for now
	set<int> vertices;
	for (int j = 0; j < 2; j++)
	{
		for (int i = 0; i < csr[j].pre_num; i++)
			vertices.insert(csr[j].adjacency_list[i].begin(), csr[j].adjacency_list[i].end());
	}
	n = vertices.size();
	return n;
}

/**
 * 获取图中边的总数
 * 第一次调用会对图中边总数进行统计，需较长时间。后续的调用将在常数时间内完成。
 * 
 * @return int 
 */
int CSRUtil::getEdgeNum()
{
    if (m != -1)
		return m;	// Only consider static graphs for now
	int ret = 0;
	for (int i = 0; i < csr[1].pre_num; i++)	// Same as summing that of csr[0]
		ret += csr[1].adjacency_list[i].size();
	m = ret;
	return m;
}

/**
 * 获取以集合中的谓词为标签的边的总数
 * 
 * @param pred_set 谓词集合
 * @return int 
 */
int CSRUtil::getSetEdgeNum(const std::vector<int> &pred_set)
{
	int ret = 0;
	for (int pred : pred_set)
		ret += csr[1].adjacency_list[pred].size();
	return ret;
}

/**
 * 获取节点在入边CSR的offset_list中对应的下标
 * 
 * @param vid 节点ID
 * @param pred 谓词编号
 * @return int 
 */
int CSRUtil::getInIndexByID(int vid, int pred)
{
	if (csr[1].vid2id[pred].find(vid) != csr[1].vid2id[pred].end())
		return csr[1].vid2id[pred][vid];
	else
		return -1;
}

/**
 * 获取节点入边所关联的邻居数
 * 
 * @param vid 节点ID
 * @param pred  谓词编号
 * @return int 
 */
int CSRUtil::getInSize(int vid, int pred)
{
	int vIndex = getInIndexByID(vid, pred);
	if (vIndex == -1)	// This vertex does not participate in this pred's relations
		return 0;
	else if (vIndex == csr[1].offset_list[pred].size() - 1)
		return csr[1].adjacency_list[pred].size() - csr[1].offset_list[pred][vIndex];
	else
		return csr[1].offset_list[pred][vIndex + 1] - csr[1].offset_list[pred][vIndex];
}

/**
 * 获取节点入边邻居ID
 * 
 * @param vid 节点ID
 * @param pred 谓词编号
 * @param pos 取第pos个邻居（邻居之间没有固定的排序标准）
 * @return int 
 */
int CSRUtil::getInVertID(int vid, int pred, int pos)
{
	if (pos >= getInSize(vid, pred))
		return -1;
	int offset = csr[1].offset_list[pred][getInIndexByID(vid, pred)];
	return csr[1].adjacency_list[pred][offset + pos];
}

/**
 * 获取节点入边邻居ID
 * 一般与获取邻居数的函数配合使用
 * 
 * @param vid 节点ID
 * @param pos 取第pos个邻居（邻居之间没有固定的排序标准）
 * @return int 
 */
int CSRUtil::getInVertID(int vid, int pos)
{
	if (distinctInEdges.find(vid) == distinctInEdges.end())
		getTotalInSize(vid, true);	// Load into cache

	if (pos < distinctInEdges[vid].size())
		return *next(distinctInEdges[vid].begin(), pos);
	else
		return -1;
}

/**
 * 获取以集合中的谓词为标签的入边所关联的邻居数（不考虑去重）
 * 
 * @param vid 节点ID
 * @param pred_set 谓词编号集合
 * @return int 
 */
int CSRUtil::getSetInSize(int vid, const std::vector<int> &pred_set)
{
	int ret = 0;
	for (int pred : pred_set)
		ret += getInSize(vid, pred);
	return ret;
}

/**
 * 获取节点总的入边数（可选是否去重）
 * 
 * @param vid 节点ID
 * @param distinct 是否去重（为真则要求去重）
 * @return int 
 */
int CSRUtil::getTotalInSize(int vid, bool distinct)
{
	int ret = 0;
	if (!distinct)
	{
		for (int i = 0; i < csr[1].pre_num; i++)
			ret += getInSize(vid, i);
	}
	else
	{
		if (distinctInEdges.find(vid) == distinctInEdges.end())
		{
			if (distinctInEdges.size() == cacheMaxSize)
			{
				int replacement = rand() % cacheMaxSize;
				distinctInEdges.erase(next(distinctInEdges.begin(), replacement));
				// cout << "distinctInEdges replaced entry " << replacement << endl;
			}

			distinctInEdges[vid] = set<int>();
			for (int pred = 0; pred < csr[1].pre_num; pred++)
			{
				int vIndex = getInIndexByID(vid, pred);
				if (vIndex == -1)	// This vertex does not participate in this pred's relations
					continue;
				else if (vIndex == csr[1].offset_list[pred].size() - 1 \
					&& csr[1].adjacency_list[pred].size() > csr[1].offset_list[pred][vIndex])
					distinctInEdges[vid].insert(next(csr[1].adjacency_list[pred].begin(), csr[1].offset_list[pred][vIndex]), \
						csr[1].adjacency_list[pred].end());
				else if (csr[1].offset_list[pred][vIndex + 1] > csr[1].offset_list[pred][vIndex])
					distinctInEdges[vid].insert(next(csr[1].adjacency_list[pred].begin(), csr[1].offset_list[pred][vIndex]), \
						next(csr[1].adjacency_list[pred].begin(), csr[1].offset_list[pred][vIndex + 1]));
			}
		}
		ret = distinctInEdges[vid].size();
	}
	return ret;
}

/**
 * 获取节点在出边CSR的offset_list中对应的下标
 * 
 * @param vid 节点ID
 * @param pred 谓词编号
 * @return int 
 */
int CSRUtil::getOutIndexByID(int vid, int pred)
{
	if (csr[0].vid2id[pred].find(vid) != csr[0].vid2id[pred].end())
		return csr[0].vid2id[pred][vid];
	else
		return -1;
}

/**
 * 获取节点出边所关联的邻居数
 * 
 * @param vid 节点ID
 * @param pred  谓词编号
 * @return int 
 */
int CSRUtil::getOutSize(int vid, int pred)
{
	int vIndex = getOutIndexByID(vid, pred);
	if (vIndex == -1)	// This vertex does not participate in this pred's relations
		return 0;
	else if (vIndex == csr[0].offset_list[pred].size() - 1)
		return csr[0].adjacency_list[pred].size() - csr[0].offset_list[pred][vIndex];
	else
		return csr[0].offset_list[pred][vIndex + 1] - csr[0].offset_list[pred][vIndex];
}

/**
 * 获取节点出边邻居ID
 * 一般与获取邻居数的函数配合使用
 * 
 * @param vid 节点ID
 * @param pred 谓词编号
 * @param pos 取第pos个邻居（邻居之间没有固定的排序标准）
 * @return int 
 */
int CSRUtil::getOutVertID(int vid, int pred, int pos)
{
	if (pos >= getOutSize(vid, pred))
		return -1;
	int offset = csr[0].offset_list[pred][getOutIndexByID(vid, pred)];
	return csr[0].adjacency_list[pred][offset + pos];
}

/**
 * 获取节点出边邻居ID
 * 一般与获取邻居数的函数配合使用
 * 
 * @param vid 节点ID
 * @param pos 取第pos个邻居（邻居之间没有固定的排序标准）
 * @return int 
 */
int CSRUtil::getOutVertID(int vid, int pos)
{
	if (distinctOutEdges.find(vid) == distinctOutEdges.end())
		getTotalOutSize(vid, true);	// Load into cache

	if (pos < distinctOutEdges[vid].size())
		return *next(distinctOutEdges[vid].begin(), pos);
	else
		return -1;
}

/**
 * 获取以集合中的谓词为标签的出边所关联的邻居数（不考虑去重）
 * 
 * @param vid 节点ID
 * @param pred_set 谓词编号集合
 * @return int 
 */
int CSRUtil::getSetOutSize(int vid, const std::vector<int> &pred_set)
{
	int ret = 0;
	for (int pred : pred_set)
		ret += getOutSize(vid, pred);
	return ret;
}

/**
 * 获取节点总的入边数（可选是否去重）
 * 
 * @param vid 节点ID
 * @param distinct 是否去重（为真则要求去重）
 * @return int 
 */
int CSRUtil::getTotalOutSize(int vid, bool distinct)
{
	int ret = 0;
	if (!distinct)
	{
		for (int i = 0; i < csr[1].pre_num; i++)
			ret += getOutSize(vid, i);
	}
	else
	{
		if (distinctOutEdges.find(vid) == distinctOutEdges.end())
		{
			if (distinctOutEdges.size() == cacheMaxSize)
			{
				int replacement = rand() % cacheMaxSize;
				distinctOutEdges.erase(next(distinctOutEdges.begin(), replacement));
				// cout << "distinctInEdges replaced entry " << replacement << endl;
			}

			distinctOutEdges[vid] = set<int>();
			for (int pred = 0; pred < csr[1].pre_num; pred++)
			{
				int vIndex = getOutIndexByID(vid, pred);
				if (vIndex == -1)	// This vertex does not participate in this pred's relations
					continue;
				else if (vIndex == csr[0].offset_list[pred].size() - 1 \
					&& csr[0].adjacency_list[pred].size() > csr[0].offset_list[pred][vIndex])
					distinctOutEdges[vid].insert(next(csr[0].adjacency_list[pred].begin(), csr[0].offset_list[pred][vIndex]), \
						csr[0].adjacency_list[pred].end());
				else if (csr[0].offset_list[pred][vIndex + 1] > csr[0].offset_list[pred][vIndex])
					distinctOutEdges[vid].insert(next(csr[0].adjacency_list[pred].begin(), csr[0].offset_list[pred][vIndex]), \
						next(csr[0].adjacency_list[pred].begin(), csr[0].offset_list[pred][vIndex + 1]));
			}
		}
		ret = distinctOutEdges[vid].size();
	}
	return ret;
}

/**
 * 路径结果集合转字符串
 * 
 * @param path_set 
 * @return std::string 
 */
std::string  CSRUtil::getPathString(std::vector<int> &path_set)
{
	stringstream ss;
	int lens = path_set.size();
    for (int i = 0; i < lens; ++i)
    {
        if (i != 0)
        {
            ss << ",";
        }
		ss << path_set[i];
	}
    return ss.str();
}

std::string  CSRUtil::getPathString(std::vector<std::string> &path_set)
{
	stringstream ss;
	int lens = path_set.size();
    for (int i = 0; i < lens; ++i)
    {
        if (i != 0)
        {
            ss << ",";
        }
		ss << path_set[i];
	}
    return ss.str();
}