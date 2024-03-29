#!/bin/bash

# go to gstore dir
# basic test
cd ../../
echo "basic build/query/add/sub/drop test start......"
bash scripts/test/basic_test.sh
echo "basic build/query/add/sub/drop test end"

# parser test
echo "parser test start......"
bash scripts/test/parser_test.sh
echo "parser test end"
# update test gtest
echo "repeatedly insertion/deletion test num:20......"
scripts/test/update_test 20
echo "repeatedly insertion/deletion test end"

