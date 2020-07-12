/*=============================================================================
# Filename: ginit.cpp
# Author: suxunbin
# Mail: suxunbin@pku.edu.cn
# Last Modified: 2018-10-17 14:59
# Description: used to initialize the system.db
# Modified by liwenjie
# Modified Date: 2020-03-26 10:33
# Description：Add the args "-cv" for updating the coreversion and the args "-av" for updating the apiversion in ginit function
# Description：Add the args "-u" for updating the version information when updating the gstore.
=============================================================================*/

#include "../Util/Util.h"
#include "../Database/Database.h"
#include <iostream>
#include <fstream>
using namespace std;

int main(int argc, char* argv[])
{
  string op;
  if (argc > 1) {
    op = argv[1];
    if (op == "-make") {
      if (boost::filesystem::exists("system.db"))
        return 0;
    } else if (op == "-d") {
      if (argc == 2) {
        cout << "You need to input at least one database name." << endl;
        return 0;
      }
    } else if (op == "-cv" || op == "-av") {
      if (argc == 2) {
        cout << "You need to input the value of version." << endl;
        return 0;
      } else {
        string version = argv[2];
        string versionname = "<CoreVersion>";
        if (op == "-av") {
          versionname = "<APIVersion>";
        }
        string sparql = "DELETE where {" + versionname + " <value> ?x.}";

        string _db_path = "system";
        Util util;
        Database* _db = new Database(_db_path);

        _db->load();
        ResultSet _rs;
        FILE* ofp = stdout;
        string msg;
        int ret = _db->query(sparql, _rs, ofp);
        sparql = "INSERT DATA {" + versionname + " <value>	\"" + version + "\".}";
        ret = _db->query(sparql, _rs, ofp);
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
        delete _db;
        _db = NULL;
        cout << "the " << versionname << " is updated successfully!" << endl;
        return 0;
      }
    } else if (op == "-u") {
      //update the gstore, and init the version info
      string file = "data/system/version.nt";
      if (boost::filesystem::exists(file) == false) {
        cout << "the file of version information is not found!" << endl;

        return 0;
      }
      string sparql = "Delete WHERE { <CoreVersion> ?x ?y. <APIVersion> ?x1 ?y1.}";

      string _db_path = "system";
      Util util;
      Database* _db = new Database(_db_path);

      _db->load();
      ResultSet _rs;
      FILE* ofp = stdout;
      string msg;
      int ret = _db->query(sparql, _rs, ofp);
      ifstream infile;
      infile.open(file.data()); //将文件流对象与文件连接起来
      string s;
      sparql = "INSERT DATA {";
      while (getline(infile, s)) {
        if (s != "")
          sparql = sparql + s;
      }
      infile.close();
      sparql = sparql + "}";
      cout << "the sparql of initversion is:" << sparql << endl;

      //sparql = "INSERT DATA {" + versionname + " <value>	\"" + version + "\".}";
      ret = _db->query(sparql, _rs, ofp);
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
      delete _db;
      _db = NULL;
      cout << "the value of version is updated successfully!" << endl;
      return 0;
    } else {
      cout << "The initialization option is not correct." << endl;
      return 0;
    }
  }

  //build system.db
  Util util;
  string _db_path = "system";
  string _rdf = "data/system/system.nt";
  Database* _db = new Database(_db_path);
  bool flag = _db->build(_rdf);
  if (flag) {
    cout << "import RDF file to database done." << endl;
    ofstream f;
    f.open("./" + _db_path + ".db/success.txt");
    f.close();
  } else //if fails, drop system.db and return
  {
    cout << "import RDF file to database failed." << endl;
    string cmd = "rm -r " + _db_path + ".db";
    system(cmd.c_str());
    delete _db;
    _db = NULL;
    return 0;
  }

  //insert built_time of system.db
  delete _db;
  _db = new Database(_db_path);
  _db->load();
  string time = Util::get_date_time();
  string sparql = "INSERT DATA {<system> <built_time> \"" + time + "\".";
  if (argc > 1) {
    op = argv[1];
    if (op == "-d") {
      for (int i = 2; i < argc; i++) {
        string db_name = argv[i];
        sparql = sparql + "<" + db_name + "> <database_status> \"already_built\".";
        sparql = sparql + "<" + db_name + "> <built_by> <root>.";
        sparql = sparql + "<" + db_name + "> <built_time> \"" + time + "\".";
      }
    }
  }
  sparql = sparql + "}";
  ResultSet _rs;
  FILE* ofp = stdout;
  string msg;
  int ret = _db->query(sparql, _rs, ofp);
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
  delete _db;
  _db = NULL;
  cout << "system.db is built successfully!" << endl;
  return 0;
}
