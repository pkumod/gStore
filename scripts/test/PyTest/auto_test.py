import os
import subprocess
import argparse
import ResultChecker
import FormatHelper


def TestCase(db_name: str, case_name: str, query_path: str,
             result_path: str, ResultCheckerFunction=ResultChecker.ResultAllSameChecker,
             function_arg={}):
    print(".....Running " + case_name + ".....")
    # Read correct result from file
    correct_results, query_vars = FormatHelper.GetResultFromFile(result_path)

    # Run query
    child = subprocess.Popen(["bin/gquery", "-db", db_name, "-q", query_path], stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT)

    query_out = child.stdout.read().decode()
    res_start = query_out.find("final result is")
    if res_start == -1:
        FormatHelper.PrintError(case_name, query_vars, correct_results, query_out, [], [])
        return False
    query_out = query_out[query_out.rfind("final result is"):]
    query_out = query_out.split('\n')

    output_results, output_vars = FormatHelper.GetOutputResult(query_out, query_vars)
    # Compare correct and output results
    passed = ResultCheckerFunction(correct_results=correct_results,
                                   output_results=output_results,
                                   **function_arg)

    if passed:
        print("..... " + case_name + " Correct.....")
    else:
        FormatHelper.PrintError(case_name, query_vars, correct_results, query_out, output_vars, output_results)
    return passed


def ParseTest():
    query_num = 50
    data_dir = "scripts/parser_test/"
    all_passed = True
    db_name = "parser_test"
    os.system("bin/gdrop -db " + db_name + " > /dev/null")
    for i in range(1, query_num + 1):
        cast_name = "Test #%d" % i
        data_path = data_dir + "parser_d" + str(i) + ".ttl"
        query_path = data_dir + "parser_q" + str(i) + ".sql"
        result_path = data_dir + "parser_r" + str(i) + ".txt"

        os.system("bin/gbuild -db " + db_name + " -f " + data_path + " > /dev/null")
        case_passed = TestCase(db_name, cast_name, query_path, result_path)
        all_passed = all_passed and case_passed
        os.system("bin/gdrop -db " + db_name + " > /dev/null")
    if all_passed:
        print("All test cases have passed")
    else:
        print("Some Errors Exist!")


def BFSTest():
    data_set = ['bbug', 'lubm', 'num']
    data_dir = "scripts/bfs_test/"
    all_passed = True
    db_name_gstore = "bfs_test"
    os.system("bin/gdrop -db " + db_name_gstore + " > /dev/null")
    for db_name in data_set:
        db_path = os.path.join(data_dir, db_name)
        files = os.listdir(db_path)
        nt_file = 'data/%s/%s.nt' % (db_name, db_name)
        query_files = list(filter(lambda x: '.sql' in x, files))
        print(nt_file)
        os.system("bin/gbuild -db " + db_name_gstore + " -f " + nt_file
                  + " > /dev/null")
        for query_file in query_files:
            query_file = os.path.join(db_path, query_file)
            result_path = query_file.replace('.sql', 'r.txt')
            cast_name = db_name + ' ' + query_file[:-4]
            case_passed = TestCase(db_name_gstore, cast_name, query_file, result_path)
            all_passed = all_passed and case_passed
        os.system("bin/gdrop -db " + db_name_gstore + " > /dev/null")
    if all_passed:
        print("All test case have passed")
    else:
        print("Some Error Exist!")


def DFSTest():
    data_set = ['bbug', 'lubm', 'num']
    data_dir = "scripts/dfs_test/"
    full_data_dir = "scripts/bfs_test/"
    all_passed = True
    db_name_gstore = "dfs_test"
    os.system("bin/gdrop -db " + db_name_gstore + " > /dev/null")
    for db_name in data_set:
        db_path = os.path.join(data_dir, db_name)
        files = os.listdir(db_path)
        nt_file = 'data/%s/%s.nt' % (db_name, db_name)
        query_files = list(filter(lambda x: '.sql' in x, files))
        print(nt_file)
        os.system("bin/gbuild -db " + db_name_gstore + " -f " + nt_file
                  + " > /dev/null")
        for query_file_name in query_files:
            query_file = os.path.join(db_path, query_file_name)
            result_path = query_file.replace('.sql', 'r.txt').replace(data_dir,full_data_dir)
            cast_name = db_name + ' ' + query_file[:-4]
            k = FormatHelper.GetLimitNum(query_file)
            case_passed = TestCase(db_name_gstore, cast_name, query_file, result_path,
                                   ResultCheckerFunction=ResultChecker.ResultLimitKChecker,
                                   function_arg={'k': k})
            all_passed = all_passed and case_passed
        os.system("bin/gdrop -db " + db_name_gstore + " > /dev/null")
    if all_passed:
        print("All test case have passed")
    else:
        print("Some Error Exist!")


TestDict = {'ParseTest': ParseTest, 'BFSTest': BFSTest, 'DFSTest':DFSTest}
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Auto Test: auto_test.py --tests='
                                                 '[ParseTest|BFSTest|DFSTest]')
    parser.add_argument('--tests', type=str, default='all')
    args = parser.parse_args()

    if args.tests in TestDict:
        TestDict[args.tests]()
    else:
        for test_name, func in TestDict.items():
            print("Now %s" % test_name)
            func()
