/*=============================================================================
# Filename: gtest.cpp
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-09-02 00:04
# Description: load index once and query, there are several ways to use this program:
1.  ./gtest								test all datasets and corresponding queries
2.	./gtest --help						simplified as -h, will print the help message
3.	./gtest -f DS_PATH					load/test a specified dataset, with all corresponding queries
4.  ./gtest -d FD_PATH					load/test a dataset folder(like WatDiv/), with all corresponding queries
5.  ./gtest -q DB_PATH q1 q2...			test a loaded database with given queries(no limit to db and query)
=============================================================================*/

//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
//#include <dirent.h>
//#include <unistd.h>
#include "../Database/Database.h"
#include "../Util/Util.h"

using namespace std;

#define NUM 4

char line1[] = "--------------------------------------------------";
char line2[] = "##################################################";
string path = "/media/wip/common/data/";
string db[NUM] = {"WatDiv/", "LUBM/", "DBpedia/", "Yago/"};

//not achieved, so do not use
void
build_logs()
{
	system("rm -rf result.log/");
	system("rm -rf time.log/");
	system("mkdir result.log");
	system("mkdir time.log");
	int i;
	string cmd;
	for(i = 0; i < NUM; ++i)
	{
		cmd = string("mkdir result.log/\"") + db[i] + "\"";		//in case of special characters like &
		system(cmd.c_str());
		cmd = string("mkdir time.log/\"") + db[i] + "\"";
		system(cmd.c_str());
	}
}

bool		//if satisfy suffix; case sensitive
judge(const char* p, const char* p0)	//both not empty
{
	int len = strlen(p), len0 = strlen(p0);
	while(len0 > 0 && len > 0)
	{
		len--;
		len0--;
		if(p[len] != p0[len0])
			return false;
	}
	return true;
}

string
getQueryFromFile(const char* path)
{
	char buf[10000];
	ifstream fin(path);
	if(!fin)
	{
		printf("can not open %s\n", path);
		return "";
	}
	memset(buf, 0, sizeof(buf));
	stringstream ss;
	while(!fin.eof())
	{
		fin.getline(buf, 9999);
		ss << buf << "\n";
	}
	fin.close();
	return ss.str();
}

void
help()
{
	printf("\
			/*=============================================================================\n\
# Filename: gtest.cpp\n\
# Author: syzz\n\
# Mail: 1181955272@qq.com\n\
# Description: load index once and query, there are several ways to use this program:\n\
1.  ./gtest                             test all datasets and corresponding queries\n\
2.  ./gtest --help                      simplified as -h, will print the help message\n\
3.  ./gtest -f DS_PATH                  load/test a specified dataset, with all corresponding queries\n\
4.  ./gtest -d FD_PATH                  load/test a dataset folder(like WatDiv/), with all corresponding queries\n\
5.  ./gtest -q DB_PATH q1 q2...         test a loaded database with given queries(no limit to db and query)\n\
=============================================================================*/\n\
			");
}

FILE *fp0 = NULL, *fp1 = NULL, *fp2 = NULL;

void								//single dataset, single query
runSS(Database* _db, string qf)		//given a query file
{
	string query = getQueryFromFile(qf.c_str());
	if(query.empty())
	{
		fprintf(fp0, "this query is empty:\n%s\n", qf.c_str());
		fflush(fp0);
		return;
		//continue;
	}
	fprintf(fp0, "%s\n", qf.c_str());
	fflush(fp0);
	//getchar();getchar();
	ResultSet _rs;
	long tv_begin, tv_final;
	tv_begin = Util::get_cur_time();
	//fprintf(fp0, "begin to query!\n");
	//fflush(fp0);
	_db->query(query, _rs, stdout);
	//fprintf(fp0, "finish this query!\n");
	tv_final = Util::get_cur_time();
	//print to logs
	fprintf(fp1, "%s\n%ld\n%s\n", qf.c_str(), tv_final - tv_begin, line1);//time is standarded as ms
	fprintf(fp2, "%s\n%s%s\n", qf.c_str(), _rs.to_str().c_str(), line1);
	fflush(fp0);
	fflush(fp1);
	fflush(fp2);
}

void								//single dataset, multi queries
runSM(Database* _db, string qd)		//given a query directory
{
	DIR* dp2;
	struct dirent* p2;
	dp2 = opendir(qd.c_str());
	//finish all queries
	while((p2 = readdir(dp2)) != NULL)
	{
		if(!judge(p2->d_name, ".sql"))
			continue;
		string file = qd + string(p2->d_name);
		runSS(_db, file);
	}
	fprintf(fp0, "finish all queries!\n");
	fflush(fp0);
	closedir(dp2);
	fprintf(fp0, "ok to close dp2!\n");
}

void					//multi datasets, multi queries
runMM(string df)		//given dataset folder, end with /
{
	DIR *dp1;
	struct dirent *p1;
	string s1, s2, cmd;
	Database* _db;
	s1 = df + string("database/");
	s2 = df + string("query/");
	dp1 = opendir(s1.c_str());		
	while((p1 = readdir(dp1)) != NULL)
	{
		if(!judge(p1->d_name, ".nt"))
			continue;
		system("echo 3 > /proc/sys/vm/drop_caches");
		//build the database index first
		string db_folder = s1 + string(p1->d_name);
		fprintf(fp0, "%s\n", db_folder.c_str());
		cmd = string("./gload \"") + string(p1->d_name) + "\" \"" + db_folder + "\" > \"gload_" + string(p1->d_name) + ".log\"";	//NOTICE:\" is used to ensure file name with &(etc...) will work ok!
		fprintf(fp0, "%s\n", cmd.c_str());
		system(cmd.c_str());
		fprintf(fp0, "ok to load database!\n");
		//getchar();getchar();
		//then load the index
		_db = new Database(string(p1->d_name));
		_db->load();
		fprintf(fp0, "ok to load index!\n");
		//print to logs
		fprintf(fp1, "%s\n%s\n", p1->d_name, line1);
		fprintf(fp2, "%s\n%s\n", p1->d_name, line1);

		runSM(_db, s2);

		fflush(fp0);
		fprintf(fp1, "%s\n", line2);
		fprintf(fp2, "%s\n", line2);
		//NOTICE:may double free due to ~Database
		//to remove one when finished, should clean firstly like ~Database()
		delete _db;//_db->release(fp0);
		cmd = string("rm -rf \"") + string(p1->d_name) + string("\"");
		system(cmd.c_str());
		fprintf(fp0, "ok to remove %s\n", p1->d_name);
		fflush(fp0);
		fflush(fp1);
		fflush(fp2);
	}
	closedir(dp1);
}

void
clean()
{
	//it is ok to check like this here!
	//in other situations, notice that fp is not null after closed!
	if(fp0 != NULL)		
	{
		fclose(fp0);
		fp0 = NULL;
	}
	if(fp1 != NULL)
	{
		fclose(fp1);
		fp1 = NULL;
	}
	if(fp2 != NULL)
	{
		fclose(fp2);
		fp2 = NULL;
	}
}

void 
EXIT(int ret)
{
	clean();
	exit(ret);
}

int			
main(int argc, const char* argv[])
{
#ifdef DEBUG
	Util util;
#endif
	//BETTER: enable user to assign the files for log
	int i, j;
	fp0 = fopen("run.log", "w+");
	if(fp0 == NULL)
	{
		printf("open error!\n");
		EXIT(1);
	}
	//build_logs();
	fp1 = fopen("time.log", "w+");
	if(fp1 == NULL)
	{
		printf("open error!\n");
		EXIT(1);
	}
	fp2 = fopen("result.log", "w+");
	if(fp2 == NULL)
	{
		printf("open error!\n");
		EXIT(1);
	}

	switch(argc)
	{
		case 1:
			help();
			printf("this command will do a complete test for Gstore, do you want to continue?YN\n");
			char c;
			i = 0;
			while(scanf("%c", &c))
			{
				if(c == 'Y' || c == 'y')
				{
					for(j = 0; j < 4; ++j)
					{
						runMM(path+db[j]);
					}
					break;
				}
				else if(c == 'N' || c == 'n')
					EXIT(0);
				i++;
				if(i > 10)				//try no more than 10 times
					EXIT(1);
			}
			break;
		case 2:
			if(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
			{
				help();
				EXIT(0);
			}
			else
				EXIT(1);
		case 3:
			if(strcmp(argv[1], "-f") == 0)		//this file must be in db[]/database/, end with ".nt"
			{
				//TODO:use judge to check if is .nt file
				int k = strlen(argv[2])	- 1;
				string db = string(argv[2]);
				while(k >= 0)
				{
					if(db[k] == '/')
					{
						break;		
					}
				}
				string s = db.substr(0, k+1) + "../query/";
				//DIR* dp = opendir(s.c_str());
				//TODO: gload the dataset and build a new function!
				printf("not achieved!\n");
			}
			else if(strcmp(argv[1], "-d") == 0)
			{
				runMM(string(argv[2]) + "/");
			}
			else
				EXIT(1);
			break;
		default:		//> 3
			if(strcmp(argv[1], "-q") == 0)
			{
				//TODO: user must gload first?
				printf("not achieved!\n");
			}
			else 
				EXIT(1);
			break;
	}

	clean();
	return 0;
}

