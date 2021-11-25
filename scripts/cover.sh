#! /usr/bin/bash

echo "test start"

#TODO:place this together with fulltest, and keep files in fulltest
#add this to makefile to do every time

lcov -z -d ./
bin/gload watdiv.db /home/data/WatDiv/database/watdiv10M.nt
gcov -a -b -c gload.cpp
lcov --no-external --directory . --capture --output-file load.info
#genhtml --output-directory COVERAGE/LOAD --frames --show-details load.info

echo "build tested"

lcov -z -d ./
bin/gquery watdiv.db /home/data/WatDiv/query/C1.sql
gcov -a -b -c gquery.cpp
lcov --no-external --directory . --capture --output-file C1.info

lcov -z -d ./
bin/gquery watdiv.db /home/data/WatDiv/query/C2.sql
gcov -a -b -c gquery.cpp
lcov --no-external --directory . --capture --output-file C2.info

lcov -z -d ./
bin/gquery watdiv.db /home/data/WatDiv/query/C3.sql
gcov -a -b -c gquery.cpp
lcov --no-external --directory . --capture --output-file C3.info

lcov -z -d ./
bin/gquery watdiv.db /home/data/WatDiv/query/F1.sql
gcov -a -b -c gquery.cpp
lcov --no-external --directory . --capture --output-file F1.info

lcov -z -d ./
bin/gquery watdiv.db /home/data/WatDiv/query/F2.sql
gcov -a -b -c gquery.cpp
lcov --no-external --directory . --capture --output-file F2.info

lcov -z -d ./
bin/gquery watdiv.db /home/data/WatDiv/query/F3.sql
gcov -a -b -c gquery.cpp
lcov --no-external --directory . --capture --output-file F3.info

lcov -z -d ./
bin/gquery watdiv.db /home/data/WatDiv/query/F4.sql
gcov -a -b -c gquery.cpp
lcov --no-external --directory . --capture --output-file F4.info

lcov -z -d ./
bin/gquery watdiv.db /home/data/WatDiv/query/F5.sql
gcov -a -b -c gquery.cpp
lcov --no-external --directory . --capture --output-file F5.info

#also need to test: api, gconsole/gclient/gserver
#Parser/Sparql* should not be tested
lcov -a load.info -a C1.info -a C2.info -a C3.info -a F1.info -a F2.info -a F3.info -a F4.info -a F5.info -o gstore.info
lcov --remove gstore.info 'Server/*' 'Main/*' 'Parser/*'
genhtml --output-directory COVERAGE --frames --show-details gstore.info

