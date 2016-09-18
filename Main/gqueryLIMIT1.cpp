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

void NonrecursiveQuickSort(vector<PPPartialRes>& res1, int matchPos, int size)  
{  
    //  typedef vector<int> Stack_t;  
    int* stack = new int[size];  
    int top = 0;  
    int low, high, i, j, pivot;  
    PPPartialRes temp;  
    //首先把整个数组入栈  
    stack[top++] = size - 1;  
    stack[top++] = 0;  
    while(top != 0)  
    {  
        //取出栈顶元素，进行划分  
        low = stack[--top];  
        high = stack[--top];  
        pivot = high;   //将最后一个元素作为轴  
        i = low;       //保证i之前的元素的不大于轴  
        //j从前往后滑动  
        for(j=low; j < high; j++)  
        {  
            //如果碰到某个元素不大于轴，则与arr[i]交换  
            if(res1[j].MatchVec[matchPos] <= res1[pivot].MatchVec[matchPos])// arr[j]<=arr[pivot])  
            {
				temp.MatchVec.assign(res1[j].MatchVec.begin(), res1[j].MatchVec.end());
				temp.TagVec.assign(res1[j].TagVec.begin(), res1[j].TagVec.end());
				temp.FragmentID = res1[j].FragmentID;

				res1[j].MatchVec.assign(res1[i].MatchVec.begin(), res1[i].MatchVec.end());
				res1[j].TagVec.assign(res1[i].TagVec.begin(), res1[i].TagVec.end());
				res1[j].FragmentID = res1[i].FragmentID;

				res1[i].MatchVec.assign(temp.MatchVec.begin(), temp.MatchVec.end());
				res1[i].TagVec.assign(temp.TagVec.begin(), temp.TagVec.end());
				res1[i].FragmentID = temp.FragmentID;
				/*
                temp = arr[j];  
                arr[j] = arr[i];  
                arr[i] = temp;  
                */
				i++;  
            }  
        }  
        //i为分界点，交换arr[i]和轴  
        if(i != pivot)  
        {  
            /*swap arr[i] and arr[pivot]*/
			temp.MatchVec.assign(res1[i].MatchVec.begin(), res1[i].MatchVec.end());
			temp.TagVec.assign(res1[i].TagVec.begin(), res1[i].TagVec.end());
			temp.FragmentID = res1[i].FragmentID;

			res1[i].MatchVec.assign(res1[pivot].MatchVec.begin(), res1[pivot].MatchVec.end());
			res1[i].TagVec.assign(res1[pivot].TagVec.begin(), res1[pivot].TagVec.end());
			res1[i].FragmentID = res1[pivot].FragmentID;

			res1[pivot].MatchVec.assign(temp.MatchVec.begin(), temp.MatchVec.end());
			res1[pivot].TagVec.assign(temp.TagVec.begin(), temp.TagVec.end());
			res1[pivot].FragmentID = temp.FragmentID;
			/*
            temp = arr[i];  
            arr[i] = arr[pivot];  
            arr[pivot] = temp;
			*/
        }  
        //判断小于轴的部分元素如果多于一个的话, 则入栈  
        if(i - low > 1)  
        {  
            stack[top++] = i - 1;  
            stack[top++] = low;  
        }  
        //判断大于轴的部分元素如果多于一个的话, 则入栈  
        if(high - i > 1)  
        {  
            stack[top++] = high;  
            stack[top++] = i + 1;  
        }  
    }  
}

void QuickSort(vector<PPPartialRes>& res1, int matchPos, int low, int high){
	if(low >= high){
		return;
	}
	int first = low;
	int last = high;
	/*用字表的第一个记录作为枢轴*/
	PPPartialRes key;
	key.MatchVec.assign(res1[first].MatchVec.begin(), res1[first].MatchVec.end());
	key.TagVec.assign(res1[first].TagVec.begin(), res1[first].TagVec.end());
	key.FragmentID = res1[first].FragmentID;

	while(first < last){
		while(first < last && res1[last].MatchVec[matchPos] >= key.MatchVec[matchPos]){
			--last;
		}
		/*将比第一个小的移到低端*/
		res1[first].MatchVec.assign(res1[last].MatchVec.begin(), res1[last].MatchVec.end());
		res1[first].TagVec.assign(res1[last].TagVec.begin(), res1[last].TagVec.end());
		res1[first].FragmentID = res1[last].FragmentID;
		while(first < last && res1[first].MatchVec[matchPos] <= key.MatchVec[matchPos]){
			++first;
		}

		res1[last].MatchVec.assign(res1[first].MatchVec.begin(), res1[first].MatchVec.end());
		res1[last].TagVec.assign(res1[first].TagVec.begin(), res1[first].TagVec.end());
		res1[last].FragmentID = res1[first].FragmentID;
		/*将比第一个大的移到高端*/
	}
	/*枢轴记录到位*/
	res1[first].MatchVec.assign(key.MatchVec.begin(), key.MatchVec.end());
	res1[first].TagVec.assign(key.TagVec.begin(), key.TagVec.end());
	res1[first].FragmentID = key.FragmentID;
	
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

void MergeJoin(set< vector<int> >& finalPartialResSet, vector<PPPartialRes>& res1, vector<PPPartialRes>& res2, int fragmentNum, int matchPos){
	if(0 == res1.size()){
		return;
	}
	
	//QuickSort(res1, matchPos, 0, res1.size() - 1);
	NonrecursiveQuickSort(res1, matchPos, res1.size());
	//QuickSort(res2, matchPos, 0, res2.size() - 1);
	NonrecursiveQuickSort(res2, matchPos, res2.size());
	//cout << "res1.size() = " << res1.size() << ", res2.size() = " << res2.size() << endl;

	vector<PPPartialRes> new_res;
	int first_pos = 0, second_pos = 0, tag = 0;
	int l_iter = 0, r_iter = 0, len = res1[0].MatchVec.size();
	while (l_iter < res1.size() && r_iter < res2.size()) {
	
		if (l_iter == res1.size()) {
			r_iter++;
		} else if('1' == res1[l_iter].TagVec[matchPos]){
			new_res.push_back(res1[l_iter]);
			l_iter++;
			continue;
		} else if (r_iter == res2.size()) {
			l_iter++;
		} else if (res1[l_iter].MatchVec[matchPos] < res2[r_iter].MatchVec[matchPos]) {
			l_iter++;
		} else if (res1[l_iter].MatchVec[matchPos] > res2[r_iter].MatchVec[matchPos]) {
			r_iter++;
		} else {
			int l_iter_end = l_iter + 1, r_iter_end = r_iter + 1;

			while (l_iter_end < res1.size()
					&& res1[l_iter_end].MatchVec[matchPos] == res1[l_iter].MatchVec[matchPos]) {
				l_iter_end++;
				if (l_iter_end == res1.size()) {
					break;
				}
			}
			while (r_iter_end < res2.size()
					&& res2[r_iter_end].MatchVec[matchPos] == res2[r_iter].MatchVec[matchPos]) {
				r_iter_end++;
				if (r_iter_end == res2.size()) {
					break;
				}
			}

			for (int i = l_iter; i < l_iter_end; i++) {
				for (int j = r_iter; j < r_iter_end; j++) {
					if(res1[i].FragmentID == res2[j].FragmentID)
						continue;
						
					tag = 0;
					PPPartialRes curPPPartialRes;
					curPPPartialRes.TagVec.assign(len, '0');
					curPPPartialRes.FragmentID = fragmentNum + res1[i].FragmentID;
					for(int k = 0; k < len; k++){
						if(res1[i].MatchVec[k] != -1 && res2[j].MatchVec[k] != -1
							&& res1[i].MatchVec[k] != res2[j].MatchVec[k]){

							tag = 1;
							break;
						}else if(res1[i].MatchVec[k] == -1 && res2[j].MatchVec[k] != -1){
							curPPPartialRes.TagVec[k] = res2[j].TagVec[k];
							curPPPartialRes.MatchVec.push_back(res2[j].MatchVec[k]);
						}else if(res1[i].MatchVec[k] != -1 && res2[j].MatchVec[k] == -1){
							curPPPartialRes.TagVec[k] = res1[i].TagVec[k];
							curPPPartialRes.MatchVec.push_back(res1[i].MatchVec[k]);
						}else{
							if('1' == res1[i].TagVec[k] || '1' == res2[j].TagVec[k])
								curPPPartialRes.TagVec[k] = '1';
							curPPPartialRes.MatchVec.push_back(res1[i].MatchVec[k]);
						}
					}
					
					//cout << "tag = " << tag << endl;
					if(tag == 1)
						continue;

					if(0 == isFinalResult(curPPPartialRes)){
						new_res.push_back(curPPPartialRes);
					}else{
						finalPartialResSet.insert(curPPPartialRes.MatchVec);
					}
					
				}
			}

			r_iter = r_iter_end;
			l_iter = l_iter_end;
		}
	}

	//res1.cur_res_list.clear();
	res1.assign(new_res.begin(), new_res.end());
	//res1.tag_vec.assign(new_tag_vec.begin(), new_tag_vec.end());
}

int HashJoin(set< vector<int> >& finalPartialResSet, vector<PPPartialRes>& res1, map<int, vector<PPPartialRes> >& res2, int fragmentNum, int matchPos){

	if(0 == res1.size()){
		return 0;
	}
	
	int tag = 0, len = res1[0].MatchVec.size();
	vector<PPPartialRes> new_res;
	for(int i = 0; i < res1.size(); i++){
		if('1' == res1[i].TagVec[matchPos]){
			new_res.push_back(res1[i]);
			continue;
		}
		if(res2.count(res1[i].MatchVec[matchPos]) == 0)
			continue;
		//cout << res2[res1[i].MatchVec[matchPos]].size() << " " << endl;
		
		vector<PPPartialRes> tmp_res = res2[res1[i].MatchVec[matchPos]];
		for(int j = 0; j < tmp_res.size(); j++){
			//cout << tmp_res.size() << "~~~~" << j << endl;
			tag = 0;
			PPPartialRes curPPPartialRes;
			curPPPartialRes.TagVec.assign(len, '0');
			curPPPartialRes.FragmentID = fragmentNum + res1[i].FragmentID;
			for(int k = 0; k < len; k++){
				//cout << "++++" << k << " " << res1[i].MatchVec[k] << " " << tmp_res[j].MatchVec[k] << endl;
				if(res1[i].MatchVec[k] != -1 && tmp_res[j].MatchVec[k] != -1
					&& res1[i].MatchVec[k] != tmp_res[j].MatchVec[k]){

					tag = 1;
					break;
				}else if(res1[i].MatchVec[k] == -1 && tmp_res[j].MatchVec[k] != -1){
					curPPPartialRes.TagVec[k] = tmp_res[j].TagVec[k];
					curPPPartialRes.MatchVec.push_back(tmp_res[j].MatchVec[k]);
				}else if(res1[i].MatchVec[k] != -1 && tmp_res[j].MatchVec[k] == -1){
					curPPPartialRes.TagVec[k] = res1[i].TagVec[k];
					curPPPartialRes.MatchVec.push_back(res1[i].MatchVec[k]);
				}else{
					if('1' == res1[i].TagVec[k] || '1' == tmp_res[j].TagVec[k])
						curPPPartialRes.TagVec[k] = '1';
					curPPPartialRes.MatchVec.push_back(res1[i].MatchVec[k]);
				}
			}
					
			//cout << "tag = " << tag << endl;
			if(tag == 1)
				continue;

			if(0 == isFinalResult(curPPPartialRes)){
				new_res.push_back(curPPPartialRes);
			}else{
				finalPartialResSet.insert(curPPPartialRes.MatchVec);
				return 1;
			}
		}
	}
	res1.assign(new_res.begin(), new_res.end());
	return 0;
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
		
		map<string, int> URIIDMap;
		map<int, string> IDURIMap;
		int id_count = 0, cur_id = 0;
		int partialResNum = 0, sizeSum = 0, PPQueryVertexCount = 0, finalResNum = 0, aResNum = 0;
		set< vector<int> > finalPartialResSet;
		
		for(int pInt = 1; pInt < p; pInt++){
			MPI_Recv(&PPQueryVertexCount, 1, MPI_INT, pInt, 10, MPI_COMM_WORLD, &status);
		}
		vector< PPPartialResVec > partialResVec(PPQueryVertexCount);
		for(i = 0; i < partialResVec.size(); i++){
			partialResVec[i].match_pos = i;
		}
		ofstream log_output("log.txt");

		for(int pInt = 1; pInt < p; pInt++){
			MPI_Recv(&size, 1, MPI_INT, pInt, 10, MPI_COMM_WORLD, &status);
			sizeSum += size;
			partialResArr = new char[size + 3];
			MPI_Recv(partialResArr, size, MPI_CHAR, pInt, 10, MPI_COMM_WORLD, &status);
			partialResArr[size] = 0;
			
			//printf("++++++++++++ %d ++++++++++++\n%s\n", pInt, partialResArr);
			//log_output << "++++++++++++ " << pInt << " ++++++++++++" << endl;
			//log_output << partialResArr << endl;
			aResNum = 0;
			
			string textline(partialResArr);
			vector<string> resVec = split(textline, "\n");
			for(i = 0; i < resVec.size(); i++){
				//curPartialResSize = resVec[i].length();
				vector<string> matchVec = split(resVec[i], "\t");
				if(matchVec.size() != PPQueryVertexCount)
					continue;
				PPPartialRes newPPPartialRes;
				vector<int> match_pos_vec;
				for(j = 0; j < matchVec.size(); j++){
					if(strcmp(matchVec[j].c_str(),"-1") != 0){
						newPPPartialRes.TagVec.push_back(matchVec[j].at(0));
						matchVec[j].erase(0, 1);

						if(URIIDMap.count(matchVec[j]) == 0){
							URIIDMap.insert(make_pair(matchVec[j], id_count));
							IDURIMap.insert(make_pair(id_count, matchVec[j]));
							id_count++;
						}
						cur_id = URIIDMap[matchVec[j]];
					}else{
						newPPPartialRes.TagVec.push_back('2');
						cur_id = -1;
					}
					newPPPartialRes.MatchVec.push_back(cur_id);
					
					if('1' == newPPPartialRes.TagVec[newPPPartialRes.TagVec.size() - 1])
						match_pos_vec.push_back(j);
				}
				newPPPartialRes.FragmentID = pInt;
				newPPPartialRes.ID = partialResNum;

				if(0 == isFinalResult(newPPPartialRes)){
					for(j = 0; j < match_pos_vec.size(); j++){
						partialResVec[match_pos_vec[j]].PartialResList.push_back(newPPPartialRes);
					}
					aResNum++;
					partialResNum++;
				}else{
					finalPartialResSet.insert(newPPPartialRes.MatchVec);
					//finalResNum++;
				}
			}
			printf("There are %d partial results and %d final results in Client %d!\n", aResNum, finalPartialResSet.size() - finalResNum, pInt);
			finalResNum = finalPartialResSet.size();

			delete[] partialResArr;
		}

		partialResEnd = MPI_Wtime();

		// If there no exist partial match, the process terminate
		double time_cost_value = partialResEnd - partialResStart;
		printf("Communication cost %f s!\n", time_cost_value);
		printf("There are %d partial results with %d size.\n", partialResNum, sizeSum);
		printf("There are %d inner matches.\n", finalPartialResSet.size());
		
		schedulingStart = MPI_Wtime();

		sort(partialResVec.begin(), partialResVec.end(), myfunction0);
		vector<int> match_pos_vec;
		int tag = 0, limit1_tag = 0;
		match_pos_vec.push_back(partialResVec[0].match_pos);
		if(0 != partialResVec.size()){
		
			stringstream intermediate_strm;

			for(i = 1; i < partialResVec.size(); i++){
				//cout << "###########  " << partialResVec[i].match_pos << endl;
				
				map<int, vector<PPPartialRes> > tmpPartialResMap;
				for(j = 0; j < partialResVec[i].PartialResList.size(); j++){
					tag = 0;
					for(k = 0; k < match_pos_vec.size(); k++){
						if('1' == partialResVec[i].PartialResList[j].TagVec[match_pos_vec[k]]){
							tag = 1;
							break;
						}
					}
					if(0 == tag){
						if(tmpPartialResMap.count(partialResVec[i].PartialResList[j].MatchVec[partialResVec[i].match_pos]) == 0){
							vector<PPPartialRes> tmpVec;
							tmpPartialResMap.insert(make_pair(partialResVec[i].PartialResList[j].MatchVec[partialResVec[i].match_pos], tmpVec));
						}
						tmpPartialResMap[partialResVec[i].PartialResList[j].MatchVec[partialResVec[i].match_pos]].push_back(partialResVec[i].PartialResList[j]);
					}
				}
				match_pos_vec.push_back(partialResVec[i].match_pos);
				if(tmpPartialResMap.size() == 0){					
					continue;
				}
				
				if(1 == HashJoin(finalPartialResSet, partialResVec[0].PartialResList, tmpPartialResMap, p, partialResVec[i].match_pos)){
					limit1_tag = 1;
					break;
				}

				if(partialResVec[0].PartialResList.size() == 0){
					break;
				}
			}
		}
		
		if(limit1_tag == 1){
			printf("There are some crossing matches.\n");
		}else{
			printf("There is no crossing match.\n");
		}
		schedulingEnd = MPI_Wtime();
		time_cost_value = schedulingEnd - partialResStart;
		printf("Total cost %f s!\n", time_cost_value);
		
		ofstream res_output("finalRes.txt");
		set< vector<int> >::iterator iter = finalPartialResSet.begin();
		while(iter != finalPartialResSet.end()){
			vector<int> tempVec = *iter;
			for(l = 0; l < tempVec.size(); l++){
				res_output << IDURIMap[tempVec[l]] << "\t";
			}
			res_output << endl;
			//total_res_count++;
			iter++;
		}
		res_output.close();
		
	}else{
		//ofstream log_output("log.txt");
		//Config _config = loadConfig("config.ini");
		//log_output << "begin loading DB_store" << endl;
		string db_folder = string(argv[1]);
		Database _db(db_folder);
		_db.load();
		//cout << db_folder.c_str() << 
		printf("finish loading DB_store:%s in Client %d.\n", db_folder.c_str(), myRank);
		
		MPI_Recv(&size, 1, MPI_INT, 0, 10, MPI_COMM_WORLD, &status);
		queryCharArr = new char[size];
		MPI_Recv(queryCharArr, size, MPI_CHAR, 0, 10, MPI_COMM_WORLD, &status);
		queryCharArr[size - 1] = 0;
		string _query_str(queryCharArr);
		
		partialResStart = MPI_Wtime();
		ResultSet _rs;
		string partialResStr;
		
		SPARQLquery cur_sparql_query = _db.query(_query_str, _rs, partialResStr, myRank);
		BasicQuery* basic_query=&(cur_sparql_query.getBasicQuery(0));
		//printf("begin partial evaluation.\n");
		_db.join_pe(basic_query, partialResStr);
		int cur_var_num = basic_query->getVarNum();
		//printf("There are %d results in Client %d!\n", (basic_query->getResultList()).size(), myRank);
		
		partialResEnd = MPI_Wtime();
		
		double time_cost_value = partialResEnd - partialResStart;
		printf("Finding local partial matches costs %f s in %d !\n", time_cost_value, myRank);
		
		partialResArr = new char[partialResStr.size() + 3];
		strcpy(partialResArr, partialResStr.c_str());
		size = strlen(partialResArr);
		
		MPI_Send(&cur_var_num, 1, MPI_INT, 0, 10, MPI_COMM_WORLD);
		MPI_Send(&size, 1, MPI_INT, 0, 10, MPI_COMM_WORLD);		
		//cout << myRank << " : " << partialResArr << endl;
		MPI_Send(partialResArr, size, MPI_CHAR, 0, 10, MPI_COMM_WORLD);
		
		delete[] partialResArr;
	}
	
	delete[] queryCharArr;
	
	MPI_Finalize();

    return 0;
}

