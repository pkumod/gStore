/*=============================================================================
# Filename: gquery.cpp
# Author: Bookug Lobert
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-20 12:23
# Description: query a database, there are several ways to use this program:
1. ./gquery                                        print the help message
2. ./gquery --help                                 simplified as -h, equal to 1
3. ./gquery db_folder query_path                   load query from given path fro given database
4. ./gquery db_folder                              load the given database and open console
=============================================================================*/

//#include <iostream>
#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "../Database/Database.h"
#include "../Util/Util.h"
#include <mpi.h>
#include <algorithm>
#include <queue>

using namespace std;

struct Config{
	string db_folder;
};

struct PPPartialRes{
	vector<int> MatchVec;
	vector<char> TagVec;
	int FragmentID;
	int ID;
};

struct PPPartialResVec{
	vector<PPPartialRes> PartialResList;
	int match_pos;
};

struct CrossingEdgeMapping{
	int head_query_id;
	int tail_query_id;
	string mapping_str;
	int fragmentID;
};

struct CrossingEdgeMappingVec{
	int tag;
	vector< vector<CrossingEdgeMapping> > CrossingEdgeMappings;
};

bool operator<(const CrossingEdgeMapping& node1, const CrossingEdgeMapping& node2)
{
	if(node1.head_query_id < node2.head_query_id)
		return true;
	if(node1.head_query_id == node2.head_query_id && node1.tail_query_id < node2.tail_query_id)
		return true;
	if(node1.head_query_id == node2.head_query_id && node1.tail_query_id == node2.tail_query_id && node1.mapping_str.compare(node2.mapping_str) < 0)
		return true;

	return false;
}

bool operator==(CrossingEdgeMapping& node1, CrossingEdgeMapping& node2)
{
	if(node1.head_query_id == node2.head_query_id && node1.tail_query_id == node2.tail_query_id && node1.mapping_str.compare(node2.mapping_str) == 0)
		return true;

	return false;
}

//WARN:cannot support soft links!
Config
loadConfig(const char* _file_path)
{
#ifdef DEBUG_PRECISE
    printf("file to open: %s\n", _file_path);
#endif
    string s;
	Config _config;
	ifstream fin(_file_path);	
    if(!fin)
    {
        printf("can not open: %s\n", _file_path);
		exit(1);
    }else{		
		getline(fin,s);
		_config.db_folder = s;
	}

    return _config;
}

vector<string> split(string textline, string tag){
	vector<string> res;
	std::size_t pre_pos = 0;
	std::size_t pos = textline.find_first_of(tag);
	while (pos != std::string::npos){
		string curStr = textline.substr(pre_pos, pos - pre_pos);
		curStr.erase(0, curStr.find_first_not_of("\r\t\n "));
		curStr.erase(curStr.find_last_not_of("\r\t\n ") + 1);
		if(strcmp(curStr.c_str(), "") != 0)
			res.push_back(curStr);
		pre_pos = pos + 1;
		pos = textline.find_first_of(tag, pre_pos);
	}

	string curStr = textline.substr(pre_pos, pos - pre_pos);
	curStr.erase(0, curStr.find_first_not_of("\r\t\n "));
	curStr.erase(curStr.find_last_not_of("\r\t\n ") + 1);
	if(strcmp(curStr.c_str(), "") != 0)
		res.push_back(curStr);

	return res;
}

void QuickSort(vector< vector<CrossingEdgeMapping> >& res1, int matchPos, int low, int high){
	if(low >= high){
		return;
	}
	int first = low;
	int last = high;
	/*用字表的第一个记录作为枢轴*/
	vector<CrossingEdgeMapping> key;
	key.assign(res1[first].begin(), res1[first].end());

	while(first < last){
		while(first < last && res1[last][matchPos].mapping_str.compare(key[matchPos].mapping_str) >= 0){
			--last;
		}
		/*将比第一个小的移到低端*/
		res1[first].assign(res1[last].begin(), res1[last].end());
		while(first < last && res1[first][matchPos].mapping_str.compare(key[matchPos].mapping_str) <= 0){
			++first;
		}

		res1[last].assign(res1[first].begin(), res1[first].end());
		/*将比第一个大的移到高端*/
	}
	/*枢轴记录到位*/
	res1[first].assign(key.begin(), key.end());
	
	QuickSort(res1, matchPos, low, first-1);
	QuickSort(res1, matchPos, first+1, high);
}

int isFinalResult(PPPartialRes curPPPartialRes){
	for(int i = 0; i < curPPPartialRes.TagVec.size(); i++){
		if('1' != curPPPartialRes.TagVec[i])
			return 0;
	}
	return 1;
}

bool myfunction0(PPPartialResVec v1, PPPartialResVec v2){
	if(v1.PartialResList.size() != v2.PartialResList.size())
		return (v1.PartialResList.size() < v2.PartialResList.size());
	return (v1.match_pos < v2.match_pos);
}

int checkJoinable(CrossingEdgeMappingVec& vec1, CrossingEdgeMappingVec& vec2){
	if(vec1.tag & vec2.tag){
		return 0;
	}
	
	for(int i = 0; i < vec1.CrossingEdgeMappings[0].size(); i++){
		for(int j = 0; j < vec2.CrossingEdgeMappings[0].size(); j++){
			if(vec1.CrossingEdgeMappings[0][i].head_query_id == vec2.CrossingEdgeMappings[0][j].head_query_id && vec1.CrossingEdgeMappings[0][i].tail_query_id == vec2.CrossingEdgeMappings[0][j].tail_query_id){
				return 1;
			}
		}
	}
	
	return 0;
}

void LECFJoin(CrossingEdgeMappingVec& final_res, CrossingEdgeMappingVec& res1, CrossingEdgeMappingVec& res2){
	if(res1.CrossingEdgeMappings.size() == 0){
		return ;
	}
	
	vector< pair<int, int> > matchPos;
	set<int> second_match_pos;
	for(int i = 0; i < res1.CrossingEdgeMappings[0].size(); i++){
		for(int j = 0; j < res2.CrossingEdgeMappings[0].size(); j++){
			
			//printf("possible match position:<%d, %d> = %s and <%d, %d> = %s\n", res1.CrossingEdgeMappings[0][i].head_query_id, res1.CrossingEdgeMappings[0][i].tail_query_id, res1.CrossingEdgeMappings[0][i].mapping_str.c_str(), res2.CrossingEdgeMappings[0][j].head_query_id, res2.CrossingEdgeMappings[0][j].tail_query_id, res2.CrossingEdgeMappings[0][j].mapping_str.c_str());
			
			if(res1.CrossingEdgeMappings[0][i].head_query_id == res2.CrossingEdgeMappings[0][j].head_query_id && res1.CrossingEdgeMappings[0][i].tail_query_id == res2.CrossingEdgeMappings[0][j].tail_query_id){
				pair<int, int> tmp_pair;
				matchPos.push_back(tmp_pair);
				matchPos[matchPos.size() - 1].first = i;
				matchPos[matchPos.size() - 1].second = j;
				second_match_pos.insert(j);
			}
		}
	}
	
	//printf("%d match positions, the last one is <%d, %d> \n", matchPos.size(), matchPos[matchPos.size() - 1].first, matchPos[matchPos.size() - 1].second);
	
	QuickSort(res1.CrossingEdgeMappings, matchPos[0].first, 0, res1.CrossingEdgeMappings.size() - 1);
	QuickSort(res2.CrossingEdgeMappings, matchPos[0].second, 0, res2.CrossingEdgeMappings.size() - 1);
	/*
	stringstream crossing_ss;
	crossing_ss << "............... res1 ..............." << endl;
	for(int i = 0; i < res1.CrossingEdgeMappings.size(); i++){
		for(int j = 0; j < res1.CrossingEdgeMappings[i].size(); j++){
			crossing_ss << res1.CrossingEdgeMappings[i][j].mapping_str << "\t";
		}
		crossing_ss << endl;
	}
	
	crossing_ss << "............... res2 ..............." << endl;
	for(int i = 0; i < res2.CrossingEdgeMappings.size(); i++){
		for(int j = 0; j < res2.CrossingEdgeMappings[i].size(); j++){
			crossing_ss << res2.CrossingEdgeMappings[i][j].mapping_str << "\t";
		}
		crossing_ss << endl;
	}
	printf("%s\n", crossing_ss.str().c_str());
*/
	final_res.tag = res1.tag | res2.tag;
	int l_iter = 0, r_iter = 0, tag = 0;
	while (l_iter < res1.CrossingEdgeMappings.size() && r_iter < res2.CrossingEdgeMappings.size()) {

		//cout << "l_iter = " << l_iter << ", r_iter = " << r_iter << endl;
		if (l_iter == res1.CrossingEdgeMappings.size()) {
			r_iter++;
		} else if (r_iter == res2.CrossingEdgeMappings.size()) {
			l_iter++;
		} else if (res1.CrossingEdgeMappings[l_iter][matchPos[0].first].mapping_str.compare(res2.CrossingEdgeMappings[r_iter][matchPos[0].second].mapping_str) < 0) {
			l_iter++;
		} else if (res1.CrossingEdgeMappings[l_iter][matchPos[0].first].mapping_str.compare(res2.CrossingEdgeMappings[r_iter][matchPos[0].second].mapping_str) > 0) {
			r_iter++;
		} else {
			int l_iter_end = l_iter + 1, r_iter_end = r_iter + 1;

			while (l_iter_end < res1.CrossingEdgeMappings.size()
					&& res1.CrossingEdgeMappings[l_iter_end][matchPos[0].first].mapping_str.compare(res1.CrossingEdgeMappings[l_iter][matchPos[0].first].mapping_str) == 0) {
				l_iter_end++;
				if (l_iter_end == res1.CrossingEdgeMappings.size()) {
					break;
				}
			}
			while (r_iter_end < res2.CrossingEdgeMappings.size()
					&& res2.CrossingEdgeMappings[r_iter_end][matchPos[0].second].mapping_str.compare(res2.CrossingEdgeMappings[r_iter][matchPos[0].second].mapping_str) == 0) {
				r_iter_end++;
				if (r_iter_end == res2.CrossingEdgeMappings.size()) {
					break;
				}
			}

			for (int i = l_iter; i < l_iter_end; i++) {
				for (int j = r_iter; j < r_iter_end; j++) {
					if(res1.CrossingEdgeMappings[i][matchPos[0].first].fragmentID == res2.CrossingEdgeMappings[j][matchPos[0].second].fragmentID)
						continue;
						
					tag = 0;
					vector<CrossingEdgeMapping> curCrossingEdgeMappingVec;
					int first_match_pos = 0;
					for(int k = 0; k < res1.CrossingEdgeMappings[i].size(); k++){
						if(first_match_pos >= matchPos.size() || k != matchPos[first_match_pos].first){
							curCrossingEdgeMappingVec.push_back(res1.CrossingEdgeMappings[i][k]);
							continue;
						}
						if(res1.CrossingEdgeMappings[i][matchPos[first_match_pos].first].mapping_str.compare(res2.CrossingEdgeMappings[j][matchPos[first_match_pos].second].mapping_str) != 0){

							tag = 1;
							break;
						}
						
						//printf("%d join position %d %d = %s and %s\n", first_match_pos, i, j, res1.CrossingEdgeMappings[i][matchPos[first_match_pos].first].mapping_str.c_str(), res2.CrossingEdgeMappings[j][matchPos[first_match_pos].second].mapping_str.c_str());
						
						first_match_pos++;
					}
					
					if(tag == 1)
						continue;
					
					for(int k = 0; k < res2.CrossingEdgeMappings[j].size(); k++){
						if(second_match_pos.count(k) == 0){
							curCrossingEdgeMappingVec.push_back(res2.CrossingEdgeMappings[j][k]);
							continue;
						}
					}
					
					final_res.CrossingEdgeMappings.push_back(curCrossingEdgeMappingVec);
				}
			}

			r_iter = r_iter_end;
			l_iter = l_iter_end;
		}
	}
	/*
	crossing_ss.str("");
	crossing_ss << "............... final_res with tag " << final_res.tag << " ..............." << endl;
	for(int i = 0; i < final_res.CrossingEdgeMappings.size(); i++){
		for(int j = 0; j < final_res.CrossingEdgeMappings[i].size(); j++){
			crossing_ss << final_res.CrossingEdgeMappings[i][j].head_query_id << "\t" << final_res.CrossingEdgeMappings[i][j].tail_query_id << "\t" << final_res.CrossingEdgeMappings[i][j].mapping_str << "\t";
		}
		crossing_ss << endl;
	}
	printf("%s\n", crossing_ss.str().c_str());
	*/
}

void HashLECFJoin(CrossingEdgeMappingVec& final_res, CrossingEdgeMappingVec& res1, CrossingEdgeMappingVec& res2){
	if(res1.CrossingEdgeMappings.size() == 0){
		return ;
	}

	vector< pair<int, int> > matchPos;
	set<int> second_match_pos;
	for(int i = 0; i < res1.CrossingEdgeMappings[0].size(); i++){
		for(int j = 0; j < res2.CrossingEdgeMappings[0].size(); j++){
			
			if(res1.CrossingEdgeMappings[0][i].head_query_id == res2.CrossingEdgeMappings[0][j].head_query_id && res1.CrossingEdgeMappings[0][i].tail_query_id == res2.CrossingEdgeMappings[0][j].tail_query_id){
				pair<int, int> tmp_pair;
				matchPos.push_back(tmp_pair);
				matchPos[matchPos.size() - 1].first = i;
				matchPos[matchPos.size() - 1].second = j;
				second_match_pos.insert(j);
			}
		}
	}
	//printf("~~~%d\n", matchPos.size());
	
	map<CrossingEdgeMapping, CrossingEdgeMappingVec> edge_LECF_map;
	for(int i = 0; i < res2.CrossingEdgeMappings.size(); i++){
		if(edge_LECF_map.count(res2.CrossingEdgeMappings[i][matchPos[0].second]) == 0){
			CrossingEdgeMappingVec tmpCrossingEdgeMappingVec;
			edge_LECF_map.insert(make_pair(res2.CrossingEdgeMappings[i][matchPos[0].second], tmpCrossingEdgeMappingVec));
		}
		edge_LECF_map[res2.CrossingEdgeMappings[i][matchPos[0].second]].CrossingEdgeMappings.push_back(res2.CrossingEdgeMappings[i]);
	}
	
	final_res.tag = res1.tag | res2.tag;
	int tag = 0;
	//printf("~~~%d\n", final_res.tag);
	
	for(int i = 0; i < res1.CrossingEdgeMappings.size(); i++){
		if(edge_LECF_map.count(res1.CrossingEdgeMappings[i][matchPos[0].first]) == 0)
			continue;
			
		CrossingEdgeMappingVec tmpCrossingEdgeMappingVec = edge_LECF_map[res1.CrossingEdgeMappings[i][matchPos[0].first]];
		
		for(int j = 0; j < tmpCrossingEdgeMappingVec.CrossingEdgeMappings.size(); j++){
		
			//printf("%d %d = %d %d\n", res1.tag, res2.tag, i, j);
		
			if(res1.CrossingEdgeMappings[i][matchPos[0].first].fragmentID == tmpCrossingEdgeMappingVec.CrossingEdgeMappings[j][matchPos[0].second].fragmentID)
				continue;
				
			tag = 0;
			vector<CrossingEdgeMapping> curCrossingEdgeMappingVec;
			int first_match_pos = 0;
			for(int k = 0; k < res1.CrossingEdgeMappings[i].size(); k++){
				if(first_match_pos >= matchPos.size() || k != matchPos[first_match_pos].first){
					curCrossingEdgeMappingVec.push_back(res1.CrossingEdgeMappings[i][k]);
					continue;
				}
				if(res1.CrossingEdgeMappings[i][matchPos[first_match_pos].first].mapping_str.compare(tmpCrossingEdgeMappingVec.CrossingEdgeMappings[j][matchPos[first_match_pos].second].mapping_str) != 0){

					tag = 1;
					break;
				}
				
				//printf("%d join position %d %d = %s and %s\n", first_match_pos, i, j, res1.CrossingEdgeMappings[i][matchPos[first_match_pos].first].mapping_str.c_str(), res2.CrossingEdgeMappings[j][matchPos[first_match_pos].second].mapping_str.c_str());
				
				first_match_pos++;
			}
			
			if(tag == 1)
				continue;
			
			for(int k = 0; k < tmpCrossingEdgeMappingVec.CrossingEdgeMappings[j].size(); k++){
				if(second_match_pos.count(k) == 0){
					curCrossingEdgeMappingVec.push_back(tmpCrossingEdgeMappingVec.CrossingEdgeMappings[j][k]);
				}
			}
			
			final_res.CrossingEdgeMappings.push_back(curCrossingEdgeMappingVec);
		}
	}
	//printf("+++--- %d \n", final_res.CrossingEdgeMappings.size());
}

//WARN:cannot support soft links!
string
getQueryFromFile(const char* _file_path)
{
#ifdef DEBUG_PRECISE
    printf("file to open: %s\n", _file_path);
#endif
    char buf[10000];
    std::string query_file;

    ifstream fin(_file_path);
    if(!fin)
    {
        printf("can not open: %s\n", _file_path);
        return "";
    }

    memset(buf, 0, sizeof(buf));
    stringstream _ss;
    while(!fin.eof())
    {
        fin.getline(buf, 9999);
        _ss << buf << "\n";
    }
    fin.close();

    return _ss.str();
}

void
help()
{
    printf("\
			/*=============================================================================\n\
# Filename: gquery.cpp\n\
# Author: Bookug Lobert\n\
# Mail: 1181955272@qq.com\n\
# Last Modified: 2015-10-20 12:23\n\
# Description: query a database, there are several ways to use this program:\n\
1. ./gquery                                        print the help message\n\
2. ./gquery --help                                 simplified as -h, equal to 1\n\
3. ./gquery db_folder query_path                   load query from given path fro given database\n\
4. ./gquery db_folder                              load the given database and open console\n\
=============================================================================*/\n");
}

int
main(int argc, char * argv[])
{
#ifdef DEBUG
    Util util;
#endif
/*
    if(argc == 1 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
    {
        help();
        return 0;
    }

    cout << "gquery..." << endl;
*/
    if(argc < 2)
    {
        cerr << "error: lack of DB_store to be queried" << endl;
        return 0;
    }
	
	int myRank, p, size, i, j, k, l = 0;
	double partialResStart, partialResEnd;
	char* queryCharArr;
	char* partialResArr;
	MPI_Status status;
	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD,&myRank);
	MPI_Comm_size(MPI_COMM_WORLD,&p);
	
	if(myRank == 0) {
		double schedulingStart, schedulingEnd;
		
		string _query_str = getQueryFromFile(argv[2]);
		queryCharArr = new char[1024];
		strcpy(queryCharArr, _query_str.c_str());
		size = strlen(queryCharArr);
		cout << "query : " << queryCharArr << endl;

		for(i = 1; i < p; i++){
			MPI_Send(&size, 1, MPI_INT, i, 10, MPI_COMM_WORLD);
			MPI_Send(queryCharArr, size, MPI_CHAR, i, 10, MPI_COMM_WORLD);
		}
		printf("Query has been sent!\n");
		partialResStart = MPI_Wtime();
		
		int PPQueryVertexCount = 0, fullTag = 0, sizeSum = 0, partialResNum = 0;
		for(int pInt = 1; pInt < p; pInt++){
			MPI_Recv(&PPQueryVertexCount, 1, MPI_INT, pInt, 10, MPI_COMM_WORLD, &status);
		}
		
		ofstream log_output("log_ask.txt");
		
		fullTag = (1 << PPQueryVertexCount) - 1;
		printf("fullTag = %d\n", fullTag);
		vector<CrossingEdgeMappingVec> intermediate_results_vec(fullTag + 1);
		for(int i = 0; i < intermediate_results_vec.size(); i++){
			intermediate_results_vec[i].tag = i;
		}
		for(int pInt = 1; pInt < p; pInt++){
			MPI_Recv(&size, 1, MPI_INT, pInt, 10, MPI_COMM_WORLD, &status);
			sizeSum += size;
			partialResArr = new char[size + 3];
			MPI_Recv(partialResArr, size, MPI_CHAR, pInt, 10, MPI_COMM_WORLD, &status);
			partialResArr[size] = 0;
			
			//log_output << "++++++++++++ " << pInt << " ++++++++++++" << endl;
			//log_output << partialResArr << endl;
			
			string textline(partialResArr);
			vector<string> resVec = split(textline, "\n");
			partialResNum += resVec.size();
			//printf("processs %d : %s\n", pInt, resVec[0].c_str());
			for(i = 0; i < resVec.size(); i++){
				//curPartialResSize = resVec[i].length();
				vector<string> matchVec = split(resVec[i], "\t");
				if((matchVec.size() % 4) != 1)
					continue;
				
				l = 0;
				for(k = 0; k < matchVec[matchVec.size() - 1].size(); k++)
				{
					l = l * 2 + matchVec[matchVec.size() - 1].at(k) - '0';
				}
				
				//printf("%s %d maps to tag %d\n", matchVec[matchVec.size() - 1].c_str(), pInt, l); 
				vector<CrossingEdgeMapping> tmp_mapping_vec;
				for(j = matchVec.size() - 2; j >= 0; j -= 4){
					CrossingEdgeMapping curCrossingEdgeMapping;
					curCrossingEdgeMapping.tail_query_id = atoi(matchVec[j - 2].c_str());
					curCrossingEdgeMapping.head_query_id = atoi(matchVec[j - 3].c_str());
					curCrossingEdgeMapping.mapping_str = matchVec[j] + "\t" + matchVec[j - 1];
					curCrossingEdgeMapping.fragmentID = pInt;
					tmp_mapping_vec.push_back(curCrossingEdgeMapping);
				}
				intermediate_results_vec[l].CrossingEdgeMappings.push_back(tmp_mapping_vec);
			}

			delete[] partialResArr;
		}

		partialResEnd = MPI_Wtime();

		// If there no exist partial match, the process terminate
		double time_cost_value = partialResEnd - partialResStart;
		printf("Communication cost %f s with %d size!\n", time_cost_value, sizeSum);
		printf("There are %d LEC features.\n", partialResNum);
		//printf("There are %d inner matches.\n", finalPartialResSet.size());
		
		schedulingStart = MPI_Wtime();
		map< int, vector<int> > query_adjacent_list;

		for(int i = 0; i < intermediate_results_vec.size(); i++){
			if(intermediate_results_vec[i].CrossingEdgeMappings.size() == 0){
				continue;
			}
			for(int j = i + 1; j < intermediate_results_vec.size(); j++){
				if(intermediate_results_vec[j].CrossingEdgeMappings.size() == 0){
					continue;
				}
				//printf("~~~~ %d %d %d\n", i, j, checkJoinable(intermediate_results_vec[i], intermediate_results_vec[j]));
				if(checkJoinable(intermediate_results_vec[i], intermediate_results_vec[j])){
					if(query_adjacent_list.count(i) == 0){
						vector<int> vec1;
						query_adjacent_list.insert(make_pair(i, vec1));
					}
					if(query_adjacent_list.count(j) == 0){
						vector<int> vec1;
						query_adjacent_list.insert(make_pair(j, vec1));
					}
					query_adjacent_list[i].push_back(j);
					query_adjacent_list[j].push_back(i);
				}
			}
		}
		/*
		stringstream query_adjacent_list_ss;
		query_adjacent_list_ss << "-------------------- query adjacent list --------------------" << endl;
		for(int i = 0; i < intermediate_results_vec.size(); i++){
			if(intermediate_results_vec[i].CrossingEdgeMappings.size() == 0){
				continue;
			}
			query_adjacent_list_ss << i << " = ";
			for(int j = 0; j < query_adjacent_list[i].size(); j++){
				query_adjacent_list_ss << query_adjacent_list[i][j] << " ";
			}
			query_adjacent_list_ss << endl;
		}
		printf("%s\n", query_adjacent_list_ss.str().c_str());
		*/
		int ask_query_res_tag = 0;
		for(int i = 0; i < intermediate_results_vec.size(); i++){
			if(query_adjacent_list.count(i) == 0){
				continue;
			}
			
			//printf("%d begin to search ! \n", i);
			queue< vector<int> > bfs_queue;
			queue<CrossingEdgeMappingVec> res_queue;
			vector<int> tmp_vec(1, i);
			bfs_queue.push(tmp_vec);
			res_queue.push(intermediate_results_vec[i]);
			while(bfs_queue.size()){
				vector<int> cur_bfs_state = bfs_queue.front();
				bfs_queue.pop();
				
				CrossingEdgeMappingVec tmpCrossingEdgeMappingVec = res_queue.front();
				res_queue.pop();
				
				int cur_mapping_vec_id = cur_bfs_state[cur_bfs_state.size() - 1];
				for(int j = 0; j < query_adjacent_list[cur_mapping_vec_id].size(); j++){
					if(query_adjacent_list[cur_mapping_vec_id][j] <= i || find(cur_bfs_state.begin(), cur_bfs_state.end(), query_adjacent_list[cur_mapping_vec_id][j]) != cur_bfs_state.end())
						continue;
					
					if(checkJoinable(tmpCrossingEdgeMappingVec, intermediate_results_vec[query_adjacent_list[cur_mapping_vec_id][j]])){
						CrossingEdgeMappingVec newCrossingEdgeMappingVec;
						HashLECFJoin(newCrossingEdgeMappingVec, tmpCrossingEdgeMappingVec, intermediate_results_vec[query_adjacent_list[cur_mapping_vec_id][j]]);
						
						if(newCrossingEdgeMappingVec.CrossingEdgeMappings.size() != 0){
							vector<int> new_state(cur_bfs_state);
							new_state.push_back(query_adjacent_list[cur_mapping_vec_id][j]);
							bfs_queue.push(new_state);
							res_queue.push(newCrossingEdgeMappingVec);
							
							if(newCrossingEdgeMappingVec.tag == fullTag){
								break;
							}
						}
					}
				}
				
				if(res_queue.size() == 0 || res_queue.back().tag == fullTag){
					break;
				}
			}
			
			if(res_queue.size() != 0 && res_queue.back().tag == fullTag){
				ask_query_res_tag = 1; 
				break;
			}
		}
		
		schedulingEnd = MPI_Wtime();
		printf("Total cost %f s!\n", (schedulingEnd - partialResStart));
		
		if(ask_query_res_tag)
			printf("true.\n");
		else
			printf("false.\n");
		
	}else{		
		string db_folder = string(argv[1]);
		Database _db(db_folder);
		_db.load();
		printf("finish loading DB_store in Client %d.\n", myRank);
		
		MPI_Recv(&size, 1, MPI_INT, 0, 10, MPI_COMM_WORLD, &status);
		queryCharArr = new char[size];
		MPI_Recv(queryCharArr, size, MPI_CHAR, 0, 10, MPI_COMM_WORLD, &status);
		queryCharArr[size - 1] = 0;
		string _query_str(queryCharArr);
		
		partialResStart = MPI_Wtime();
		ResultSet _rs;
		string partialResStr;
		//cout << "begin" << endl;
		int cur_var_num = _db.queryASK(_query_str, _rs, partialResStr, myRank);
		
		partialResArr = new char[partialResStr.size() + 3];
		strcpy(partialResArr, partialResStr.c_str());
		size = strlen(partialResArr);
		
		partialResEnd = MPI_Wtime();
		double time_cost_value = partialResEnd - partialResStart;
		printf("Finding local partial matches costs %f s in %d !\n", time_cost_value, myRank);
		
		MPI_Send(&cur_var_num, 1, MPI_INT, 0, 10, MPI_COMM_WORLD);
		//printf("++++++++++++ %d ++++++++++++\n%s\n", myRank, partialResArr);
		
		MPI_Send(&size, 1, MPI_INT, 0, 10, MPI_COMM_WORLD);
		MPI_Send(partialResArr, size, MPI_CHAR, 0, 10, MPI_COMM_WORLD);
		
		/*
		char* queryGraphArr = new char[queryGraphStr.size() + 3];
		strcpy(queryGraphArr, queryGraphStr.c_str());
		size = strlen(queryGraphArr);
		
		MPI_Send(&size, 1, MPI_INT, 0, 10, MPI_COMM_WORLD);
		MPI_Send(queryGraphArr, size, MPI_CHAR, 0, 10, MPI_COMM_WORLD);
		
		delete[] queryGraphArr;
		*/
		delete[] partialResArr;
	}
	
	MPI_Finalize();
	delete[] queryCharArr;
    return 0;
}

