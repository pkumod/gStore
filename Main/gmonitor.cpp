/*=============================================================================
# Filename: gmonitor.cpp
# Author: suxunbin
# Mail: suxunbin@pku.edu.cn
# Last Modified: 2019-07-26 16:00
# Description: used to show information of the database
=============================================================================*/

#include "../Util/Util.h"
#include "../Database/Database.h"

using namespace std;

int main(int argc, char* argv[])
{
  Util util;
  string db_name;
  if (argc == 1) {
    cout << "You need to input the database name that you want to show." << endl;
    return 0;
  } else if (argc == 2) {
    db_name = argv[1];
    int len = db_name.length();
    if (db_name.length() > 3 && db_name.substr(len - 3, 3) == ".db") {
      cout << "The database name can not end with .db" << endl;
      return 0;
    }
  } else {
    cout << "The number of parameters is not correct." << endl;
    return 0;
  }

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

  cout << "start loading the database......" << endl;
  Database _db(db_name);
  _db.load();
  cout << "finish loading" << endl;

  sparql = "select ?p ?o where{<" + db_name + "> ?p ?o.}";
  ResultSet _rs;
  FILE* ofp = stdout;
  ret = system_db.query(sparql, _rs, ofp);
  string creator;
  string built_time;
  for (int i = 0; i < _rs.ansNum; i++) {
    string p = _rs.answer[i][0];
    string o = _rs.answer[i][1];
    if (p == "<built_by>")
      creator = o.substr(1, o.length() - 2);
    else if (p == "<built_time>")
      built_time = o;
  }
  unsigned triple_num = _db.getTripleNum();
  unsigned entity_num = _db.getEntityNum();
  unsigned literal_num = _db.getLiteralNum();
  unsigned subject_num = _db.getSubNum();
  unsigned predicate_num = _db.getPreNum();

  cout << "\n========================================\n";
  string output = "database: " + db_name + "\ncreator: " + creator + "\nbuilt_time: " + built_time + "\n";
  output = output + "triple num: " + Util::int2string(triple_num) + "\nentity num: " + Util::int2string(entity_num) + "\nliteral num: " + Util::int2string(literal_num) + "\nsubject num: " + Util::int2string(subject_num) + "\npredicate num: " + Util::int2string(predicate_num)
           + "\n========================================\n";
  cout << output;
  return 0;
}
