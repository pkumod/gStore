/*
 * gquery.cpp
 *
 *  Created on: 2014-7-31
 *      Author: liyouhuan
 */
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "../Database/Database.h"
#include "../util/util.h"

using namespace std;

//WARN:cannot support soft links!
std::string 
getQueryFromFile(const char* _file_path)
{
    char buf[10000];
    std::string query_file;

    ifstream fin(_file_path);
    if(!fin)
    {
        cout << "can not open: " << _file_path << endl;
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
/*
 * [0]./gquery [1]db_folder_path
 */
int 
main(int argc, char * argv[])
{
	//cout << "gquery..." << endl;
	if(argc < 2)
	{
		cerr << "error: lack of DB_store to be queried" << endl;
		return 0;
	}
	{
		//cout << "argc: " << argc << "\t";
		//cout << "DB_store:" << argv[1] << "\t";
		//cout << endl;
	}

	string db_folder = string(argv[1]);
	Database _db(db_folder);

	_db.load();
	//cout << "finish loading" << endl;

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

        //cout << "query is:" << endl;
        //cout << query << endl << endl;

        ResultSet _rs;

        _db.query(query, _rs);


        if (argc >= 4)
        {
            util::save_to_file(argv[3], _rs.to_str());
        }

        return 0;
    }

    // read query file path from terminal.
	// BETTER: sighandler ctrl+C/D/Z
	char *buf, prompt[] = "gsql>";
	//const int commands_num = 3;
	char commands[][20] = {"help", "quit", "sparql"};
	printf("Type `help` for information of all commands\n");
	rl_bind_key('\t', rl_complete);
	while(true)
	{
		//cout << "please input query file path:" << endl;
		buf = readline(prompt);
		if(buf == NULL)
			continue;
		else
			add_history(buf);
		if(strcmp(buf, "help") == 0)
		{
			//print commands message
			printf("help - print commands message\n");
			printf("quit - quit the console normally\n");
			printf("sparql - load query from the second argument\n");
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
		//cin >> query_file;

        //string query = getQueryFromFile(query_file.c_str());
		//BETTER:build a parser for this console
		char* p = buf + strlen(buf) - 1;
		while(*p == ' ' || *p == '\t')	//set the end of path
			p--;
		*(p+1) = '\0';
		p = buf + 6;
		while(*p == ' ' || *p == '\t')	//acquire the start of path
			p++;
        string query = getQueryFromFile(p);

        if (query.empty())
        {
			free(buf);
            continue;
        }

		//cout << "query is:" << endl;
		//cout << query << endl << endl;

		ResultSet _rs;

		_db.query(query, _rs);

		//test...
		//std::string answer_file = query_file+".out";
		//util::save_to_file(answer_file.c_str(), _rs.to_str());
		free(buf);
	}

	return 0;
}




