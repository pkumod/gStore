#include "PathQueryHandler.h"

using namespace std;

/**
	lovain

	@param phase1_loop_num 第一阶段最大迭代轮数(>=1) .
	@param min_modularity_increase 模块度增益阈值(0~1) .
	@param pred_set 谓词集合（无该属性的边不参与计算）.
	@param directed 图是否存在方向（为false时认为是无向图）
	@return 社区数.
*/
void PathQueryHandler::louvain(int phase1_loop_num,float min_modularity_increase,std::vector<int> &pred_set,bool directed, std::pair<size_t, std::set<std::set<int> > > &result){
	//所有节点id
  	set<int> vids;
	//当前存在的节点/超节点,以及存在于哪个社区
	map<int,int> super_nodes;
	//超节点对应的真实vid节点
	map<int,set<int> > super2vid;
	//当前的社区
	map<int,set<int> > community;
	//所有从节点i指其他节点的权重（包括自己）（出边）
	map<int,map<int,double> > weights;
	//首先，遍历满足谓词集合的点和边，初始化上面的变量
	for (int pred : pred_set){
		vids.insert(csr[1].adjacency_list[pred].begin(), csr[1].adjacency_list[pred].end());
	}
	for(int vid:vids){
		super_nodes[vid]=vid;
		super2vid[vid]=set<int> ();
		super2vid[vid].insert(vid);
		community[vid]=set<int> ();
		community[vid].insert(vid);
		weights[vid]=map<int,double>();
	}
	//有向图所有边的权值和，即公式中的2m
	double m=0;
	// 初始化k_i_in
	for(int pred:pred_set){
		for(int vid:vids){
			int vIndex = getInIndexByID(vid, pred);
			if(vIndex!=-1){
				int begin=csr[1].offset_list[pred][vIndex];
				int end=0;
				// 如果是offset_list中的最后一个
				if(vIndex == csr[1].offset_list[pred].size() - 1){
					end=csr[1].adjacency_list[pred].size();
				}else{
					end=csr[1].offset_list[pred][vIndex+1];
				}
				for(int i=begin;i<end;i++){
					int out_vid=csr[1].adjacency_list[pred][i];
					if(weights[out_vid].find(vid)==weights[out_vid].end()){
						weights[out_vid][vid]=1;
					}else{
						weights[out_vid][vid]+=1;
					}
					m++;
					// 如果是双向的
					if(!directed){
						if(weights[vid].find(out_vid)==weights[vid].end()){
						weights[vid][out_vid]=1;
					}else{
						weights[vid][out_vid]+=1;
					}
					m++;
					}
				}
			}
		}
	}

	bool finished=false;
	while (!finished)
	{
		//第一阶段
		bool phase1_finished=true;
		//社区C的内部的所有边的权值和
		map<int,double> s_in;
		//所有指向区域C中的节点的边的权值和
		map<int,double> s_tot;
		//计算s_in,s_out
		for(auto weight:weights){
			int out_id=weight.first;
			//出发点所在社区
			int out_community=super_nodes[out_id];
			for(auto in_node:weight.second){
				int in_id=in_node.first;
				//入点所在社区
				int in_community=super_nodes[in_id];
				if(in_community==out_community){
					if(s_in.find(in_community)==s_in.end()){
						s_in[in_community]=in_node.second;
					}else{
						s_in[in_community]+=in_node.second;
					}
					//s_tot
					if(s_tot.find(in_community)==s_tot.end()){
						s_tot[in_community]=in_node.second;
					}else{
						s_tot[in_community]=in_node.second;
					}
				}else{
					if(s_tot.find(in_community)==s_tot.end()){
						s_tot[in_community]=in_node.second;
					}else{
						s_tot[in_community]=in_node.second;
					}
				}
			}
		}
		
		for(int phase1_loop=0;phase1_loop<phase1_loop_num;phase1_loop++){
			phase1_finished=true;
			//遍历超节点
			//super_node.first-vid super_node.second-community
			for(auto super_node:super_nodes){
				//目前最大的deta_q
				double max_deta_q=min_modularity_increase;
				//所有从节点i指向区域C的边的权值和
				double max_k_i_in=0;
				//所有从区域C指向节点i的边的权值和
				double max_k_i_out=0;
				//指向节点i的所有边的权值和
				double k_i=0;
				//最大deta_q所在社区
				int max_community=-1;
				//计算k_i
				for(auto weight:weights){
					if(weight.second.find(super_node.first)!=weight.second.end()){
						k_i+=weight.second[super_node.first];
					}
				}
				//com.first-community com.second-vids
				for(auto com:community){
					//不是super_node所在的社区
					if(com.first!=super_node.second){
						double k_i_in=0;
						double k_i_out=0;
						for(auto node:com.second){
							//社区中存在指向i的点
							if(weights[node].find(super_node.first)!=weights[node].end()){
								k_i_out+=weights[node][super_node.first];
							}
							//节点i指向社区中的点
							if(weights[super_node.first].find(node)!=weights[super_node.first].end()){
								k_i_in+=weights[super_node.first][node];
							}
						}
						double deta_q=(k_i_in-s_tot[com.first]*k_i*2/m)/m;
						if(deta_q>max_deta_q){
							max_deta_q=deta_q;
							max_k_i_in=k_i_in;
							max_k_i_out=k_i_out;
							max_community=com.first;
						}
					}
				}

				//如果存在可以并入的社区
				if(max_community!=-1){
					phase1_finished=false;
					//旧社区id
					int old_community=super_node.second;
					//更新super_nodes community s_in s_tot
					//i自环
					int i_to_i=0;
					if(weights[super_node.first].find(super_node.first)!=weights[super_node.first].end()){
						i_to_i=weights[super_node.first][super_node.first];
					}
					community[old_community].erase(super_node.first);
					//如果转移之后旧社区中还存在点，就需要更新旧社区的community s_in s_tot
					if(community[old_community].size()!=0){
						//分别是i指向旧社区中点的，旧社区中点指向i的，不包括i自环
						int i_to_old=0;
						int old_to_i=0;
						for(auto node:community[old_community]){
							if(weights[super_node.first].find(node)!=weights[super_node.first].end()){
								i_to_old+=weights[super_node.first][node];
							}
							if(weights[node].find(super_node.first)!=weights[node].end()){
								old_to_i+=weights[node][super_node.first];
							}
						}
						s_in[old_community]=s_in[old_community]-i_to_old-old_to_i-i_to_i;
						s_tot[old_community]=s_tot[old_community]-k_i;
					}else{
						s_in.erase(old_community);
						s_tot.erase(old_community);
						community.erase(old_community);

					}
					//分别是i指向新社区中点的，新社区中点指向i的，不包括i自环
					int i_to_new=0;
					int new_to_i=0;
					for(auto node:community[max_community]){
						if(weights[super_node.first].find(node)!=weights[super_node.first].end()){
							i_to_new+=weights[super_node.first][node];
						}
						if(weights[node].find(super_node.first)!=weights[node].end()){
							new_to_i+=weights[node][super_node.first];
						}
					}
					s_in[max_community]=s_in[max_community]+i_to_new+new_to_i+i_to_i;
					s_tot[max_community]=s_tot[max_community]+k_i;
					community[max_community].insert(super_node.first);
					super_nodes[super_node.first]=max_community;
				}
			}
			if(phase1_finished)break;
		}
		//判断是否结束
		finished=true;
		//如果社区中存在超过一个的点
		for(auto com:community){
			if(com.second.size()>1){
				finished=false;
				break;
			}
		}
		if(finished)break;
		//第二阶段,更新super_nodes，super2vid，community，weights
		map<int,int> new_super_nodes;
		map<int,set<int> > new_community;
		map<int,map<int,double> > new_weights;
		map<int,set<int> > new_super2vid;
		for(auto com:community){
			new_super_nodes[com.first]=com.first;
			new_community[com.first]=set<int> ();
			new_community[com.first].insert(com.first);
			new_weights[com.first]=map<int,double> ();
			new_super2vid[com.first]=set<int> ();
			//超节点的自环
			new_weights[com.first][com.first]=s_in[com.first];
			//更新super2vid
			for(auto node:com.second){
				new_super2vid[com.first].insert(super2vid[node].begin(),super2vid[node].end());
			}
		}
		for(auto weight:weights){
			int out_id=weight.first;
			int out_community=super_nodes[out_id];
			for(auto in_node:weight.second){
				int in_id=in_node.first;
				int in_community=super_nodes[in_id];
				if(in_community!=out_community){
					if(new_weights[out_community].find(in_community)==new_weights[out_community].end()){
						new_weights[out_community][in_community]=weights[out_id][in_id];
					}else{
						new_weights[out_community][in_community]+=weights[out_id][in_id];
					}
				}
			}
		}
		super_nodes=new_super_nodes;
		community=new_community;
		weights=new_weights;
		super2vid=new_super2vid;
	}
	
	result.first=super2vid.size();
	for(auto v:super2vid)
	{
		result.second.insert(v.second);
	}
}