#include "PathQueryHandler.h"

using namespace std;

double PathQueryHandler::degreeCorrelation(int uid, int k, const std::vector<int> &pred_sets)
{
    double ret = 0,d1=0,d2=0,d3=0,e=getSetEdgeNum(pred_sets);
    queue<int> q;
    q.push(uid);
    map<int,bool> vi;
    auto oneHopDe = [&](int id)
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
                if(vi[inNei]==false)
                {
                    int tmpD1 = getTotalInSize(id,false)+getTotalOutSize(id,false);
                    int tmpD2 = getTotalInSize(inNei,false)+getTotalOutSize(inNei,false);

                    d1+=tmpD1*tmpD2;
                    d2+=0.5*(tmpD1+tmpD2);
                    d3+=0.5*(tmpD1*tmpD1+tmpD2*tmpD2);
                }
            }
            for(int i=0;i<outSz;i++)
            {
                outNei = getOutVertID(id,pred,i);
                oneHop_ret.insert(outNei);
                if(vi[outNei]==false)
                {
                    int tmpD1 = getTotalInSize(id,false)+getTotalOutSize(id,false);
                    int tmpD2 = getTotalInSize(outNei,false)+getTotalOutSize(outNei,false);

                    d1+=tmpD1*tmpD2;
                    d2+=0.5*(tmpD1+tmpD2);
                    d3+=0.5*(tmpD1*tmpD1+tmpD2*tmpD2);
                }
            }

        }
        return oneHop_ret;
    };
    while(k!=0&&q.size())
    {
        unordered_set<int> oneHopAdj=oneHopDe(q.front());
        vi[q.front()]=true;
        q.pop();
        
        if(k>0) k--;
        if(!k) break;
        else for(auto v:oneHopAdj)
        {
            if(vi[v]==false)
            q.push(v);
        }     
    }
    ret = (pow(e,-1)*d1-pow(pow(e,-1)*d2,2))/(pow(e,-1)*d3-pow(pow(e,-1)*d2,2));
    return ret;
}