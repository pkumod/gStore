/*
  author: zz
  created time: 2021.8.22
  This test scripts is for DEBUGGING
  All bugs reported should be listed here
  Add a test function to verify bug or debug
*/
#include <iostream>
#include <set>
#include <vector>
#include <deque>
#include <algorithm>
#include <fstream>
#include <mutex>
#include <thread>
#include <string>
#include <atomic>
#include <sstream>
#include "../Database/Txn_manager.h"

void preload4bug(vector<string>& adds, vector<string>& subs, const int _nums)
{
    ifstream insert_if;
    insert_if.open("scripts/lubm4debug.nt");
    string sparql1 = "INSERT DATA {";
    string sparql2 = "DELETE DATA {";
    string line;
    
    int cnt = 0;
    while(!insert_if.eof() && cnt < _nums){
        sparql1 = "INSERT DATA {";
        sparql2 = "DELETE DATA {";
        getline(insert_if, line);
        if(line.size() > 0)
            line[line.size() - 1] = ' ';
        sparql1 += line;
        sparql1 += "}";
        sparql2 += line;
        sparql2 += "}";
        adds.push_back(sparql1);
        subs.push_back(sparql2);
        cnt++;
    }
    insert_if.close();
}
//bug#1: crash by the SITree.
void Bug4SItree(Database *_db)
{
    const int _nums = 2000;
    vector<string> adds, subs;
    preload4bug(adds, subs, _nums);
    ResultSet rs;
    FILE *fp = NULL;
    for(int i = 0; i < _nums; i++)
    {
        _db->query(adds[i], rs, fp);
    }

    for(int i = 0; i < _nums; i++)
    {
        _db->query(subs[i], rs, fp);
    }
}

void Bug20220408(Txn_manager &txn_m)
{
    string insert = "insert data {<zhangzhe> <name> \"zhangzhe\"}";
    string remove = "delete data {<zhangzhe> <name> \"zhangzhe\"}";
    string query  = "select ?x where {<zhangzhe> ?p ?x}";
    string res;
    txn_id_t TID = txn_m.Begin();
    txn_m.Query(TID, remove, res);
    txn_m.Query(TID, query, res);
    txn_m.Commit(TID);
    txn_m.Checkpoint();
    TID = txn_m.Begin();
    txn_m.Query(TID, insert, res);
    txn_m.Query(TID, query, res);
    txn_m.Commit(TID);
    txn_m.Checkpoint();
}

int main(int argc, char* argv[])
{
    Util util;
	string db_folder = "lubm";
    
	Database _db(db_folder);
	_db.load();

    Txn_manager txn_m(&_db, "test");
    Bug20220408(txn_m);
}