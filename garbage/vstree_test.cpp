/*=============================================================================
# Filename: vstree_test.cpp
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-24 19:57
# Description: 
=============================================================================*/

#include "../Util/Util.h"
#include "../Database/Database.h"

using namespace std;

int main()
{
#ifdef DEBUG
  Util util;
#endif
  cout << "vstree_test" << endl;
  string _yago_example = "/media/wip/common/data/yago_example";
  string _rdf = _yago_example;
  Database _db("db_test");
  _db.build(_rdf);
  Database::log("finish build");

  return 0;
}
