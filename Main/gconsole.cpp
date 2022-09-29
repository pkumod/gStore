/*=============================================================================
# Filename:		gconsole.cpp
# Author: Bookug Lobert, modified by Wang Libo, overwritten by Yuan Zhiqiu
# Mail: 1181955272@qq.com
# Last Modified:	2022-09-26 23:45
# Description:
This is a console integrating all commands in Gstore System and others. It
provides completion of command names, line editing features, and access to the
history list.
NOTICE: Commands end with ;. Cross line input is allowed.
Comment start with #. Redirect (> and >>) is supported.
CTRL+C to quit current command. CTRL+D to exit this console.
=============================================================================*/
#include "../Database/Database.h"
#include "../Util/Util.h"
#include <termios.h>

using namespace std;

/* PROGRAM STRUCTURE:
   1. parse cmd first, then demultiplex into handlers with arguments according to first cmd word;
   2. check privilege and loaded-or-not in handlers (instead of out of handlers);
   3. use GNU Readline for line editing/history/completion feature
*/

/* **************************************************************** */
/*                                                                  */
/*                            config                                */
/*                                                                  */
/* **************************************************************** */
//#define _GCONSOLE_TRACE
//#define _GCONSOLE_DEBUG
//#define _GCONSOLE_SHOW_SYSDB_QUERY

#define INIT_CONF_FILE "conf.ini"
#define MAX_WRONG_PSWD_TIMES 7
#define PRIVILEGE_NUM 8
// 0:root
// 1:query
// 2:load
// 3:unload
// 4:update
// 5:backup
// 6:restore
// 7:export
#define ROOT_PRIVILEGE_BIT 1u
#define QUERY_PRIVILEGE_BIT (1u << 1)
#define LOAD_PRIVILEGE_BIT (1u << 2)
#define UNLOAD_PRIVILEGE_BIT (1u << 3)
#define UPDATE_PRIVILEGE_BIT (1u << 4)
#define BACKUP_PRIVILEGE_BIT (1u << 5)
#define RESTORE_PRIVILEGE_BIT (1u << 6)
#define EXPORT_PRIVILEGE_BIT (1u << 7)
#define ALL_PRIVILEGE_BIT 0xFEu // 1111 1110, creater of db will be assigned this priv level to this db
const unordered_map<string, unsigned> privstr2bitset = {
	// not including root_priv and all_priv
	{"<has_query_priv>", QUERY_PRIVILEGE_BIT},
	{"<has_load_priv>", LOAD_PRIVILEGE_BIT},
	{"<has_unload_priv>", UNLOAD_PRIVILEGE_BIT},
	{"<has_update_priv>", UPDATE_PRIVILEGE_BIT},
	{"<has_backup_priv>", BACKUP_PRIVILEGE_BIT},
	{"<has_restore_priv>", RESTORE_PRIVILEGE_BIT},
	{"<has_export_priv>", EXPORT_PRIVILEGE_BIT},
};
// LSH offset of priv in bitset, to its name
const char *priv_offset2name[PRIVILEGE_NUM] = {"root", "query", "load", "unload", "update", "backup", "restore", "export"};

#define TOTAL_COMMAND_NUM 25
#define RAW_QUERY_CMD_OFFSET (TOTAL_COMMAND_NUM - 1) // rsw_query cmd offset in array commands, for fetching raw_query needed privilege_bitset for raw_query
#define QUIT_CMD_OFFSET 0

// ret_val of cmd_handler is reserved
// param: <cmd> <arg>: <arg>
// eg: connect ip port usr_name pswd, then param is (ip port usr_name pswd)
int help_handler(const vector<string> &);
int version_handler(const vector<string> &);
int settings_handler(const vector<string> &);
int quit_handler(const vector<string> &);
int clear_handler(const vector<string> &);
int pwd_handler(const vector<string> &);

int create_handler(const vector<string> &);
int drop_handler(const vector<string> &);
int show_handler(const vector<string> &);
int use_handler(const vector<string> &);
int backup_handler(const vector<string> &);
int restore_handler(const vector<string> &);
int export_handler(const vector<string> &);
int sparql_handler(const vector<string> &);
int raw_sparql_handler(string query);

int flushpriv_handler(const vector<string> &);
int pusr_handler(const vector<string> &);
int pdb_handler(const vector<string> &);
int showdbs_handler(const vector<string> &);

int setpswd_handler(const vector<string> &);
int setpriv_handler(const vector<string> &);
int addusr_handler(const vector<string> &);
int delusr_handler(const vector<string> &);
int showusrs_handler(const vector<string> &);

// int print_arg_handler(const vector<string> &);

typedef struct
{
	const char *name;					 // User printable name of the function
	int (*func)(const vector<string> &); // Function to call to do the job
	const char *doc;					 // Documentation for this function
	const char *usage;					 // Usage for this function
	unsigned privilege_bitset;			 // usr's privilege_bitset must cover this to use this command(test: through &)
} COMMAND;
COMMAND commands[] =
	{
		///*
		{"quit", quit_handler, "Quit this console.", "quit;", 0},

		// database op
		{"sparql", sparql_handler, "Answer SPARQL query(s) in file.", "sparql <; separated SPARQL file>;", QUERY_PRIVILEGE_BIT}, // file query
		{"create", create_handler, "Build a database from a dataset or create an empty database.", "create <database_name> [<nt_file_path>];", 0},
		{"use", use_handler, "Set current database.", "use <database_name>;", LOAD_PRIVILEGE_BIT | UNLOAD_PRIVILEGE_BIT},
		{"drop", drop_handler, "Drop a database.", "drop <database_name>;", ALL_PRIVILEGE_BIT},
		{"show", show_handler, "Show info and specified number of triples of current database or other database.", "show [<database_name>] [-n <displayed_triple_num>];", QUERY_PRIVILEGE_BIT},
		{"showdbs", showdbs_handler, "Display all databases the current user has query privilege on.", "showdbs;", 0},
		{"backup", backup_handler, "Backup current database.", "backup [<backup_path>];", BACKUP_PRIVILEGE_BIT},
		{"restore", restore_handler, "Restore a database.", "restore <database_name> <backup_path>;", RESTORE_PRIVILEGE_BIT},
		{"export", export_handler, "Export a database to .nt file.", "export <database_name> <file_path>;", EXPORT_PRIVILEGE_BIT},
		{"pdb", pdb_handler, "Display current database name.", "pdb;", 0},

		// id and usr manage
		{"flushpriv", flushpriv_handler, "Flush priv for current user, updating the in-memory structure.", "flushpriv;", 0},
		{"pusr", pusr_handler, "Display user's username and privilege.", "pusr; pusr <database_name>; pusr <database_name> <usr_name>;", 0},
		{"setpswd", setpswd_handler, "Set your password. Be able to set other's password if you are root.", "setpswd; setpswd <usrname>;", ROOT_PRIVILEGE_BIT},
		{"setpriv", setpriv_handler, "Set user's privilege.", "setpriv <usrname> <database_name>;", ROOT_PRIVILEGE_BIT},
		{"addusr", addusr_handler, "Add user.", "addusr <usrname>;", ROOT_PRIVILEGE_BIT},
		{"delusr", delusr_handler, "Del user.", "delusr <usrname>;", ROOT_PRIVILEGE_BIT},
		{"showusrs", showusrs_handler, "Show all users and privilege for each.", "showusrs;", ROOT_PRIVILEGE_BIT},

		// other
		// {"cancel", 0, "Quit current input command.", "enter \"cancel;\" whenever you need to quit current input, remember the ;", 0}, // execute_line, check whether the line ends with cancel
		{"help", help_handler, "Display help msg. Enter 'help/?' see more about usage.", "help/? [edit/usage/<command>];", 0},
		{"?", help_handler, "Synonym for \"help\".", "help/? [edit/usage/<command>];", 0},
		{"settings", settings_handler, "Display settings.", "settings [<conf_name>];", 0},
		{"version", version_handler, "Display gstore core version.", "version;", 0},

		// linux shell cmd
		{"pwd", pwd_handler, "Print name of current/working directory.", "pwd;", 0},
		{"clear", clear_handler, "Clear screen.", "clear;", 0},

		// raw_sparql
		{"raw_sparql", 0, "Support enter sparql query directedly in gconsole.",
		 "Begin with SELECT, INSERT, DELETE, PREFIX or BASE. For more about SPARQL, see https://www.w3.org/TR/sparql11-query/ and http://www.gstore.cn/pcsite/index.html#/documentation", QUERY_PRIVILEGE_BIT},
		// handler: int raw_sparql_handler(string query);

		//*/
		/* //for debug: print_arg_handler and quit_handler
		{"query", print_arg_handler},
		{"create", print_arg_handler},
		{"use", print_arg_handler},
		{"drop", print_arg_handler},
		{"show", print_arg_handler},
		{"backup", print_arg_handler},
		{"restore", print_arg_handler},
		{"load", print_arg_handler},
		{"unload", print_arg_handler},
		{"source", print_arg_handler},
		{"clear", print_arg_handler},
		{"quit", quit_handler},
		{"help", print_arg_handler},
		{"?", print_arg_handler},
		*/
};

/* **************************************************************** */
/*                                                                  */
/*                    macro and declaration                         */
/*                                                                  */
/* **************************************************************** */
#define CROSS_LINE_PROMPT "     -> "
#define PROMPT_LEN 8
#define PRINT_ENTER_HELP_MSG                                                                                                 \
	cout << "Gstore Ver " << gstore_version << " for Linux on x86_64 (Source distribution)" << endl;                         \
	cout << "Gstore Console(gconsole), an interactive shell based utility to communicate with gStore repositories." << endl; \
	cout << "Copyright (c) 2016, 2022, pkumod and/or its affiliates." << endl;                                               \
	cout << "" << endl;                                                                                                      \
	cout << "Usage: bin/gconsole [OPTIONS]" << endl;                                                                         \
	cout << "  -?, --help          Display this help and exit." << endl;                                                     \
	cout << "  -u, --user          username. " << endl;                                                                      \
	cout << "  " << endl;                                                                                                    \
	cout << "Supported command in gconsole: Type \"?\" or \"help\" in the console to see info of all commands." << endl;     \
	cout << "  " << endl;                                                                                                    \
	cout << "For bug reports and suggestions, see https://github.com/pkumod/gStore" << endl                                  \
		 << endl;

#define PRINT_WRONG_USG    \
	cout << "Wrong usage." \
		 << endl;
#define SYSDB_QUERY_FAILED(sparql)                                      \
	cout << "System db query failed. The query is: " << sparql << endl; \
	return -1;
#define PRINT_VERSION                                                               \
	cout << "Gstore version: " << gstore_version << " Source distribution" << endl; \
	cout << "Copyright (c) 2016, 2022, pkumod and/or its affiliates." << endl;
#define CHECK_CURRENT_DB_LOADED                                                                                    \
	if (current_database == 0)                                                                                     \
	{                                                                                                              \
		cout << "Current database not selected. Please select it first, through \"USE <database_name>\"." << endl; \
		return -1;                                                                                                 \
	}

char *dupstr(const char *);
char *stripwhite(char *);
string stripwhite(const string &s);
string rm_comment(string line);
void replace_cr(string &line);
int find_command(const char *name);
void single_cmd();
int execute_line(char *);
bool parse_arguments(char *, vector<string> &);

void ctrlc_handler(int);
void initialize_readline();
int save_history();
int load_history();

int enter_pswd(string prompt);
int silence_sysdb_query(const string &query, ResultSet &_rs);
vector<int> silence_sysdb_query(const string &query, vector<ResultSet> &_rs);
int read_pswd(string usr_name, string &pswd);
unsigned read_priv(string usr, string db_name);
unsigned get_priv(string usr, string db_name);

/* **************************************************************** */
/*                                                                  */
/*                    global var and main                           */
/*                                                                  */
/* **************************************************************** */
bool gconsole_done = false; // 1: quit
bool gconsole_eoq = false;	// 1: current command input done
// bool gconsole_inputing = false; // 1: still inputing current command: control prompt

pthread_t child_th = 0;
int in_readline = 0;
// FILE *output = stdout;
string usrname, stdpswd;
Database *current_database = NULL;
unsigned current_privilege_bitset = 0;	 // when no current_database: 0, i.e. no priv
int current_cmd_offset = -1;			 // current_cmd offset in commands
unordered_map<string, unsigned> db2priv; // for current usr, cache in memory, avoiding fetch from sysdb everytime

string db_home = ".";
string gstore_version, root_username, root_password;

int main(int argc, char **argv)
{
	Util util; // This is needed for database loading(Database_instance.load()) and other Util static member fetching situation
	//  read conf from conf.ini: version, root_name, root_pswd
	{
		ifstream fin(INIT_CONF_FILE);
		if (fin.is_open() == 0)
		{
			cout << string("File opened failed: ") << INIT_CONF_FILE << endl;
			return 0;
		}
		string line;
		while (getline(fin, line))
		{
			if (line.find("root_username") != string::npos) // len(root_username):13
				root_username = line.substr(15, line.size() - 16);
			else if (line.find("root_password") != string::npos)
				root_password = line.substr(15, line.size() - 16);
			else if (line.find("version") != string::npos) // len(version):7
				gstore_version = line.substr(8, line.size() - 8);
		}
		fin.close();

#ifdef _GCONSOLE_TRACE
		cout << "[root_username:]" << root_username << endl;
		cout << "[root_password:]" << root_password << endl;
		cout << "[gstore_version:]" << gstore_version << endl;
#endif //_GCONSOLE_TRACE
	}

	if (argc == 2)
	{
		if ((strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0))
		{
			PRINT_ENTER_HELP_MSG
			return 0;
		}
		if (strcmp(argv[1], "--version") == 0)
		{
			PRINT_VERSION
			return 0;
		}
	}
	if ((argc != 1 && argc != 3) || (argc == 3 && strcmp("-u", argv[1]) && strcmp("--user", argv[1]) == 0))
	{
		PRINT_WRONG_USG
		PRINT_ENTER_HELP_MSG
		return 0;
	}

	db_home = Util::global_config["db_home"];

	/* parse options */
	if (argc == 3)
	{
		usrname = argv[2];
	}
	else
	{
		cout << "Enter user name: ";
		cin >> usrname;
		getchar(); // absorb the '\n'
	}

	/* check usrname and pswd */
	{
		// get stdpswd according to usrname
		if (usrname == root_username)
			stdpswd = root_password;
		else
		{
			int wrong_usr_cnt = 0;
			while (wrong_usr_cnt < MAX_WRONG_PSWD_TIMES && read_pswd(usrname, stdpswd) && usrname != root_username)
			{
				cout << "user " << usrname << " doesn't exists! Please try again." << endl;
				cout << "Enter user name: ";
				cin >> usrname;
				getchar(); // absorb the '\n'
				++wrong_usr_cnt;
				stdpswd.clear();
			}
			if (wrong_usr_cnt >= MAX_WRONG_PSWD_TIMES)
			{
				cout << "Please check your remember list for your usrname." << endl;
				return 0;
			}
			if (usrname == root_username)
			{
				stdpswd = root_password;
			}
		}
#ifdef _GCONSOLE_TRACE
		cout << "[stdpswd:]" << stdpswd << endl;
#endif //_GCONSOLE_TRACE
		if (stdpswd.empty())
		{
			cout << "Warn: pswd for you(" << usrname << ") is empty!" << endl;
		}

		if (enter_pswd("Enter password: "))
		{
			return 0;
		}
	}

	/* welcome and work */
	cout << endl;
	cout << "Gstore Console(gconsole), an interactive shell based utility to communicate with gStore repositories." << endl;
	PRINT_VERSION
	cout << "" << endl;
	cout << "Welcome to the gStore Console." << endl;
	cout << "Commands end with ;. Cross line input is allowed." << endl;
	cout << "Comment start with #. Redirect (> and >>) is supported." << endl;
	cout << "CTRL+C to quit current command. CTRL+D to exit this console." << endl;
	cout << "Type 'help;' for help. " << endl
		 << endl;

	// signal handler for ctrl+c
	signal(SIGINT, ctrlc_handler);
#ifdef _GCONSOLE_DEBUG
	cout << "[main_th_id:]" << pthread_self() << endl;
#endif //#ifdef _GCONSOLE_DEBUG

	initialize_readline(); // set completer and readline end char(;)
	rl_catch_signals = 0;  // If this variable is non-zero, Readline will install signal handlers for SIGINT, SIGQUIT, SIGTERM, SIGALRM, SIGTSTP, SIGTTIN, and SIGTTOU.
	// the following behavior prevents quitting current inputing instantly:
	// Readline contains an internal signal handler that is installed for a number of signals
	// (SIGINT, SIGQUIT, SIGTERM, SIGALRM, SIGTSTP, SIGTTIN, and SIGTTOU).
	// When one of these signals is received,
	// the signal handler will reset the terminal attributes to those that were in effect before readline () was called,
	// reset the signal handling to what it was before readline () was called,
	// and resend the signal to the calling application.
	// If and when the calling application's signal handler returns, Readline will reinitialize the terminal and continue to accept input.
	// When a SIGINT is received, the Readline signal handler performs some additional work, which will cause any partially-entered line to be aborted (see the description of rl_free_line_state ()).

	using_history();
	load_history();

	// Loop reading and executing lines until the user quits.
	while (!gconsole_done)
	{
		thread th(single_cmd);
		child_th = th.native_handle();
		th.join();
		child_th = 0;
	}
	save_history();
	exit(0);
}

/* **************************************************************** */
/*                                                                  */
/*                  Some Utilities                                  */
/*                                                                  */
/* **************************************************************** */
// implement redirect stdout in class: exception safety, RAII
// destructors of all objects are ensured to be called when exception occurs(ensured by compiler)
// (definition explain: if <des> is descriptor to <file>, then we call <des>'s refer is <file>)
// redirect: redirect descriptor 1's refer ori_file to file(func's param)(and push ori_file to static stk top)
// destruct: restore descriptor 1's refer to stk top(and pop stk) for redirect_cnt times, fflush at each restore
// static stk design ensures when there's no RedirectStdout instance, descriptor 1's refer is stdout
class RedirectStdout
{
	static stack<int> ori_file_stk;
	int redirect_cnt = 0;

public:
	RedirectStdout() : redirect_cnt(0)
	{
	}

	RedirectStdout(const char *file, int append = 0) : redirect_cnt(0)
	{
		redirect(file, append);
	}

	/* redirect descriptor 1's refer ori_file to file(and push ori_file to static stk top) */
	// append: if set, then open file with |O_APPEND
	int redirect(const char *file, int append = 0)
	{
		// TODO: check these syscall's return value and return accordingly
		// now 1 is descriptor to ori_file(if stk is empty, ori_file is STDOUT; else ori_file is stk.top())

		int pfd;
		if (append)
		{
			pfd = open(file, O_WRONLY | O_CREAT | O_APPEND, 0777);
		}
		else
		{
			pfd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0777);
		}
		// now pfd is descriptor to file
		int saved = dup(1); // now 1 and saved both are descriptor to ori_file(1 previously refer to)
		// int dup2(int oldfd, int newfd);
		dup2(pfd, 1); // would close descriptor 1 first, then now 1 and pfd both are descriptor to file(pfd previously refer to)
		close(pfd);	  // close descriptor pfd

		ori_file_stk.push(saved);
		// now saved(stk.top()) is descriptor to ori_file, 1 is descriptor to file

		++redirect_cnt;
		return 0;
	}

	/* fflush and restore descriptor 1's refer to stk top(and pop stk) */
	~RedirectStdout()
	{
		// TODO: check these syscall's return value
		while (redirect_cnt--)
		{
			fflush(stdout); // NOTE that flush the buffer is necessary to indeed push content into file

			int saved = ori_file_stk.top();
			ori_file_stk.pop();
			dup2(saved, 1); // would close descriptor 1 first, then now 1 and saved both are descriptor to "saved previously refer to"
			close(saved);	// close descriptor saved

			// now only 1 is descriptor to "saved previously refer to"
		}
	}
};
stack<int> RedirectStdout::ori_file_stk;

// RAII
class HideStdinDisplay
{
	termios oldt;

public:
	HideStdinDisplay()
	{
		// set attribute of stdin: hide input pswd
		tcgetattr(STDIN_FILENO, &oldt);
		termios newt = oldt;
		newt.c_lflag &= ~ECHO;					 // ECHO bit: Enables echo. If this flag is set, characters are echoed as they are received.
		tcsetattr(STDIN_FILENO, TCSANOW, &newt); // TCSANOW: Change attributes immediately.
	}
	~HideStdinDisplay()
	{
		// recover attribute
		tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	}
};
// finally enter correct pswd: return 0; failed return -1
// usrname and stdpswd must have been filled
int enter_pswd(string prompt)
{
	HideStdinDisplay hide_ins;

	int wrong_pswd_cnt = 0;
	string pswd;
	while (wrong_pswd_cnt < MAX_WRONG_PSWD_TIMES)
	{
		cout << prompt;
		char c;
		while ((c = getchar()) != -1 && c != '\n' && c != '\r')
		{
			pswd.push_back(c);
		}
		if (feof(stdin))
		{
			cout << "End of stdin!" << endl;
			return 0;
		}
		cout << endl;
		if (pswd == stdpswd)
		{
			break;
		}
		++wrong_pswd_cnt;
		pswd.clear();
		cout << "Wrong password. Please try again." << endl;
	}
	if (wrong_pswd_cnt >= MAX_WRONG_PSWD_TIMES)
	{
		cout << "Please check the user name and password." << endl;
		return -1;
	}

	return 0;
}

class ReadlineWrapper
{
	int &flag;

public:
	ReadlineWrapper(int &flag, char *&line, const char *prompt) : flag(flag)
	{
		flag = 1;
		// printf(prompt);
		line = readline(prompt);
		if (line == NULL)
			return;

		// replace CROSS_LINE_PROMPT from line
		string tmp(line); // line is copied into
		size_t idx;
		while ((idx = tmp.find(CROSS_LINE_PROMPT)) != string::npos)
		{
			tmp.erase(idx, PROMPT_LEN);
		}
		free(line);
		line = dupstr(tmp.c_str());
	}
	~ReadlineWrapper() { flag = 0; }
};

char *
dupstr(const char *s)
{
	char *r;
	int len = strlen(s) + 1;
	r = (char *)malloc(len); // BETTER:xmalloc?
	memset(r, 0, sizeof(char) * (len));
	strcpy(r, s);
	return r;
}

// thread routine: parse command and exec; set gconsole_done if quit is needed
void single_cmd()
{
	char *line;
#ifdef _GCONSOLE_DEBUG
	cout << "[in single_cmd]" << pthread_self() << endl;
#endif //#ifdef _GCONSOLE_DEBUG

	{
		volatile ReadlineWrapper rl(in_readline, line, "gstore> ");
	}

#ifdef _GCONSOLE_TRACE
	cout << "[rl_line_buffer:]" << rl_line_buffer << endl;
	cout << "[readline_get:]" << line << endl;
#endif //_GCONSOLE_TRACE

	if (line == NULL) // EOF or Ctrl-D
	{
		if (current_database != NULL)
		{
			// current_database->unload(); //NOTE: destructor of Database would call unload to release mem, if call unload explicitly would end up double free
			delete current_database;
			current_database = NULL;
		}
		cout << endl
			 << endl;
		gconsole_done = 1;
		return;
	}

	// Remove comment and leading-trailing whitespace from the line.
	// Then, if there is anything left, add it to the history
	// list and execute it.

	// a copy of the null-terminated character string pointed to by s. The length of the string is determined by the first null character.
	string strline(line);
	free(line);
	strline = rm_comment(std::move(strline));
	strline = stripwhite(std::move(strline));
	replace_cr(strline);

	if (strline.empty() == 0)
	{
		add_history(strline.c_str()); // cross-line-style cmd is stored by concating in line("line" is one line!)
		if (execute_line(const_cast<char *>(strline.c_str())))
			gconsole_done = 1;
	}
}

// Execute a command line. won't check priv
// return quit(-1) or not(0)
int execute_line(char *line)
{
#ifdef _GCONSOLE_DEBUG
	cout << "[exec:]" << line << endl;
#endif //_GCONSOLE_DEBUG

	// // whether cancel
	// int i = strlen(line) - 1;
	// while (i > -1 && whitespace(line[i]))
	// {
	// 	--i;
	// }					  // now i hits the last word(line is not empty, so there must be at least one word)
	// if (i > -1 && i >= 5) // len(cancel):6
	// {
	// 	// line[i+1] is white or '\0'
	// 	line[i + 1] = 0;
	// 	if (strcmp("cancel", line + i - 5) == 0) // cancel
	// 	{
	// 		return 0;
	// 	}
	// 	// no need to recover line[i+1], since it's the end() of last word
	// }

	int i = 0;
	char *word = NULL;

	// whether redirected: > or >>
	RedirectStdout redirect;
	bool is_redirected = false;
	int j = strlen(line) - 1;
	while (j > -1)
	{
		// search for > from tail of line before }: sparql triple may contain >, which would only be in {} or before {}
		if (line[j] == '"' || line[j] == '}')
			break;
		else if (line[j] == '>') // pos of last '>'
		{
			is_redirected = true;
			i = j;
			break;
		}
		else
			j--;
	}
	if (is_redirected)
	{
		j++;
		while (line[j] && whitespace(line[j]))
		{
			j++;
		}
		cout << "Redirect output to file: " << (line + j) << endl;
		// >>
		if (i > 0 && line[i - 1] == '>')
		{
			// output = fopen(line + j, "a+");
			line[i - 1] = '\0';
			redirect.redirect(line + j, 1); // redirect stdout
		}
		// >
		else
		{
			// output = fopen(line + j, "w+");
			line[i] = '\0';
			redirect.redirect(line + j); // redirect stdout
		}
		// if (output == NULL)
		// {
		// 	cout << "Failed to open " << (line + j) << endl;
		// 	output = stdout;
		// }
	}

	// Isolate the command word.
	i = 0;
	while (line[i] && whitespace(line[i]))
		i++;
	word = line + i; // start of cmd_word

	while (line[i] && !whitespace(line[i]))
		i++;
	// now line[i] is the char just after the command word

	char recover_ch = line[i];
	if (line[i])
		line[i] = '\0';

#ifdef _GCONSOLE_TRACE
	cout << "[cmdword:]" << word << endl;
#endif //#ifdef _GCONSOLE_TRACE

	// cmd raw query
	if ((current_cmd_offset = find_command(word)) == RAW_QUERY_CMD_OFFSET)
	{
		line[i] = recover_ch; // recover line: line is total sparql
#ifdef _GCONSOLE_DEBUG
		cout << "[The query is: ]" << line << endl;
#endif //_GCONSOLE_DEBUG
		raw_sparql_handler(line);
	}
	// other command
	else
	{
		if (current_cmd_offset < 0)
		{
			cout << word << ": No such command for gconsole." << endl
				 << endl;
			// if (output != stdout)
			// {
			// 	fclose(output);
			// 	output = stdout;
			// }
			return 0;
		}

		if (recover_ch) // line[i] is not '\0', there are args
		{				// note that if line[i] is '\0', the following logic is undefined!
			// before this line: line[i] is the char just after the command word, and was set to '\0'
			++i;
			// Get argument to command, if any.
			while (line[i] && whitespace(line[i]))
				i++;
			word = line + i;
		}
		else // line[i] is '\0', there's no arg
		{
			word[0] = '\0';
		}

		vector<string> args;
		if (parse_arguments(word, args) == 0)
		{
			cout << endl;
			return 0;
		}

		// quit
		if (current_cmd_offset == QUIT_CMD_OFFSET)
		{
			if (quit_handler(args))
			{ // wrong args, so don't quit
				return 0;
			}
			return -1; // return quit(-1) or not(0)
		}

		commands[current_cmd_offset].func(args);
	}

	// if (output != stdout)
	// {
	// 	fclose(output);
	// 	output = stdout;
	// }
	cout << endl;
	return 0;
}

// Look up NAME as the name of a command.
// return cmd offset in commands.(return RAW_QUERY_CMD_OFFSET if NAME indicates raw_query cmd.)
// return -1 if NAME isn't a command name.
int find_command(const char *name)
{
	int i;
	string lower_name = name;
	for (char &c : lower_name)
	{
		c = tolower(c);
	}
	// sparql query
	// TODO: better parsing of sparql
	if (lower_name == "select" || lower_name == "insert" || lower_name == "delete" || lower_name == "prefix" || lower_name == "base")
	{
		return RAW_QUERY_CMD_OFFSET;
	}

	for (i = 0; i < TOTAL_COMMAND_NUM; i++)
	{
		if (lower_name == commands[i].name)
		{
			return i;
		}
	}

	return -1;
}

// Strip whitespace from the start and end of STRING. Return a pointer into STRING.
char *stripwhite(char *string)
{
	char *s, *t;

	for (s = string; whitespace(*s); s++)
		;

	if (*s == 0)
		return s;

	t = s + strlen(s) - 1;
	while (t > s && (whitespace(*t) || *t == '\r' || *t == '\n'))
	{
		t--;
	}
	*++t = '\0';

	return s;
}

// Parse arguments
bool parse_arguments(char *word, vector<string> &args)
{
	if (word == NULL)
	{
		return true;
	}

#ifdef _GCONSOLE_TRACE
	cout << "[search args with:]" << word << "\t[args:]";
#endif //#ifdef _GCONSOLE_TRACE

	while (*word)
	{
		int i = 0;

		if (*word == '\"')
		{
			i++;
			while (word[i] && word[i] != '\"')
			{
				i++;
			}
			char tmp = word[i + 1];
			if (word[i] == '\"' && (whitespace(tmp) || tmp == '\0'))
			{
				word[++i] = '\0';
				args.push_back(string(word));
				word[i] = tmp;
				while (word[i] && whitespace(word[i]))
				{
					i++;
				}
				word += i;
				continue;
			}
			else
			{
				cout << "Invalid arguments!" << endl;
				return false;
			}
		}

		while (word[i] && !whitespace(word[i]))
		{
			i++;
		}
		char tmp = word[i];
		word[i] = '\0';
		args.push_back(string(word));
#ifdef _GCONSOLE_TRACE
		cout << word << " ";
#endif //#ifdef _GCONSOLE_TRACE

		word[i] = tmp;
		while (word[i] && whitespace(word[i]))
		{
			i++;
		}
		word += i;
	}
#ifdef _GCONSOLE_TRACE
	cout << endl;
#endif //#ifdef _GCONSOLE_TRACE
	return true;
}

// history is saved separately for each user
// Save command history
// Notice that only by quitting gconsole normally can command history be saved
int save_history()
{
	// Limit the number of history to save
	static const int max_history = 1024;
	stifle_history(max_history);

	HIST_ENTRY **pHisList = history_list();

	if (pHisList == NULL)
	{
		return 0;
	}

	// TODO: check this return value
	system("mkdir -p bin/.gconsole_history/");
	ofstream fout("bin/.gconsole_history/" + usrname);

	if (fout.is_open() == 0)
	{
		cout << "File open failed: bin/.gconsole_history/" + usrname << ". will create one." << endl;
		return -1;
	}

	while (pHisList[0] != NULL)
	{
		fout << string(pHisList[0]->line) << endl;
		pHisList++;
	}

	fout.close();

	return 0;
}

// Load command history
int load_history()
{
	ifstream fin("bin/.gconsole_history/" + usrname);

	if (fin.is_open() == 0)
	{
		cout << "File open failed: bin/.gconsole_history/" + usrname << endl;
		return -1;
	}

	const int line_length = 1024;
	char line[line_length];

	while (fin.getline(line, line_length))
	{
		add_history(line);
	}

	fin.close();

	return 0;
}

/* **************************************************************** */
/*                                                                  */
/*                  query system_db related                         */
/*                                                                  */
/* **************************************************************** */
// has user:return 0, else return -1; query failed return -1
int read_pswd(string usr_name, string &pswd)
{
	ResultSet rs;
	string sqarql = "select ?x where{<" + usr_name + "><has_password>?x.}";
	if (silence_sysdb_query(sqarql, rs) == 0)
	{
		if (rs.ansNum)
		{
			pswd = rs.answer[0][0];
			// strip ""
			pswd = pswd.substr(1, pswd.size() - 2);
			return 0;
		}
		return -1;
	}
	return -1;
}

// print lowest sz bits of priv
void print_lowbits(unsigned priv, int sz)
{
	for (int i = sz - 1; i >= 0; --i)
	{
		cout << (((1 << i) & priv) != 0);
	}
}

// fetch usr priv on db_name from sysdb. return priv
// return -1u on fail
unsigned read_priv(string usr, string db_name)
{
	unsigned priv = 0;
	string query = "SELECT ?p WHERE{<" + usr + "> ?p <" + db_name + "> .}";
	ResultSet rs;
	if (silence_sysdb_query(query, rs))
	{
		SYSDB_QUERY_FAILED(query)
	}
	for (unsigned i = 0; i < rs.ansNum; ++i)
	{
		string privstr = rs.answer[i][0];
#ifdef _GCONSOLE_TRACE
		cout << privstr << endl;
#endif //_GCONSOLE_TRACE
		if (privstr2bitset.count(privstr) == 0)
		{
			cout << "Strange priv: " << privstr << ".\n Please check system db contents." << endl;
			return -1;
		}
		priv |= privstr2bitset.at(privstr);
	}
#ifdef _GCONSOLE_TRACE
	cout << "[priv in binary]:0b";
	print_lowbits(priv, 8);
	cout << endl;
#endif //_GCONSOLE_TRACE
	return priv;
}

// return priv bitset of usr on db_name
// check db_name exist or not
unsigned get_priv(string usr, string db_name)
{
	// db not exist
	if (access(string(db_name + ".db").c_str(), F_OK))
	{
		cout << "Database " << db_name << " does not exist." << endl;
		return -1u;
	}

	if (usr == root_username)
	{
		return ROOT_PRIVILEGE_BIT;
	}
	if (usr == usrname && db2priv.count(db_name))
	{
		return db2priv[db_name];
	}

	unsigned priv = read_priv(usr, db_name);
	if (priv == -1u)
	{
		cout << "Read priv failed." << endl;
		return -1u;
	}
	if (usr == usrname)
		db2priv[db_name] = priv;
	return priv;
}

// usrname has request_priv on db_name: return 0, else return -1;
// db_name doesn't exist: return -1
int check_priv(string db_name, unsigned request_priv)
{
	unsigned priv = get_priv(usrname, db_name);
	if (priv == -1u)
	{
		return -1;
	}
	if (priv != ROOT_PRIVILEGE_BIT && (priv & request_priv) != request_priv)
	{
		cout << "Permission denied. Check your privilege with database: " << db_name << endl;
		return -1;
	}
	return 0;
}

// Query system db. Support multi query, separated by ;
// for each query: return -1: failed, error report is done inside. return 0: succeed.
// absorb query output to cout
//! _rs NEED to be already sized to correct size: because vector<ResultSet> grow step by step is dangerous(key: for ResultSet, copy: LOW copy; destruct: release all pointers), refer to the comments in function body for further explaination
vector<int> silence_sysdb_query(const string &query, vector<ResultSet> &_rs)
{
#ifdef _GCONSOLE_SHOW_SYSDB_QUERY
	cout << "\x1b[34m[sparql to sysdb]:" << query << "\x1b[0m" << endl;
#endif //_GCONSOLE_SHOW_SYSDB_QUERY

	vector<int> retv;

	// redirect stdout to bin/.gconsole_tmp_out: for silencing load&query output of system.db
	{
		RedirectStdout silence("bin/.gconsole_tmp_out");

		Database system_db("system");
		system_db.load();

		stringstream ss(query);
		string sparql;
		int has_success_update = 0;
		int sz = 0;
		while (getline(ss, sparql, ';'))
		{
			/* vector<ResultSet> grow step by step is dangerous(eg: _rs.resize(sz + 1)):
			when reallocating is needed,
			would first copy elements to new mem then call DESTRUCTOR on previous elements,
			which would release all pointers of destructing objects;
			and copy assignment operator only carry out LOW copy */
			int ret = system_db.query(sparql, _rs[sz]);
			if ((ret <= -100 && ret != -100) || (ret > -100 && ret < 0)) // select query failed or update query failed
			{
				cout << "System db query failed. The query is: " << query << endl;
				retv.push_back(-1);
			}
			else
			{
				if (ret >= 0)
				{
					has_success_update = 1;
				}
				retv.push_back(0);
			}
			++sz;

			/*NOTE: this would fail:
			ResultSet rs; //would call destructor after this turn while scope
			_rs.push_back(rs); //call copy assignment operator: LOW copy
			rs destructor would release all pointer to heap mem, so pointers of this rs in vector now points to released mem!\*/
		}

		if (has_success_update)
		{
			system_db.save();
		}

		// system_db.unload(); //NOTE: destructor of Database would call unload to release mem, if call unload explicitly would end up double free
	}

	// remove tmpout file //TODO: check this return value
	system("rm -rf bin/.gconsole_tmp_out");

	return move(retv);
}
// single query
// return -1: failed, error report is done inside. return 0: succeed.
int silence_sysdb_query(const string &query, ResultSet &_rs)
{
#ifdef _GCONSOLE_SHOW_SYSDB_QUERY
	cout << "\x1b[34m[sparql to sysdb]:" << query << "\x1b[0m" << endl;
#endif //_GCONSOLE_SHOW_SYSDB_QUERY

	int ret;

	// redirect stdout to bin/.gconsole_tmp_out: for silencing load&query output of system.db
	{
		RedirectStdout silence("bin/.gconsole_tmp_out");

		Database system_db("system");
		system_db.load();

		ret = system_db.query(query, _rs);
		if (ret >= 0)
		{ // update and update succeed
			system_db.save();
		}

		// system_db.unload(); //NOTE: destructor of Database would call unload to release mem, if call unload explicitly would end up double free
	}

	// remove tmpout file //TODO: check this return value
	system("rm -rf bin/.gconsole_tmp_out");

	if ((ret <= -100 && ret != -100) || (ret > -100 && ret < 0)) // select query failed or update query failed
	{
		SYSDB_QUERY_FAILED(query)
	}
	return 0;
}

/* **************************************************************** */
/*                                                                  */
/*                  Interface to Readline Completion                */
/*                                                                  */
/* **************************************************************** */

char *command_generator(const char *, int);
char **gconsole_completion(const char *, int, int);
int gconsole_bind_cr(int count, int key);
// TODO: bind backspace and delete key to better support cross line command editing
// note: binding to '\b''\d' or 8 46 takes no effect
int gconsole_bind_eoq(int count, int key);
// int gconsole_bind_del(int count, int key);
// /*delete edit*/
// int gconsole_bind_del(int count, int key)
// {
// 	cout << "[in gconsole_bind_del]" << endl;
// 	// check whether is "     -> "
// 	const char *special = "\n     -> "; // len9
// 	int cross = 1;
// 	// if rl_point is next char
// 	if (rl_point >= 9)
// 	{
// 		for (int i = 0; i < 9; ++i)
// 		{
// 			if (rl_line_buffer[rl_point - i - 1] != special[9 - i])
// 			{
// 				cout << "not eq[rl_line_buffer(" << rl_point - i - 1 << "):" << rl_line_buffer[rl_point - i - 1] << "]" << endl;
// 				cout << "[special(" << 9 - i << "):" << special[9 - i] << "]" << endl;
// 				cross = 0;
// 			}
// 		}
// 	}
// 	if (cross)
// 	{
// 		// if[b,e)
// 		rl_delete_text(rl_point - 10, rl_point);
// 		rl_point -= 10;
// 	}
// 	else
// 	{
// 		rl_delete_text(rl_point - 1, rl_point);
// 		--rl_point;
// 	}
// 	return 0;
// }

// ctrl+c signal handler: quit current cmd inputting or executing
void ctrlc_handler(int signo)
{
#ifdef _GCONSOLE_DEBUG
	cout << "[exec ctrl+c handler:]" << pthread_self() << endl;
	cout << "[pthread_cancel:]" << child_th << endl;
#endif //#ifdef _GCONSOLE_DEBUG
	if (child_th)
	{ // quit current cmd exec
		if (in_readline)
		{
			rl_free_line_state();
			rl_cleanup_after_signal();
		}
		pthread_cancel(child_th);
		cout << "\x1b[0m" << endl; // reset shell color
	}
}

/* Tell the GNU Readline library how to complete.  We want to try to complete
on command names if this is the first word in the line, or on filenames
if not. */
void initialize_readline()
{
	// This variable is set to a unique name by each application using Readline.
	// Allow conditional parsing of the ~/.inputrc file.
	rl_readline_name = "gconsole";

	// complete function: when pressing tab, would call this function
	rl_attempted_completion_function = gconsole_completion;

	// bind keys: for end with ;
	// function is the address of the function to call when key is pressed
	rl_bind_key('\n', gconsole_bind_cr);
	rl_bind_key('\r', gconsole_bind_cr);
	rl_bind_key(';', gconsole_bind_eoq);
}

/* Attempt to complete on the contents of TEXT. START and END bound the
region of rl_line_buffer that contains the word to complete. TEXT is
the word to complete. We can use the entire contents of rl_line_buffer
in case we want to do some simple parsing. Return the array of matches,
or NULL if there aren't any. */
char **
gconsole_completion(const char *text, int start, int end)
{
	char **matches;

	matches = (char **)NULL;

	// If this word is at the start of the line, then it is a command
	// to complete. Otherwise it is the name of a file in the current directory.
	if (start == 0)
	{
		matches = rl_completion_matches(text, command_generator);
	}

	return matches;
}

/* Generator function for command completion. STATE lets us know whether
to start from scratch; without any state(i.e. STATE == 0), then we
start at the top of the list. */
char *
command_generator(const char *text, int state)
{
	static int list_index, len;
	const char *name;

	/* If this is a new word to complete, initialize now.  This includes
	saving the length of TEXT for efficiency, and initializing the index
	variable to 0. */
	if (!state)
	{
		list_index = 0;
		len = strlen(text);
	}

	/* Return the next name which partially matches from the command list. */
	while (list_index < TOTAL_COMMAND_NUM)
	{
		name = commands[list_index].name;
		list_index++;

		if (strncmp(name, text, len) == 0)
			return dupstr(name);
	}

	/* If no names matched, then return NULL. */
	return (char *)NULL;
}

/* send for exec, if a command has already finished */
int gconsole_bind_cr(int count, int key)
{
	if (gconsole_eoq == 1)
	{
		rl_done = 1;
		gconsole_eoq = 0; // clear eoq flag
		// printf("\n");
		rl_crlf();
	}
	else
	{
		rl_insert_text("\n"); // Insert text into the line at the current cursor position.
		rl_insert_text(CROSS_LINE_PROMPT);
		rl_redisplay(); // Change what's displayed on the screen to reflect the current contents of rl_line_buffer.
						// so this would show the inserted \n to console
	}
	return 0;
}

/* command ends with ; */
int gconsole_bind_eoq(int count, int key)
{
	gconsole_eoq = 1;
	printf(";");
	return 0;
}

/* **************************************************************** */
/*                                                                  */
/*                       gconsole commands                          */
/*                                                                  */
/* **************************************************************** */
//  invalid:return -1 valid:return 0
int check_argc_or(int argc, int std_argc_num, ...)
{
	/*
	cout << "[passing argc:]" << argc << endl;
	cout << "[std_argc_num:]" << std_argc_num << " [std_argc:]";
	{
		va_list valist;
		va_start(valist, std_argc_num);
		for (int i = 0; i < std_argc_num; i++)
		{
			cout << va_arg(valist, int) << " ";
		}
		cout << endl;
		va_end(valist);
	}
	*/
	va_list valist;
	// enables access to the variable arguments following the named argument std_argc_num.
	va_start(valist, std_argc_num);
	int is_valid = -1;
	for (int i = 0; i < std_argc_num; i++)
	{
		if (va_arg(valist, int) == argc)
		{
			is_valid = 0;
			break;
		}
	}
	va_end(valist);
	return is_valid;
}
#define CHECK_ARGC(std_argc_num, ...)                                    \
	if (check_argc_or(args.size(), std_argc_num, __VA_ARGS__))           \
	{                                                                    \
		PRINT_WRONG_USG                                                  \
		cout << "Usage: " << commands[current_cmd_offset].usage << endl; \
		return -1;                                                       \
	}

/*int print_arg_handler(const vector<string> &args)
{
	cout << commands[current_cmd_offset].name << endl;
	for (const auto &s : args)
	{
		cout << s << " ";
	}
	cout << endl;
	return 0;
}*/

// update db2priv for current usr
int flushpriv_handler(const vector<string> &args)
{
	CHECK_ARGC(1, 0)
	for (auto &p : db2priv)
	{
		unsigned priv = read_priv(usrname, p.first);
		if (priv == -1u)
		{
			cout << "Warn: update priv on " << p.first << ".db failed." << endl;
		}
		else
		{
#ifdef _GCONSOLE_TRACE
			if (p.second != priv)
			{
				cout << "\t[Update priv on " << p.first << "] before:";
				print_lowbits(p.second, 8);
				cout << "after:";
				print_lowbits(priv, 8);
				cout << endl;
			}
#endif //_GCONSOLE_TRACE
			p.second = priv;
		}
	}
#ifdef _GCONSOLE_TRACE
	cout << "[db2priv after flush priv:][db:priv]:";
	for (auto p : db2priv)
	{
		cout << p.first << ":";
		print_lowbits(p.second, 8);
		cout << endl;
	}
#endif //_GCONSOLE_TRACE

	cout << "Privilige Flushed for current user successfully." << endl;
	return 0;
}

// ofp is set to output, and output need to be closed outer
// query success:return 0; failed:return -1
int raw_sparql_handler(string query)
{
	CHECK_CURRENT_DB_LOADED
	if (check_priv(current_database->getName(), commands[current_cmd_offset].privilege_bitset))
	{
		return -1;
	}
	if (query.empty())
	{
		cout << "Empty SPARQL, skip." << endl;
		return 0;
	}

	ResultSet _rs;
	FILE *ofp = stdout;
	// FILE *ofp = output;
	bool export_flag = false;
	// if (ofp != stdout)
	// {
	// 	export_flag = true;
	// }
	long tv_begin = Util::get_cur_time();
	int ret = current_database->query(query, _rs, ofp, true, export_flag, nullptr);
	current_database->save();
	if ((ret <= -100 && ret != -100) || (ret > -100 && ret < 0)) // select query failed or update query failed
	{
		return -1;
	}

	long tv_end = Util::get_cur_time();
	cout << "query database successfully, Used " << (tv_end - tv_begin) << " ms" << endl;
	// TODO: pretty print final result
	return 0;
}

string stripwhite(const string &s)
{
#ifdef _GCONSOLE_TRACE
	cout << "[before stripwhite the line is:]\n"
		 << s << endl;
#endif //_GCONSOLE_TRACE
	int i = 0, sz = s.size();
	while (i < sz && whitespace(s[i]))
	{
		++i;
	}
	if (i >= sz)
	{
		return "";
	}
	// s[i] not whitespace
	int j = sz - 1;
	while (j >= 0 && whitespace(s[j]))
	{ // in fact, j can not less than 0
		--j;
	}
	// s[j] not whitespace

#ifdef _GCONSOLE_TRACE
	cout << "[after stripwhite:]\n"
		 << s.substr(i, j - i + 1) << endl;
#endif //_GCONSOLE_TRACE
	return s.substr(i, j - i + 1);
}

// rm # comment from line
string rm_comment(string line)
{
#ifdef _GCONSOLE_TRACE
	cout << "[before rm comment the line is:]\n"
		 << line << endl;
#endif //_GCONSOLE_TRACE
	line.push_back('#');
	// deal with #: look for #, the content after it and before the nearest \n is comments
	string sparql;
	int i = 0, sz = line.size();
	int seg_start_pos = 0;
	while (i < sz)
	{
		// met a new comment: [seg_start_pos,i) -> sparql
		// if seg_start_pos<0, it means we have met a # but haven't met a \n, so it's inside a comment
		if (line[i] == '#' && seg_start_pos >= 0)
		{
			sparql += line.substr(seg_start_pos, i - seg_start_pos);
			seg_start_pos = -1; // mark as meeting #
		}
		// have met a # before, now meet \n
		else if (line[i] == '\n' && seg_start_pos == -1)
		{
			seg_start_pos = i + 1;
			sparql.push_back('\n');
		}
		++i;
	}
#ifdef _GCONSOLE_TRACE
	cout << "[after rm comment:]\n"
		 << sparql << endl;
#endif //_GCONSOLE_TRACE
	return sparql;
}

// replace '\n' with ' '
void replace_cr(string &line)
{
	for (char &c : line)
	{
		if (c == '\n')
			c = ' ';
	}
}

int sparql_handler(const vector<string> &args)
{
	CHECK_ARGC(1, 1)
	CHECK_CURRENT_DB_LOADED
	if (check_priv(current_database->getName(), commands[current_cmd_offset].privilege_bitset))
	{
		return -1;
	}

	ifstream fin(args[0]);
	if (fin.is_open() == 0)
	{
		cout << "File open failed: " << args[0] << endl;
		return -1;
	}
	string line;
	while (getline(fin, line, ';'))
	{
		/*parse one sparql*/
		line.push_back('#');
		// deal with #: look for #, the content after it and before the nearest \n is comments
		string sparql;
		int i = 0, sz = line.size();
		int seg_start_pos = 0;
		while (i < sz)
		{
			// met a new comment: [seg_start_pos,i) -> sparql
			// if seg_start_pos<0, it means we have met a # but haven't met a \n, so it's inside a comment
			if (line[i] == '#' && seg_start_pos >= 0)
			{
				sparql += line.substr(seg_start_pos, i - seg_start_pos);
				seg_start_pos = -1; // mark as meeting #
			}
			// have met a # before, now meet \n
			else if (line[i] == '\n' && seg_start_pos == -1)
			{
				seg_start_pos = i + 1;
				sparql.push_back('\n');
			}
			++i;
		}

		/*query sparql*/
		sparql = stripwhite(sparql);

		if (sparql.empty() == 0 && raw_sparql_handler(sparql))
		{
			cout << "Query failed: " << sparql << endl;
		}
		cout << endl
			 << endl;
	}
	fin.close();
	return 0;
}

int help_handler(const vector<string> &args)
{
	CHECK_ARGC(2, 0, 1)
	// all info
	if (args.size() == 0)
	{
		cout << "Gstore Console(gconsole), an interactive shell based utility to communicate with gStore repositories." << endl;
		cout << "" << endl;
		cout << "For information about gStore products and services, visit:" << endl;
		cout << "   http://www.gstore.cn/" << endl;
		cout << "For developer information, including the gStore Reference Manual, visit:" << endl;
		cout << "   http://www.gstore.cn/pcsite/index.html#/documentation" << endl;
		cout << "" << endl;
		cout << "Commands end with ;. Cross line input is allowed." << endl;
		cout << "Comment start with #." << endl;
		cout << "CTRL+C to quit current command. CTRL+D to exit this console." << endl;
		cout << "List of all gconsole commands:" << endl;

		for (int i = 0; i < TOTAL_COMMAND_NUM; ++i)
		{
			cout << commands[i].name << "\t" << commands[i].doc << endl;
		}
		cout << endl;
		cout << "Other help arg:" << endl;
		cout << "edit\tDisplay line editing shortcut keys supported by gconsole." << endl;
		cout << "usage\tDisplay all commands as well as their usage." << endl
			 << endl;
		return 0;
	}
	string name = args[0];
	// help usage
	if (name == "usage")
	{
		cout << "List of all gconsole commands:" << endl;
		cout << "Note that all text commands must be end with ';' but need not be in one line." << endl;

		for (int i = 0; i < TOTAL_COMMAND_NUM; ++i)
		{
			cout << commands[i].name << "\t" << commands[i].doc << "\n\t\t" << commands[i].usage << endl;
		}
		return 0;
	}
	// help edit
	if (name == "edit")
	{
		cout << "Frequently used GNU Readline shortcuts:" << endl;
		cout << "CTRL-a"
			 << "\t"
			 << "move cursor to the beginning of line" << endl;
		cout << "CTRL-e"
			 << "\t"
			 << "move cursor to the end of line" << endl;
		cout << "CTRL-d"
			 << "\t"
			 << "delete a character" << endl;
		cout << "CTRL-f"
			 << "\t"
			 << "move cursor forward (right arrow)" << endl;
		cout << "CTRL-b"
			 << "\t"
			 << "move cursor backward (left arrow)" << endl;
		cout << "CTRL-p"
			 << "\t"
			 << "previous line, previous command in history (up arrow)" << endl;
		cout << "CTRL-n"
			 << "\t"
			 << "next line, next command in history (down arrow)" << endl;
		cout << "CTRL-k"
			 << "\t"
			 << "kill the line after the cursor, add to clipboard" << endl;
		cout << "CTRL-u"
			 << "\t"
			 << "kill the line before the cursor, add to clipboard" << endl;
		cout << "CTRL-y"
			 << "\t"
			 << "paste from the clipboard" << endl;
		cout << "ALT-b"
			 << "\t"
			 << "move cursor back one word" << endl;
		cout << "ALT-f"
			 << "\t"
			 << "move cursor forward one word" << endl;

		cout << "For more about GNU Readline shortcuts, see https://en.wikipedia.org/wiki/GNU_Readline#Emacs_keyboard_shortcuts" << endl;
		return 0;
	}
	// help <command>
	int idx;
	if ((idx = find_command(name.c_str())) >= 0)
	{
		cout << commands[idx].name << "\t" << commands[idx].doc << "\n\t\t" << commands[idx].usage << endl;
		return 0;
	}
	// not a cmd
	cout << name << ": Not a supported gconsole command. \nList of gconsole command:" << endl;
	for (int i = 0; i < TOTAL_COMMAND_NUM; ++i)
	{
		cout << commands[i].name << " ";
	}
	cout << endl;
	return 0;
}

int quit_handler(const vector<string> &args)
{
	CHECK_ARGC(1, 0)

	if (current_database != NULL)
	{
		// current_database->unload(); //NOTE: destructor of Database would call unload to release mem, if call unload explicitly would end up double free
		delete current_database;
	}

	gconsole_done = true;
	return 0;
}

int show_handler(const vector<string> &args)
{
	CHECK_ARGC(4, 0, 1, 2, 3)
	int argc = args.size();

	string db_name;
	string lines = "10";
	// show
	// show db
	if (argc == 1)
	{
		db_name = args[0];
	}
	// show db -n num
	else if (argc == 3)
	{
		if (args[0] == "-n")
		{
			lines = args[1];
			db_name = args[2];
		}
		else if (args[1] == "-n")
		{
			lines = args[2];
			db_name = args[0];
		}
		else
		{
			PRINT_WRONG_USG
			cout << commands[current_cmd_offset].usage << endl;
		}
	}
	// show -n num
	else if (argc == 2)
	{
		if (args[0] == "-n")
		{
			lines = args[1];
		}
		else
		{
			PRINT_WRONG_USG
			cout << commands[current_cmd_offset].usage << endl;
		}
	}

	// check priv (check priv for current_db is done later, because get current_db's name need it to be already loaded)
	if (db_name.empty() == 0 && check_priv(db_name, commands[current_cmd_offset].privilege_bitset))
	{
		return -1;
	}

	Database *db = current_database;
	if (db_name.empty() == 0)
	{
		db = new Database(db_name);
		cout << "load " << db_name << " db ..." << endl;
		db->load();
	}
	else
	{
		CHECK_CURRENT_DB_LOADED
		if (check_priv(current_database->getName(), commands[current_cmd_offset].privilege_bitset))
		{
			return -1;
		}
	}

	// do query here: because query would output MUCH process info to stdout
	ResultSet re;
	int ret = db->query("SELECT ?x ?y ?z WHERE{ ?x ?y ?z. } LIMIT " + lines, re); // limit query result to lines
	if (ret != -100)															  // select query failed
	{
		cout << "Show database " << db->getName() << " failed: Query this database failed." << endl;
		return -1;
	}

	cout << "\n\n===================================================" << endl;
	// meta info
	cout << "Name:\t" << db->getName() << endl;
	cout << "TripleNum:\t" << db->getTripleNum() << endl;
	cout << "EntityNum:\t" << db->getEntityNum() << endl;
	cout << "LiteralNum:\t" << db->getLiteralNum() << endl;
	cout << "SubNum:\t" << db->getSubNum() << endl;
	cout << "PreNum:\t" << db->getPreNum() << endl;
	cout << "===================================================" << endl;

	// head lines triple
	unsigned ed = min(re.ansNum, unsigned(stoi(lines)));
	for (unsigned i = 0; i < ed; ++i)
	{
		for (unsigned j = 0; j < 3; ++j)
		{
			cout << re.answer[i][j] << "\t";
		}
		cout << endl;
	}
	cout << endl;

	if (db_name.empty() == 0)
	{
		// db->unload(); //NOTE: destructor of Database would call unload to release mem, if call unload explicitly would end up double free
		delete db;
	}
	return 0;
}

int showdbs_handler(const vector<string> &args)
{
	CHECK_ARGC(1, 0)

	// only show those with query priv, all if root
	string sparql;
	if (usrname == root_username)
	{
		// display all db
		sparql = "SELECT ?dbname ?usr WHERE { ?dbname <built_by> ?usr. }; SELECT ?dbname ?stat WHERE { ?dbname <database_status> ?stat. };";
	}
	else
	{
		// display those with query priv
		string addstr = "<" + usrname + "> <has_query_priv> ?dbname. OPTIONAL{ ";
		sparql = "SELECT ?dbname ?usr WHERE { " + addstr + "?dbname <built_by> ?usr. } }; SELECT ?dbname ?stat WHERE { " + addstr + "?dbname <database_status> ?stat. } };";
	}
	vector<ResultSet> rsv(2);
	vector<int> retv = silence_sysdb_query(sparql, rsv);
	if (retv.size() != 2 || retv[0] || retv[1])
	{
		SYSDB_QUERY_FAILED(sparql)
	}
	unordered_map<string, string> db2stat;

	int sz = rsv[1].ansNum;
	string **ans = rsv[1].answer;
	for (int i = 0; i < sz; ++i)
	{
		db2stat[ans[i][0]] = ans[i][1];
	}

	cout << "\"database\"\t\"creater\"\tstatus\"" << endl;
	sz = rsv[0].ansNum;
	ans = rsv[0].answer;
	for (int i = 0; i < sz; ++i)
	{
		string dbname = ans[i][0];
		cout << dbname << "\t" << ans[i][1];
		if (db2stat.count(dbname))
			cout << "\t" << db2stat[dbname];
		cout << endl;
	}

	return 0;
}

int create_handler(const vector<string> &args)
{
	CHECK_ARGC(2, 1, 2)

	string db_name = args[0];
	if (db_name == "system")
	{
		cout << "Your db name can NOT be \"system\". Database create failed." << endl;
		return -1;
	}
	/* int access(const char *pathname, int mode);
	   On success (all requested permissions granted, or mode is F_OK
	   and the file exists), zero is returned.  On error (at least one
	   bit in mode asked for a permission that is denied, or mode is
	   F_OK and the file does not exist, or some other error occurred),
	   -1 is returned, and errno is set to indicate the error.*/
	if (access(string(db_name + ".db").c_str(), F_OK) == 0)
	{
		cout << "Database " << db_name << " has been created. Database create failed." << endl;
		return -1;
	}

	string nt_file;
	if (args.size() == 1)
	{
		// create tmp nt file
		string time = Util::get_date_time();
		nt_file = "bin/.tmp_nt.nt";
		ofstream fout(nt_file);
		fout << "<" << db_name << "><built_time>\"" << time << "\"." << endl;
		fout.close();
	}
	else
		nt_file = args[1];

	Database *tmp_database = new Database(db_name);
	int flag = tmp_database->build(nt_file);
	delete tmp_database;
	if (args.size() == 1)
	{
		// rm bin/.tmp_nt.nt
		system("rm -rf bin/.tmp_nt.nt");
	}

	if (flag == 0)
	{
		cout << "Database create failed. " << endl;
		system(string("rm -rf " + db_name + ".db").c_str());
		return -1;
	}

	// sysdb
	string time = Util::get_date_time();
	string record_newdb_sparql = "INSERT DATA {<" + db_name + "> <database_status> \"already_built\"." + "<" + db_name + "> <built_by> <" + usrname + ">." + "<" + db_name + "> <built_time> \"" + time + "\".}";
	{
		ResultSet rs;
		if (silence_sysdb_query(record_newdb_sparql, rs))
		{
			cout << "Newly created db record added failed! Database create failed.\nWarn: Please check contents of system.db." << endl;
			system(string("rm -rf " + db_name + ".db").c_str());
			return -1;
		}
	}

	if (usrname == root_username)
	{
		return 0;
	}
	// assign ALL priv to current usr on this db
	db2priv[db_name] = ALL_PRIVILEGE_BIT;
	string add_priv_sparql = "INSERT DATA{";
	for (auto p : privstr2bitset)
	{
		add_priv_sparql += ("<" + usrname + ">" + p.first + "<" + db_name + ">.");
	}
	add_priv_sparql += "}";

	{
		ResultSet rs;
		if (silence_sysdb_query(add_priv_sparql, rs))
		{
			cout << "Priv added failed! Database create failed.\nWarn: Please check contents of system.db." << endl;
			db2priv.erase(db_name);
			system(string("rm -rf " + db_name + ".db").c_str());
			return -1;
		}
	}

	cout << "Database " << db_name << "created successfully. " << endl;

	return 0;
}

int drop_handler(const vector<string> &args)
{
	CHECK_ARGC(1, 1)
	string db_name = args[0];
	if (db_name == "system")
	{
		cout << "You can NOT drop system database. " << endl;
		return -1;
	}
	if (current_database != NULL && current_database->getName() == db_name)
	{
		cout << "You can NOT drop current database. Please switch current database through \"USE <database_name>;\" before you drop it." << endl;
		return -1;
	}
	if (check_priv(db_name, commands[current_cmd_offset].privilege_bitset))
	{
		cout << "Database drop failed." << endl;
		return -1;
	}

	string sparql = "DELETE WHERE { <" + db_name + "> ?x ?y. }; DELETE WHERE { ?x ?y <" + db_name + ">. }";
	//! NOTE to Main/gdrop.cpp: DELETE WHERE { ?x ?y \"" + db_name + "\" is needed too! But gdrop.cpp doesn't contain it
	vector<ResultSet> rs(2);
	vector<int> ret = silence_sysdb_query(sparql, rs);
	if (ret[0] || ret[1])
	{
		cout << "Warn: Drop info about database " << db_name << " failed! Please check system db. \nNote that " << db_name << ".db and backlog are removed." << endl;
	}

	string cmd = "rm -r " + db_name + ".db";
	system(cmd.c_str());
	Util::delete_backuplog(db_name);

	cout << "Database " << db_name << " dropped successfully." << endl;
	return 0;
}

int export_handler(const vector<string> &args)
{
	CHECK_ARGC(1, 1)
	CHECK_CURRENT_DB_LOADED
	if (check_priv(current_database->getName(), commands[current_cmd_offset].privilege_bitset))
	{
		return -1;
	}

	string filepath = args[0];
	FILE *ofp = fopen(filepath.c_str(), "w");
	current_database->export_db(ofp);
	fflush(ofp);
	fclose(ofp);
	ofp = NULL;

	cout << "Database " << current_database->getName() << " exported successfully." << endl;
	return 0;
}

int backup_handler(const vector<string> &args)
{
	CHECK_ARGC(2, 0, 1)
	CHECK_CURRENT_DB_LOADED
	if (check_priv(current_database->getName(), commands[current_cmd_offset].privilege_bitset))
	{
		return -1;
	}

	string backup_path;
	if (args.empty() == 0)
		backup_path = args[0];
	string db_name = current_database->getName();

	string default_backup_path = "./backups";
	if (backup_path.empty())
	{
		backup_path = default_backup_path;
	}
	if (backup_path == "." || backup_path == "./")
	{
		cout << "<backup_path> is the same as current storage. Please choose another <backup_path>." << endl;
		cout << "Database " << current_database->getName() << " backup failed." << endl;
		return -1;
	}
	if (backup_path[0] == '/')
		backup_path = '.' + backup_path;
	if (backup_path[backup_path.length() - 1] == '/')
		backup_path = backup_path.substr(0, backup_path.length() - 1);
	Util::backup_path = backup_path + "/";

	// string db_path = current_database->getDBInfoFile();
	// {
	// 	size_t idx = db_path.find_last_of('/');
	// 	db_path = db_path.substr(0, idx);
	// }
	// int ret = copy(db_path, backup_path);
	// if (ret)

	if (current_database->backup() == 0)
	{
		cout << "Database " << current_database->getName() << " backup failed." << endl;
		return -1;
	}

	// rename backup folder with current timestamp
	string timestamp = Util::get_timestamp();
	backup_path = backup_path + "/" + db_name + ".db";
	string _backup_path = backup_path + "_" + timestamp;
	string sys_cmd = "mv " + backup_path + " " + _backup_path;
	system(sys_cmd.c_str());

	cout << "Database " << current_database->getName() << " backup successfully." << endl;
	return 0;
}

int restore_handler(const vector<string> &args)
{
	CHECK_ARGC(1, 2)
	string db_name = args[0];
	if (check_priv(db_name, commands[current_cmd_offset].privilege_bitset))
	{
		return -1;
	}

	bool is_current_db = 0;
	if (current_database && db_name == current_database->getName())
	{
		cout << "WARNNING: The database you restored just now is current database(" << db_name << "), will restore then reload it." << endl;
		delete current_database;
		// current_database->unload(); // destructor of Database would call unload()
		current_database = 0;
		is_current_db = 1;
	}
	string backup_path = args[1];

	// from grestore.cpp
	Util util;
	if (backup_path[0] == '/')
		backup_path = '.' + backup_path;
	if (backup_path[backup_path.length() - 1] == '/')
		backup_path = backup_path.substr(0, backup_path.length() - 1);

	if (!Util::dir_exist(backup_path))
	{
		cout << "Backup Path Error, Restore Failed" << endl;
		return 0;
	}

	string sparql = "ASK WHERE{<" + db_name + "> <database_status> \"already_built\".}";
	ResultSet ask_rs;
	silence_sysdb_query(sparql, ask_rs);
	if (ask_rs.answer[0][0] == "\"false\"^^<http://www.w3.org/2001/XMLSchema#boolean>")
	{
		cout << "The database does not exist. Rebuild" << endl;
		string time = Util::get_backup_time(backup_path, db_name);
		if (time.size() == 0)
		{
			cout << "Backup Path Does not Match DataBase Name, Restore Failed" << endl;
			return 0;
		}
		string sparql = "INSERT DATA {<" + db_name + "> <database_status> \"already_built\"." + "<" + db_name + "> <built_by> <root>." + "<" + db_name + "> <built_time> \"" + time + "\".}";
		ResultSet _rs;
		int ret = silence_sysdb_query(sparql, _rs);

		if (ret >= 0)
			cout << "update num : " + Util::int2string(ret) << endl;
		else
		{
			// update error
			cout << "Rebuild Error, Restore Failed" << endl;
			return 0;
		}
		Util::add_backuplog(db_name);
	}

	//!单独命令行执行以下命令可以实现恢复效果，而执行restore_handler则会出现db_info_file.dat稳定为备份后修改后的版本
	//!可能和文件复制的落盘有关系
	// cp -r backups/eg.db_220929114732 .
	// rm -rf eg.db
	// mv eg.db_220929114732 eg.db
	string sys_cmd = "cp -r " + backup_path + " .";
	cout << "[" << sys_cmd << "]" << std::endl;
	if (system(sys_cmd.c_str()))
	{
		cout << sys_cmd << " failed. Restore failed." << endl;
		return -1;
	}

	string db_path = db_name + ".db";
	sys_cmd = "rm -rf " + db_path;
	cout << "[" << sys_cmd << "]" << std::endl;
	if (system(sys_cmd.c_str()))
	{
		cout << sys_cmd << " failed. Restore failed." << endl;
		return -1;
	}

	string path = Util::get_folder_name(backup_path, db_name);
	// {
	// 	size_t idx = backup_path.find_last_of('/');
	// 	path = backup_path.substr(idx + 1);
	// }
	sys_cmd = "mv " + path + ' ' + db_path;
	// sys_cmd = "cp -r " + path + ' ' + db_path;
	cout << "[" << sys_cmd << "]" << std::endl;
	if (system(sys_cmd.c_str()))
	{
		cout << sys_cmd << " failed. Restore failed." << endl;
		return -1;
	}

	if (is_current_db)
	{
		cout << "WARNNING: The database you restored just now is current database(" << db_name << "), will restore then reload it.\nRestore is done, now reload it." << endl;
		current_database = new Database(db_name);
		if (current_database->load() == 0)
		{
			cout << "WARNNING: The database you restored just now is current database(" << db_name << "), and we tried to reload it but failed.\nWe suggest type `USE " << db_name << "` command to reload current database." << std::endl;
			cout << "Database(current database) " << db_name << " restored successfully, but reload failed." << endl;
			return -1;
		}
	}
	cout << "Database " << db_name << " restored successfully." << endl;
	return 0;
}

int use_handler(const vector<string> &args)
{
	CHECK_ARGC(1, 1)
	string new_db_name = args[0];
	if (check_priv(new_db_name, commands[current_cmd_offset].privilege_bitset))
	{
		return -1;
	}

	Database *pre_db = current_database;

	current_database = new Database(new_db_name);
	cout << "load " << new_db_name << " db ..." << endl;
	bool flag = current_database->load();
	if (!flag)
	{
		cout << "Database change failed: Fail to load new database " << new_db_name << ".\nReturn to previout current database. Current database unchanged." << endl;
		// current_database->unload(); //NOTE: destructor of Database would call unload to release mem, if call unload explicitly would end up double free
		delete current_database;
		if (pre_db != NULL)
		{
			current_database = pre_db;
		}
		else
		{
			current_database = NULL;
		}
		return -1;
	}
	if (pre_db != NULL)
	{
		// pre_db->unload(); //NOTE: destructor of Database would call unload to release mem, if call unload explicitly would end up double free
		delete pre_db;
	}

	cout << "Current database switch to " << new_db_name << " successfully." << endl;
	return 0;
}

int clear_handler(const vector<string> &args)
{
	if (system("clear"))
	{
		cout << "Clear failed." << endl;
		return -1;
	}
	return 0;
}

int pwd_handler(const vector<string> &args)
{
	system("pwd");
	return 0;
}

int settings_handler(const vector<string> &args)
{
	CHECK_ARGC(2, 0, 1);
	ifstream fin(INIT_CONF_FILE);

	// display all settings
	if (args.empty())
	{
		cout << "Settings: " << endl;
		string line;
		while (getline(fin, line))
		{
			size_t idx = line.find('=');
			if (idx != string::npos)
			{
				size_t comment_idx = line.find('#');
				if (comment_idx == string::npos)
				{
					line[idx] = '\t';
					cout << line << endl;
				}
				else if (idx < comment_idx)
				{
					cout << line.substr(0, idx) << "\t" << line.substr(idx + 1, comment_idx - idx - 1) << endl;
				}
			}
		}
	}
	// display args[0]
	else
	{
		string line, target = stripwhite(args[0]);
		bool found_target = 0;
		while (getline(fin, line))
		{
			size_t idx = line.find(target);
			if (idx != string::npos && (idx = line.find('=')) != string::npos)
			{
				if (stripwhite(line.substr(0, idx)) == target)
				{
					found_target = 1;
					size_t comment_idx = line.find('#');
					if (comment_idx == string::npos)
					{
						cout << line.substr(idx + 1) << endl;
					}
					else if (idx < comment_idx)
					{
						cout << line.substr(idx + 1, comment_idx - idx - 1) << endl;
					}
					break;
				}
			}
		}

		if (found_target == 0)
		{
			cout << target << " not found in settings." << endl;
		}
	}
	fin.close();

	cout << string("You can Edit configuration file to change settings: ") << INIT_CONF_FILE << endl;
	return 0;
}

int version_handler(const vector<string> &args)
{
	PRINT_VERSION
	return 0;
}

int pdb_handler(const vector<string> &args)
{
	CHECK_CURRENT_DB_LOADED
	cout << current_database->getName() << endl;
	return 0;
}

int setpswd_handler(const vector<string> &args)
{
	CHECK_ARGC(2, 0, 1)
	string prompt, tar_usr;
	// set args[0]'s pswd
	if (args.size() == 1)
	{
		if (usrname != root_username)
		{
			cout << "Permission denied. Only root is allowed to set other's pswd." << endl;
			if (usrname == args[0])
			{
				cout << "If you want to set your pswd, just enter 'setpswd;'." << endl;
			}
			return -1;
		}

		prompt = "Enter your password: ";
		tar_usr = args[0];
	}
	// set usrname pswd
	else
	{
		prompt = "Enter old password: ";
		tar_usr = usrname;
	}

	if (enter_pswd(prompt))
	{
		cout << "Fail to varify your id. Password set failed." << endl;
		return -1;
	}

	HideStdinDisplay hide_ins; // hide stdin input and recover when out of scope

	string new_pswd, confirm;
	int not_match_cnt = 0;
	do
	{
		if (not_match_cnt)
		{
			cout << "Not Matched." << endl;
		}
		++not_match_cnt;
		cout << "Enter new password: ";
		cin >> new_pswd;
		cout << endl;
		cout << "Enter new password again: ";
		cin >> confirm;
		cout << endl;
	} while (not_match_cnt < MAX_WRONG_PSWD_TIMES && confirm != new_pswd);

	if (not_match_cnt >= MAX_WRONG_PSWD_TIMES)
	{
		cout << "Too much not match. Password set failed." << endl;
		return -1;
	}

	// write new_pswd(for tar_usr) to sysdb: delete then insert
	string query = "DELETE WHERE { <" + tar_usr + "> <has_password> ?pswd. }; INSERT DATA { <" + tar_usr + "> <has_password> \"" + new_pswd + "\". }";
	vector<ResultSet> rs;
	vector<int> re = silence_sysdb_query(query, rs);
	if (re.size() != 2 || re[0] || re[1])
	{
		cout << "System db update failed. Password set failed." << endl;
		return -1;
	}

	if (tar_usr == usrname)
	{
		stdpswd = new_pswd;
	}
	if (tar_usr == root_username)
	{
		root_password = new_pswd;

		// write to config file
		string res;
		{
			ifstream fin(INIT_CONF_FILE);
			if (fin.is_open() == 0)
			{
				cout << string("File opened failed: ") << INIT_CONF_FILE << endl;
				return 0;
			}
			string line;
			while (getline(fin, line))
			{
				if (line.find("root_password") != string::npos)
					res += "root_password=\"" + new_pswd + "\"\n";
				else
					res += line + "\n";
			}
		}
		{
			ofstream fout(INIT_CONF_FILE);
			if (fout.is_open() == 0)
			{
				cout << string("File opened failed: ") << INIT_CONF_FILE << endl;
				return 0;
			}
			fout << res;
		}
	}
	cout << "Password set successfully." << endl;
	return 0;
}

int setpriv_handler(const vector<string> &args)
{
	CHECK_ARGC(1, 2)
	if (usrname != root_username)
	{
		cout << "Permission denied. Only root is allowed to set other's privilege." << endl;
		return -1;
	}
	string usr = args[0], db = args[1];
	if (usr == root_username)
	{
		cout << "Root has all privilege on all databases. No need to set root's privilege.\nPrivilege set failed." << endl;
		return -1;
	}
	if (access(string(db + ".db").c_str(), F_OK))
	{
		cout << "Database " << db << " does not exist. \nPrivilege set failed." << endl;
		return -1;
	}
	if (enter_pswd("Enter your password: "))
	{
		cout << "Fail to varify your id. Privilege set failed." << endl;
		return -1;
	}
	// i=1: skip root priv
	for (int i = 1; i < PRIVILEGE_NUM; ++i)
	{
		cout << "[" << i << "]" << priv_offset2name[i] << " ";
	}
	cout << "[" << PRIVILEGE_NUM << "]all\nEnter privilege number to assign separated by whitespace: " << endl;
	string line;
	// getchar(); // absorb from enter_pswd stage
	getline(cin, line);
#ifdef _GCONSOLE_TRACE
	cout << "[priv str from usr input:]" << line << endl;
#endif //_GCONSOLE_TRACE
	stringstream ss(line);
	unsigned num, priv = 0;
	while (ss >> num)
	{
		if (num > 0 && num < PRIVILEGE_NUM)
		{
			priv |= (1u << num);
		}
		else if (num == PRIVILEGE_NUM)
		{
			priv |= ALL_PRIVILEGE_BIT;
		}
	}

	cout << "[will set priv:]";
	print_lowbits(priv, 8);
	cout << endl;

	// write to sysdb

	// setpriv <usrname> <database_name>
	string query = "DELETE WHERE { <" + usr + "> ?y <" + db + ">. };\n INSERT DATA { ";
	priv = (priv >> 1); // shift root bit
	for (int i = 1; i < PRIVILEGE_NUM; ++i)
	{
		if (priv & 1)
		{
			query += ("<" + usr + "> <has_" + priv_offset2name[i] + "_priv> <" + db + ">.\n");
		}
		priv = (priv >> 1);
	}
	query += " }";

	vector<ResultSet> rs(2);
	vector<int> re = silence_sysdb_query(query, rs);
	if (re.size() != 2 || re[0] || re[1])
	{
		cout << "System db update failed. Privilege set failed." << endl;
		return -1;
	}
	cout << "Privilege set successfully." << endl;
	return 0;
}

// add or del succeed: return 0 ;failed: return -1
int adddelusr_handler(int add, string usr)
{
	if (usrname != root_username)
	{
		cout << "Permission denied. Only root is allowed to add user." << endl;
		return -1;
	}
	if (enter_pswd("Enter your password: "))
	{
		cout << "Fail to varify your id. User add failed." << endl;
		return -1;
	}

	// check whether usr exist
	ResultSet rs;
	if (silence_sysdb_query("SELECT ?y WHERE { <" + usr + "> <has_password> ?y. }", rs))
	{
		cout << "System db query failed(check usr exists or not). Add usr failed." << endl;
		return -1;
	}

	string query;
	if (add)
	{
		if (rs.ansNum)
		{
			cout << "User " << usr << " already exists." << endl;
			return -1;
		}
		cout << "Enter password for new user: ";
		string new_pswd;
		cin >> new_pswd;
		query = "INSERT DATA { <" + usr + "> <has_password> \"" + new_pswd + "\". }";
	}
	else
	{
		if (rs.ansNum == 0)
		{
			cout << "User " << usr << " doesn't exists." << endl;
			return -1;
		}
		query = "DELETE WHERE { <" + usr + "> <has_password> ?y. }";
	}

	if (silence_sysdb_query(query, rs))
	{
		cout << "System db update failed." << endl;
		return -1;
	}
	return 0;
}

int addusr_handler(const vector<string> &args)
{
	CHECK_ARGC(1, 1)
	string usr = args[0];
	if (adddelusr_handler(1, usr))
	{
		cout << "Add usr " << usr << " failed." << endl;
		return -1;
	}
	cout << "Add usr " << usr << " successfully." << endl;
	return 0;
}

int delusr_handler(const vector<string> &args)
{
	CHECK_ARGC(1, 1)
	string usr = args[0];
	if (adddelusr_handler(0, usr))
	{
		cout << "Del usr " << usr << " failed." << endl;
		return -1;
	}
	cout << "Del usr " << usr << " successfully." << endl;
	return 0;
}

int pusr_handler(const vector<string> &args)
{
	CHECK_ARGC(3, 0, 1, 2)
	string tar_usr;
	// pusr <database_name> <usrname>
	if (args.size() == 2)
	{
		if (usrname != root_username)
		{
			cout << "Permission denied. Only root is allowed to view other's privilege." << endl;
			if (usrname == args[1])
			{
				cout << "If you want to view your privilege, just enter 'pusr;' or 'pusr <database_name>;'." << endl;
			}
			return -1;
		}
		tar_usr = args[1];
	}
	// pusr; pusr <database_name>
	else
	{
		cout << "usrname: " << usrname << endl;
		if (args.size() == 0)
			return 0;
		tar_usr = usrname;
	}

	unsigned priv = get_priv(tar_usr, args[0]);
	if (priv == -1u)
	{
		return -1;
	}
	cout << "privilege on " << args[0] << ": ";
	for (int i = 0; i < PRIVILEGE_NUM; ++i)
	{
		if (priv & 1)
		{
			cout << priv_offset2name[i] << " ";
		}
		priv = (priv >> 1);
	}
	cout << endl;
	return 0;
}

int showusrs_handler(const vector<string> &args)
{
	CHECK_ARGC(1, 0)
	if (usrname != root_username)
	{
		cout << "Permission denied. Only root is allowed to view all usrs." << endl;
		return -1;
	}

	cout << root_username << "\n"
		 << string(root_username.size(), '-') << "\nall privilege on all db\n"
		 << endl;

	// print all usr
	ResultSet allusr_rs;
	if (silence_sysdb_query("SELECT ?usr WHERE {?usr <has_password> ?pswd.}", allusr_rs))
	{
		return -1;
	}

	// cout << "allusr_rs.ansNum: "<<allusr_rs.ansNum << endl;
	for (unsigned i = 0; i < allusr_rs.ansNum; ++i)
	{
		string tar_usr = allusr_rs.answer[i][0];
		tar_usr = tar_usr.substr(1, tar_usr.size() - 2); // strip <>

		if (tar_usr == root_username)
			continue;
		cout << tar_usr << "\n"
			 << string(tar_usr.size(), '-') << "\n";

		// print all db which tar_usr has some priv on
		ResultSet rs;
		if (silence_sysdb_query("SELECT DISTINCT ?db WHERE {<" + tar_usr + "> ?priv ?db. MINUS{<" + tar_usr + "> <has_password> ?db.}} ", rs))
		{
			cout << "<fetch databases from sysdb failed>\n"
				 << endl;
			continue;
		}

		// cout << "rs.ansNum: " << rs.ansNum << endl;
		for (unsigned j = 0; j < rs.ansNum; ++j)
		{
			string db_name = rs.answer[j][0];
			db_name = db_name.substr(1, db_name.size() - 2); // strip <>
			unsigned priv = get_priv(tar_usr, db_name);
			cout << db_name << ": ";
			if (priv == -1u)
			{
				cout << "<fetch privilege from sysdb failed>" << endl;
				continue;
			}
			for (int i = 0; i < PRIVILEGE_NUM; ++i)
			{
				if (priv & 1)
				{
					cout << priv_offset2name[i] << " ";
				}
				priv = (priv >> 1);
			}
			cout << endl;
		}
		cout << endl;
	}
	return 0;
}