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

using namespace std;

struct MyTriple
{
    string subject;
    string predicate;
    string object;
    MyTriple(string _subject, string _predicate, string _object):subject(_subject), predicate(_predicate), object(_object){

    }
};
const int txn_num = 1000;
const int read_num = 5;
const int update_num = 1;
const int threads_num = 4;
const int oltp_query_num = 2;
const int oltp_update_num = 1;
const bool is_wat = false;
const int model = 3; //1:seq_txn 2:part_txn 3:oltp_txn other: no_txn
const string updates_path = "../../insert_lubm.nt";
//const string updates_path = "../../../../watdiv_insert.nt";
const string wat_name = "wat";
const string lubm_name = "lubm";
const string dir = lubm_name + to_string(oltp_update_num) + '.' + to_string(oltp_query_num) ;
const string time_statics = "/time_statics.txt";
const string restart_statics = "/restart_statics.txt";
vector<long> time_rec(threads_num * txn_num);
vector<int> restartcnts(threads_num * txn_num);
vector<string> updates_line;
int line_num = 0;
atomic<int> offset = {0};
atomic<int> restart_times = {0};
string lubm_q[21] = {
        "prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> prefix ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#> select ?x where{?x	rdf:type	ub:GraduateStudent.?y	rdf:type	ub:University.?z	rdf:type	ub:Department.?x	ub:memberOf	?z.?z	ub:subOrganizationOf	?y.?x	ub:undergraduateDegreeFrom	?y.}",
        "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#> select ?x where{?x	rdf:type	ub:Course.?x	ub:name	?y.}",
        "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#> select ?x where { ?x	rdf:type	ub:UndergraduateStudent. ?y	rdf:type	ub:University. ?z	rdf:type	ub:Department. ?x	ub:memberOf	?z. ?z	ub:subOrganizationOf	?y. ?x 	ub:undergraduateDegreeFrom	?y. }",
        "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#> select ?x ?y1 ?y2 ?y3 where { ?x	ub:worksFor	<http://www.Department0.University0.edu>. ?x	rdf:type	ub:FullProfessor. ?x	ub:name	?y1. ?x	ub:emailAddress	?y2. ?x	ub:telephone	?y3. } ",
        "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#> select ?x where { ?x	ub:subOrganizationOf	<http://www.Department0.University0.edu>. ?x	rdf:type	ub:ResearchGroup. } ",
        "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#> select ?x ?y where { ?y	ub:subOrganizationOf	<http://www.University0.edu>.	?y	rdf:type	ub:Department. ?x	ub:worksFor	?y. ?x	rdf:type	ub:FullProfessor. }",
        "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#> select ?x ?y ?z where { ?x	rdf:type	ub:UndergraduateStudent. ?y	rdf:type	ub:FullProfessor. ?z	rdf:type	ub:Course. ?x	ub:advisor	?y. ?x	ub:takesCourse	?z. ?y	ub:teacherOf	?z.}",
        "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#> select ?X where { ?X	rdf:type	ub:GraduateStudent. ?X	ub:takesCourse	<http://www.Department0.University0.edu/GraduateCourse0>. }",
        "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#> select ?X ?Y ?Z where { ?X	rdf:type	ub:GraduateStudent. ?Y	rdf:type	ub:University. ?Z	rdf:type	ub:Department. ?X	ub:memberOf	?Z. ?Z	ub:subOrganizationOf	?Y. ?X	ub:undergraduateDegreeFrom	?Y.}",
        "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#> select ?X where { ?X	rdf:type	ub:Publication. ?X	ub:publicationAuthor	<http://www.Department0.University0.edu/AssistantProfessor0>.}",
        "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#> select ?Y1 ?Y2 ?Y3 where { ?X	rdf:type	ub:FullProfessor. ?X	ub:worksFor	<http://www.Department0.University0.edu>. ?X	ub:name	?Y1. ?X	ub:emailAddress	?Y2. ?X	ub:telephone	?Y3.}",
        "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#> select ?X where { ?X	ub:memberOf	<http://www.Department0.University0.edu>.	}",
        "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#> select ?X where { ?X	rdf:type	ub:UndergraduateStudent. }",
        "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#> select ?X ?Y where{ ?X	rdf:type	ub:Student. ?Y	rdf:type	ub:Course. ?X	ub:takesCourse	?Y. <http://www.Department0.University0.edu/AssociateProfessor0>	ub:teacherOf	?Y.}",
        "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#> select ?X where { ?X	rdf:type	ub:UndergraduateStudent. ?Y	rdf:type	ub:Department.?X	ub:memberOf	?Y. ?Y	ub:subOrganizationOf	<http://www.University0.edu>. ?X	ub:emailAddress	?Z.}",
        "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#> select ?X ?Y ?Z where { ?X	rdf:type	ub:UndergraduateStudent. ?Z	rdf:type	ub:Course. ?X	ub:advisor	?Y. ?Y	ub:teacherOf	?Z. ?X	ub:takesCourse	?Z.}",
        "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#> select ?X where { ?X	rdf:type	ub:GraduateStudent. ?X	ub:takesCourse	<http://www.Department0.University0.edu/GraduateCourse0>. }",
        "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#> select ?X where{ ?X	rdf:type	ub:ResearchGroup. ?X	ub:subOrganizationOf	<http://www.University0.edu>.}",
        "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#> select ?X ?Y where { ?Y	rdf:type	ub:Department. ?X	ub:worksFor	?Y. ?Y	ub:subOrganizationOf	<http://www.University0.edu>. }",
        "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#> select ?X where { <http://www.University0.edu>	ub:undergraduateDegreeFrom	?X.}",
        "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#> select ?X where{ ?X	rdf:type	ub:UndergraduateStudent.}"
};

vector<MyTriple> triples;

void write_statics()
{
    map<long, int> rec;
    ofstream time_if;
    cerr << dir << endl;
    string path = dir + time_statics;
    cerr << path << endl;
    time_if.open(path.c_str());
    for(auto t: time_rec)
    {
        rec[t]++;
    }

    for(auto it = rec.begin(); it != rec.end(); it++)
    {
        time_if << it->first << " " <<  it->second << endl;
    }
    time_if.close();
    string path1 = dir + restart_statics;
    time_if.open(path1.c_str());
    map<int, int> cnt;
    for(auto t: restartcnts)
    {
        cnt[t]++;
    }

    for(auto it = cnt.begin(); it != cnt.end(); it++)
    {
        time_if << it->first << " " << it->second << endl;
    }
    time_if.close();
}

void read_triples()
{
    ifstream insert_if;
    insert_if.open(updates_path.c_str());
    string line;

    while(!insert_if.eof()){
        if(is_wat){
        int idx = 0;
        getline(insert_if, line);
        cout << line << endl;
        string s = "", p = "", o = "";
        while(line[idx] != '>') s.push_back(line[idx++]);
        s.push_back(line[idx++]);
        while(line[idx] == '\t') idx++;
        while(line[idx] != '>') p.push_back(line[idx++]);
        p.push_back(line[idx++]);
        while(line[idx] == '\t') idx++;
        char ch = line[idx++];
        o.push_back(ch);
        if(ch == '\"'){
            while(line[idx] != '\"') idx++;
            o.push_back(line[idx]);
        }
        else if(ch == '<'){
            while(line[idx] != '>') {
                o.push_back(line[idx]);
                idx++;
            }
            o.push_back('>');
        }
        //cerr << s << "  " << p << "  " << o << endl;
        MyTriple tri(s, p, o);
        triples.push_back(tri);
        line_num++;
        }
        else{
            getline(insert_if, line);
            cout << line << endl;
            stringstream ss(line);
            string s, p, o;
            ss >> s >> p >> o;
            //cerr << s << "  " << p << "  " << o << endl;
            MyTriple tri(s, p, o);
            triples.push_back(tri);
            line_num++;
        }
        
    }
    cout << "lines:  " << line_num << endl;
    insert_if.close();
}

vector<string> get_node_query(vector<MyTriple> &triples)
{
    vector<string> ret;
    int k = 0;
    for(auto &tri: triples){
        string q1 = "select ?x where {";
        //q1 = q1 + tri.subject + " ?p ?x}";
        q1 = q1 + "?x ?p " + tri.object + "}";
        string q2 = "select ?x where {";
        //q2 = q2 + "?x ?p " + tri.object + "}";
        q2 = q2 + tri.subject + " ?p ?x}";
        ret.push_back(q1);
        ret.push_back(q2);
        ret.push_back(q1);
        ret.push_back(q2);
        
        k += 4;
        if(k >= oltp_query_num) return ret;
    }
    while(k < oltp_query_num){
        ret.push_back(ret[ret.size()-2]);
        ret.push_back(ret[ret.size()-2]);
        k += 2;
    }
    return ret;
}

string get_insert_sparql(MyTriple& triple)
{
    string ret = "insert data { ";
    ret = ret + triple.subject + " "  + triple.predicate + " " + triple.object + " .}";
    return ret;
}


long start = Util::get_cur_time();
struct task
{
    vector<string> query;
    vector<string> updates;
    task() {
        query.clear();
        updates.clear();
    }
    
    ~task(){
        query.clear();
        updates.clear();
    }
    void add_query(string line){
        query.push_back(line);
    }
    
    void add_updates(string line){
        updates.push_back(line);
    }

    vector<string>& get_query(){
        return query;
    }

    vector<string>& get_updates(){
        return updates;
    }

    bool is_empty()
    {
        return updates.size() <= 0;
    }
};

void open_file()
{
    ifstream insert_if;
    insert_if.open(updates_path.c_str());
    string sparql = "INSERT DATA {";
    string line;
    while(!insert_if.eof()){
        sparql = "INSERT DATA {";
        getline(insert_if, line);
        if(line.size() > 0)
            line[line.size() - 1] = ' ';
        sparql += line;
        sparql += "}";
        updates_line.push_back(sparql);
        line_num++;
        if(line_num % 100000 == 0) cout << line_num << endl;
    }
    cout << updates_line[2] << endl;
    cout << "lines:  " << line_num << endl;
    insert_if.close();
}


task get_task_1(Txn_manager& txn_m, int k)
{
    task ret;
    for(int i = 0; i < read_num; i++)
    {
        ret.add_query(lubm_q[rand() % 21]);
    }

    for(int i = 0, j = k; i < update_num; i++, k++)
    {
        //cerr << idx << endl;
        if(j >= line_num) return ret;
        ret.add_updates(updates_line[j]);
    }
    cout << "get task success" << endl;
    return ret;
}

task get_task(Txn_manager& txn_m)
{
    task ret;
    for(int i = 0; i < read_num; i++)
    {
        ret.add_query(lubm_q[rand() % 21]);
    }
    // ret.add_query(lubm_q[0]);
    // ret.add_query(lubm_q[3]);
    for(int i = 0; i < update_num; i++)
    {
        int idx =  offset.fetch_add(1);
        //cerr << idx << endl;
        if(idx >= line_num) return ret;
        if(idx != 0 && idx % 1000 == 0){
           long end =  Util::get_cur_time();
           cerr << idx << " : " << end - start << " ms" << endl; 
           //cerr << "check times " << txn_m.GetDatabase()->check_times.load() << endl;
           //cerr << "restart times:" << restart_times.load() << endl;
           start = end;
        }
        if(idx % 20000 == 0 && idx != 0){
            txn_m.Checkpoint();
            cerr << "checkpoint done!" << endl;
        }
        ret.add_updates(updates_line[idx]);
    }
    cout << "get task success" << endl;
    return ret;
}

task get_oltp_task(int k)
{
    task ret;
    vector<MyTriple> data;
    for(int i = 0, j = k; i < oltp_update_num; i++, j++)
    {
        if(j >= line_num) break;
        //ret.add_updates(get_insert_sparql(triples[j]));
        data.push_back(triples[j]);
    }
    auto qv = get_node_query(data);
    for(auto q: qv) ret.add_query(q);
    for(auto t: data)
    {
        auto iv = get_insert_sparql(t);
        ret.add_updates(iv);
    }
    return ret;
}


task get_oltp_task_wat()
{
    task ret;
    vector<MyTriple> data;
    for(int i = 0; i < oltp_update_num; i++)
    {
        int idx =  offset.fetch_add(1);
        //cerr << idx << endl;
        if(idx >= line_num) return ret;
        //ret.add_updates(get_insert_sparql(triples[j]));
        data.push_back(triples[idx]);
    }
    auto qv = get_node_query(data);
    for(auto q: qv) ret.add_query(q);
    for(auto t: data)
    {
        auto iv = get_insert_sparql(t);
        ret.add_updates(iv);
    }
    return ret;
}


void run_transaction_1(int isolevel, Txn_manager& txn_m, int idx)
{
    int update_lines = line_num / threads_num;
    int cnt = 0;
    int k = update_lines * idx;
    //cerr << "thread " << idx << "    k: " << k << endl;
    task t;
    string res, sparql;
    int base_time = 1000;
    txn_id_t TID;
    bool prev_succ = true;
    task tsk;
    long start_tv =  Util::get_cur_time();
    while(true){
        if(prev_succ){//reset
            tsk = get_task_1(txn_m, k);
            //print_task(tsk);
            base_time = 1000;
            if(tsk.is_empty()) return;
        }
        else prev_succ = true; //retry
        vector<string>& query = tsk.get_query();
        vector<string>& updates = tsk.get_updates();
        //cout << "query.size()" << query.size() << " " << "updates.size()" << updates.size() << endl;
        int ret = 0;
        TID = txn_m.Begin(static_cast<IsolationLevelType>(isolevel));
        
        for(unsigned i = 0; i < query.size(); i++){
           // cout << "query " << i << " "  << query[i] << endl;
            ret = txn_m.Query(TID, query[i], res);
            if(ret != -100) {
                usleep(base_time);
                //base_time = min(base_time*2, 1000);
                prev_succ = false;
                //cerr << "query failed and abort!" << endl;
                restart_times++;
                break;
            }
        }
        if(prev_succ == false) continue;
        
        for(unsigned i  = 0; i < updates.size(); i++){
            //cout << "update " << i << " " << updates[i] << endl;
            ret = txn_m.Query(TID, updates[i], res);
            if(ret < 0) {
                usleep(base_time);
                //base_time = min(base_time*2, 1000);
                prev_succ = false;
                restart_times++;
                break;
            }
            //if(ret >= 0) cout << "update nums" << ret << endl;
        }
        if(prev_succ == false) continue;
       
        if(txn_m.Commit(TID) != 0) {
            prev_succ = false;
        }
        else{
            k += update_num;
            cnt++;
            if(cnt == 1000){
                long end_tv =  Util::get_cur_time();
                cerr << (end_tv - start_tv) << " ms" << endl;
                return; 
            }
        }
    }
}


void OLTP_workload(int isolevel, Txn_manager& txn_m, int idx)
{
    const int max_time = 250000;
    const int base = 1000;
    // const int wait_time = 1000;
    int update_lines = (line_num+1) / threads_num;
    int k = update_lines * idx;
    //cerr << "oltp thread " << idx << "    k: " << k << endl;
    task t;
    string res, sparql;
    int base_time = base;
    txn_id_t TID;
    // int num = 0;
    bool prev_succ = true;
    task tsk;
    int cnt = 0;
    long start = Util::get_cur_time();
    int restart_cnt = 0;
    while(true){
        if(prev_succ){//reset
            if(is_wat) tsk = get_oltp_task_wat();
            else tsk = get_oltp_task(k);
            k += oltp_update_num;
            //print_task(tsk);
            base_time = base;
            if(tsk.is_empty()) return;
        }
        else prev_succ = true; //retry
        vector<string>& query = tsk.get_query();
        vector<string>& updates = tsk.get_updates();
        //cout << "query.size()" << query.size() << " " << "updates.size()" << updates.size() << endl;
        int ret = 0;
        TID = txn_m.Begin(static_cast<IsolationLevelType>(isolevel));
        
        //int qsize = min(query.size(), oltp_query_num);
        int j = 0;
        for (unsigned i  = 0; i < updates.size(); i++) {
            for (int tt = 0; tt < 2 && j < oltp_query_num; tt++, j++) {
            // cout << "query " << i << " "  << query[i] << endl;
                ret = txn_m.Query(TID, query[j], res);
                if(ret != -100) {
                    usleep(base_time);
                    restart_cnt++;
                    base_time = min(base_time*2, max_time);
                    prev_succ = false;
                    //cerr << "query failed and abort!" << endl;
                    restart_times++;
                    if(restart_times.load() % 1000 == 0) cerr << restart_times.load() << endl;
                    break;
                }
            }
            if(prev_succ == false) break;
        
            //cout << "update " << i << " " << updates[i] << endl;
            ret = txn_m.Query(TID, updates[i], res);
            if(ret < 0) {
                usleep(base_time);
                restart_cnt++;
                base_time = min(base_time*2, max_time);
                prev_succ = false;
                restart_times++;
                if(restart_times.load() % 1000 == 0) cerr << restart_times.load() << endl;
                break;
            }
            //if(ret >= 0) cout << "update nums" << ret << endl;
            if(prev_succ == false) break;

            for(int t = 0;t < 2 && j < oltp_query_num; t++, j++){
            // cout << "query " << i << " "  << query[i] << endl;
                ret = txn_m.Query(TID, query[j], res);
                if(ret != -100) {
                    usleep(base_time);
                    restart_cnt++;
                    base_time = min(base_time*2, max_time);
                    prev_succ = false;
                    //cerr << "query failed and abort!" << endl;
                    restart_times++;
                    if(restart_times.load() % 1000 == 0) cerr << restart_times.load() << endl;
                    break;
                }
            }
            if(prev_succ == false) break;
        }
        if(prev_succ == false) continue;
        for(;j < oltp_query_num; j++){
            // cout << "query " << i << " "  << query[i] << endl;
                ret = txn_m.Query(TID, query[j], res);
                if(ret != -100) {
                    usleep(base_time);
                    restart_cnt++;
                    base_time = min(base_time*2, max_time);
                    prev_succ = false;
                    //cerr << "query failed and abort!" << endl;
                    restart_times++;
                    if(restart_times.load() % 1000 == 0) cerr << restart_times.load() << endl;
                    break;
           }
        }
        if(prev_succ == false) continue;
        if(txn_m.Commit(TID) != 0) {
            restart_cnt++;
            prev_succ = false;
        }
        else{
            long end = Util::get_cur_time();
            time_rec[idx*txn_num+cnt] = end - start;
            start = end;
            restartcnts[idx*txn_num+cnt] = restart_cnt;
            restart_cnt = 0;
            cnt++;

            if(cnt % 100 == 0) cerr << cnt << endl;
            if(cnt == txn_num) return;

        }
    }
}

void print_task(task & tsk)
{
    vector<string> query = tsk.get_query();
    vector<string> updates = tsk.get_updates();
    cout << "print task" << endl;
    for(auto q: query) cout << q << endl;

    for(auto u: updates) cout << u << endl;
}


void run_transaction(int isolevel, Txn_manager& txn_m)
{
    //cerr << "run_transaction thread  "<< endl;
    task t;
    string res, sparql;
    const int base = 1000;
    const int max_time = 250000;
    int base_time = base;
    txn_id_t TID;
    bool prev_succ = true;
    task tsk;
    int k = 0;
    
    while(true){
        if(prev_succ){//reset
            tsk = get_task(txn_m);
            print_task(tsk);
            base_time = base;
            if(tsk.is_empty()) return;
        }
        else prev_succ = true; //retry
        vector<string>& query = tsk.get_query();
        vector<string>& updates = tsk.get_updates();
        //cout << "query.size()" << query.size() << " " << "updates.size()" << updates.size() << endl;
        int ret = 0;
        TID = txn_m.Begin(static_cast<IsolationLevelType>(isolevel));
        
        for(unsigned i = 0; i < query.size(); i++){
           // cout << "query " << i << " "  << query[i] << endl;
            ret = txn_m.Query(TID, query[i], res);
            if(ret != -100) {
                usleep(base_time);
                base_time = min(base_time*2, max_time);
                prev_succ = false;
                //cerr << "query failed and abort!" << endl;
                restart_times++;
                //if(restart_times.load() % 1000 == 0) cerr << restart_times.load() << endl;
                break;
            }
        }
        if(prev_succ == false) continue;
        
        for(unsigned i  = 0; i < updates.size(); i++){
            //cout << "update " << i << " " << updates[i] << endl;
            ret = txn_m.Query(TID, updates[i], res);
            if(ret < 0) {
                usleep(base_time);
                base_time = min(base_time*2, max_time);
                prev_succ = false;
                restart_times++;
                //if(restart_times.load() % 1000 == 0) cerr << restart_times.load() << endl;
                break;
            }
            //if(ret >= 0) cout << "update nums" << ret << endl;
        }
        if(prev_succ == false) continue;
       
        if(txn_m.Commit(TID) != 0) {
            restart_times++;
            if(restart_times.load() % 1000 == 0) cerr << restart_times.load() << endl;
            prev_succ = false;
        }
        else{
            k++;
            
            if(k == 1000){
                
                return;
            } 
        }
    }
}


void read_only_transaction(int isolevel, Txn_manager& txn_m)
{
    int times = 50;
    txn_id_t TID;
    int ret = 0;
    string res;
    cerr << "read_only_transaction" << endl;
    for(int i = 0; i < times; i++)
    {
        //cerr << i << endl;
        TID = txn_m.Begin(static_cast<IsolationLevelType>(isolevel));
        ret = txn_m.Query(TID, lubm_q[1], res);
        if(ret < -100) {
            cerr << " read_only_transaction failed!" << ret << endl;
            continue;
        }
        ret = txn_m.Query(TID, lubm_q[1], res);
        if(ret != -100) {
            cerr << " read_only_transaction failed!  " << ret <<endl;
            continue;
        }
        if(txn_m.Commit(TID) != 0) {
            cerr << "read only transaction commit failed!" << endl;
            continue;
        }

    }
}

void create_versions(Txn_manager& txn_m)
{
	string query = "select ?v {<V1> <R1> ?v.}";
	string insert1 = "insert data { <V1> <R1> \"5\"^^<http://www.w3.org/2001/XMLSchema#integer> }";
	string insert2 = "insert data { <V1> <R1> \"15\"^^<http://www.w3.org/2001/XMLSchema#integer> }";
	string delete1 = "delete data { <V1> <R1> \"5\"^^<http://www.w3.org/2001/XMLSchema#integer> }";
	string delete2 = "delete data { <V1> <R1> \"15\"^^<http://www.w3.org/2001/XMLSchema#integer> }";

	string res;

	txn_id_t id = txn_m.Begin();
	txn_m.Query(id, query, res);
	txn_m.print_txn_dataset(id);
	txn_m.Commit(id);

	id = txn_m.Begin();
	txn_m.Query(id, insert1, res);
	txn_m.print_txn_dataset(id);
	txn_m.Commit(id);

	id = txn_m.Begin();
	txn_m.Query(id, insert2, res);
	txn_m.print_txn_dataset(id);
	txn_m.Commit(id);

	id = txn_m.Begin();
	txn_m.Query(id, delete1, res);
	txn_m.print_txn_dataset(id);
	txn_m.Commit(id);

	id = txn_m.Begin();
	txn_m.Query(id, delete2, res);
	txn_m.print_txn_dataset(id);
	txn_m.Commit(id);
}

void no_txn_update(Database &db)
{
    const string insert_filename = "./scripts/insert.nt";
    const string delete_filename = "./scripts/delete.nt";
    fstream in; 
    
    string line, sparql, res;
    ResultSet rs;
    in.open(insert_filename, ios::in);
    while(getline(in, line))
    {
         sparql = "insert data {" + line + "}";
         FILE *fp = stdout;
         db.query(sparql, rs, fp);
    }
    in.close();

    in.open(delete_filename, ios::in);
    while(getline(in, line))
    {
         sparql = "delete data {" + line + "}";
         FILE *fp = stdout;
         db.query(sparql, rs, fp);
    }
    in.close();
}

bool single_txn(int threads_num, Txn_manager& txn_m)
{
    txn_id_t TID =  txn_m.Begin(static_cast<IsolationLevelType>(3));
    ifstream in;
    string line, sparql, res;
    in.open("./scripts/insert.nt", ios::in);
    int num = 0;
    while(getline(in, line))
    {
        sparql = "insert data {" + line + "}";
        int ret = txn_m.Query(TID, sparql, res);
        num += ret;
        //txn_m.Get_Transaction(TID)->print_all();
        //if(ret != 0) cerr << "wrong answer!" << endl;
        // getline(in, line);
        // sparql = "insert data {" + line + "}";
        // ret = txn_m.Query(TID, sparql, res);
        //txn_m.Get_Transaction(TID)->print_all();
        //if(ret != 0) cerr << "wrong answer!" << endl;
    }
    in.close();
    in.open("./scripts/delete.nt", ios::in);
    while(getline(in, line))
    {
        sparql = "delete data{" + line + "}";
        int ret = txn_m.Query(TID, sparql, res);
        //if(ret == 0) cerr << "wrong answer!wrong answer!wrong answer!wrong answer!" << endl;
        num += ret;
    }
    //txn_m.Get_Transaction(TID)->print_all();
    in.close();
    txn_m.Commit(TID);
    cout << "update num: " << num << endl;
    return true;
}

void check_results(int threads_num, Txn_manager& txn_m)
{
    txn_id_t TID =  txn_m.Begin();
    ifstream in;
    in.open("./scripts/insert.nt", ios::in);
    for(int i = 0; i < threads_num; i++)
    {
        string line, sparql, res;
        getline(in, line);
        sparql = "ask where {" + line + "}";
		txn_m.Query(TID, sparql, res);
        // int ret = txn_m.Query(TID, sparql, res);
        cout << res << endl;
        getline(in, line);
        sparql = "ask where {" + line + "}";
		txn_m.Query(TID, sparql, res);
        // ret = txn_m.Query(TID, sparql, res);
        cout << res << endl;
    }
    in.close();
    in.open("./scripts/delete.nt", ios::in);
    for(int i = 0; i < threads_num; i++)
    {
        string line, sparql, query, res;
        getline(in, line);
        sparql = "ask where{" + line + "}";
        int ret = txn_m.Query(TID, sparql, res);
        cout << ret << endl;
    }
    txn_m.Commit(TID);
}

int main(int argc, char* argv[])
{
    //open_file();
    if(model == 3 ){
    read_triples();
    cerr << "triples read" << endl;
    auto tk = get_oltp_task_wat();
    auto qs = tk.get_query();
    auto us = tk.get_updates();
    for(unsigned i = 0; i < qs.size(); i++) cerr << qs[i] << endl;
    for(unsigned i = 0; i < us.size(); i++) cerr << us[i] << endl;
    }
    else{
        open_file();
        // auto tk = get_task();
        // auto qs = tk.get_query();
        // auto us = tk.get_updates();
        // for(int i = 0; i < qs.size(); i++) cerr << qs[i] << endl;
        // for(int i = 0; i < us.size(); i++) cerr << us[i] << endl;
    }
    // atomic<__uint128_t> l;
    // cerr << l.is_lock_free() << endl;
    Util util;
    string lubm_10M = "lubm10M";
    string lubm_100M = "lubm100M";
    string lubm_1M = "lubm1M";
    string wat_1M = "watdiv1M";
    string wat_10M = "watdiv10M";
	string db_folder = "lubm";
    
	Database _db(db_folder);
	_db.load();
	cerr << "finish loading" << endl;
    Txn_manager txn_m(&_db, string("lubm_1M"));
    

    TID = txn_m.Begin(2);

    string res;
    int ret = txn_m.Query(TID, "select ?x where { <m> ?x <t>.}", res);
    cerr << ret << " " << res << endl;


    

/*     select ?x where { <m> ?x <t>.}
    insert data { <m> <s> <t>.}
    select ?x where { <m> ?x <t>.}
    delete data { <m> <s> <t>.}
    select ?x where { <m> ?x <t>.}
    insert data { <m> <s> <t>.}
    select ?x where { <m> ?x <t>.} */

    return;

    //threads_num = thread::hardware_concurrency()-1;
    vector<thread> pool(threads_num);
    int n = pool.size();
    long start1 = Util::get_cur_time();
    if(model == 1){
        for(int i = 0; i < n ; i++)
        {
            pool[i] = thread(run_transaction, 1 , ref(txn_m));
        }
    }
    else if(model == 2){
        for(int i = 0; i < n ; i++)
        {
            pool[i] = thread(run_transaction_1, 2 , ref(txn_m), i);
        }
    }
    else if(model == 3){
        for(int i = 0; i < n ; i++)
        {
            pool[i] = thread(OLTP_workload, 1 , ref(txn_m), i);
        }
    }

    // // for(int i = 0; i < n; i++)
    // // {
    // //     pool[i].join();
    // // }

    // // for(int i = 0; i < n; i++)
    // // {
    // //     pool[i] = thread(read_only_transaction, 1, ref(txn_m));
    // // }
    if(model == 1 || model == 2 || model == 3){
        for(int i = 0; i < n; i++)
        {
            pool[i].join();
        }
    }
    long end1 = Util::get_cur_time();
    cerr << end1 - start1 << "ms" << endl;
    //cerr << restart_times.load() << endl;
    // cout << "workload finished!" << endl;
    cerr << "restart times" << restart_times.load() << endl;
    cerr << time_rec[0] << "   " << restartcnts[0] << endl;
    write_statics();
    //cerr << "write statics over" << endl;
    return 0;
}