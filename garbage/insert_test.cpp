/*
 * insert_test.cpp
 *
 *  Created on: 2014-12-03
 *      Author: Caesar11
 */

#include "../Util/Util.h"
#include "../Database/Database.h"

using namespace std;

int main(int argc, char* argv[])
{
#ifdef DEBUG
  Util util;
#endif
  string db_folder = string(argv[1]);
  string insert_rdf_file = string(argv[2]);
  Database _db(db_folder);

  _db.insert(insert_rdf_file);

  return 0;
}
