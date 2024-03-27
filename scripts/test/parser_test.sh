#!/bin/bash

query_num=50
# query_num=1
data_dir="scripts/test/parser_test/"
gbuild='bin/gbuild '
gquery='bin/gquery '
gdrop='bin/gdrop '

all_passed=true

$gdrop -db parser_test > /dev/null

for i in $(seq 1 $query_num)
do
    data=$data_dir"parser_d"$i".ttl"
    query=$data_dir"parser_q"$i".sql"
    result=$data_dir"parser_r"$i".txt"

    "grep" "." $result | "sort" > "result_a.txt"

    $gbuild -db parser_test -f $data > /dev/null
    $gquery -db parser_test -q $query | grep -A 10000 "Finish Database \`parser_test\` Load" > all.txt
    grep -A 10000 "final result is :" all.txt  > tmp.txt
    "sed" "-i" "1d" "tmp.txt"
    "sed" "-i" "\$d" "tmp.txt"
    # "sed" "-i" "\$d" "tmp.txt"
    "grep" "." "tmp.txt" | "sort" > "result_b.txt"
    
    "diff" "result_a.txt" "result_b.txt" > "equal.txt"
    if [ -s "equal.txt" ]; then
        echo -e "\033[43;35m parser test #"$i" failed \033[0m"
        all_passed=false
    else
        echo "parser test #"$i" passed"
    fi

	$gdrop -db parser_test > /dev/null
    "rm" "result_a.txt" "result_b.txt" "tmp.txt" "equal.txt" "all.txt"
done

if [ $all_passed = true ]; then
    echo "All parser test cases passed!"
else
    echo -e "\033[43;35m Some parser test cases failed! \033[0m"
fi
