/*
 * Gstore.cpp
 *
 *  Created on: 2014-7-1
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
  string _yago2_triple = "/media/wip/common/data/yago2_triple";
  string _yago = "/media/wip/common/data/yago_triple";
  string _yago_1000000 = "/media/wip/common/data/yago_1000000";
  string _yago_example = "/media/wip/common/data/yago_example";
  string _rdf = _yago_1000000;
  Database _db("db_test");
  _db.build(_rdf);

  return 0;
}
