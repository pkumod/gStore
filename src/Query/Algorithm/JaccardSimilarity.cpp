#include "PathQueryHandler.h"
#include <algorithm>
#include <iterator>
using namespace std;

std::vector<std::pair<int, float>> PathQueryHandler::JaccardSimilarity(int uid, const std::vector<int> &pred_sets, int k, int retNum)
{
    std::vector<std::pair<int, float>> ret;
    auto getNeighbourWithPredSet=[&](int id,const std::vector<int> &pred_set)
    {
        std::set<int> adj_ret = set<int>();
        for(auto pred : pred_set)
        {
            int inSz = getInSize(id,pred),inNei;
            int outSz = getOutSize(id,pred),outNei;
            for(int i = 0;i<inSz;i++)
            {
                inNei = getInVertID(id,pred,i);
                adj_ret.insert(inNei);
            }
            for(int i=0;i<outSz;i++)
            {
                outNei = getOutVertID(id,pred,i);
                adj_ret.insert(outNei);
            }
        }
        return adj_ret;
    };

    auto binaryInsert = [&](vector<pair<int,float>>& a,pair<int,float> insertPair)
    {
        int l=0,r=a.size();
        if(a.size()>0)
        while(l<r)
        {
            int loc = l+(r-l)/2;
            if(a[loc].second>insertPair.second)
                {
                     l = loc+1;
                }
            else
            {
                r = loc;
            }
        }          
        a.insert(a.begin()+l,insertPair);
    };
    std::set<int> uids = getNeighbourWithPredSet(uid, pred_sets);
    if (uids.size() == 0)
        return ret;
    std::vector<pair<int,float>> JS;
    std::set<int> no_used; //二跳未使用节点
    for (auto neighbour :uids)
    {
        std::set<int> comp_neis = getNeighbourWithPredSet(neighbour,pred_sets);
        float num = 0;
        float total_num = uids.size() + comp_neis.size();
        //去除比较节点
        if (uids.find(neighbour) != uids.end())
            total_num -= 1;
        for (int comp_uid : comp_neis)
        {
            //去除比较节点
            if (comp_uid == uid)
            {
                total_num -= 1;
                continue;
            }
            if (uids.find(comp_uid) == uids.end())
            {
                no_used.insert(comp_uid);
                continue;
            }
            num++;
        }
        float js = total_num - num > 0 ? num/(total_num - num):0;            
        binaryInsert(JS,pair<int,float>(neighbour,js));
    }
    if (k > 1)
    {
        for (auto neighbour :no_used)
        {
            std::set<int> comp_neis = getNeighbourWithPredSet(neighbour,pred_sets);
            float num = 0;
            float total_num = uids.size() + comp_neis.size();
            //去除比较节点
            if (uids.find(neighbour) != uids.end())
                total_num -= 1;
            for (int comp_uid : comp_neis)
            {
                //去除比较节点
                if (comp_uid == uid)
                {
                    total_num -= 1;
                    continue;
                }
                if (uids.find(comp_uid) == uids.end())
                {
                    continue;
                }
                num++;
            }
            float js = total_num - num > 0 ? num/(total_num - num):0;            
            binaryInsert(JS,pair<int,float>(neighbour,js));
        }
    }
    if (JS.size() < retNum)
    {
        vector<int> kHop = kHopNeighbor(uid,false,k,pred_sets,retNum);
        set<int> others = set<int>();
        for(auto vid:kHop)
        {
            if(uids.find(vid)==uids.end()&&no_used.find(vid)==no_used.end()&&others.find(vid)==others.end())
            {
                JS.emplace_back(pair<int,float>(vid,0));
                others.insert(vid);
            }
        }
    }
    //返回相似度最大的retNum个顶点
    for(int i=0;i<retNum&&i<JS.size();i++)
    {
        ret.emplace_back(JS[i]);
    }
    std::cout << "Jaccard 节点uid:" << uid << std::endl;
    std::cout << "Jaccard 允许出现的谓词:" << pred_sets.size() << std::endl;
    return ret;
}