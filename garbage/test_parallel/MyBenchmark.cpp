#include "client.h"
#include <pthread.h>
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <fstream>
#include <time.h>
#include <sys/time.h>
using namespace std;
#define tnum 6
#define bnum 10
#define lnum 10
#define unum 10
bool correctness = true;
bool lcorrectness = true;
bool ucorrectness = true;
int port = 3502;

pthread_mutex_t mutex;

long get_cur_time()
{
  timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

string int2string(int n)
{
  string s;
  stringstream ss;
  ss << n;
  ss >> s;
  return s;
}

struct MyThread_args {
  int num;
  string sparql;
  int rnum;
  bool parallel;
};
struct mm_args {
  string db_name;
};
void* MyThread_run(void* thread_args)
{
  struct MyThread_args* args;
  args = (struct MyThread_args*)thread_args;
  CHttpClient hc;
  string res;
  int ret;
  //ret = hc.Get("http://172.31.222.91:10086/?operation=query&db_name=DBPEDIA&format=json&sparql="+args->sparql,res);
  ret = hc.Get("http://172.31.222.91:10086/?operation=query&db_name=watdiv10M&format=json&sparql=" + args->sparql, res);

  //open result file
  //ifstream f("result/"+int2string(args->num)+".txt");
  //f>>res;
  //f.close();

  //get result count
  int m = 0;
  for (int i = 0; i < args->sparql.length(); ++i) {
    if (args->sparql[i] == '?')
      ++m;
    if (args->sparql[i] == '{')
      break;
  }
  int n = 0;
  for (int i = 0; i < res.length(); ++i) {
    if (res[i] == '{')
      ++n;
  }
  int Num = (n - 3) / (m + 1);

  if (!args->parallel) {
    args->rnum = Num;
  } else if (args->rnum != Num) {

    correctness = false;
    cout << "sparql: " << args->sparql << endl;
    cout << "Num: " << Num << endl;
    cout << "Result is: " << res << endl;
  }
  pthread_exit(NULL);
}
/*void* load_thread(void* thread_args)
{
	struct mm_args *args;
	args = (struct mm_args *)thread_args;
	string db_name = args->db_name;
	CHttpClient hc;
	string res;
	int ret;
	ret = hc.Get("http://172.31.222.93:9016/?operation=load&db_name="+db_name+"&username=root&password=123456", res);
	res ="load: " + res + "\n";
	cout << res;
	pthread_exit(NULL);
}
void* unload_thread(void* thread_args)
{
	struct mm_args *args;
	args = (struct mm_args *)thread_args;
	string db_name = args->db_name;

	CHttpClient hc;
	string res;
	int ret;
	ret = hc.Get("http://172.31.222.93:9016/?operation=unload&db_name="+db_name+"&username=root&password=123456", res);
	res = "unload: " + res + "\n";
	cout << res;
	pthread_exit(NULL);
}*/
int main()
{
  ifstream fin("queries.txt");
  int num;
  fin >> num;

  int* result = new int[num];

  // generating sparql
  string* sparql = new string[num];
  getline(fin, sparql[0]);

  for (int i = 0; i < num; i++) {
    getline(fin, sparql[i]);
    sparql[i] = sparql[i].substr(1, sparql[i].length() - 1);
    sparql[i] = "select ?x where {" + sparql[i] + ".}";
  }

  pthread_t* qt = new pthread_t[num];
  struct MyThread_args* args = new MyThread_args[num];
  void* status;
  // query in series to get correct answers
  for (int i = 0; i < num; i++) {
    args[i].num = i;
    args[i].sparql = sparql[i];
    pthread_create(&qt[i], NULL, MyThread_run, (void*)&args[i]);
    pthread_join(qt[i], &status);
    result[i] = args[i].rnum;
  }

  // Now test parallel version
  long t1 = get_cur_time();
  for (int i = 0; i < num; i++) {
    args[i].num = i;
    args[i].sparql = sparql[i];
    args[i].rnum = result[i];
    pthread_create(&qt[i], NULL, MyThread_run, (void*)&args[i]);
  }
  long t3 = get_cur_time();
  printf("creating threads, cost %dms\n", t3 - t1);

  for (int i = 0; i < num; i++) {
    pthread_join(qt[i], &status);
  }
  if (correctness == true)
    cout << "The answers are correct!" << endl;
  else
    cout << "The answers exist errors!" << endl;

  long t2 = get_cur_time();
  cout << "Total time used " << t2 - t1 << "ms" << endl;
  pthread_exit(NULL);
  return 0;
}
