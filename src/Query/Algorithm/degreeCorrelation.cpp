#include "PathQueryHandler.h"

using namespace std;

double PathQueryHandler::degreeCorrelation(int uid, int k, const std::vector<int> &pred_sets)
{
    double ret = 0,e=getSetEdgeNum(pred_sets);
    double averageD = 0,s=0;
    queue<int> q,de;
    q.push(uid);
    unordered_map<int,bool> vi;
    unordered_map<int,int> degree;
    vector<pair<float,float>> dpair;

    auto getVDegree = [&](int id)
    {
        int sum = 0;
        for(auto pred:pred_sets)
        {
            int inSz = getInSize(id,pred);
            int outSz = getOutSize(id,pred);
            sum+=inSz+outSz;
        }
        degree[id] = sum;
        return sum;
    };
    auto oneHop = [&](int id)
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
                if(degree.find(inNei)==degree.end())
                getVDegree(inNei);
                if(vi[inNei]==false)
                {
                    dpair.emplace_back(pair<double,double>(degree[id],degree[inNei]));
                    averageD+=0.5*(degree[id]+degree[inNei]);
                }

            }
            for(int i=0;i<outSz;i++)
            {
                outNei = getOutVertID(id,pred,i);
                oneHop_ret.insert(outNei);
                if(degree.find(outNei)==degree.end())
                getVDegree(outNei);
                if(vi[outNei]==false)
                {
                    dpair.emplace_back(pair<double,double>(degree[id],degree[outNei]));
                    averageD+=0.5*(degree[id]+degree[outNei]);
                }
            }
        }
        return oneHop_ret;
    };
       
    getVDegree(uid);
    while(k!=0&&q.size())
    {
        int vid = q.front();
        unordered_set<int> oneHopAdj=oneHop(vid);
        vi[vid]=true;
        q.pop();
        for(auto v:oneHopAdj)
        {
            if(vi[v]==false)
            {              
                q.push(v);
            }
        }

        if(k>0) k--;
    }
    averageD/=dpair.size();
    for(auto v:dpair)
    {
        s+=pow((v.first+v.second)/2-averageD,2);
        ret+=(v.first-averageD)*(v.second-averageD);
    }
    ret /= s;
    return ret;
}