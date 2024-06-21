#include "PathQueryHandler.h"
#include <algorithm>
#include <iterator>
using namespace std;

std::vector<int> PathQueryHandler::JaccardSimilarity(int uid, const std::vector<int> &pred_sets, int k, int retNum)
{
    //参数uid是待计算Jaccard相似度顶点id,pred_sets是边过滤的谓词集合，k是计算uid k跳内邻居的相似度,retNum是返回相似度最高的retNum个顶点id
    std::vector<int> ret;
    std::vector<pair<int,float>> JS;
    
    std::set<int> adj_oneHop = set<int>();
    std::set<int> adj_twoHop = set<int>();
    
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
        int loc = a.size()/2,l=0,r=a.size();
        vector<pair<int,float>>::iterator it = a.begin()+loc;
        if(a.size()>0)
        while((*it).second>insertPair.second||(it!=a.begin()&&(*(it-1)).second<insertPair.second))
        {
            if((*it).second>insertPair.second)
            {
                l = loc+1;
                loc = (l+r)/2;
            }
            else
            {
                r = loc-1;
                loc = (l+r)/2;
            }
            it = a.begin()+loc;
        }        
        a.insert(it,insertPair);
    };
    if(k>0||k<0)
    {   
        //获取uid的一阶邻居，包括入边邻居和出边邻居 
        adj_oneHop = getNeighbourWithPredSet(uid,pred_sets);
        //对于每个一阶邻居，计算它们的一阶邻居与uid的一阶邻居的交集，用于计算相似度，同时不在交集内的则加入到uid的二阶邻居
        for(auto neighbour:adj_oneHop)
        {
            std::set<int> _adj_oneHop = set<int>();
            _adj_oneHop = getNeighbourWithPredSet(neighbour,pred_sets);
            int isa = 0;
            for(auto v:_adj_oneHop)
            {
                if(adj_oneHop.find(v)!=adj_oneHop.end())
                    isa++;
                else if(adj_twoHop.find(v)==adj_twoHop.end())
                    adj_twoHop.insert(v);
            }
            float js = (float)(isa)/(_adj_oneHop.size()+adj_oneHop.size()-isa-2);            
            binaryInsert(JS,pair<int,float>(neighbour,js));
        }
    }
    if(k>1||k<0)
    {
        //对于二阶邻居计算交集和相似度
        for(auto neighbour:adj_twoHop)
        {
            std::set<int> _adj_oneHop = set<int>();
            _adj_oneHop = getNeighbourWithPredSet(neighbour,pred_sets);
            int isa = 0;
            for(auto v:_adj_oneHop)
            {
                if(adj_oneHop.find(v)!=adj_oneHop.end())
                    isa++;
            }
            float js = (float)(isa)/(_adj_oneHop.size()+adj_oneHop.size()-isa-2);
            vector<pair<int,float>>::iterator it = JS.begin();
            binaryInsert(JS,pair<int,float>(neighbour,js));
        }
    }
    set<int> others = set<int>();
    //如果顶点在uid两跳范围外，则它们的相似度一定为0
    if(retNum>JS.size())
    {
        if(k<0)
        {
            int j=0;
            while (retNum>JS.size()&&j<2)
            {
                for(auto pred=0;csr[j].pre_num;pred++)
                {
                    for(int i=0;i<csr[j].id2vid[pred].size();i++)
                    {
                        auto vid = csr[j].id2vid[pred][i];
                        if(adj_oneHop.find(vid)==adj_oneHop.end()&&adj_twoHop.find(vid)==adj_twoHop.end()&&others.find(vid)==others.end())
                        {
                            JS.emplace_back(pair<int,float>(vid,0));
                            others.insert(vid);
                            if(retNum<=JS.size())
                                break;
                        }
                    }
                    if(retNum<=JS.size())
                        break;
                }
                j++;
            }
        }
        else
       {
            vector<int> kHop = kHopNeighbor(uid,false,k,pred_sets,retNum);
            for(auto vid:kHop)
            {
                if(adj_oneHop.find(vid)==adj_oneHop.end()&&adj_twoHop.find(vid)==adj_twoHop.end()&&others.find(vid)==others.end())
                {
                    JS.emplace_back(pair<int,float>(vid,0));
                    others.insert(vid);
                }
            }
       }
    }
    //返回相似度最大的retNum个顶点
    for(int i=0;i<retNum&&i<JS.size();i++)
    {
        ret.emplace_back(JS[i].first);
    }
    std::cout << "Jaccard 节点uid:" << uid << std::endl;
    std::cout << "Jaccard 允许出现的谓词:" << pred_sets.size() << std::endl;
    return ret;
}