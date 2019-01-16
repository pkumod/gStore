#!/bin/bash

query_num=30
data_dir="scripts/parser_test/"
gbuild='bin/gbuild '
gquery='bin/gquery '
gdrop='bin/gdrop '

all_passed=true

for i in $(seq 1 $query_num)
do
    data=$data_dir"parser_d"$i".ttl"
    query=$data_dir"parser_q"$i".sql"
    result=$data_dir"parser_r"$i".txt"

    "grep" "." $result | "sort" > "result_a.txt"

    $gbuild "parser_test" $data > "tmp.txt"
    $gquery "parser_test" $query | "grep" "-A" "10000" "final result is :" > "tmp.txt"
    "sed" "-i" "1d" "tmp.txt"
    "grep" "." "tmp.txt" | "sort" > "result_b.txt"
    
    "diff" "result_a.txt" "result_b.txt" > "equal.txt"
    if [ -s "equal.txt" ]; then
        echo "parser test #"$i" failed"
        all_passed=false
    #else
        #echo "parser test #"$i" passed"
    fi

	$gdrop "parser_test" > "tmp.txt"
    "rm" "result_a.txt" "result_b.txt" "tmp.txt" "equal.txt" 
done

if [ $all_passed = true ]; then
    echo "All parser test cases passed!"
else
    echo "Some parser test cases failed!"
fi
