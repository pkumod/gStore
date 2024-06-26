#include "PathQueryHandler.h"

using namespace std;

double PathQueryHandler::degreeCorrelation(int uid, int k, const std::vector<int> &pred_sets)
{
    double ret = 0,e=getSetEdgeNum(pred_sets);
    double averageD1 = 0,averageD2=0,s1=0,s2=0;
    queue<int> q,de;
    q.push(uid);
    unordered_map<int,bool> vi;
    unordered_map<int,int> degree;
    vector<pair<float,float>> dpair;
    unordered_set<int> Set;
    auto getVDegree = [&](int id,int K)
    {
        int sum = 0;
        for(auto pred:pred_sets)
        {
            int inSz = getInSize(id,pred),inNei;
            int outSz = getOutSize(id,pred),outNei;
            if(K==0)
            {
                for(int i=0;i<inSz;i++)
                {
                    inNei = getInVertID(id,pred,i);
                    if(Set.find(inNei)!=Set.end())
                        sum++;
                }
                for(int i=0;i<outSz;i++)
                {
                    outNei = getOutVertID(id,pred,i);
                    if(Set.find(outNei)!=Set.end())
                        sum++;
                }
            }
            else
            sum+=inSz+outSz;
        }
        degree[id] = sum;
        return sum;
    };
    auto oneHop = [&](int id,int K)
    {
        unordered_set<int> oneHop_ret = unordered_set<int>();
        for(auto pred:pred_sets)
        {
            int inSz = getInSize(id,pred),inNei;
            int outSz = getOutSize(id,pred),outNei;

            for(int i=0;i<inSz;i++)
            {
                inNei = getInVertID(id,pred,i);
                oneHop_ret.insert(inNei);
                if(vi[inNei]==false&&(K!=0||(Set.find(inNei)!=Set.end())))
                {
                    dpair.emplace_back(pair<double,double>(inNei,id));
                }

            }
            for(int i=0;i<outSz;i++)
            {
                outNei = getOutVertID(id,pred,i);
                oneHop_ret.insert(outNei);
                if(vi[outNei]==false&&(K!=0||(Set.find(outNei)!=Set.end())))
                {
                    dpair.emplace_back(pair<double,double>(id,outNei));
                }
            }
        }
        return oneHop_ret;
    };
    Set.insert(uid);  
    while(q.size())
    {
        int vid = q.front();
        getVDegree(vid,k);
        unordered_set<int> oneHopAdj=oneHop(vid,k);
        vi[vid]=true;
        q.pop();
        if(k!=0)
        for(auto v:oneHopAdj)
        {
            if(Set.find(v)==Set.end())
            {              
                q.push(v);
                Set.insert(v);
            }
        }
        if(k>0) k--;
    }
    for(auto v:dpair)
    {
        averageD1 += degree[v.first];
        averageD2 += degree[v.second];
    }
    averageD1/=dpair.size();averageD2/=dpair.size();
    for(auto v:dpair)
    {        
        s1+=pow(degree[v.first]-averageD1,2);s2+=pow(degree[v.second]-averageD2,2);
        ret+=(degree[v.first]-averageD1)*(degree[v.second]-averageD2);
    }
    s1 = pow(s1,0.5);s2=pow(s2,0.5);
    if(s1*s2==0)
        ret=0;
    else
        ret /= (s1*s2);
    return ret;
}