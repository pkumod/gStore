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

#include "../Database/Database.h"
#include "../Util/Util.h"
//#include "../Util/Slog.h"

using namespace std;

// WARN:cannot support soft links!

void help()
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

int main(int argc, char *argv[])
{
	// chdir(dirname(argv[0]));
	//#ifdef DEBUG
	Util util;
	//#endif
	// Log.init("slog.properties");
	if (argc < 2)
	{
		/*cout << "please input the complete command:\t" << endl;
		cout << "\t bin/gadd -h" << endl;*/
		cout << "Invalid arguments! Input \"bin/gquery -h\" for help." << endl;
		return 0;
	}
	else if (argc == 2)
	{
		string command = argv[1];
		if (command == "-h" || command == "--help")
		{
			cout << endl;
			cout << "gStore Query Data Tools(gquery)" << endl;
			cout << endl;
			cout << "Usage:\tbin/gquery -db [dbname] -q [backup_path] " << endl;
			cout << endl;
			cout << "Options:" << endl;
			cout << "\t-h,--help\t\tDisplay this message." << endl;
			cout << "\t-db,--database,\t\t the database name. Notice that the name can not end with .db " << endl;
			cout << "\t-q,--queryfile ,\t\tthe query file path" << endl;
			cout << "\t-f,--resultfile [option] ,\t\tthe result file path" << endl;
			cout << endl;
			return 0;
		}
		else
		{
			// cout << "the command is not complete." << endl;
			cout << "Invalid arguments! Input \"bin/gquery -h\" for help." << endl;
			return 0;
		}
	}
	else
	{
		string db_folder = Util::getArgValue(argc, argv, "db", "database");
		if (db_folder.empty())
		{
			cout << "You need to input the database name that you want to query. Input \"bin/gquery -h\" for help." << endl;
			return 0;
		}
		int len = db_folder.length();
		if (db_folder.length() > 3 && db_folder.substr(len - 3, 3) == ".db")
		{
			cout << "The database name can not end with .db" << endl;
			return 0;
		}
		if (Util::dir_exist(db_folder + ".db") == false)
		{
			cout << "the db path is " << db_folder << endl;
			cout << "The database is not exist!" << endl;
			return 0;
		}
		Database _db(db_folder);
		// test
		if (argc > 3)
		{
			string queryfile = Util::getArgValue(argc, argv, "q", "queryfile");
			if (queryfile.empty())
			{
				cout << "The query file path can not be empty!" << endl;
				return 0;
			}
			else if (Util::file_exist(queryfile) == false)
			{
				cout << "The query file is not exist!" << endl;
				return 0;
			}
			else
			{
				long tv_begin = Util::get_cur_time();

				_db.load();
				cout << "finish loading" << endl;

				string query = Util::getQueryFromFile(queryfile.c_str());

				if (query.empty())
				{
					cout << "the query file is empty!" << endl;
					return 0;
				}
				cout << "the query is :" + query << endl;
				string resultfile = Util::getArgValue(argc, argv, "f", "resultfile");
				ResultSet _rs;
				FILE *ofp = stdout;
				bool export_flag = false;
				if (resultfile.empty() == false)
				{
					ofp = fopen(resultfile.c_str(), "w");
					export_flag = true;
				}
				string msg;
				shared_ptr<Transaction> ptxn = make_shared<Transaction>(db_folder, 1, 1);
				// cout << ptxn << endl;
				int ret = _db.query(query, _rs, ofp, true, export_flag, nullptr);
				if (resultfile.empty() == false)
				{
					fclose(ofp);
					ofp = NULL;
				}
				if (ret <= -100) // select query
				{
					if (ret == -100)
					{
						msg = _rs.to_str();
					}
					else // query error
					{
						msg = "query failed.";
					}
				}
				else // update query
				{
					if (ret >= 0)
					{
						msg = "update num: " + Util::int2string(ret);
					}
					else // update error
					{
						msg = "update failed.";
					}
				}

				long tv_end = Util::get_cur_time();
				// stringstream ss;
				cout << "query database successfully, Used " << (tv_end - tv_begin) << " ms" << endl;
				// Log.Info(ss.str().c_str());
				return 0;
			}
		}
		else
		{
			// read query from file.

			// read query file path from terminal.
			// BETTER: sighandler ctrl+C/D/Z
			string query;
			_db.load();
			cout << "finish loading" << endl;
			// char resolved_path[PATH_MAX+1];
#ifdef READLINE_ON
			char *buf, prompt[] = "gsql>";
			// const int commands_num = 3;
			// char commands[][20] = {"help", "quit", "sparql"};
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
						// print commands message
						printf("help - print commands message\n");
						printf("quit - quit the console normally\n");
						printf("sparql - load query from the second argument\n");
					}
					else
					{
						// TODO: help for a given command
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
				// TODO: sparql + string, not only path
				string query_file;
				// BETTER:build a parser for this console
				bool ifredirect = false;

				char *rp = buf;
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

				char *p = buf + strlen(buf) - 1;
				FILE *fp = stdout; /// default to output on screen
				if (ifredirect)
				{
					char *tp = p;
					while (*tp == ' ' || *tp == '\t')
						tp--;
					*(tp + 1) = '\0';
					tp = rp + 1;
					while (*tp == ' ' || *tp == '\t')
						tp++;
					fp = fopen(tp, "w"); // NOTICE:not judge here!
					p = rp - 1;			 // NOTICE: all separated with ' ' or '\t'
				}
				while (*p == ' ' || *p == '\t') // set the end of path
					p--;
				*(p + 1) = '\0';
				p = buf + 6;
				while (*p == ' ' || *p == '\t') // acquire the start of path
					p++;
				// TODO: support the soft links(or hard links)
				// there are also readlink and getcwd functions for help
				// http://linux.die.net/man/2/readlink
				// NOTICE:getcwd and realpath cannot acquire the real path of file
				// in the same directory and the program is executing when the
				// system starts running
				// NOTICE: use realpath(p, NULL) is ok, but need to free the memory
				char *q = realpath(p, NULL); // QUERY:still not work for soft links
#ifdef DEBUG_PRECISE
				printf("%s\n", p);
#endif
				if (q == NULL)
				{
					printf("invalid path!\n");
					free(q);
					free(buf);
					continue;
				}
				else
					printf("%s\n", q);
				// query = getQueryFromFile(p);
				query = Util::getQueryFromFile(q);
				if (query.empty())
				{
					free(q);
					// free(resolved_path);
					free(buf);
					if (ifredirect)
						fclose(fp);
					continue;
				}
				printf("query is:\n");
				printf("%s\n\n", query.c_str());
				ResultSet _rs;
				// shared_ptr<Transaction> ptxn = make_shared<Transaction>(db_folder, 1, 1);
				int ret = _db.query(query, _rs, fp, true, false, nullptr);
				// int ret = _db.query(query, _rs, NULL);
				string msg;

				// cout<<"gquery ret: "<<ret<<endl;
				if (ret <= -100) // select query
				{
					if (ret == -100)
					{
						msg = "";
					}
					else // query error
					{
						msg = "query failed.";
					}
				}
				else // update query
				{
					if (ret >= 0)
					{
						msg = "update num: " + Util::int2string(ret);
					}
					else // update error
					{
						msg = "update failed.";
					}
				}

				if (ret != -100)
				{
					cout << msg << endl;
				}

				// test...
				// string answer_file = query_file+".out";
				// Util::save_to_file(answer_file.c_str(), _rs.to_str());
				free(q);
				// free(resolved_path);
				free(buf);
				if (ifredirect)
					fclose(fp);
#ifdef DEBUG_PRECISE
				printf("after buf freed!\n");
#endif
			}
		}

		//#else					//DEBUG:this not work!
		//    while(true)
		//    {
		//        cout << "please input query file path:" << endl;
		//        string query_file;
		//        cin >> query_file;
		//        //char* q = realpath(query_file.c_str(), NULL);
		//        string query = getQueryFromFile(query_file.c_str());
		//        if(query.empty())
		//        {
		//            //free(resolved_path);
		//            continue;
		//        }
		//        cout << "query is:" << endl;
		//        cout << query << endl << endl;
		//        ResultSet _rs;
		//        _db.query(query, _rs, stdout);
		//        //free(resolved_path);
		//    }
#endif // READLINE_ON

		return 0;
	}
}
