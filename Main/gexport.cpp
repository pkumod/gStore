/*=============================================================================
# Filename: gexport.cpp
# Author: suxunbin
# Last Modified: 2019-07-23 15:37
# Description: export a database to get .nt file
=============================================================================*/

#include "../Database/Database.h"
#include "../Util/Util.h"

using namespace std;

int
main(int argc, char* argv[])
{
  Util util;

  string db_name;
  string filepath;
  if (argc == 1) {
    cout << "You need to input the database name that you want to export." << endl;
    return 0;
  } else if (argc == 2) {
    db_name = argv[1];
    int len = db_name.length();
    if (db_name.length() > 3 && db_name.substr(len - 3, 3) == ".db") {
      cout << "The database name can not end with .db" << endl;
      return 0;
    }
    filepath = db_name + ".nt";
  } else if (argc == 3) {
    db_name = argv[1];
    int len = db_name.length();
    if (db_name.length() > 3 && db_name.substr(len - 3, 3) == ".db") {
      cout << "The database name can not end with .db" << endl;
      return 0;
    }
    filepath = argv[2];
    boost::filesystem::path filePath(filepath);
    if (!boost::filesystem::exists(filePath.parent_path()))
      boost::filesystem::create_directories(filePath.parent_path());
  }

  cout << "gexport..." << endl;

  Database system_db("system");
  system_db.load();

  string sparql = "ASK WHERE{<" + db_name + "> <database_status> \"already_built\".}";
  ResultSet ask_rs;
  FILE* ask_ofp = stdout;
  int ret = system_db.query(sparql, ask_rs, ask_ofp);
  if (ask_rs.answer[0][0] == "false") {
    cout << "The database does not exist." << endl;
    return 0;
  }

  cout << "start exporting the database......" << endl;
  Database _db(db_name);
  _db.load();
  cout << "finish loading" << endl;

  sparql = "select * where{?x ?y ?z.}";
  ResultSet _rs;
  FILE* ofp = fopen(filepath.c_str(), "w");
  ret = _db.query(sparql, _rs, ofp, true, true);
  fflush(ofp);
  fclose(ofp);
  ofp = NULL;
  cout << "finish exporting the database." << endl;

  return 0;
}
