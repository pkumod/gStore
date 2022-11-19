query_num = 47
data_dir = "scripts/parser_test/"
all_passed = True
db_name = "parser_test"

import os
import subprocess
import string

os.system("bin/gdrop -db " + db_name + " > /dev/null")

for i in range(1, query_num + 1):
    print(".....Running Test #" + str(i) + ".....")
    data_path = data_dir + "parser_d" + str(i) + ".ttl"
    query_path = data_dir + "parser_q" + str(i) + ".sql"
    result_path = data_dir + "parser_r" + str(i) + ".txt"

    # Read correct result from file
    query_vars = []
    correct_results = []
    with open(result_path, 'r') as f:
        var_line = True
        num_line = 1
        for line in f:
            if var_line:
                if "[empty result]" in line:
                    break
                query_vars = line.split()
                var_line = False
                num_line += 1
            else:
                # Cannot split, may have space inside literal
                correct_results.append(["" for i in range(len(query_vars))])
                if "paths" in line:
                    correct_results[-1][0] = line[line.find('"'):line.rfind('"') + 1]
                else:
                    curr_start = 0
                    curr_idx = 0    # For optional (inconsistent col num)
                    while curr_start < len(line):
                        while line[curr_start] in [' ', '\t'] and curr_start < len(line):
                            curr_start += 1
                        if curr_start >= len(line):
                            break
                        curr_end = curr_start + 1
                        if line[curr_start] == '\n':
                            break
                        elif line[curr_start] == '"':
                            while line[curr_end] != '"' and curr_end < len(line):
                                curr_end += 1
                            if line[curr_end] != '"':
                                print(result_path + " ERROR: literal quotation marks unclosed in Line " + str(num_line))
                                break
                            if curr_end + 1 < len(line):
                                if line[curr_end + 1] == '@':
                                    curr_end += 3   # lang suffix
                                elif line[curr_end + 1] == '^':
                                    while line[curr_end] != '>' and curr_end < len(line):
                                        curr_end += 1
                                    if line[curr_end] != '>':
                                        print(result_path + " ERROR: literal suffix unclosed in Line " + str(num_line))
                                        break
                        elif line[curr_start] == '<':
                            while line[curr_end] != '>' and curr_end < len(line):
                                curr_end += 1
                            if line[curr_end] != '>':
                                print(result_path + " ERROR: IRI unclosed in Line " + str(num_line))
                                break
                        elif line[curr_start] == '-':
                            # NULL caused by OPTIONAL
                            curr_end = curr_start
                            curr_start += 1
                        else:
                            print(result_path + " ERROR: unrecognized character in Line " + str(num_line))
                            break
                        correct_results[-1][curr_idx] = line[curr_start:curr_end + 1]
                        curr_idx += 1
                        curr_start = curr_end + 1
                num_line += 1
                if len(correct_results[-1]) != len(query_vars):
                    print(result_path + " ERROR: column num inconsistent in Line " + str(num_line))

    # Run query
    os.system("bin/gbuild -db " + db_name + " -f " + data_path + " > /dev/null")
    child = subprocess.Popen(["bin/gquery", "-db", db_name, "-q", query_path], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    
    query_out = child.stdout.read().decode()
    res_start = query_out.find("final result is")
    if res_start == -1:
        print(".....Test #" + str(i) + " ERROR.....")
        print("query_vars:", query_vars)
        print("correct_results:", correct_results)
        print("query_out:")
        print("output_vars:")
        print("output_results:")
        os.system("bin/gdrop -db " + db_name + " > /dev/null")
        continue
    query_out = query_out[query_out.find("final result is"):]
    query_out = query_out.split('\n')

    # Get output vars
    output_vars_line = query_out[2].split()
    output_vars = []
    for tok in output_vars_line:
        if tok[0] == '|':
            continue
        output_vars.append(tok.strip())
    if len(query_vars) > 0:
        output2query = [query_vars.index(v) for v in output_vars]

    # Get output rows
    output_results = []
    for j in range(4, len(query_out) - 2):
        if query_out[j][0] != '|':
            continue
        tmp = query_out[j].split('|')
        idx = 0
        output_results_unmapped = []
        for tok in tmp:
            if len(tok) == 0:
                continue
            output_results_unmapped.append(tok.strip())
        if len(output_results_unmapped) != len(output_vars):
            print("Output ERROR: column num inconsistent in Line " + str(int(j - 4) / 2))
        output_results.append(["" for j in range(len(output_vars))])
        for j in range(len(output_vars)):
            output_results[-1][output2query[j]] = output_results_unmapped[j]

    # Compare correct and output results
    correct_results.sort()
    output_results.sort()
    if correct_results == output_results:
        print(".....Test #" + str(i) + " Correct.....")
    else:
        print(".....Test #" + str(i) + " ERROR.....")
        print("query_vars:", query_vars)
        print("correct_results:", correct_results)
        print("query_out:", query_out)
        print("output_vars:", output_vars)
        print("output_results:", output_results)
    
    os.system("bin/gdrop -db " + db_name + " > /dev/null")