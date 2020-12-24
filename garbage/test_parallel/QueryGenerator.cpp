#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <getopt.h>
#include <time.h>

using namespace std;

#define READ 0
#define WRITE 1

string* subject = NULL, *predicate = NULL, *object = NULL, *triple = NULL;
int subcnt, predcnt, objcnt, tripcnt;

int
ReadStrings(string** _str, int& _cnt, char* file)
{
  ifstream _fin(file);
  if (!_fin)
    return -1;

  int cnt;
  _fin >> cnt;
  *_str = new string[cnt];

  for (int i = 0; i < cnt; i++)
    getline(_fin, (*_str)[i]);

  _fin.close();
  _cnt = cnt;
}

int
ReadOrWrite(int Pread, int Pwrite)
{
  int x = rand() % (Pread + Pwrite);
  if (x < Pread)
    return READ;
  else
    return WRITE;
}

string
GenerateReadOp()
{
  string ret = "q ";
  int bound1 = subcnt, bound2 = subcnt + predcnt, bound3 = subcnt + predcnt + objcnt;
  int x = rand() % bound3;
  if (x < bound1) // Generating queris about subject
  {
    ret += subject[rand() % subcnt] + " ?x ?y";
    return ret;
  } else if (x < bound2) {
    ret += "?x " + predicate[rand() % predcnt] + " ?y";
    return ret;
  } else {
    ret += "?x ?y " + object[rand() % objcnt];
    return ret;
  }
}

string
GenerateWriteOp()
{
  string ret = "";
  if (rand() % 2 == 0) // delete a triple
  {
    ret = "d " + triple[rand() % tripcnt];
    return ret;
  } else // insert a triple
  {
    ret = "i " + subject[rand() % subcnt];
    string ret2 = " " + predicate[rand() % predcnt];
    string ret3 = " " + object[rand() % objcnt];
    return ret + ret2 + ret3;
  }
}

int
Generate(const char* OutputFile, int Pread, int Pwrite, int num)
{
  ofstream _fout(OutputFile);
  if (!_fout) {
    printf("Open %s error, generating queries fail.\n", OutputFile);
    return -1;
  }
  _fout << num << endl;

  for (int i = 0; i < num; i++) {
    if (ReadOrWrite(Pread, Pwrite) == READ) {
      string tmp = GenerateReadOp();
      _fout << tmp << endl;
    } else {
      string tmp = GenerateWriteOp();
      _fout << tmp << endl;
    }
  }

  _fout.close();
  return 0;
}

int
main(int argc, char** argv)
{
  srand(time(0));
  // Begin load subjects, predicates, objects and triples
  char SubFile[] = "./test_parallel/subject.txt";
  char PredFile[] = "./test_parallel/predicate.txt";
  char ObjFile[] = "./test_parallel/object.txt";
  char TripFile[] = "./test_parallel/triple.txt";
  char OutputFile[] = "./queries.txt";

  int ret = ReadStrings(&subject, subcnt, SubFile);
  if (ret < 0) {
    cout << "Read subjects failed." << endl;
    return -1;
  }

  ret = ReadStrings(&predicate, predcnt, PredFile);
  if (ret < 0) {
    cout << "Read predicates failed." << endl;
    return -1;
  }

  ret = ReadStrings(&object, objcnt, ObjFile);
  if (ret < 0) {
    cout << "Read objects failed." << endl;
    return -1;
  }

  ret = ReadStrings(&triple, tripcnt, TripFile);
  if (ret < 0) {
    cout << "Read triples failed." << endl;
    return -1;
  }
  // Finish load subjects, predicates, objects and triples.
  // Start parsing options
  int opt, Pread = 10, Pwrite = 0, num = 10000;

  while ((opt = getopt(argc, argv, "r:w:n:")) != -1) {
    switch (opt) {
    case 'r':
      Pread = atoi(optarg);
      break;
    case 'w':
      Pwrite = atoi(optarg);
      break;
    case 'n':
      num = atoi(optarg);
      break;
    default:
      printf("Note: MyBenchmark only support query!!!");
      printf("You input option %c\n", opt);
      printf("Valid options include:\n");
      printf("-r percentage of reading operations(int)\n");
      printf("-w percentage of writing operations(int)\n");
      printf("-n total number of operations\n");
      printf("-h print this manual\n");
      return 0;
    }
  }

  // Finish parsing options
  // Start generating operations

  ret = Generate(OutputFile, Pread, Pwrite, num);
  if (ret != 0) {
    return -1;
  }
  printf("Generating queries success.\n");

  // End generating operations
  return 0;
}
