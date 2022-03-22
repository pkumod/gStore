/*=============================================================================
# Filename: gquery.cpp
# Author: Bookug Lobert
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-20 12:23
# Description: query a database, there are several ways to use this program:
1. ./gquery                                        print the help message
2. ./gquery --help                                 simplified as -h, equal to 1
3. ./gquery db_folder query_path                   load query from given path fro given database
4. ./gquery db_folder                              load the given database and open console
=============================================================================*/

#include "../Database/Database.h"
#include "../Util/Util.h"
//#include "../Util/Slog.h"

using namespace std;


/**
 * ref from: https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
 */
template<typename ... Args>
std::string string_format( const std::string& format, Args ... args )
{
  int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
  if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
  auto size = static_cast<size_t>( size_s );
  auto buf = std::unique_ptr<char[]>(new char[size]);
  std::snprintf( buf.get(), size, format.c_str(), args ... );
  return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}
string db_folder = "Yago";
string QueryDir = "YagoQueries/";

int queryNum = 9;

int
main(int argc, char * argv[])
{
  Util util;
  //#endif
  //Log.init("slog.properties");

  Database _db(db_folder);
  _db.load();
  cout << "finish loading" << endl;

  for(int query_index = 1;query_index<=queryNum;query_index++)
  {
    string queryfilename = string_format("%s%d.sql", QueryDir.c_str(),query_index );
    string resultfile = string_format("%s%d.result.txt", QueryDir.c_str(),query_index );

    string query = Util::getQueryFromFile(QueryDir .c_str());


    FILE *ofp = stdout;
    ofp = fopen(resultfile.c_str(), "w");

    string msg;
    shared_ptr<Transaction> ptxn = make_shared<Transaction>(db_folder, 1, 1);
    string method_name[2] = {"RankAfterMatching","DPB"};
    for(int mode=0;mode<2;mode++) {
      long time_acc = 0;
      RankAfterMatching = mode==0;
      ResultSet rs0;
      int ret0 = _db.query(query, rs0, ofp, true, false, nullptr);
      for (int test_time = 0; test_time < 10; test_time++) {
        test_time =0;
        ResultSet _rs;
        int ret = _db.query(query, _rs, ofp, true, false, nullptr);
        time_acc += test_time;
      }
      cerr<<queryfilename<< " mode["<<method_name[mode]<<"] uses "<< time_acc/10<<" ms."<<endl;
    }

    fclose(ofp);
    ofp = NULL;

  }




  return 0;
}
