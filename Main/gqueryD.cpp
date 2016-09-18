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

#include "../Database/Database.h"
#include "../Util/Util.h"
#include <mpi.h>

using namespace std;

//WARN:cannot support soft links!

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
	//chdir(dirname(argv[0]));
    Util util;
    if(argc == 1 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
    {
        help();
        return 0;
    }
    //cout << "gquery..." << endl;
    if(argc < 2)
    {
        cerr << "error: lack of DB_store to be queried" << endl;
        return 0;
    }
	/*
    {
        cout << "argc: " << argc << "\t";
        cout << "DB_store:" << argv[1] << "\t";
        cout << endl;
    }
	*/
	
    // read query from file.
    if (argc >= 3)
    {
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
			
			string _query_str = Util::getQueryFromFile(argv[2]);
			queryCharArr = new char[1024];
			strcpy(queryCharArr, _query_str.c_str());
			size = strlen(queryCharArr);
			cout << "query : " << queryCharArr << endl;
			
			string query_file_str = string(argv[2]);
			query_file_str = query_file_str.substr(query_file_str.find("/") + 1);
			query_file_str = "log_" + query_file_str;
			ofstream log_output(query_file_str.c_str());
			string partial_res_str;
			//printf("%s\n", query_file_str.c_str());

			for(i = 1; i < p; i++){
				MPI_Send(&size, 1, MPI_INT, i, 10, MPI_COMM_WORLD);
				MPI_Send(queryCharArr, size, MPI_CHAR, i, 10, MPI_COMM_WORLD);
			}
			partialResStart = MPI_Wtime();
			printf("The query has been sent!\n");
			
			ResultSet _result_set;
			int PPQueryVertexCount = -1;
			QueryTree::QueryForm query_form = QueryTree::Ask_Query;
			GeneralEvaluation parser_evaluation(NULL, NULL, _result_set);
			
			parser_evaluation.onlyParseQuery(_query_str, PPQueryVertexCount, query_form);
			
			if(query_form == QueryTree::Ask_Query){
			
				int fullTag = 0, sizeSum = 0, partialResNum = 0;
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
					vector<string> resVec = Util::split(textline, "\n");
					partialResNum += resVec.size();
					//printf("processs %d : %s\n", pInt, resVec[0].c_str());
					for(i = 0; i < resVec.size(); i++){
						//curPartialResSize = resVec[i].length();
						vector<string> matchVec = Util::split(resVec[i], "\t");
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
						if(Util::checkJoinable(intermediate_results_vec[i], intermediate_results_vec[j])){
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
							
							if(Util::checkJoinable(tmpCrossingEdgeMappingVec, intermediate_results_vec[query_adjacent_list[cur_mapping_vec_id][j]])){
								CrossingEdgeMappingVec newCrossingEdgeMappingVec;
								Util::HashLECFJoin(newCrossingEdgeMappingVec, tmpCrossingEdgeMappingVec, intermediate_results_vec[query_adjacent_list[cur_mapping_vec_id][j]]);
								
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
			
				map<string, int> URIIDMap;
				map<int, string> IDURIMap;
				int id_count = 0, cur_id = 0;
				int partialResNum = 0, sizeSum = 0, finalResNum = 0, aResNum = 0;
				set< vector<int> > finalPartialResSet;
				
				vector< PPPartialResVec > partialResVec(PPQueryVertexCount);
				for(i = 0; i < partialResVec.size(); i++){
					partialResVec[i].match_pos = i;
				}
				//ofstream log_output("log.txt");

				for(int pInt = 1; pInt < p; pInt++){
					MPI_Recv(&size, 1, MPI_INT, pInt, 10, MPI_COMM_WORLD, &status);
					sizeSum += size;
					partialResArr = new char[size + 3];
					MPI_Recv(partialResArr, size, MPI_CHAR, pInt, 10, MPI_COMM_WORLD, &status);
					partialResArr[size] = 0;
					
					//printf("++++++++++++ %d ++++++++++++\n%s\n", pInt, partialResArr);
					//log_output << "++++++++++++ " << pInt << " ++++++++++++" << endl;
					//log_output << partialResArr << endl;
					//partial_res_str = string(partialResArr);
					aResNum = 0;
					
					string textline(partialResArr);
					vector<string> resVec = Util::split(textline, "\n");
					for(i = 0; i < resVec.size(); i++){
						//curPartialResSize = resVec[i].length();
						vector<string> matchVec = Util::split(resVec[i], "\t");
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

						if(0 == Util::isFinalResult(newPPPartialRes)){
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

				sort(partialResVec.begin(), partialResVec.end(), Util::myfunction0);
				vector<int> match_pos_vec;
				int tag = 0;
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
						
						Util::HashJoin(finalPartialResSet, partialResVec[0].PartialResList, tmpPartialResMap, p, partialResVec[i].match_pos);

						if(partialResVec[0].PartialResList.size() == 0){
							break;
						}
					}
				}
				
				printf("There are %d final matches.\n", finalPartialResSet.size());
				schedulingEnd = MPI_Wtime();
				time_cost_value = schedulingEnd - partialResStart;
				printf("Total cost %f s!\n", time_cost_value);
				
				//log_output << partial_res_str << endl;
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
			}
			
		}else{
			string db_folder = string(argv[1]);
			//if(db_folder[0] != '/' && db_folder[0] != '~')  //using relative path
			//{
				//db_folder = string("../") + db_folder;
			//}
			Database _db(db_folder);
			_db.load();
			printf("Client %d finish loading!\n", myRank);
	
			MPI_Recv(&size, 1, MPI_INT, 0, 10, MPI_COMM_WORLD, &status);
			queryCharArr = new char[size];
			MPI_Recv(queryCharArr, size, MPI_CHAR, 0, 10, MPI_COMM_WORLD, &status);
			queryCharArr[size - 1] = 0;
			
			partialResStart = MPI_Wtime();
			string _query_str(queryCharArr);
			
			ResultSet _rs;
			string partialResStr;
			//_db.query(query, _rs, stdout);
			_db.query(_query_str, _rs, partialResStr, myRank, stdout);
			
			partialResEnd = MPI_Wtime();
		
			double time_cost_value = partialResEnd - partialResStart;
			printf("Finding local partial matches costs %f s in %d !\n", time_cost_value, myRank);
			
			partialResArr = new char[partialResStr.size() + 3];
			strcpy(partialResArr, partialResStr.c_str());
			size = strlen(partialResArr);
			
			MPI_Send(&size, 1, MPI_INT, 0, 10, MPI_COMM_WORLD);		
			//cout << myRank << " : " << partialResArr << endl;
			MPI_Send(partialResArr, size, MPI_CHAR, 0, 10, MPI_COMM_WORLD);
			
			delete[] partialResArr;
		}
		
		delete[] queryCharArr;

		MPI_Finalize();
        /*
        if (argc >= 4)
        {
            Util::save_to_file(argv[3], _rs.to_str());
        }
		*/
        return 0;
    }

    // read query file path from terminal.
    // BETTER: sighandler ctrl+C/D/Z
    string query;
    //char resolved_path[PATH_MAX+1];
#ifdef READLINE_ON
    char *buf, prompt[] = "gsql>";
    //const int commands_num = 3;
    //char commands[][20] = {"help", "quit", "sparql"};
    printf("Type `help` for information of all commands\n");
	printf("Type `help command_t` for detail of command_t\n");
    rl_bind_key('\t', rl_complete);
    while(true)
    {
        buf = readline(prompt);
        if(buf == NULL)
            continue;
        else
            add_history(buf);
        if(strncmp(buf, "help", 4) == 0)
        {
			if(strcmp(buf, "help") == 0)
			{
            //print commands message
            printf("help - print commands message\n");
            printf("quit - quit the console normally\n");
            printf("sparql - load query from the second argument\n");
			}
			else
			{
				//TODO: help for a given command
			}
            continue;
        }
        else if(strcmp(buf, "quit") == 0)
            break;
        else if(strncmp(buf, "sparql", 6) != 0)
        {
            printf("unknown commands\n");
            continue;
        }
		//TODO: sparql + string, not only path
        string query_file;
        //BETTER:build a parser for this console
		bool ifredirect = false;

		char* rp = buf;
		int pos = strlen(buf) - 1;
		while(pos > -1)
		{
			if(*(rp+pos) == '>')
			{
				ifredirect = true;
				break;
			}
			pos--;
		}
		rp += pos;

        char* p = buf + strlen(buf) - 1;
		FILE* fp = stdout;      ///default to output on screen
		if(ifredirect)
		{
			char* tp = p;
			while(*tp == ' ' || *tp == '\t')
				tp--;
			*(tp+1) = '\0';
			tp = rp + 1;
			while(*tp == ' ' || *tp == '\t')
				tp++;
			fp = fopen(tp, "w");	//NOTICE:not judge here!
			p = rp - 1;					//NOTICE: all separated with ' ' or '\t'
		}
        while(*p == ' ' || *p == '\t')	//set the end of path
            p--;
        *(p+1) = '\0';
        p = buf + 6;
        while(*p == ' ' || *p == '\t')	//acquire the start of path
            p++;
        //TODO: support the soft links(or hard links)
        //there are also readlink and getcwd functions for help
        //http://linux.die.net/man/2/readlink
        //NOTICE:getcwd and realpath cannot acquire the real path of file
        //in the same directory and the program is executing when the
        //system starts running
        //NOTICE: use realpath(p, NULL) is ok, but need to free the memory
        char* q = realpath(p, NULL);	//QUERY:still not work for soft links
#ifdef DEBUG_PRECISE
        printf("%s\n", p);
#endif
        if(q == NULL)
        {
			printf("invalid path!\n");
			free(q);
			free(buf);
			continue;
        }
        else
			printf("%s\n", q);
        //query = getQueryFromFile(p);
        query = Util::getQueryFromFile(q);
        if(query.empty())
        {
			free(q);
            //free(resolved_path);
            free(buf);
			if(ifredirect)
				fclose(fp);
            continue;
        }
        printf("query is:\n");
        printf("%s\n\n", query.c_str());
        ResultSet _rs;
        //_db.query(query, _rs, fp);
        //test...
        //string answer_file = query_file+".out";
        //Util::save_to_file(answer_file.c_str(), _rs.to_str());
		free(q);
        //free(resolved_path);
        free(buf);
		if(ifredirect)
			fclose(fp);
#ifdef DEBUG_PRECISE
        printf("after buf freed!\n");
#endif
    }
//#else					//DEBUG:this not work!
//    while(true)
//    {
//        cout << "please input query file path:" << endl;
//        string query_file;
//        cin >> query_file;
//        //char* q = realpath(query_file.c_str(), NULL);
//        string query = getQueryFromFile(query_file.c_str());
//        if(query.empty())
//        {
//            //free(resolved_path);
//            continue;
//        }
//        cout << "query is:" << endl;
//        cout << query << endl << endl;
//        ResultSet _rs;
//        _db.query(query, _rs, stdout);
//        //free(resolved_path);
//    }
#endif // READLINE_ON

    return 0;
}

