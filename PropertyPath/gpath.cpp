/*=============================================================================
# Filename:		gpath.cpp
# Author: Bookug Lobert 
# Mail: zengli-bookug@pku.edu.cn
# Last Modified:	2017-05-22 23:27
# Description: 
=============================================================================*/

//BETTER: run queries in batch, so as to reduce duplicate computing
//

#include "../Util/Util.h"
#include "../Database/Database.h"
#include "../KVstore/KVstore.h"

using namespace std;

//NOTICE: we don't consider recombination path* here, like (P1|P2)* or (P1*/P2)*
//^path, path(n) and path(n,m) are currently not considered
//WARN: the direction is from left to right, no edge with different directions!(so all are entity except that the last one cna be literal)
//(P1/P2)* and P1*/P2*are considered here
//NOTICE: we assume that ';' can be used to divide src and dest and pres
typedef struct PreUnitType
{
	//not dividable
	vector<string> pset;
	// |, *, ? (^path, path(n), path(n,m))
	char opt;  
}PreUnit;
typedef list< vector<unsigned> > ResultType;
typedef ResultType::iterator ResultIterator;

KVstore* kvstore;

void convert(string _query, unsigned& src, unsigned& dest, vector<PreUnit>& _pres)
{
	vector<unsigned> divid;
	int i = 0, j = _query.size()-1, k = 0;
	while(_query[k] != ';')
	{
		k++;
	}
	string str1 = _query.substr(0, k);
	src = kvstore->getIDByEntity(str1);
	//get the dest, which may be a literal
	k = j;
	while(_query[k] != ';')
	{
		k--;
	}
	string str2 = _query.substr(k+1, j-k);
	if(Util::is_entity_ele(str2))
	{
		dest = kvstore->getIDByEntity(str2);
	}
	else
	{
		dest = kvstore->getIDByLiteral(str2);
	}

	//TODO:divide all pres
}

void project(ResultType& _rs, unsigned _idx, vector<unsigned>& _cand)
{
	//TODO: project a column and remove duplicates
}

//NOTICE: if src or dest is variable, use -1 instead 
void query(Database& _db, ResultType& _rs, unsigned _src, unsigned _dest)
{
	//TODO: use gstore to answer queries, set NULL, convert to unsigned
}

void compute(Database& _db, ResultType& _rs, unsigned _src, unsigned _dest, vector<PreUnit>& _pres)
{
	//TODO: border case, one query
	//divide and conquer: find the longest P sequence
	//search in DFS manner
	//TODO: deal with (p1p2)*
}

void output(ResultType& _rs, FILE* _fp = NULL)
{
		//ResultSet _rs;
		//int ret = _db.query(query, _rs, fp);
		//TODO: transform ID to string and output paths
}

int
main(int argc, char* argv[])
{
	//chdir(dirname(argv[0]));
//#ifdef DEBUG
	Util util;
//#endif
	if(argc < 3)  //./gbuild
	{
		//output help info here
		cout << "the usage of gpath: " << endl;
		cout << "bin/gpath -s dataset_name" << endl;
		cout << "or use an existing database:"<<endl;
		cout << "bin/gpath -d database_name" << endl;
		cout <<"after the databse is built/loaded, you can input queries in the console"<<endl;
		return 0;
	}
	//system("clock");

	string opt = string(argv[1]);
	string name = string(argv[2]);
	Database db(name);
	if(opt == "-s")
	{
		bool flag = db.build(name);
		if (flag)
		{
			cout << "import RDF file to database done." << endl;
		}
		else
		{
			cout << "import RDF file to database failed." << endl;
		}
	}
	else if(opt == "-d")
	{
		db.load();
		cout << "finish loading" << endl;
	}
	else
	{
		cout<<"ERROR: invalid option!"<<endl;
		return -1;
	}

	//assign kvstore
	kvstore = db.getKVstore();

	//query console - input or a file
	string query;
	//char resolved_path[PATH_MAX+1];
#ifdef READLINE_ON
	char *buf, prompt[] = "gsql>";
	//const int commands_num = 3;
	//char commands[][20] = {"help", "quit", "sparql"};
	printf("Type `help` for information of all commands\n");
	printf("Type `help command_t` for detail of command_t\n");
	rl_bind_key('\t', rl_complete);
	while (true)
	{
		buf = readline(prompt);
		if (buf == NULL)
			continue;
		else
			add_history(buf);
		if (strncmp(buf, "help", 4) == 0)
		{
			if (strcmp(buf, "help") == 0)
			{
				//print commands message
				printf("help - print commands message\n");
				printf("quit - quit the console normally\n");
				printf("sparql - load query from the second argument\n");
			}
			else
			{
				//TODO: help for a given command
			}
			continue;
		}
		else if (strcmp(buf, "quit") == 0)
			break;
		else if (strncmp(buf, "sparql", 6) != 0)
		{
			printf("unknown commands\n");
			continue;
		}
		//TODO: sparql + string, not only path
		string query_file;
		//BETTER:build a parser for this console
		bool ifredirect = false;

		char* rp = buf;
		int pos = strlen(buf) - 1;
		while (pos > -1)
		{
			if (*(rp + pos) == '>')
			{
				ifredirect = true;
				break;
			}
			pos--;
		}
		rp += pos;

		char* p = buf + strlen(buf) - 1;
		FILE* fp = stdout;      ///default to output on screen
		if (ifredirect)
		{
			char* tp = p;
			while (*tp == ' ' || *tp == '\t')
				tp--;
			*(tp + 1) = '\0';
			tp = rp + 1;
			while (*tp == ' ' || *tp == '\t')
				tp++;
			fp = fopen(tp, "w");	//NOTICE:not judge here!
			p = rp - 1;					//NOTICE: all separated with ' ' or '\t'
		}
		while (*p == ' ' || *p == '\t')	//set the end of path
			p--;
		*(p + 1) = '\0';
		p = buf + 6;
		while (*p == ' ' || *p == '\t')	//acquire the start of path
			p++;
		//there are also readlink and getcwd functions for help
		//http://linux.die.net/man/2/readlink
		//NOTICE:getcwd and realpath cannot acquire the real path of file
		//in the same directory and the program is executing when the
		//system starts running
		//NOTICE: use realpath(p, NULL) is ok, but need to free the memory
		char* q = realpath(p, NULL);	//QUERY:still not work for soft links
		if (q == NULL)
		{
			printf("invalid path!\n");
			free(q);
			free(buf);
			continue;
		}
		else
			printf("%s\n", q);
		//query = getQueryFromFile(p);
		query = Util::getQueryFromFile(q);
		if (query.empty())
		{
			free(q);
			//free(resolved_path);
			free(buf);
			if (ifredirect)
				fclose(fp);
			continue;
		}
		printf("query is:\n");
		printf("%s\n\n", query.c_str());

		//property path handling: the query format is s regex_path t
		//NOTICE: we only find one path if reachable, or empty if not reachable
		unsigned src = INVALID, dest = INVALID;
		vector<PreUnit> pres;
		convert(query, src, dest, pres);
		//BETTER: when query() returns, it should be IDs instead of Strings?
		ResultType rs;
		compute(db, rs, src, dest, pres);
		output(rs, fp);

		//string msg;
		free(q);
		//free(resolved_path);
		free(buf);
		if (ifredirect)
			fclose(fp);
	}
#endif

	//system("clock");
	return 0;
}

