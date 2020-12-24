/*=============================================================================
# Filename: gshow.cpp
# Author: suxunbin
# Mail: suxunbin@pku.edu.cn
# Last Modified: 2019-07-25 17:00
# Description: used to show all the databases that have already been built
=============================================================================*/

#include "../Util/Util.h"
#include "../Database/Database.h"

using namespace std;

struct DBInfo {
  public:
  string db_name;
  string creator;
  string built_time;
  DBInfo()
  {
  }
  DBInfo(string _db_name)
  {
    db_name = _db_name;
  }
  ~DBInfo()
  {
  }
};

string Getstr(string s)
{
  int len = s.length();
  return s.substr(1, len - 2);
}

int main(int argc, char* argv[])
{
  Util util;

  Database system_db("system");
  system_db.load();
  string sparql = "select ?s where{?s <database_status> \"already_built\".}";
  ResultSet _rs;
  FILE* ofp = stdout;
  int ret = system_db.query(sparql, _rs, ofp);
  DBInfo* databases = new DBInfo[_rs.ansNum + 1];
  databases[0].db_name = "<system>";
  for (int i = 0; i < _rs.ansNum; i++) {
    databases[i + 1].db_name = _rs.answer[i][0];
    string sparql1 = "select ?p ?o where{" + _rs.answer[i][0] + " ?p ?o.}";
    ResultSet _rs1;
    FILE* ofp1 = stdout;
    int ret1 = system_db.query(sparql1, _rs1, ofp1);
    for (int j = 0; j < _rs1.ansNum; j++) {
      string p = _rs1.answer[j][0];
      string o = _rs1.answer[j][1];
      if (p == "<built_by>")
        databases[i + 1].creator = o;
      else if (p == "<built_time>")
        databases[i + 1].built_time = o;
    }
  }
  string sparql1 = "select ?p ?o where{<system> ?p ?o.}";
  ResultSet _rs1;
  FILE* ofp1 = stdout;
  int ret1 = system_db.query(sparql1, _rs1, ofp1);
  for (int j = 0; j < _rs1.ansNum; j++) {
    string p = _rs1.answer[j][0];
    string o = _rs1.answer[j][1];
    if (p == "<built_by>")
      databases[0].creator = o;
    else if (p == "<built_time>")
      databases[0].built_time = o;
  }
  cout << "\n========================================\n";
  for (int i = 0; i < _rs.ansNum + 1; i++) {
    string output = "database: " + Getstr(databases[i].db_name) + "\ncreator: " + Getstr(databases[i].creator) + "\nbuilt_time: " + databases[i].built_time + "\n========================================\n";
    cout << output;
  }
  return 0;
}
