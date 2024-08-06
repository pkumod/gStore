#!/bin/bash

# go to gstore dir
if [ $1 ] && [ $1 != '.' ];
then
    cd ../../
fi

log_mode=$(grep -m 1 'log_mode=' ./conf/conf.ini)
if [ ${log_mode:9:${#log_mode}-9} = '"./conf/slog.properties"' ]
then
	first_line=$(head -n 1 ./conf/slog.properties)
	log_level=$(echo $first_line | sed -n 's/^log4cplus.rootLogger=\([^,]*\).*/\1/p')
	if [ "$log_level" != "TRACE" ]; then
		echo "please set "log4cplus.rootLogger=TRACE" in the slog.properties ."
		exit
	fi
fi

# basic test
echo "basic build/query/add/sub/drop test start......"
bash scripts/test/basic_test.sh
echo "basic build/query/add/sub/drop test end"

# parser test
echo "parser test start......"
bash scripts/test/parser_test.sh
echo "parser test end"
# update test gtest
echo "repeatedly insertion/deletion test num:20......"
scripts/test/update_test 20 > /dev/null
echo "repeatedly insertion/deletion test end"

