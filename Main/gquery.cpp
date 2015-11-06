/*=============================================================================
# Filename: gquery.cpp
# Author: Bookug Lobert
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-20 12:23
# Description: query a database, there are several ways to use this program:
1. ./gquery                                        print the help message
2. ./gquery --help                                 simplified as -h, equal to 1
3. ./gquery db_folder query_path                   load query from given path fro given database
4. ./gquery db_folder                              load the given database and open console
=============================================================================*/

//#include <iostream>
#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "../Database/Database.h"
#include "../Util/Util.h"

using namespace std;

//WARN:cannot support soft links!
string
getQueryFromFile(const char* _file_path)
{
#ifdef DEBUG_PRECISE
    printf("file to open: %s\n", _file_path);
#endif
    char buf[10000];
    std::string query_file;

    ifstream fin(_file_path);
    if(!fin)
    {
        printf("can not open: %s\n", _file_path);
        return "";
    }

    memset(buf, 0, sizeof(buf));
    stringstream _ss;
    while(!fin.eof())
    {
        fin.getline(buf, 9999);
        _ss << buf << "\n";
    }
    fin.close();

    return _ss.str();
}

void
help()
{
    printf("\
			/*=============================================================================\n\
# Filename: gquery.cpp\n\
# Author: Bookug Lobert\n\
# Mail: 1181955272@qq.com\n\
# Last Modified: 2015-10-20 12:23\n\
# Description: query a database, there are several ways to use this program:\n\
1. ./gquery                                        print the help message\n\
2. ./gquery --help                                 simplified as -h, equal to 1\n\
3. ./gquery db_folder query_path                   load query from given path fro given database\n\
4. ./gquery db_folder                              load the given database and open console\n\
=============================================================================*/\n");
}

int
main(int argc, char * argv[])
{
#ifdef DEBUG
    Util util;
#endif
    if(argc == 1 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
    {
        help();
        return 0;
    }
    cout << "gquery..." << endl;
    if(argc < 2)
    {
        cerr << "error: lack of DB_store to be queried" << endl;
        return 0;
    }
    {
        cout << "argc: " << argc << "\t";
        cout << "DB_store:" << argv[1] << "\t";
        cout << endl;
    }

    string db_folder = string(argv[1]);
    Database _db(db_folder);
    _db.load();
    cout << "finish loading" << endl;

    // read query from file.
    if (argc >= 3)
    {
        //        ifstream fin(argv[2]);
        //        if(!fin)
        //        {
        //            cout << "can not open: " << buf << endl;
        //            return 0;
        //        }
        //
        //        memset(buf, 0, sizeof(buf));
        //        stringstream _ss;
        //        while(!fin.eof()){
        //            fin.getline(buf, 9999);
        //            _ss << buf << "\n";
        //        }
        //        fin.close();
        //
        //        string query = _ss.str();

        string query = getQueryFromFile(argv[2]);
        if (query.empty())
        {
            return 0;
        }
        printf("query is:\n%s\n\n", query.c_str());
        ResultSet _rs;
        _db.query(query, _rs, stdout);
        if (argc >= 4)
        {
            Util::save_to_file(argv[3], _rs.to_str());
        }
        return 0;
    }

    // read query file path from terminal.
    // BETTER: sighandler ctrl+C/D/Z
    string query;
    //char resolved_path[PATH_MAX+1];
#ifdef READLINE_ON
    char *buf, prompt[] = "gsql>";
    //const int commands_num = 3;
    //char commands[][20] = {"help", "quit", "sparql"};
    printf("Type `help` for information of all commands\n");
	printf("Type `help command_t` for detail of command_t\n");
    rl_bind_key('\t', rl_complete);
    while(true)
    {
        buf = readline(prompt);
        if(buf == NULL)
            continue;
        else
            add_history(buf);
        if(strncmp(buf, "help", 4) == 0)
        {
			if(strcmp(buf, "help") == 0)
			{
            //print commands message
            printf("help - print commands message\n");
            printf("quit - quit the console normally\n");
            printf("sparql - load query from the second argument\n");
			}
			else
			{
				//TODO: sparql path > file	
			}
            continue;
        }
        else if(strcmp(buf, "quit") == 0)
            break;
        else if(strncmp(buf, "sparql", 6) != 0)
        {
            printf("unknown commands\n");
            continue;
        }
        std::string query_file;
        //BETTER:build a parser for this console
		bool ifredirect = false;
		char* rp = buf;
		while(*rp != '\0')
		{
			if(*rp == '>')
			{
				ifredirect = true;
				break;
			}
			rp++;
		}
        char* p = buf + strlen(buf) - 1;
		FILE* fp = stdout;      ///default to output on screen
		if(ifredirect)
		{
			char* tp = p;
			while(*tp == ' ' || *tp == '\t')
				tp--;
			*(tp+1) = '\0';
			tp = rp + 2;
			while(*tp == ' ' || *tp == '\t')
				tp++;
			fp = fopen(tp, "w");	//NOTICE:not judge here!
			p = rp - 2;					//NOTICE: all separated with ' ' or '\t'
		}
        while(*p == ' ' || *p == '\t')	//set the end of path
            p--;
        *(p+1) = '\0';
        p = buf + 6;
        while(*p == ' ' || *p == '\t')	//acquire the start of path
            p++;
        //TODO: support the soft links(or hard links)
        //there are also readlink and getcwd functions for help
        //http://linux.die.net/man/2/readlink
        //NOTICE:getcwd and realpath cannot acquire the real path of file
        //in the same directory and the program is executing when the
        //system starts running
        //NOTICE: use realpath(p, NULL) is ok, but need to free the memory
        char* q = realpath(p, NULL);	//QUERY:still not work for soft links
#ifdef DEBUG_PRECISE
        printf("%s\n", p);
#endif
        if(q == NULL)
        {
			printf("invalid path!\n");
			free(q);
			free(buf);
			continue;
        }
        else
			printf("%s\n", q);
        //query = getQueryFromFile(p);
        query = getQueryFromFile(q);
        if(query.empty())
        {
			free(q);
            //free(resolved_path);
            free(buf);
			if(ifredirect)
				fclose(fp);
            continue;
        }
        cout << "query is:" << endl;
        cout << query << endl << endl;
        ResultSet _rs;
        _db.query(query, _rs, fp);
        //test...
        //std::string answer_file = query_file+".out";
        //Util::save_to_file(answer_file.c_str(), _rs.to_str());
		free(q);
        //free(resolved_path);
        free(buf);
		if(ifredirect)
			fclose(fp);
#ifdef DEBUG_PRECISE
        //printf("after buf freed!\n");
#endif
    }
#else					//DEBUG:this not work!
    while(true)
    {
        cout << "please input query file path:" << endl;
        std::string query_file;
        cin >> query_file;
        //char* q = realpath(query_file.c_str(), NULL);
        string query = getQueryFromFile(query_file.c_str());
        if(query.empty())
        {
            //free(resolved_path);
            continue;
        }
        cout << "query is:" << endl;
        cout << query << endl << endl;
        ResultSet _rs;
        _db.query(query, _rs, stdout);
        //free(resolved_path);
    }
#endif

    return 0;
}

