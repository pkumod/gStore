/*=============================================================================
# Filename: gdrop.cpp
# Author: suxunbin
# Mail: suxunbin@pku.edu.cn
# Last Modified: 2018-10-30 16:21
# Description: used to drop database
=============================================================================*/

#include "../Util/Util.h"
#include "../Database/Database.h"

using namespace std;

int main(int argc, char* argv[])
{
  Util util;
  string db_name;
  if (argc == 1) {
    cout << "You need to input the database name that you want to drop." << endl;
    return -1;
  } else if (argc == 2) {
    db_name = argv[1];
    int len = db_name.length();
    if (db_name.length() > 3 && db_name.substr(len - 3, 3) == ".db") {
      cout << "The database name can not end with .db" << endl;
      return -1;
    }
  } else {
    cout << "The number of parameters is not correct." << endl;
    return -1;
  }
  if (!boost::filesystem::exists(db_name + ".db")) {
    cout << "The database that you want to drop does not exist." << endl;
    return -1;
  };
  //delete database information in system.db
  Database system_db("system");
  system_db.load();
  string sparql = "DELETE WHERE {<" + db_name + "> ?x ?y.}";
  ResultSet _rs;
  FILE* ofp = stdout;
  string msg;
  int ret = system_db.query(sparql, _rs, ofp);
  if (ret <= -100) // select query
  {
    if (ret == -100)
      msg = _rs.to_str();
    else //query error
      msg = "query failed";
  } else //update query
  {
    if (ret >= 0)
      msg = "update num : " + Util::int2string(ret);
    else //update error
      msg = "update failed.";
    if (ret != -100)
      cout << msg << endl;
  }
  string cmd = "rm -r " + db_name + ".db";
  system(cmd.c_str());
  cout << db_name + ".db is dropped successfully!" << endl;
  return 0;
}
