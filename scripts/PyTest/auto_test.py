import os
import subprocess

import ResultChecker
import FormatHelper

def TestCase(db_name: str, case_name: str, data_path: str,
             query_path: str, result_path: str, ResultCheckerFunction= ResultChecker.ResultAllSameChecker):
    os.system("bin/gdrop -db " + db_name + " > /dev/null")
    print(".....Running " + case_name + ".....")
    # Read correct result from file
    correct_results, query_vars = FormatHelper.GetResultFromFile(result_path)

    # Run query
    os.system("bin/gbuild -db " + db_name + " -f " + data_path + " > /dev/null")
    child = subprocess.Popen(["bin/gquery", "-db", db_name, "-q", query_path], stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT)

    query_out = child.stdout.read().decode()
    res_start = query_out.find("final result is")
    if res_start == -1:
        FormatHelper.PrintError(case_name, query_vars, correct_results, query_out, [], [])
        os.system("bin/gdrop -db " + db_name + " > /dev/null")
        return False
    query_out = query_out[query_out.find("final result is"):]
    query_out = query_out.split('\n')

    output_results, output_vars = FormatHelper.GetOutputResult(query_out, query_vars)
    # Compare correct and output results
    passed = ResultCheckerFunction(correct_results, output_results)

    if passed:
        print("..... " + case_name + " Correct.....")
    else:
        FormatHelper.PrintError(case_name, query_vars, correct_results, query_out, output_vars, output_results)
    os.system("bin/gdrop -db " + db_name + " > /dev/null")
    return passed


def ParseTest():
    query_num = 47
    data_dir = "scripts/parser_test/"
    all_passed = True
    db_name = "parser_test"
    for i in range(1, query_num + 1):
        cast_name = "Test #%d" % i
        data_path = data_dir + "parser_d" + str(i) + ".ttl"
        query_path = data_dir + "parser_q" + str(i) + ".sql"
        result_path = data_dir + "parser_r" + str(i) + ".txt"
        case_passed = TestCase(db_name, cast_name, data_path, query_path, result_path)
        all_passed = all_passed and case_passed
    if all_passed:
        print("All test cases have passed")
    else:
        print("Some Errors Exist!")


if __name__ == '__main__':
    ParseTest()
