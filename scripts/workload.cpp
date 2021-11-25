#include <iostream>
#include <set>
#include <vector>
#include <deque>
#include <algorithm>
#include <fstream>
#include <mutex>
#include <thread>
#include "../Database/Txn_manager.h"

using namespace std;

struct task
{
    string op1;
    string op2;
    string op3;
    task() {op1 = op2 = op3 = "";}
};

class task_manager
{
private:
    ifstream insert_if;
    ifstream delete_if;
    mutex l;
    const string insert_filename = "./scripts/insert.nt";
    const string delete_filename = "./scripts/delete.nt";
public:
    task get_task();
    task_manager();
    ~task_manager();
};

task_manager::task_manager()
{
    insert_if.open(insert_filename, ios::in);
    delete_if.open(delete_filename, ios::in);
}

task_manager::~task_manager()
{
    insert_if.close();
    delete_if.close();

}

task task_manager::get_task()
{
    task t;
    l.lock();
    string sparql;
    getline(insert_if, sparql);
    if(insert_if.eof()) {
        l.unlock();
        return t;
    }
    t.op1 = sparql;
    getline(insert_if, sparql);
    t.op2 = sparql;
    getline(delete_if, sparql);
    t.op3 = sparql;
    l.unlock();
    return t;
}

class writer
{
private:
    ofstream of;
    mutex lock;
    atomic<int> n;
    const string out_filename = "./scripts/statics.txt";
public:
    writer();
    ~writer();
    void write_task(task &t, int code);
    void write_num(int num);
    void write_dataset(vector<IDSet>& readset, vector<IDSet>& writeset, txn_id_t TID);
};

writer::writer()
{
    of.open(out_filename, ios::out);
    n.store(1);
}

writer::~writer()
{
    of.close();
}

void writer::write_task(task &t, int code)
{
    lock.lock();
    of << "#"  << code << endl << t.op1 << endl << t.op2 << endl << t.op3 << endl;
    n++;
    lock.unlock();
}

void writer::write_num(int num)
{
    lock.lock();
    of << "###" << n.load() << ":   " << num << endl;
    n++; 
    lock.unlock();
}

void writer::write_dataset(vector<IDSet>& readset, vector<IDSet>& writeset, txn_id_t TID)
{
    lock.lock();
    of << TID << endl;
    of << "read set: " << endl;
    for(int i = 0; i < 3; i++){
        for(auto id: readset[i])
        {
            of << id << "  ";
        }
        of << endl;
    }
    of << "write set: " << endl;
    for(int i = 0; i < 3; i++){
        for(auto id: writeset[i])
        {
            of << id << " ";
        }
        of << endl;
    }
    of << endl;
    lock.unlock();
}

task_manager t_m;
writer w;

void func()
{
    task t;
    int cnt = 0;
    while(1){
        t = t_m.get_task();
        if(t.op1 == "") break;
        //start transaction
        w.write_task(t, 1);
        cnt++;
    }
    cout << this_thread::get_id() << "         " << cnt << endl;
}

void run_transaction(int isolevel, Txn_manager& txn_m)
{
    task t;
    while(true){
    t = t_m.get_task();
    if(t.op1 == "") return;
    string res, sparql;
    int cnt = 0;
    int base_time = 1000;
    txn_id_t TID;
    int num = 0;
    while(true){
        TID = txn_m.Begin(static_cast<IsolationLevelType>(isolevel));
        //cerr << "transaction #" << TID << endl;
        int ret;
        cnt++;
        sparql = "insert data {" + t.op1 + "}";
        ret = txn_m.Query(TID, sparql, res);
        if(ret < 0) {
            //cerr << "insert 1 failed !   " << ret << endl;
            usleep(base_time);
            base_time = min(base_time*2, 1000000);
            continue;
        }
        sparql = "insert data {" + t.op2 + "}";
        ret = txn_m.Query(TID, sparql, res);
        if(ret < 0) {
            //cerr << "insert 2 failed !   " << ret << endl;
            usleep(base_time);
            base_time = min(base_time*2, 1000000);
            continue;
        }
        sparql = "delete data {" + t.op3 + "}";
        ret = txn_m.Query(TID, sparql, res);
        if(ret < 0) {
            //cerr << "delete 1 failed !   " << ret << endl;
            usleep(base_time);
            base_time = min(base_time*2, 1000000);
            continue;
        }
        if(txn_m.Commit(TID) == 0) break;
    }
    w.write_num(cnt);
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
        int ret = txn_m.Query(TID, sparql, res);
        cout << res << endl;
        getline(in, line);
        sparql = "ask where {" + line + "}";
        ret = txn_m.Query(TID, sparql, res);
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
        cout << res << endl;
    }
    txn_m.Commit(TID);
}
int main(int argc, char* argv[])
{
    Util util;

	string db_folder = "lubm";

	Database _db(db_folder);
	_db.load();
	cout << "finish loading" << endl;
    Txn_manager txn_m(&_db, string("lubm"));
    int threads_num;
    //threads_num = thread::hardware_concurrency()-1;
    threads_num = 17;
    vector<thread> pool(threads_num);
    int n = pool.size();
    cout << n;
    for(int i = 0; i < n; i++)
    {
        pool[i] = thread(run_transaction, 1 , ref(txn_m));
        //pool[i] = thread(create_versions, ref(txn_m));
        //pool[i] = thread(func);
    }

    for(int i = 0; i < n; i++)
    {
        pool[i].join();
    }

    // single_txn(61, txn_m);
    // check_results(61, txn_m);
    // no_txn_update(_db);
    // single_txn(61, txn_m);
    cout << "workload finished!" << endl;
    return 0;
}