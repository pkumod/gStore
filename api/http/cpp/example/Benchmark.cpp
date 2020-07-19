/*=============================================================================
# Filename: Benchmark.cpp
# Author: suxunbin
# Last Modified: 2019-5-17 21:02
# Description: a simple example of multi-thread query
=============================================================================*/

#include "client.h"
#include <pthread.h>
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <fstream>
using namespace std;

// before you run this example, make sure that you have started up ghttp service (using bin/ghttp db_name port)
// default db_name: lubm(must be built in advance)
#define IP "127.0.0.1"
#define Port 9000
#define username "root"
#define password "123456"
#define tnum 1000
#define RequestType "POST"

bool correctness = true;

struct MyThread_args {
  int num;
  int rnum;
  string sparql;
  string filename;
  string request_type;
};

void* MyThread_run(void* thread_args)
{
  struct MyThread_args* args;
  args = (struct MyThread_args*)thread_args;
  GstoreConnector gc(IP, Port, username, password);

  // query
  string res = gc.query("lubm", "json", args->sparql, args->request_type);

  // fquery
  //gc.fquery("lubm", "json", args->sparql, args->filename, args->request_type);
  //ifstream f(args->filename);
  //stringstream buffer;
  //buffer << f.rdbuf();
  //string res = buffer.str();
  //f.close();

  //count the num
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

  // compare the result
  if (args->rnum != Num) {
    correctness = false;
    cout << "sparql: " << args->sparql << endl;
    cout << "Num: " << Num << endl;
  }
  pthread_exit(NULL);
}
int main()
{
  int result[6] = { 15, 0, 828, 27, 27, 5916 };
  string* sparql = new string[6];
  sparql[0] = "select ?x where \
                { \
                ?x    <ub:name>    <FullProfessor0>. \
                }";
  sparql[1] = "select distinct ?x where\
                { \
                ?x <rdf:type>  <ub:GraduateStudent>.\
                ?y <rdf:type>  <ub:GraduateStudent>.\
                ?z <rdf:type>  <ub:GraduateStudent>.\
                ?x <ub:memberOf>  ?z.\
                ?z <ub:subOrganizationOf> ?y.\
                ?x <ub:undergaduateDegreeFrom> ?y.\
                }";
  sparql[2] = "select distinct ?x where\
                { \
                ?x   <rdf:type>  <ub:Course>.\
                ?x   <ub:name>   ?y.\
                }";
  sparql[3] = "select ?x where \
                { \
                ?x    <rdf:type>    <ub:UndergraduateStudent>. \
                ?y    <ub:name> <Course1>. \
                ?x    <ub:takesCourse>  ?y. \
                ?z    <ub:teacherOf>    ?y. \
                ?z    <ub:name> <FullProfessor1>. \
                ?z    <ub:worksFor>    ?w. \
                ?w    <ub:name>    <Department0>. \
                }";
  sparql[4] = "select distinct ?x where\
                { \
                ?x    <rdf:type>    <ub:UndergraduateStudent>. \
                ?y    <ub:name> <Course1>. \
                ?x    <ub:takesCourse>  ?y. \
                ?z    <ub:teacherOf>    ?y. \
                ?z    <ub:name> <FullProfessor1>. \
                ?z    <ub:worksFor>    ?w. \
                ?w    <ub:name>    <Department0>. \
                }";
  sparql[5] = "select distinct ?x where\
                { \
                ?x    <rdf:type>    <ub:UndergraduateStudent>.\
                }";

  pthread_t qt[tnum];
  void* status;
  struct MyThread_args args[tnum];
  for (int i = 0; i < tnum; i++) {
    string filename = "result/res" + to_string(i) + ".txt";
    args[i].num = i;
    args[i].rnum = result[i % 6];
    args[i].sparql = sparql[i % 6];
    args[i].filename = filename;
    args[i].request_type = RequestType;
    pthread_create(&qt[i], NULL, MyThread_run, (void*)&args[i]);
  }
  for (int i = 0; i < tnum; i++) {
    pthread_join(qt[i], &status);
  }
  if (correctness == true)
    cout << "The answers are correct!" << endl;
  else
    cout << "The answers exist errors!" << endl;
  pthread_exit(NULL);
  return 0;
}
