import re


def GetResultFromFile(result_path: str):
    correct_results = []
    query_vars = []
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
                    curr_idx = 0  # For optional (inconsistent col num)
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
                                    curr_end += 3  # lang suffix
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
    return correct_results, query_vars


def PrintError(case_name, query_vars, correct_results, query_out, output_vars, output_results):
    print("..... " + case_name + " ERROR.....")
    print("query_vars:", query_vars)
    if len(correct_results) < 10 :
        print("correct_results:", correct_results)
    else:
        print("correct_results:", correct_results[:10])

    print("query_out:", query_out)
    print("output_vars:", output_vars)
    if len(output_results) < 10 :
        print("output_results:", output_results)
    else:
        print("output_results:", output_results[:10])



def GetOutputResult(query_out, query_vars):
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
    return output_results, output_vars


def GetSelectVar(file_path):
    with open(file_path) as f:
        sql = f.read().replace('  ', ' ')
        select_phase = re.search("((select)|(SELECT)).*((where)|(WHERE))", sql)
        if select_phase is None:
            return False, []
        select_phase = select_phase.group(0)
        return True, list(filter(lambda x: '?' in x, set(select_phase.split())))

def GetLimitNum(file_path):
    with open(file_path) as f:
        sql = f.read().replace('  ', ' ')
        limit_phase = re.search("((limit)|(LIMIT)) *\d*", sql)
        if limit_phase is None:
            return -1
        select_phase = limit_phase.group(0).split()[-1]
        return int(select_phase)


def WriteResultToFile(file_path, output_vars, output_results):
    with open(file_path, 'w') as f:
        f.write('\t'.join(output_vars) + '\n')
        for output_result in output_results:
            first_output = True
            for tok in output_result:
                if first_output:
                    first_output = False
                else:
                    f.write('\t')
                if tok == "":
                    f.write('-')
                else:
                    f.write(tok)
            f.write('\n')
