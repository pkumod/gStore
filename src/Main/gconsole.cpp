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
#include <termios.h>
#include "../Util/Util.h"
#include "../Api/APIConnector.h"
#include <readline/readline.h>
#include <readline/history.h>

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
// #define _GCONSOLE_TRACE
// #define _GCONSOLE_DEBUG
// #define _GCONSOLE_SHOW_SYSDB_QUERY
#define BASE_URL "http://127.0.0.1:" + _server_port
#define API_URL BASE_URL + "/api"

#define INIT_CONF_FILE "./conf/conf.ini"
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

#define TOTAL_COMMAND_NUM 43
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
int unload_handler(const vector<string>&);
int batchinsert_handler(const vector<string>&);
int batchremove_handler(const vector<string>&);
server::MessageReasonManageResponse reason_manage_handler(int, const string&);
int addreason_handler(const vector<string>&);
int listreason_handler(const vector<string>&);
int compilereason_handler(const vector<string>&);
int executereason_handler(const vector<string>&);
int disablereason_handler(const vector<string>&);
int showreason_handler(const vector<string>&);
int deletereason_handler(const vector<string>&);

int funquery_handler(const vector<string>&);
int funcudb_handler(int, const string&);
int funcreate_handler(const vector<string>&);
int funupdate_handler(const vector<string>&);
int fundelete_handler(const vector<string>&);
int funbuild_handler(const vector<string>&);
int funreview_handler(const vector<string>&);


int flushpriv_handler(const vector<string> &);
int pusr_handler(const vector<string> &);
int pdb_handler(const vector<string> &);
int showdbs_handler(const vector<string> &);

int setpswd_handler(const vector<string> &);
int setpriv_handler(const vector<string> &);
int clearpriv_handler(const vector<string> &);
int addusr_handler(const vector<string> &);
int delusr_handler(const vector<string> &);
int showusrs_handler(const vector<string> &);

int refreshconf_handler(const vector<string> &);
int init_handler(const vector<string> &);

int txnlog_handler(const vector<string> &);
int querylogdate_handler(const vector<string> &);
int querylog_handler(const vector<string> &);
int accesslogdate_handler(const vector<string> &);
int accesslog_handler(const vector<string> &);

int begin_handler(const vector<string>&);
int tquery_handler(const vector<string>&);
int commit_handler(const vector<string>&);
int rollback_handler(const vector<string>&);
int checkpoint_handler(const vector<string>&);

// int importlicense_handler(const vector<string>&);
// int licenseinfo_handler(const vector<string>&);
// int removelicense_handler(const vector<string>&);

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
		{"quit", quit_handler, "Quit this console.", "quit;", 0},
		{"refreshconf", refreshconf_handler, "Refresh the configuration params.", "refreshconf;", 0},
		// database op
		{"init", init_handler, "Initializes the existing database to the system library.", "init <database_name>[,anothers];", 0},
		{"sparql", sparql_handler, "Answer SPARQL query(s) in file.", "sparql <sparql_file_path>;", QUERY_PRIVILEGE_BIT}, // file query
		{"create", create_handler, "Build a database from a dataset or create an empty database.", "create <database_name> [<nt_file_path>];", 0},
		{"use", use_handler, "Set current database.", "use <database_name>;", LOAD_PRIVILEGE_BIT | UNLOAD_PRIVILEGE_BIT},
		{"drop", drop_handler, "Drop a database.", "drop <database_name>;", ALL_PRIVILEGE_BIT},
		{"show", show_handler, "Show info and specified number of triples of current database or other database.", "show [<database_name>];", QUERY_PRIVILEGE_BIT},
		{"showdbs", showdbs_handler, "Display all databases the current user has query privilege on.", "showdbs;", 0},
		{"backup", backup_handler, "Backup current database.", "backup [<backup_path>];", BACKUP_PRIVILEGE_BIT},
		{"restore", restore_handler, "Restore a database.", "restore <database_name> <backup_path>;", RESTORE_PRIVILEGE_BIT},
		{"export", export_handler, "Export a database to .nt file.", "export [<file_path>];", EXPORT_PRIVILEGE_BIT},
		{"pdb", pdb_handler, "Display current database name.", "pdb;", 0},
        {"unload", unload_handler, "Unload the current database.","unload;", UNLOAD_PRIVILEGE_BIT},
        {"batchinsert", batchinsert_handler, "Batch inserts data into the current database.","batchinsert <nt_file_path>;", UPDATE_PRIVILEGE_BIT},
        {"batchremove", batchremove_handler, "Batch deletes the current database data.","batchremove <nt_file_path>;", UPDATE_PRIVILEGE_BIT},
		{"addreason", addreason_handler, "add reason rule into current database.", "addreason <json_file_path>;", 0},
		{"listreason", listreason_handler, "show all reason rules of current database.", "listreason;", 0},
		{"compilereason", compilereason_handler, "compile reason rule of current database with rulename.", "compilereason <rule_name>;", 0},
		{"executereason", executereason_handler, "execute reason rule of current database with rulename.", "executereason <rule_name>;", 0},
		{"disablereason", disablereason_handler, "disable reason rule of current database with rulename.", "disablereason <rule_name>;", 0},
		{"showreason", showreason_handler, "show detailed information of some rule with rulename in current database.", "showreason <rule_name>;", 0},
		{"deletereason", deletereason_handler, "delete reason rule of current database with rulename.", "deletereason <rule_name>;", 0},
		// id and usr manage
		{"flushpriv", flushpriv_handler, "Flush priv for current user, updating the in-memory structure.", "flushpriv;", 0},
		{"pusr", pusr_handler, "Display user's username and privilege.", "pusr; pusr <database_name>; pusr <database_name> <usr_name>;", 0},
		{"setpswd", setpswd_handler, "Set your password. Be able to set other's password if you are root.", "setpswd; setpswd <usrname>;", ROOT_PRIVILEGE_BIT},
		{"setpriv", setpriv_handler, "Set user's privilege.", "setpriv <usrname> <database_name>;", ROOT_PRIVILEGE_BIT},
		{"clearpriv", clearpriv_handler, "clear users's privilege.", "clearpriv <usrname>", ROOT_PRIVILEGE_BIT},
		{"addusr", addusr_handler, "Add user.", "addusr <usrname>;", ROOT_PRIVILEGE_BIT},
		{"delusr", delusr_handler, "Del user.", "delusr <usrname>;", ROOT_PRIVILEGE_BIT},
		{"showusrs", showusrs_handler, "Show all users and privilege for each.", "showusrs;", ROOT_PRIVILEGE_BIT},


		// custom function
		{"showpfn", funquery_handler, "query custom function.", "showpfn", 0},
		{"addpfn", funcreate_handler, "create custom function.", "addpfn <json_file_path>", 0},
		{"mdfpfn", funupdate_handler, "update custom function.", "mdfpfn <json_file_path>", 0},
		{"delpfn", fundelete_handler, "delete custom function.", "delpfn <func_name>", 0},
		{"buildpfn", funbuild_handler, "build custom function.", "buildpfn <func_name>", 0},
		// {"funreview", funreview_handler, "review custom function", "funreview <json_file_path>", 0},

		// license
		// {"importlicense", importlicense_handler, "import your license", "importlicense <license_file_path>", 0},
		// {"licenseinfo", licenseinfo_handler, "show your license information", "licenseinfo", 0},
		// {"removelicense", removelicense_handler, "remove your current license", "removelicense", 0},
		

		// other
		// {"cancel", 0, "Quit current input command.", "enter \"cancel;\" whenever you need to quit current input, remember the ;", 0}, // execute_line, check whether the line ends with cancel
		{"help", help_handler, "Display help msg. Enter 'help;' see more about usage.", "help [edit/usage/<command>];", 0},
		{"?", help_handler, "Synonym for \"help\".", "help [edit/usage/<command>];", 0},
		{"usage", help_handler, "Display all the commands and their usage", 0},
		
		// {"settings", settings_handler, "Display settings.", "settings [<conf_name>];", 0},
		{"version", version_handler, "Display  core version.", "version;", 0},

		// linux shell cmd
		{"pwd", pwd_handler, "Print name of current/working directory.", "pwd;", 0},
		{"clear", clear_handler, "Clear screen.", "clear;", 0},

		// raw_sparql
		{"raw_sparql", 0, "Support enter sparql query directedly in gconsole.",
		 "Begin with SELECT, INSERT, DELETE, PREFIX or BASE. For more about SPARQL, see https://www.w3.org/TR/sparql11-query/ ", QUERY_PRIVILEGE_BIT},

		// log
		// {"txnlog", txnlog_handler, "show database transaction log", "txnlog <pageNo> <pageSize>", 0},
		// {"querylogdate", querylogdate_handler, "get all query log dates for querylog operation", "querylogdate", 0},
		// {"querylog", querylog_handler, "show database query log", "querylog <date> <pageNo> <pageSize>", 0},
		// {"accesslogdate", accesslogdate_handler, "get all access log dates for accesslog operation", "accesslogdate", 0},
		// {"accesslog", accesslog_handler, "show database access log", "accesslog <date> <pageNo> <pageSize>", 0},

		// database transaction
		// {"begin", begin_handler, "begin transaction", "begin <db_name> <iso_level>", 0},
		// {"tquery", tquery_handler, "transactional query", "tquery <db_name> <tid> <sparql>", 0},
		// {"commit", commit_handler, "commit transaction", "commit <db_name> <tid>", 0},
		// {"rollback", rollback_handler, "transaction rollback", "rollback <db_name> <tid>", 0},
		// {"checkpoint", checkpoint_handler, "confirm the data modify", "checkpoint <db_name>", 0}


};

/* **************************************************************** */
/*                                                                  */
/*                    macro and declaration                         */
/*                                                                  */
/* **************************************************************** */
#define CROSS_LINE_PROMPT "     -> "
#define PROMPT_LEN 8
#define PRINT_ENTER_HELP_MSG                                                                                                 \
	cout << product_name << " Ver " << product_version << " for Linux on x86_64 (Source distribution)" << endl;                         \
	cout << product_name << " Console(gconsole), an interactive shell based utility to communicate with " << product_name_lower << " repositories." << endl; \
	cout << "Copyright (c) 2016-"<< current_year << ", pkumod and topgraph and/or its affiliates." << endl;                                               \
	cout << "" << endl;                                                                                                      \
	cout << "Usage: bin/gconsole [OPTIONS]" << endl;                                                                         \
	cout << "  -?, --help          Display this help and exit." << endl;                                                     \
	cout << "  -u, --user          username. " << endl;                                                                      \
	cout << "  " << endl;                                                                                                    \
	cout << "Supported command in gconsole: Type \"?\" or \"help\" in the console to see info of all commands." << endl;     \
	cout << "  " << endl;                                                                                                    \
	// cout << "For bug reports and suggestions, see https://github.com/pkumod/gStore" << endl                                  \
		//  << endl;

#define PRINT_WRONG_USG    \
	cout << "Wrong usage." \
		 << endl;
#define SYSDB_QUERY_FAILED(sparql)                                      \
	cout << "System db query failed. The query is: " << sparql << endl; \
	return -1;
#define PRINT_VERSION                                                               \
	cout << product_name<<" version: " << product_version << " Source distribution" << endl; \
	cout << "Copyright (c) 2016-" << current_year << ", pkumod and topgraph and/or its affiliates." << endl;
#define CHECK_CURRENT_DB_LOADED                                                                                    \
	if (_current_database.empty())                                                                                     \
	{                                                                                                              \
		cout << "Current database not selected. Please select it first, through \"USE <database_name>\"." << endl; \
		return -1;                                                                                                 \
	}
#define CHECK_CURRENT_DB_NOT_SYSDB                                                                                      \
	if (_current_database == GlobalTypedef::system_db)                                                                                 \
	{                                                                                                              \
		cout << "You can NOT do this for system database." << endl;                          \
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
// int silence_sysdb_query(const string &query, ResultSet &_rs);
// vector<int> silence_sysdb_query(const string &query, vector<ResultSet> &_rs);
bool login(const string& usrname, const string& password);
unsigned read_priv(string usr, string db_name);
unsigned get_priv(string usr, string db_name);
bool check_license();
bool pure_digit(const string& s);

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
unsigned current_privilege_bitset = 0;	 // when no current_database: 0, i.e. no priv
int current_cmd_offset = -1;			 // current_cmd offset in commands
unordered_map<string, unsigned> db2priv; // for current usr, cache in memory, avoiding fetch from sysdb everytime

string _db_home, _db_suffix, default_backup_path;
string product_name, product_name_lower, product_version;
string current_year;
string root_username, root_password;
string _server_port;
string _current_database;
string _website;
// global
int main(int argc, char **argv)
{
	Util util;
	//  read conf from conf.ini: version, root_name, root_pswd
	_server_port = util.getConfigureValue("port");
	_db_home = GlobalTypedef::db_home();
	_db_suffix = GlobalTypedef::db_suffix();
	_website = GlobalTypedef::product_website;
	
	default_backup_path = GlobalTypedef::backup_path();
	root_username = GlobalTypedef::root_uname();
	product_version = GlobalTypedef::product_version;
	product_name = GlobalTypedef::product_name;
	product_name_lower = product_name;
	product_name_lower[0] = tolower(product_name_lower[0]);
	current_year = TimeUtil::now("%Y");
	if (argc == 2)
	{
		if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
		{
			PRINT_ENTER_HELP_MSG
			return 0;
		}
		if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)
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
	enter_pswd("Enter password: ");
	int wrong_usr_cnt = 0;
	while (wrong_usr_cnt < MAX_WRONG_PSWD_TIMES && !login(usrname, stdpswd))
	{
		cout << "Username or password wrong! Please try again." << endl;
		++wrong_usr_cnt;
		cout << "Enter user name: ";
		cin >> usrname;
		getchar(); // absorb the '\n'
		enter_pswd("Enter password: ");
	}
	if (wrong_usr_cnt >= MAX_WRONG_PSWD_TIMES)
	{
		cout << "Please check your remember list for your usrname and password." << endl;
		return 0;
	}
	if (usrname == root_username)
		root_password = stdpswd;
	/* welcome and work */
	cout << endl;
	cout << product_name<<" Console , an interactive shell based utility to communicate with "<< product_name_lower <<" repositories." << endl;
	PRINT_VERSION
	// bool isvalid = check_license();
	cout << "" << endl;
	cout << "Welcome to the "<<product_name<<" Console." << endl;
	cout << "Commands end with ;. Cross line input is allowed." << endl;
	cout << "Comment start with #. Redirect (> and >>) is supported." << endl;
	cout << "CTRL+C to quit current command. CTRL+D to exit this console." << endl;
	cout << "Type 'help;' for help. " << endl
		 << endl;
	// if (!isvalid) cout << "-----Warning: License Expired, please update your license in time------" << endl;
	
	// signal handler for ctrl+c
	signal(SIGINT, ctrlc_handler);

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
		if (pfd == -1)
		{
			cout << "Failed to open file: " << file << endl;
			return -1;
		}
		// now pfd is descriptor to file
		int saved = dup(1); // now 1 and saved both are descriptor to ori_file(1 previously refer to)
		// int dup2(int oldfd, int newfd);
		if (saved == -1)
		{
			cout << "Failed to save file descprition" << endl;
			close(pfd);
			return -1;
		}
		if (dup2(pfd, 1) == -1)
		{
			cout << "Failed to redirect stdout" << endl;
			close(pfd);
			close(saved);
			return -1;
		} // would close descriptor 1 first, then now 1 and pfd both are descriptor to file(pfd previously refer to)
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
			if (dup2(saved, 1) == -1)
			{
				cout << "Failed to flush redirected stdout" << endl;
			}; // would close descriptor 1 first, then now 1 and saved both are descriptor to "saved previously refer to"
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
	stdpswd.clear();
	cout << prompt;
	cout.flush();
	char c;
	while ((c = getchar()) != -1 && c != '\n' && c != '\r')
	{
		stdpswd.push_back(c);
	}
	if (feof(stdin))
	{
		cout << "End of stdin!" << endl;
		return -1;
	}
	cout << endl;
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
	{
		string msg = "";
		if (_current_database.empty())
		{
			msg = product_name + "[no database]> ";
		}
		else
		{
			msg = product_name + "[" + _current_database + "]> ";
		}
		// cout << "msg:" << msg << endl;
		volatile ReadlineWrapper rl(in_readline, line, msg.c_str());
	}
	if (line == NULL) // EOF or Ctrl-D
	{
		if (!_current_database.empty())
		{
			_current_database = "";
		}
		cout << endl
			 << endl;
		gconsole_done = 1;
		return;
	}
	// a copy of the null-terminated character string pointed to by s. The length of the string is determined by the first null character.
	string strline(line);
	free(line);
	// strline = rm_comment(move(strline));
	strline = stripwhite(move(strline));
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
	int i = 0;
	char *word = NULL;

	// whether redirected: > or >>
	RedirectStdout redirect;
	bool is_redirected = false;
	int j = strlen(line) - 1;
	while (j > -1)
	{
		// search for > from tail of line before }: sparql triple may contain >, which would only be in {} or before {}
		if (line[j] == '"' || line[j] == '}' || line[j] == ')')
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
			line[i - 1] = '\0';
			redirect.redirect(line + j, 1); // redirect stdout
		}
		// >
		else
		{
			line[i] = '\0';
			redirect.redirect(line + j); // redirect stdout
		}
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

	// cmd raw query
	if ((current_cmd_offset = find_command(word)) == RAW_QUERY_CMD_OFFSET)
	{
		line[i] = recover_ch; // recover line: line is total sparql
		raw_sparql_handler(line);
	}
	// other command
	else
	{
		if (current_cmd_offset < 0)
		{
			cout << word << ": No such command for gconsole." << endl
				 << endl;
			return 0;
		}

		if (recover_ch) // line[i] is not '\0', there are args
		{	
			// note that if line[i] is '\0', the following logic is undefined!
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
		word[i] = tmp;
		while (word[i] && whitespace(word[i]))
		{
			i++;
		}
		word += i;
	}
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
	if (!FileUtil::dirExists("bin/.gconsole_history"))
	{
		if (!FileUtil::createDirs("bin/.gconsole_history")) 
		{
			cout << "Failed to create history directory" << endl;
			return -1;
		}
	}
	ofstream fout("bin/.gconsole_history/" + usrname);

	if (fout.is_open() == 0)
	{
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

bool login(const string& usrname, const string& password)
{
	server::MessageLoginRequest login_request(usrname, password);
	server::MessageResponse login_response = APIConnector::login(API_URL, login_request);
	if(login_response.StatusCode == WFT_STATE_SYS_ERROR)
	{
		cout << "Could not connect to server. Please check server status" << endl;
		exit(0);
	}
	if (!login_response.success())
	{
		cout << "login failed: " << login_response.StatusMsg << endl;
		return 0;
	}
	return 1;
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
	// TODO
	server::MessageShowUserRequest showuser_request;
	server::MessageShowUserResponse showuser_response = APIConnector::showUser(API_URL, true, showuser_request);	
	int priv = 0;
	for (const auto &m : showuser_response.ResponseBody)
	{
		if (usr != m.username) continue;
		priv |= QUERY_PRIVILEGE_BIT & (m.query_privilege.find(db_name) != string::npos) |
				LOAD_PRIVILEGE_BIT & (m.load_privilege.find(db_name) != string::npos) |
				UNLOAD_PRIVILEGE_BIT & (m.load_privilege.find(db_name) != string::npos) |
				UPDATE_PRIVILEGE_BIT & (m.update_privilege.find(db_name) != string::npos) |
				BACKUP_PRIVILEGE_BIT & (m.backup_privilege.find(db_name) != string::npos) |
				RESTORE_PRIVILEGE_BIT & (m.restore_privilege.find(db_name) != string::npos) |
				EXPORT_PRIVILEGE_BIT & (m.export_privilege.find(db_name) != string::npos);
		break;
	}
	return priv;
}

// return priv bitset of usr on db_name
// check db_name exist or not
unsigned get_priv(string usr, string db_name)
{
	// TODO
	if (access(string(_db_home + db_name + _db_suffix).c_str(), F_OK))
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
	// TODO
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

bool check_license()
{

	server::MessageRequest request;
	server::MessageLicenseResponse response = APIConnector::licenseInfo(BASE_URL, true, request);

	if (response.isvalid)
	{
		cout << "Licensed to " + response.company << endl
			 << "Active Until " << response.enddate << endl;
		return true;
	}
	return false;
}

bool pure_digit(const string& s) 
{
	for (const auto &c : s)
	{
		int ASCII = (int) c;
		if (ASCII < 48 || ASCII > 57) return false;
	}
	return true;
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

// ctrl+c signal handler: quit current cmd inputting or executing
void ctrlc_handler(int signo)
{
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
		rl_redisplay(); // Change what's displayed on the screen to reflect the current contents of rl_line_buffer.
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


// update db2priv for current usr
int flushpriv_handler(const vector<string> &args)
{
	//TODO
	CHECK_ARGC(1, 0)
	for (auto &p : db2priv)
	{
		unsigned priv = read_priv(usrname, p.first);
		if (priv == -1u)
		{
			cout << "Warn: update priv on " << p.first << _db_suffix << " failed." << endl;
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
int raw_sparql_handler(string sparql)
{
	CHECK_CURRENT_DB_LOADED
	string sparql_head;
	for (int i = 0; i < 6; ++i) {
		sparql_head += tolower(sparql[i]);
	}
	check_priv(_current_database, sparql_head == "select" ? QUERY_PRIVILEGE_BIT : UPDATE_PRIVILEGE_BIT);
	string query_url;
	if (_current_database == GlobalTypedef::system_db)
	{
		query_url = BASE_URL + "/sys/query";
	}
	else
	{
		query_url = API_URL;
	}
	server::MessageQueryRequest query_request(_current_database, sparql, "n-triple");
	query_request.username = root_username;
	query_request.password = root_password;
	server::MessageQueryResponse query_response = APIConnector::query(query_url, true, query_request);
	if (!query_response.success())
	{
		cout << "Query failed: " << query_response.StatusMsg << endl;
		return 1;
	}
	else if (!query_response.head.empty())
	{
		if (query_response.results.size() == 1 && query_response.results[0].size() == 1)
		{
		    string str = query_response.results[0][0];
		    size_t str_size = str.size();
		    // start with "{ and end with }"
		    if (str.find("\"{") == 0 && str.find_last_of("}\"") == str_size-1 )
		    {
		        str = str.substr(1, str_size - 2);
		        str = StringUtil::replace_all(str, "\\\"", "\"");
		        if (nlohmann::json::accept(str))
		        {
		            try
		            {
		                nlohmann::json doc = nlohmann::json::parse(str);
		                std::cout << doc.dump(4) << std::endl;
						cout << "Query ans num " << query_response.ansNum << ", use " << query_response.queryTime << " ms." << endl;
						return 0;
		            }
		            catch(const nlohmann::json::parse_error &e) 
					{
						SLOG_ERROR("JSON parse error: " << e.what());
					}
		        }
		    }
			else if (str.size() >= 8 && str.substr(0, 8) == "pfn_type")
			{
				std::cout << str.substr(8, -1) << std::endl;
				cout << "pfn query use " << query_response.queryTime << " ms." << endl;
				return 0;
			}
		}
		Util::printConsole(query_response.head, query_response.results);
		cout << "Query ans num " << query_response.ansNum << ", use " << query_response.queryTime << " ms." << endl;
	} 
	else
	{
		cout << "Update ans num " << query_response.ansNum << ", use " << query_response.queryTime << " ms." << endl;
	}
	return 0;
}

string stripwhite(const string &s)
{
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
	return s.substr(i, j - i + 1);
}

// rm # comment from line
string rm_comment(string line)
{
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
	ifstream fin(args[0]);
	if (fin.is_open() == 0)
	{
		cout << "File open failed: " << args[0] << endl;
		return -1;
	}
	// string line;
	// while (getline(fin, line, ';'))
	// {
	// 	/*parse one sparql*/
	// 	line.push_back('#');
	// 	// deal with #: look for #, the content after it and before the nearest \n is comments
	// 	string sparql;
	// 	int i = 0, sz = line.size();
	// 	int seg_start_pos = 0;
	// 	while (i < sz)
	// 	{
	// 		// met a new comment: [seg_start_pos,i) -> sparql
	// 		// if seg_start_pos<0, it means we have met a # but haven't met a \n, so it's inside a comment
	// 		if (line[i] == '#' && seg_start_pos >= 0)
	// 		{
	// 			sparql += line.substr(seg_start_pos, i - seg_start_pos);
	// 			seg_start_pos = -1; // mark as meeting #
	// 		}
	// 		// have met a # before, now meet \n
	// 		else if (line[i] == '\n' && seg_start_pos == -1)
	// 		{
	// 			seg_start_pos = i + 1;
	// 			sparql.push_back('\n');
	// 		}
	// 		++i;
	// 	}

	// 	/*query sparql*/
	// 	sparql = stripwhite(sparql);

	// 	if (sparql.empty() == 0 && raw_sparql_handler(sparql))
	// 	{
	// 		cout << "Query failed: " << sparql << endl;
	// 	}
	// 	cout << endl
	// 		 << endl;
	// }
	std::string sparql((std::istreambuf_iterator<char>(fin)),  
                 std::istreambuf_iterator<char>()); 
	fin.close();

	raw_sparql_handler(sparql);
	std::cout << std::endl;
	return 0;
}

int help_handler(const vector<string> &args)
{
	CHECK_ARGC(2, 0, 1)
	// all info
	if (args.size() == 0)
	{
		cout << product_name << " Console , an interactive shell based utility to communicate with " << product_name_lower << " repositories." << endl;
		cout << "" << endl;
		cout << "For information about " << product_name_lower << " products and services, visit:" << endl;
		cout << "   "<< _website << endl;
		cout << "For developer information, including the " << product_name_lower << " Reference Manual, visit:" << endl;
		cout << "   "<< _website + "/pcsite/index.html#/documentation" << endl;
		cout << "" << endl;
		cout << "Commands end with ;. Cross line input is allowed." << endl;
		cout << "Comment start with #." << endl;
		cout << "CTRL+C to quit current command. CTRL+D to exit this console." << endl;
		cout << "List of all console commands:" << endl;
		vector<string> headers = {"name", "description"};
		vector<vector<string>> rows;
		for (int i = 0; i < TOTAL_COMMAND_NUM; ++i)
		{
			rows.push_back({commands[i].name, commands[i].doc});
		}
		rows.push_back({"Other help arg:", ""});
		rows.push_back({"edit", "Display line editing shortcut keys supported by console."});
		rows.push_back({"usage", "Display all commands as well as their usage."});
		Util::printConsole(headers, rows);
		return 0;
	}
	string name = args[0];
	// help usage
	if (name == "usage")
	{
		cout << "List of all gconsole commands:" << endl;
		cout << "Note that all text commands must be end with ';' but need not be in one line." << endl;
		vector<string> headers = {"name", "description", "usage"};
		vector<vector<string>> rows;
		for (int i = 0; i < TOTAL_COMMAND_NUM; ++i)
		{
			rows.push_back({commands[i].name, commands[i].doc, commands[i].usage});
		}
		Util::printConsole(headers, rows);
		return 0;
	}
	// help edit
	if (name == "edit")
	{
		cout << "Frequently used GNU Readline shortcuts:" << endl;
		vector<string> headers = {"name", "description"};
		vector<vector<string>> rows;
		rows.push_back({"CTRL-a", "move cursor to the beginning of line"});
		rows.push_back({"CTRL-e", "move cursor to the end of line"});
		rows.push_back({"CTRL-d", "delete a character"});
		rows.push_back({"CTRL-f", "move cursor forward (right arrow)"});
		rows.push_back({"CTRL-b", "move cursor backward (left arrow)"});
		rows.push_back({"CTRL-p", "previous line, previous command in history (up arrow)"});
		rows.push_back({"CTRL-n", "next line, next command in history (down arrow)"});
		rows.push_back({"CTRL-k", "kill the line after the cursor, add to clipboard"});
		rows.push_back({"CTRL-u", "kill the line before the cursor, add to clipboard"});
		rows.push_back({"CTRL-y", "paste from the clipboard"});
		rows.push_back({"ALT-b", "move cursor back one word"});
		rows.push_back({"ALT-f", "move cursor forward one word"});
		Util::printConsole(headers, rows);
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
	cout << name << ": Not a supported console command. \nList of gconsole command:" << endl;
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

	if (!_current_database.empty())
	{
		_current_database = "";
	}
	gconsole_done = true;
	return 0;
}

int show_handler(const vector<string> &args)
{
	CHECK_ARGC(2, 0, 1)
	int argc = args.size();
	string db_name = _current_database;
	if (argc == 1)
	{
		db_name = args[0];
	}
	else
	{
		CHECK_CURRENT_DB_LOADED
	}

	// monitor
	server::MessageMonitorRequest monitor_request(db_name);
	server::MessageMonitorResponse monitor_response = APIConnector::monitor(API_URL, true, monitor_request);
	if (!monitor_response.success())
	{
		cout << "Failed to monitor database: " << monitor_response.StatusMsg << endl;
		return -1;
	}
	
	vector<string> header = {"name", "value"};
	vector<vector<string>> rows;
	rows.push_back({"database", monitor_response.database});
	rows.push_back({"creator", monitor_response.creator});
	rows.push_back({"builtTime", monitor_response.builtTime});  
	rows.push_back({"triple_num", monitor_response.tripleNum});
	rows.push_back({"literalNum", to_string(monitor_response.literalNum)});
	rows.push_back({"subjectNum", to_string(monitor_response.subjectNum)});
	rows.push_back({"predicateNum", to_string(monitor_response.predicateNum)});
	Util::printConsole(header, rows);
	return 0;
}

int showdbs_handler(const vector<string> &args)
{
	CHECK_ARGC(1, 0)
	server::MessageShowRequest show_request;
	server::MessageShowResponse show_response = APIConnector::show(API_URL, true, show_request);
	if (!show_response.success())
	{
		cout << "show databases failed: " << show_response.StatusMsg << endl;
		return -1;
	}

	vector<string> headers = {"database", "creater", "builtTime", "status"};
	vector<vector<string>> rows;
	for (auto &db : show_response.responseBody)
	{
		rows.push_back({db.database, db.creator, db.builtTime, db.status});
	}

	Util::printConsole(headers, rows);
	return 0;
}

int create_handler(const vector<string> &args)
{
	CHECK_ARGC(2, 1, 2)

	string db_name = "";
	string db_path = "";
	if (args.size() < 1 || args[0].size() < 3)
	{
		cout << "build param error, need param num at least 1 or database name need at least 3 character" << endl;
		return -1;
	}
	if (args.size() == 1)
	{
		db_name = args[0];
	}
	else
	{
		db_name = args[0];
		db_path = args[1];
	}
	if (db_name == GlobalTypedef::system_db)
	{
		cout << "Your db name can NOT be \"system\"." << endl;
		return -1;
	}
	server::MessageBuildRequest build_request(db_name, db_path);
	build_request.username = root_username;
	build_request.password = root_password;	
	if (StringUtil::start_with(db_path, "http://") || StringUtil::start_with(db_path, "https://"))
	{
		build_request.remote = true;
	}
	
	int64_t t1 = gutil::TimeUtil::timestamp();
	server::MessageBuildResponse build_response = APIConnector::build(API_URL, true, build_request);
	int64_t t2 = gutil::TimeUtil::timestamp();
	if (!build_response.success())
	{
		cout << "Build RDF database " << db_name << " failed: " << build_response.StatusMsg << endl;
		return -1;
	}
	if (build_response.failed_num > 0)
	{
		cout<< "RDF parse error num " << build_response.failed_num << endl;
		cout<< "See log file for details "<< endl;
	}
	cout << "Build RDF database " << db_name << " successfully! use " << (t2-t1) << " ms." << endl;
	return 0;
}

int drop_handler(const vector<string> &args)
{
	CHECK_ARGC(1, 1)
	string db_name = args[0];
	if (db_name == GlobalTypedef::system_db)
	{
		cout << "You can NOT drop system database. " << endl;
		return -1;
	}

	if (db_name == _current_database)
	{
		cout << "You can NOT drop current database. Please UNLOAD current database through \"UNLOAD <database_name>;\" before you drop it.";
		return -1;
	}
	server::MessageDropRequest drop_request(db_name, false);
	drop_request.username = root_username;
	drop_request.password = root_password;
	server::MessageResponse drop_response = APIConnector::drop(API_URL, true, drop_request);
	if (!drop_response.success())
	{
		cout << "Drop database " << db_name << " failed: " << drop_response.StatusMsg << endl;
		return -1;
	}
	cout << "Drop database " << db_name << " successfully! " << endl;
	return 0;
}

int export_handler(const vector<string> &args)
{
	// // TODO
	CHECK_ARGC(2, 0, 1)
	CHECK_CURRENT_DB_LOADED
	CHECK_CURRENT_DB_NOT_SYSDB
	check_priv(_current_database, EXPORT_PRIVILEGE_BIT);

	string export_path = GlobalTypedef::export_path;
	
	server::MessageExportRequest export_request(_current_database, export_path, false);
	export_request.username = root_username;
	export_request.password = root_password;
	server::MessageResponse export_response = APIConnector::exportDb(API_URL, true, export_request);
	if (!export_response.success())
	{
		cout << "Database " << _current_database << " exported failed: " << export_response.StatusMsg << endl;
		return -1;
	}
	cout << "Database " << _current_database << " exported successfully." << endl;
	return 0;
}

int backup_handler(const vector<string> &args)
{
	CHECK_ARGC(2, 0, 1)
	CHECK_CURRENT_DB_LOADED
	CHECK_CURRENT_DB_NOT_SYSDB
	check_priv(_current_database, BACKUP_PRIVILEGE_BIT);
	// TODO
	string backup_path;
	if (!args.empty()) 
	{
		backup_path = args[0];
	} 
	else 
	{
		backup_path = default_backup_path;
	}
	server::MessageBackupRequest backup_request(_current_database, backup_path, false, "", false);
	backup_request.username = root_username;
	backup_request.password = root_password;
	server::MessageBackupResponse backup_response = APIConnector::backup(API_URL, true, backup_request);
	if (!backup_response.success())
	{
		cout << "Database " << _current_database << " backup failed: " << backup_response.StatusMsg << endl;
		return -1;
	}
	// cout << "Backup path: " << backup_path << endl;
	cout << "Database " << _current_database << " backup successfully." << endl;
	return 0;
}

int restore_handler(const vector<string> &args)
{
	CHECK_ARGC(1, 2)
	// CHECK_CURRENT_DB_LOADED
	// CHECK_CURRENT_DB_NOT_SYSDB
	// TODO
	string db_name = args[0], backup_path = args[1];
	check_priv(db_name, RESTORE_PRIVILEGE_BIT);
	if (db_name == _current_database)
	{
		cout << "Database " + _current_database + " restore failed: Database alreay load, need unload it first through \"UNLOAD <database_name>;\" before you restore it" << endl;
		return -1;
	}
	if (backup_path[0] == '/')
		backup_path = '.' + backup_path;
	if (backup_path[backup_path.length() - 1] == '/')
		backup_path = backup_path.substr(0, backup_path.length() - 1);

	// if (Util::get_backup_time(backup_path).size() == 0) 
	// {
	// 	cout << "Backup Path Does not match the Database name, Restore Failed" << endl;
	// 	return -1;
	// }

	// if (!FileUtil::dirExists(backup_path))
	// {
	// 	cout << "backup file path is not exist, restore Failed" << endl;
	// 	return -1;
	// }

	bool db_exist = false;
	server::MessageShowRequest show_request;
	show_request.username = root_username;
	show_request.password = root_password;
	server::MessageShowResponse show_response = APIConnector::show(API_URL, true, show_request);
	if (!show_response.success())
	{
		cout << "failed to get database information: " << show_response.StatusMsg << endl;		
	}

	for (const auto& db : show_response.responseBody) 
	{
		if (db.database == db_name) db_exist = true;
	}

	if (!db_exist) {
		cout << "Database " << db_name << " not exist, Now Rebuild it!" << endl;
		server::MessageBuildRequest build_request(db_name, "");
		build_request.username = root_username;
		build_request.password = root_password;
		server::MessageBuildResponse build_response = APIConnector::build(API_URL, true, build_request);
		if (!build_response.success())
		{
			cout << "Rebuild Error, Restore Failed: " << build_response.StatusMsg << endl;
			return -1; 
		}
		else
		{
			cout << "Add database info success." << endl;
		}
		Util::add_backuplog(db_name);
	}

	server::MessageRestoreRequest restore_request(db_name, backup_path, false, "", false);
	restore_request.username = root_username;
	restore_request.password = root_password;
	server::MessageRestoreResponse restore_response = APIConnector::restore(API_URL, true, restore_request);
	
	if (!restore_response.success())
	{
		cout << "Database " << db_name << " restore failed: " << restore_response.StatusMsg << endl;
		return -1;
	}

	// if (db_name == _current_database)
	// {
	// 	cout << "WARNNING: The database you restored just now is current database(" << db_name << "), will restore.\nRestore is done" << endl;
	// }

	cout << "Database " << db_name << " restored successfully." << endl;
	return 0;
}

int use_handler(const vector<string> &args)
{
	CHECK_ARGC(1, 1)
	string new_db_name = args[0];
	check_priv(new_db_name, LOAD_PRIVILEGE_BIT);
	check_priv(new_db_name, UNLOAD_PRIVILEGE_BIT);
	if (new_db_name == GlobalTypedef::system_db)
	{
		if (usrname == root_username)
		{
			_current_database = new_db_name;
			return 0;
		}
		else
		{
			cout << "You can NOT use system database. " << endl;
			return -1;
		}
	}
	server::MessageLoadRequest load_request(new_db_name, "0");
	server::MessageLoadResponse load_response = APIConnector::load(API_URL, true, load_request);
	if (!load_response.success())
	{
		cout << "Load database " << new_db_name << " failed: " << load_response.StatusMsg << endl;
		return -1;
	}
	_current_database = new_db_name;
	cout << "Current database switch to " << new_db_name << " successfully." << endl;
	return 0;
}

int unload_handler(const vector<string> &args)
{
	CHECK_CURRENT_DB_LOADED
	CHECK_CURRENT_DB_NOT_SYSDB
	check_priv(_current_database, UNLOAD_PRIVILEGE_BIT);
	if (_current_database.empty())
	{
		cout << "Use no database!";
		return -1;
	}

	server::MessageUnloadRequest unload_request(_current_database);
	server::MessageResponse unload_response = APIConnector::unload(API_URL, true, unload_request);
	if (!unload_response.success())
	{
		cout << "Unload database " << _current_database << " failed: " << unload_response.StatusMsg << endl;
		return -1;
	}

	cout << "Unload database " << _current_database <<" successfully." << endl;
	_current_database = "";
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
	cout << _current_database << endl;
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
	string old_stdpswd = stdpswd;
	enter_pswd(prompt);
	if (stdpswd != old_stdpswd)
	{
		cout << "Fail to varify your id. Password set failed." << endl;
		return -1;
	}
	old_stdpswd = stdpswd;

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

		prompt = "Enter new password: ";
		enter_pswd(prompt);
		new_pswd = stdpswd;

		prompt = "Enter new password again: ";
		enter_pswd(prompt);
		confirm = stdpswd;

	} while (not_match_cnt < MAX_WRONG_PSWD_TIMES && confirm != new_pswd);
	stdpswd = old_stdpswd;
	if (not_match_cnt >= MAX_WRONG_PSWD_TIMES)
	{
		cout << "Too much not match. Password set failed." << endl;
		return -1;
	}
	
	if (args.size() == 0)
	{
		server::MessageUserPasswordRequest password_request(tar_usr, stdpswd, new_pswd);
		server::MessageUserPasswordResponse password_response = APIConnector::userPassword(API_URL, true, password_request);
		if (!password_response.success()) 
		{
			cout << "System db update failed : " + password_response.StatusMsg + ". Password set failed." << endl;
			return -1;
		}
	}
	else 
	{
		server::MessageUserManageRequest password_request(3, tar_usr, new_pswd);
		password_request.username = root_username;
		password_request.password = root_password;
		server::MessageResponse password_response = APIConnector::userManage(API_URL, true, password_request);
		if (!password_response.success()) 
		{
			cout << "System db update failed : " + password_response.StatusMsg + ". Password set failed." << endl;
			return -1;
		}
	}

	if (tar_usr == usrname)
	{
		stdpswd = new_pswd;
	}
	if (tar_usr == root_username)
	{
		root_password = new_pswd;
	}

	cout << "Password set successfully." << endl;
	return 0;
}


int setpriv_handler(const vector<string> &args)
{
	CHECK_ARGC(1, 2)
	// TODO
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

	enter_pswd("Enter your password: ");
	
	if (stdpswd != root_password)
	{
		cout << "Fail to varify your id. Privilege set failed." << endl;
		return -1;
	}

	for (int i = 1; i < PRIVILEGE_NUM; ++i)
	{
		cout << "[" << i << "]" << priv_offset2name[i] << " ";
	}
	cout << "[" << PRIVILEGE_NUM << "]all\nEnter privilege number to assign separated by whitespace: " << endl;
	
	string line;
	cin.clear();
	getline(cin, line);
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

	//TO DO;
	unsigned origin_priv = read_priv(usr, db);
	string priv_string;
	for (int i = 1; i < PRIVILEGE_NUM; ++i) 
	{
		if (origin_priv & (1u << i)) 
		{
			priv_string += to_string(i);
			priv_string += ',';
		}
	}
	if (!priv_string.empty()) 
	{
		priv_string.pop_back();
		server::MessageUserPrivilegeManageRequest deletepriv_request(2, usr, priv_string, db);
		deletepriv_request.username = root_username;
		deletepriv_request.password = root_password;
		server::MessageUserPrivilegeManageResponse deletepriv_response = APIConnector::userPrivilegeManage(API_URL, true, deletepriv_request);
		if (!deletepriv_response.success())
		{
			cout << "Privilege set failed " + deletepriv_response.StatusMsg << endl;
			return -1;
		}
	}

	

	// Atomicity Problem
	string addpriv_string;
	for (int i = 1; i < PRIVILEGE_NUM; ++i) 
	{
		if (priv & (1 << i)) 
		{
			addpriv_string += to_string(i);
			addpriv_string += ',';
		}
	}
	if (!addpriv_string.empty())
	{
		addpriv_string.pop_back();
		server::MessageUserPrivilegeManageRequest addpriv_request(1, usr, addpriv_string, db);
		addpriv_request.username = root_username;
		addpriv_request.password = root_password;
		server::MessageUserPrivilegeManageResponse addpriv_response = APIConnector::userPrivilegeManage(API_URL, true, addpriv_request);
		if (!addpriv_response.success())
		{
			cout << "Privilege set failed " + addpriv_response.StatusMsg << endl;
			return -1;
		}
	}

	

	cout << "Privilege set successfully." << endl;
	return 0;
}

int clearpriv_handler(const vector<string> &args)
{
	CHECK_ARGC(1, 1)

	if (usrname != root_username)
	{
		cout << "Permission denied. Only root is allowed to set other's privilege." << endl;
		return -1;
	}

	string tar_usr = args[0];
	if (tar_usr == root_username)
	{
		cout << "Forbidden Behavior: clear root priv." << endl;
		return -1;
	}

	server::MessageUserPrivilegeManageRequest clearpriv_request(3, tar_usr, "", "");
	clearpriv_request.username = root_username;
	clearpriv_request.password = root_password;
	server::MessageUserPrivilegeManageResponse clearpriv_response = APIConnector::userPrivilegeManage(API_URL, true, clearpriv_request);

	if (!clearpriv_response.success())
	{
		cout << "Privilege clear failed " + clearpriv_response.StatusMsg << endl;
		return -1;
	}

	cout << "clear " + tar_usr + " for all databases privilege successfully!" << endl;
	return 0;
}

// add or del succeed: return 0 ;failed: return -1
int adddelusr_handler(int add, string usr)
{
	// TODO
	if (usrname != root_username)
	{
		cout << "Permission denied. Only root is allowed to manage user." << endl;
		return -1;
	}
	enter_pswd("Enter your password: ");
	
	if (stdpswd != root_password)
	{
		cout << "Fail to varify your id. User manage failed." << endl;
		return -1;
	}

	if (add == 1)
		enter_pswd("set password for new usr: ");
	
	server::MessageUserManageRequest adddeluser_request(add, usr, stdpswd);
	adddeluser_request.username = root_username;
	adddeluser_request.password = root_password;
	server::MessageResponse adddeluser_response = APIConnector::userManage(API_URL, true, adddeluser_request);
	if (!adddeluser_response.success())
	{
		cout << adddeluser_response.StatusMsg << endl;
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
		cout << "Add usr " << usr << " failed" << endl;
		return -1;
	}
	cout << "Add usr " << usr << " successfully." << endl;
	return 0;
}

int delusr_handler(const vector<string> &args)
{
	CHECK_ARGC(1, 1)
	string usr = args[0];
	if (adddelusr_handler(2, usr))
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
	std::string tar_db;
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
		tar_db = args[0];
	}
	// pusr; pusr <database_name>
	else
	{
		if (args.size() == 0)
		{
			cout << "usrname: " << usrname << endl;
			return 0;
		}
		tar_usr = usrname;
		tar_db = args[0];
	}

	vector<string> headers = {"user", "database", "privilege"};
	vector<vector<string>> rows;
	if (tar_usr == root_username)
	{
		rows.push_back({root_username, tar_db, "all"});
		Util::printConsole(headers, rows);
		return 0;
	}
	
	server::MessageShowUserRequest showuser_request;
	showuser_request.username = root_username;
	showuser_request.password = root_password;
	server::MessageShowUserResponse showuser_response = APIConnector::showUser(API_URL, true, showuser_request);
	if (!showuser_response.success())
	{
		cout << "Users Query failed: " << showuser_response.StatusMsg << endl;
		return -1;
	}

	auto parse_priv = [] (const string &db_string, const string &priv_string, unordered_map<string, string> &db_priv) -> void
	{
		string buff;
		for (const char &c : db_string)
		{
			if (c != ',') 
			{
				buff += c;
			}
			else
			{
				db_priv[buff] += priv_string;
				db_priv[buff] += ',';
				buff.clear();
			}
			
		}
	};

	for (const auto &info : showuser_response.ResponseBody) {
		if (info.username == root_username) continue;
		unordered_map<string, string> db_priv;
		parse_priv(info.query_privilege, "query",db_priv);
		parse_priv(info.load_privilege, "load", db_priv);
		parse_priv(info.update_privilege, "update", db_priv);
		parse_priv(info.unload_privilege, "unload", db_priv);
		parse_priv(info.backup_privilege, "backup", db_priv);
		parse_priv(info.restore_privilege, "restore", db_priv);
		parse_priv(info.export_privilege, "export", db_priv);
		for (auto &pair : db_priv) {
			if (pair.second.empty()) 
				continue;
			pair.second.pop_back();
			if (count(pair.second.begin(), pair.second.end(), ',') == PRIVILEGE_NUM - 2) 
				pair.second = "all";
			if (info.username == tar_usr && pair.first == tar_db)
				rows.push_back({info.username, pair.first, pair.second});
		}
		if (db_priv.empty())
		{
			rows.push_back({info.username, tar_db, "no priv"});
		}
	}
	//TO DO
	Util::printConsole(headers, rows);
	return 0;

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

	vector<string> headers = {"user", "database", "privilege"};
	vector<vector<string>> rows;
	rows.push_back({root_username, "all", "all"});
	
	server::MessageShowUserRequest showuser_request;
	showuser_request.username = root_username;
	showuser_request.password = root_password;
	server::MessageShowUserResponse showuser_response = APIConnector::showUser(API_URL, true, showuser_request);
	if (!showuser_response.success())
	{
		cout << "Users Query failed: " << showuser_response.StatusMsg << endl;
		return -1;
	}

	auto parse_priv = [] (const string &db_string, const string &priv_string, unordered_map<string, string> &db_priv) -> void
	{
		string buff;
		for (const char &c : db_string)
		{
			if (c != ',') 
			{
				buff += c;
			}
			else
			{
				db_priv[buff] += priv_string;
				db_priv[buff] += ',';
				buff.clear();
			}
			
		}
	};

	for (const auto &info : showuser_response.ResponseBody) {
		if (info.username == root_username) continue;
		unordered_map<string, string> db_priv;
		parse_priv(info.query_privilege, "query",db_priv);
		parse_priv(info.load_privilege, "load", db_priv);
		parse_priv(info.update_privilege, "update", db_priv);
		parse_priv(info.unload_privilege, "unload", db_priv);
		parse_priv(info.backup_privilege, "backup", db_priv);
		parse_priv(info.restore_privilege, "restore", db_priv);
		parse_priv(info.export_privilege, "export", db_priv);
		for (auto &pair : db_priv) {
			if (pair.second.empty()) 
				continue;
			pair.second.pop_back();
			if (count(pair.second.begin(), pair.second.end(), ',') == PRIVILEGE_NUM - 2) 
				pair.second = "all";
			rows.push_back({info.username, pair.first, pair.second});
		}
		if (db_priv.empty())
		{
			rows.push_back({info.username, "all", "no priv"});
		}
	}
	//TO DO
	Util::printConsole(headers, rows);
	return 0;
}

int init_handler(const vector<string> &args)
{
	CHECK_ARGC(1, 1)
	string db_names = args[0];
	if (db_names.find(GlobalTypedef::system_db) != string::npos)
	{
		cout << "You can NOT init system database. " << endl;
		return -1;
	}
	server::MessageInitRequest init_request(db_names);
	init_request.username = root_username;
	init_request.password = root_password;
	server::MessageInitResponse init_response = APIConnector::init(API_URL, true, init_request);
	if (!init_response.success())
	{
		cout << "Init database " << db_names << " failed: " << init_response.StatusMsg << endl;
		return -1;
	}
	cout << "Init database result: " << endl;
	vector<string> headers = {"db_name", "status", "msg"};
	vector<vector<string>> rows;
	for (auto &db : init_response.data)
	{
		rows.push_back({db.db_name, db.status, db.msg});
	}
	Util::printConsole(headers, rows);
	return 0;
}

int refreshconf_handler(const vector<string> &args)
{
	Util::configure();
	server::MessageRefreshconfRequest refresh_request;
	refresh_request.username = root_username;
	refresh_request.password = root_password;
	server::MessageResponse refresh_response = APIConnector::refreshConf(API_URL, true, refresh_request);
	if (!refresh_response.success())
	{
		cout << "Refresh config failed: " << refresh_response.StatusMsg << endl;
		return -1;
	}
	cout << "Refresh config successfully." << endl;
	return 0;
}

int batchinsert_handler(const vector<string> &args)
{
	CHECK_CURRENT_DB_LOADED
	CHECK_CURRENT_DB_NOT_SYSDB
	CHECK_ARGC(1, 1)

	string file_path = args[0];
	server::MessageBatchInsertRequest insert_request(_current_database, file_path);
	insert_request.username = root_username;
	insert_request.password = root_password;
	if (StringUtil::start_with(file_path, "http://") || StringUtil::start_with(file_path, "https://"))
	{
		insert_request.remote = true;
	}
	long duration_time = gutil::TimeUtil::timestamp();
	server::MessageBatchInsertResponse insert_response = APIConnector::batchInsert(API_URL, true, insert_request);
	duration_time = gutil::TimeUtil::timestamp() - duration_time;
	if (!insert_response.success())
	{
		cout << "Insert data into " << _current_database << " failed: " << insert_response.StatusMsg << endl;
		return -1;
	}
	cout << "After inserted triples num " << insert_response.successNum << ",failed num " << insert_response.failedNum <<",used " << duration_time << " ms" << endl;
	return 0;
}

int batchremove_handler(const vector<string> &args)
{
	CHECK_CURRENT_DB_LOADED
	CHECK_CURRENT_DB_NOT_SYSDB
	CHECK_ARGC(1, 1)

	string file_path = args[0];
	server::MessageBatchRemoveRequest remove_request(_current_database, file_path);
	remove_request.username = root_username;
	remove_request.password = root_password;
	if (StringUtil::start_with(file_path, "http://") || StringUtil::start_with(file_path, "https://"))
	{
		remove_request.remote = true;
	}
	long duration_time = gutil::TimeUtil::timestamp();
	server::MessageBatchRemoveResponse remove_response = APIConnector::batchRemove(API_URL, true, remove_request);
	duration_time = gutil::TimeUtil::timestamp() - duration_time;
	if (!remove_response.success())
	{
		cout << "Delete the " << _current_database << " data failed: " << remove_response.StatusMsg << endl;
		return -1;
	}
	cout << "After removed triples num " << remove_response.successNum << ",failed num " << remove_response.failedNum <<",used " << duration_time << " ms" << endl;
	return 0;
}


server::MessageReasonManageResponse reason_manage_handler(int type, const string &arg)
{
	server::MessageReasonManageResponse response;
	if (type == 1)
	{

		ifstream file(arg);
		if (!file.is_open())
		{
			cout << "failed to open file: " << arg << endl;
		}

		nlohmann::json ruleinfo;
		file >> ruleinfo;
		file.close();

		server::MessageAddReasonRequest request(_current_database, ruleinfo);
		request.username = root_username;
		request.password = root_password;
		response = APIConnector::addReason(API_URL, true, request);
	}
	else if (type == 2)
	{
		server::MessageListReasonRequest request(_current_database);
		request.username = root_username;
		request.password = root_password;
		response = APIConnector::listReason(API_URL, true, request);
	} 
	else
	{
		server::MessageCedsdReasonRequest request(_current_database, to_string(type), arg);
		request.username = root_username;
		request.password = root_password;
		response = APIConnector::cedsdReason(API_URL, true, request);
	}

	return move(response);
}

int addreason_handler(const vector<string> &args)
{
	CHECK_CURRENT_DB_LOADED
	CHECK_CURRENT_DB_NOT_SYSDB
	CHECK_ARGC(1, 1)

	if (!FileUtil::fileExists(args[0]))
	{
		cout << "File " << args[0] << " does not exist." << endl;
		return -1;
	}

	server::MessageReasonManageResponse response = reason_manage_handler(1, args[0]);
	// if (response.success())
	// {
	// 	cout <<  endl; 
	// }
	// else
	// {
	// 	cout << "ADD REASON FAILED!" << endl;
	// }
	cout << response.StatusMsg << endl;
	return response.success();
}

int listreason_handler(const vector<string> &args)
{
	CHECK_CURRENT_DB_LOADED
	CHECK_CURRENT_DB_NOT_SYSDB
	CHECK_ARGC(1, 0)

	server::MessageReasonManageResponse response = reason_manage_handler(2, args[0]);
	vector<string> headers = {"ruleid", "rulename", "description"};
	vector<vector<string> > rows;
	int id = 1;
	for (auto &rule_json : response.list)
	{
		vector<string> content;
		content.push_back(to_string(id));
		string rulename = "";
		
		rows.push_back({to_string(id), rule_json.at("rulename"), rule_json.at("description")});
		Util::printConsole(headers, rows);
		id++;
	}
	// if (!ret)
	// {
	// 	cout << "LIST REASON SUCCESSFULLY!" << endl; 
	// }
	// else
	// {
	// 	cout << "LIST REASON FAILED!" << endl;
	// }
	cout << "list reasons successfully!" << endl;
	return response.success();
}

int compilereason_handler(const vector<string> &args)
{
	CHECK_CURRENT_DB_LOADED
	CHECK_CURRENT_DB_NOT_SYSDB
	CHECK_ARGC(1, 1)

	server::MessageReasonManageResponse response = reason_manage_handler(3, args[0]);
	// if (!ret)
	// {
	// 	cout << "COMPILE REASON SUCCESSFULLY!" << endl; 
	// }
	// else
	// {
	// 	cout << "COMPILE REASON FAILED!" << endl;
	// }
	if (response.success())
	{
		vector<string> headers = {"insert_sparql", "delete_sparql"};
		vector<vector<string> > rows = {{response.insert_sparql, response.delete_sparql}};
		Util::printConsole(headers, rows);
	}
	cout << "compile reason" + args[0] << "successfully!" << endl;
	return response.success();
}

int executereason_handler(const vector<string> &args)
{
	CHECK_CURRENT_DB_LOADED
	CHECK_CURRENT_DB_NOT_SYSDB
	CHECK_ARGC(1, 1)


	server::MessageReasonManageResponse response = reason_manage_handler(4, args[0]);
	// if (!ret)
	// {
	// 	cout << "EXECUTE REASON SUCCESSFULLY!" << endl; 
	// }
	// else
	// {
	// 	cout << "EXECUTE REASON FAILED!" << endl;
	// }
	if (response.success())
	{
		vector<string> headers = {"insert_sparql"};
		vector<vector<string> > rows = {{response.insert_sparql}};
		Util::printConsole(headers, rows);
	}
	cout << "execute reason" + args[0] + "successfully!" << endl;
	return response.success();
}

int disablereason_handler(const vector<string> &args)
{
	CHECK_CURRENT_DB_LOADED
	CHECK_CURRENT_DB_NOT_SYSDB
	CHECK_ARGC(1, 1)

	server::MessageReasonManageResponse response = reason_manage_handler(5, args[0]);
	// if (!ret)
	// {
	// 	cout << "DISABLE REASON SUCCESSFULLY!" << endl; 
	// }
	// else
	// {
	// 	cout << "DISABLE REASON FAILED!" << endl;
	// }
	if (response.success())
	{
		vector<string> headers = {"delete_sparql"};
		vector<vector<string> > rows = {{response.delete_sparql}};
		Util::printConsole(headers, rows);
	}
	cout << "disable reason" + args[0] + "successfully!" << endl;
	return response.success();
}

int showreason_handler(const vector<string> &args)
{
	CHECK_CURRENT_DB_LOADED
	CHECK_CURRENT_DB_NOT_SYSDB
	CHECK_ARGC(1, 1)

	server::MessageReasonManageResponse response = reason_manage_handler(6, args[0]);
	vector<string> headers = {"rulename", "description", "patterns", "filters"};
	vector<vector<string> > rows;

	server::RuleInfo rule_info;
	rule_info.from_json(response.ruleinfo);
	cout << rule_info.conditions.size() << endl;
	for (auto &condition : rule_info.conditions)
	{
		string filter_str;
		for (const auto &filter : condition.filters)
		{
			filter_str += filter;
			filter_str += ';';
		}

		string pattern_str;
		for (const auto &pattern : condition.patterns)
		{
			pattern_str += "<";
			pattern_str += pattern.subject;
			pattern_str += ",";
			pattern_str += pattern.object;
			pattern_str += ",";
			pattern_str += pattern.subject;
			pattern_str += ">";
			rows.push_back({rule_info.rulename, rule_info.description,
							pattern_str, filter_str});
			pattern_str.clear();
		}

	}
	Util::printConsole(headers, rows);
	// if (!ret)
	// {
	// 	cout << "SHOW REASON SUCCESSFULLY!" << endl; 
	// }
	// else
	// {
	// 	cout << "SHOW REASON FAILED!" << endl;
	// }
	cout << "show reason " + args[0] + " successfully!" << endl;
	return response.success();
}

int deletereason_handler(const vector<string> &args)
{
	CHECK_CURRENT_DB_LOADED
	CHECK_CURRENT_DB_NOT_SYSDB
	CHECK_ARGC(1, 1)

	server::MessageReasonManageResponse response = reason_manage_handler(7, args[0]);
	// if (!ret)
	// {
	// 	cout << "DELETE REASON SUCCESSFULLY!" << endl; 
	// }
	// else
	// {
	// 	cout << "DELETE REASON FAILED!" << endl;
	// }
	cout << "delete reason" + args[0] + "successfully!" << endl;
	return response.success();
}

int funquery_handler(const vector<string>& args)
{
	CHECK_ARGC(1, 0);
	PFNInfo funInfo;
	server::MessageFunQueryRequest funquery_request;
	funquery_request.funInfo = funInfo;
	funquery_request.username = root_username;
	funquery_request.password = root_password;
	server::MessageFunQueryResponse funquery_response = APIConnector::funQuery(API_URL, true, funquery_request);
	if (!funquery_response.success())
	{
		cout << "failed to query custom function: " << funquery_response.StatusMsg << endl;
		return -1;
	}
	
	vector<string> headers = {"name", "desc", "returnType", "status", "lastBuildTime"};
	vector<vector<string> > rows ={};

	for (const PFNInfo &item : funquery_response.list)
	{
		rows.push_back({item.funName, item.funDesc, 
						"./pfn/" + item.funName + ".cpp", item.funStatus, item.lastTime});
	}
	
	Util::printConsole(headers, rows);
	cout << "query custom function successfully!" << endl;
	return 0;
}

int funcudb_handler(int type, const string& arg)
{
	
	PFNInfo funInfo;
	if (type == 1 || type == 2)
	{
		ifstream file(arg);
		if (!file.is_open())
		{
			cout << "failed to open file: " << arg << endl;
			return -1;
		}
		nlohmann::json json;
		file >> json;
		file.close();
		funInfo = PFNInfo(json);
	}
	else if(type == 3 || type == 4)
	{
		funInfo.funName = arg;
	}
	else 
	{
		cout << "invalid type" << endl;
		return -1;
	}
	server::MessageFunCudbRequest funcudb_request(to_string(type));
	funcudb_request.username = root_username;
	funcudb_request.password = root_password;
	funcudb_request.funInfo = funInfo;
	server::MessageFunCudbResponse funcudb_response = APIConnector::funCudb(API_URL, true, funcudb_request);
	if (!funcudb_response.success())
	{
		cout << "function operation failed: " << funcudb_response.getStatusMsg() << endl;
		return -1;
	}

	return 0;
}

int funcreate_handler(const vector<string>& args)
{
	CHECK_ARGC(1, 1);

	if (!FileUtil::fileExists(args[0]))
	{
		cout << "File " << args[0] << " does not exist." << endl;
		return -1;
	}

	int ret = funcudb_handler(1, args[0]);
	if (ret == -1)
	{
		cout << "failed to create custom function" << endl;
		return -1;
	}
	cout << "create custom function successfully!" << endl;
	return 0;
}

int funupdate_handler(const vector<string>& args)
{
	CHECK_ARGC(1, 1);

	if (!FileUtil::fileExists(args[0]))
	{
		cout << "File " << args[0] << " does not exist." << endl;
		return -1;
	}

	int ret = funcudb_handler(2, args[0]);
	if (ret == -1)
	{
		cout << "failed to update custom function" << endl;
		return -1;
	}
	cout << "update custom function successfully!" << endl;
	return 0;
}

int fundelete_handler(const vector<string>& args)
{
	CHECK_ARGC(1, 1);

	int ret = funcudb_handler(3, args[0]);
	if (ret == -1)
	{
		cout << "failed to delete custom function" << endl;
		return -1;
	}
	cout << "delete custom function successfully!" << endl;	
	return 0;
}

int funbuild_handler(const vector<string>& args)
{
	CHECK_ARGC(1, 1);

	int ret = funcudb_handler(4, args[0]);
	if (ret == -1)
	{
		cout << "failed to build custom function" << endl;
		return -1;
	}
	cout << "custom function build successfully!" << endl;
	return 0;
}

int funreview_handler(const vector<string>& args)
{
	CHECK_ARGC(1, 1);
	if (!FileUtil::fileExists(args[0]))
	{
		cout << "File " << args[0] << " does not exist." << endl;
		return -1;
	}

	ifstream file(args[0]);
	if (!file.is_open())
	{
		cout << "failed to open file: " << args[0] << endl;
		return -1;
	}

	nlohmann::json funInfo;
	file >> funInfo;
	server::MessageFunReviewRequest review_request;
	review_request.funInfo = PFNInfo(funInfo);
	server::MessageFunReviewResponse review_response = APIConnector::funReview(API_URL, true, review_request);
	if (!review_response.success())
	{
		cout << "failed to review custom function: " << review_response.getStatusMsg() << endl;
	}

	vector<string> headers = {"result"};
	if (funInfo.contains("funName"))
	{
		vector<string> header = {"result"};
		string file_path = funInfo.at("funName");
		vector<vector<string> > rows = {{"./pfn/" + file_path + ".cpp"}};
		Util::printConsole(headers, rows);
	}
		
	cout << "review custom function successfully!" << endl;
	return 0;
}

int txnlog_handler(const vector<string>& args)
{
	CHECK_ARGC(1, 2)
	// check args if numeric
	if (!pure_digit(args[0]) || !pure_digit(args[1])) 
	{
		cout << "Illegal Argument: arg0 && arg1 must be number" << endl;
		return -1;
	}

	int pageNo = stoi(args[0]);
	int pageSize = stoi(args[1]);
	server::MessageTxnLogRequest txnlog_request(pageNo, pageSize);
	txnlog_request.username = root_username;
	txnlog_request.password = root_password;
	server::MessageTxnLogResponse txnlog_response = APIConnector::txnLog(API_URL, true, txnlog_request);
	if (!txnlog_response.success())
	{
		cout << "Query txnlog failed: " << txnlog_response.StatusMsg << endl;
		return -1;
	}
	vector<string> headers = {"dbname", "TID", "user", "state", "begintime", "endtime"};
	vector<vector<string> > rows;
	for (auto json : txnlog_response.list) {
		server::TxnLog txnlog;
		txnlog.from_json(json);
		rows.push_back({txnlog.db_name, txnlog.TID, txnlog.user,
						txnlog.state, txnlog.begin_time, txnlog.end_time});
	}
	Util::printConsole(headers, rows);
	cout << "Query txnlog successfully." << endl;
	return 0;
}

int querylogdate_handler(const vector<string>& args)
{
	CHECK_ARGC(1, 0)
	server::MessageQueryLogDateRequest querylogdate_request;
	querylogdate_request.username = root_username;
	querylogdate_request.password = root_password;
	server::MessageQueryLogDateResponse querylogdate_response = APIConnector::queryLogDate(API_URL, true, querylogdate_request);
	if (!querylogdate_response.success())
	{
		cout << "querylogdate failed: " << querylogdate_response.StatusMsg << endl;
		return -1;
	}
	vector<string> headers = {"querylogdate"};
	vector<vector<string> > rows;
	for (auto date : querylogdate_response.list) {
		rows.push_back({date});
	}
	Util::printConsole(headers, rows);
	cout << "querylogdate successfully." << endl;
	return 0;
}

int querylog_handler(const vector<string>& args)
{
	CHECK_ARGC(1, 3)
	string date = args[0];
	if (!pure_digit(args[1]) || !pure_digit(args[2])) 
	{
		cout << "Illegal Argument: arg1 && arg2 must be number" << endl;
		return -1;
	}
	int pageNo = stoi(args[1]);
	int pageSize = stoi(args[2]);
	
	server::MessageQueryLogRequest querylog_request(date, pageNo, pageSize);
	querylog_request.username = root_username;
	querylog_request.password = root_password;
	server::MessageQueryLogResponse querylog_response = APIConnector::queryLog(API_URL, true, querylog_request);
	if (!querylog_response.success())
	{
		cout << "querylog failed: " << querylog_response.StatusMsg << endl;
		return -1;
	}
	vector<string> headers = {"QueryDateTime", "Sparql", "Format", "RemoteIP", "FileName", "QueryTime", "AnsNum"};
	vector<vector<string> > rows;
	for (auto json : querylog_response.list) {
		server::QueryLog querylog;
		querylog.from_json(json);
		rows.push_back({querylog.QueryDateTime, querylog.Sparql, querylog.Format,
						querylog.RemoteIP, querylog.FileName, to_string(querylog.QueryTime),
						to_string(querylog.AnsNum)});
	}
	Util::printConsole(headers, rows);
	return 0;
}

int accesslogdate_handler(const vector<string>& args) {
	CHECK_ARGC(1, 0)
	server::MessageAccessLogDateRequest accesslogdate_request;
	accesslogdate_request.username = root_username;
	accesslogdate_request.password = root_password;
	server::MessageAccessLogDateResponse accesslogdate_response = APIConnector::accessLogDate(API_URL, true, accesslogdate_request);
	if (!accesslogdate_response.success())
	{
		cout << "accesslogdate failed: " << accesslogdate_response.StatusMsg << endl;
		return -1;
	}
	vector<string> headers = {"accesslogdate"};
	vector<vector<string> > rows;
	for (auto date : accesslogdate_response.list) {
		rows.push_back({date});
	}
	Util::printConsole(headers, rows);
	cout << "accesslogdate successfully." << endl;
	return 0;
}

int accesslog_handler(const vector<string>& args) {
	CHECK_ARGC(1, 3)
	string date = args[0];
	if (!pure_digit(args[1]) || !pure_digit(args[2])) 
	{
		cout << "Illegal Argument: arg1 && arg2 must be number" << endl;
		return -1;
	}
	int pageNo = stoi(args[1]);
	int pageSize = stoi(args[2]);
	
	server::MessageAccessLogRequest accesslog_request(date, pageNo, pageSize);
	accesslog_request.username = root_username;
	accesslog_request.password = root_password;
	server::MessageAccessLogResponse accesslog_response = APIConnector::accessLog(API_URL, true, accesslog_request);
	if (!accesslog_response.success())
	{
		cout << "accessLog failed: " << accesslog_response.StatusMsg << endl;
		return -1;
	}
	vector<string> headers = {"ip", "operation", "createtime", "code", "msg"};
	vector<vector<string> > rows;
	for (auto json : accesslog_response.list) {
		server::AccessLog accesslog;
		accesslog.from_json(json);
		rows.push_back({accesslog.ip, accesslog.operation, accesslog.createtime,
						accesslog.code, accesslog.msg});
	}
	Util::printConsole(headers, rows);
	cout << "accessLog successfully." << endl;
	return 0;
}

int begin_handler(const vector<string>& args)
{
	CHECK_ARGC(1, 2)
	if (!pure_digit(args[1])) 
	{
		cout << "Illegal Argument: arg1 must be number" << endl;
		return -1;
	}

	string db_name = args[0];
	string isolevel = args[1];
	server::MessageBeginRequest begin_request(db_name, isolevel);
	begin_request.username = root_username;
	begin_request.password = root_password;
	server::MessageBeginResponse begin_response = APIConnector::begin(API_URL, true, begin_request);
	if (!begin_response.success())
	{
		cout << "begin operation failed: " << begin_response.StatusMsg << endl;
		return -1; 
	}

	return 0;
}

int tquery_handler(const vector<string>& args)
{
	CHECK_ARGC(1, 3)
	string db_name = args[0];
	string tid = args[1];
	string sparql = args[2];
	server::MessageTqueryRequest tquery_request(db_name, tid, sparql);
	tquery_request.username = root_username;
	tquery_request.password = root_password;
	server::MessageTqueryResponse tquery_response = APIConnector::tquery(API_URL, true, tquery_request);
	if (!tquery_response.success())
	{
		cout << "tquery failed: " << tquery_response.StatusMsg << endl;
		return -1;
	}
	cout << "tquery successfully." << endl;
	return 0;

}

int commit_handler(const vector<string>& args)
{
	CHECK_ARGC(1, 2)
	string db_name = args[0];
	string tid = args[1];
	server::MessageCommitRequest commit_request(db_name, tid);
	commit_request.username = root_username;
	commit_request.password = root_password;
	server::MessageCommitResponse commit_response = APIConnector::commit(API_URL, true, commit_request);
	if (!commit_response.success())
	{
		cout << "commit failed: " << commit_response.StatusMsg << endl;
		return -1;
	}
	cout << "commit successfully." << endl;
	return 0;	
}

int rollback_handler(const vector<string>& args)
{
	CHECK_ARGC(1, 2)
	string db_name = args[0];
	string tid = args[1];
	server::MessageRollbackRequest rollback_request(db_name, tid);
	rollback_request.username = root_username;
	rollback_request.password = root_password;
	server::MessageRollbackResponse rollback_response = APIConnector::rollBack(API_URL, true, rollback_request);
	if (!rollback_response.success())
	{
		cout << "rollback failed: " << rollback_response.StatusMsg << endl;
		return -1;
	}
	cout << "rollback successfully." << endl;
	return 0;
}

int checkpoint_handler(const vector<string>& args)
{
	CHECK_ARGC(1, 1)
	string db_name = args[0];
	server::MessageCheckPointRequest checkpoint_request(db_name);
	checkpoint_request.username = root_username;
	checkpoint_request.password = root_password;
	server::MessageCheckPointResponse checkpoint_response = APIConnector::checkPoint(API_URL, true, checkpoint_request);
	if (!checkpoint_response.success())
	{
		cout << "checkpoint failed: " << checkpoint_response.StatusMsg << endl;
		return -1;
	}
	cout << "checkpoint successfully." << endl;
	return 0;
}

// int importlicense_handler(const vector<string>& args)
// {
// 	CHECK_ARGC(1, 1)

// 	string filepath = args[0];
// 	if (!FileUtil::fileExists(filepath))
// 	{
// 		cout << "failed to import license: file " + filepath + "does not exist" << endl;
// 		return -1;
// 	}

// 	server::MessageRequest request;
// 	request.username = root_username;
// 	request.password = root_password;
// 	server::MessageLicenseResponse response = APIConnector::importLicense(BASE_URL, true, request, filepath);
	
// 	if (!response.success())
// 	{
// 		cout << "failed to import license: " << response.StatusMsg << endl; 
// 		return -1;
// 	}
	
// 	vector<string> headers = {"isvalid", "product", "version", "cpu", "mac", "startdate", "enddate", "company", "type", "desc"};
// 	vector<vector<string> > rows = {{to_string(response.isvalid), response.product, response.version,
// 									response.cpu, response.mac, response.startdate, response.enddate, 
// 									response.company, response.type, response.desc}};
// 	Util::printConsole(headers, rows);
// 	cout << "import license successfully!" << endl;
// 	return 0;
// }

// int licenseinfo_handler(const vector<string>& args)
// {
// 	CHECK_ARGC(1, 0)
// 	server::MessageRequest request;
// 	request.username = root_username;
// 	request.password = root_password;
// 	server::MessageLicenseResponse response = APIConnector::licenseInfo(BASE_URL, true, request);
	
// 	if (!response.success())
// 	{
// 		cout << "failed to fetch license information: " << response.StatusMsg << endl; 
// 		return -1;
// 	}

// 	vector<string> headers = {"isvalid", "product", "version", "cpu", "mac", "startdate", "enddate", "company", "type", "desc"};
// 	vector<vector<string> > rows = {{to_string(response.isvalid), response.product, response.version,
// 									response.cpu, response.mac, response.startdate, response.enddate, 
// 									response.company, response.type, response.desc}};
// 	Util::printConsole(headers, rows);
// 	cout << "show license successfully!" << endl;
// 	return 0;
// }

// int removelicense_handler(const vector<string>& args)
// {
// 	CHECK_ARGC(1, 0)
// 	server::MessageRequest request;
// 	request.username = root_username;
// 	request.password = root_password;
// 	server::MessageLicenseResponse response = APIConnector::removeLicense(BASE_URL, true, request);
	
// 	if (!response.success())
// 	{
// 		cout << "failed to remove license: " << response.StatusMsg << endl; 
// 		return -1;
// 	}
// 	cout << "remove license successfully!" << endl;
// 	return 0;
// }