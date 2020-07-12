/*
 * join_test.cpp
 *
 *  Created on: 2014-7-30
 *      Author: liyouhuan
 */

#include "../Util/Util.h"
#include "../Database/Database.h"

using namespace std;
int main()
{
#ifdef DEBUG
  Util util;
#endif
  cout << "build signature" << endl;
  string _yago2_triple = "/media/wip/common/data/yago2_triple";
  string _yago = "/media/wip/common/data/yago_triple";
  string _yago_1000000 = "/media/wip/common/data/yago_1000000";
  string _yago_10000 = "/media/wip/common/data/yago_10000";
  string _yago_example = "/media/wip/common/data/yago_example";
  string _rdf = _yago_10000;
  Database _db("db_test");
  Database::log("RDF:" + _rdf);
  _db.build(_rdf);
  _db.test();
  Database::log("finish build");
  _db.test_join();

  return 0;
}
