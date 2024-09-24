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
#include "../Api/HttpUtil.h"

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

#define TOTAL_COMMAND_NUM 18
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
int unload_handler(const std::vector<std::string>&);
int batchinsert_handler(const std::vector<std::string>&);
int batchremove_handler(const std::vector<std::string>&);

int flushpriv_handler(const vector<string> &);
int pusr_handler(const vector<string> &);
int pdb_handler(const vector<string> &);
int showdbs_handler(const vector<string> &);

int setpswd_handler(const vector<string> &);
int setpriv_handler(const vector<string> &);
int addusr_handler(const vector<string> &);
int delusr_handler(const vector<string> &);
int showusrs_handler(const vector<string> &);

int refreshconf_handler(const vector<string> &);
int init_handler(const vector<string> &);

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
		// {"backup", backup_handler, "Backup current database.", "backup [<backup_path>];", BACKUP_PRIVILEGE_BIT},
		// {"restore", restore_handler, "Restore a database.", "restore <database_name> <backup_path>;", RESTORE_PRIVILEGE_BIT},
		// {"export", export_handler, "Export a database to .nt file.", "export <file_path>;", EXPORT_PRIVILEGE_BIT},
		// {"pdb", pdb_handler, "Display current database name.", "pdb;", 0},
        {"unload", unload_handler, "Unload the current database.","unload;", UNLOAD_PRIVILEGE_BIT},
        {"batchinsert", batchinsert_handler, "Batch inserts data into the current database.","batchinsert <nt_file_path>;", UPDATE_PRIVILEGE_BIT},
        {"batchremove", batchremove_handler, "Batch deletes the current database data.","batchremove <nt_file_path>;", UPDATE_PRIVILEGE_BIT},

		// id and usr manage
		// {"flushpriv", flushpriv_handler, "Flush priv for current user, updating the in-memory structure.", "flushpriv;", 0},
		// {"pusr", pusr_handler, "Display user's username and privilege.", "pusr; pusr <database_name>; pusr <database_name> <usr_name>;", 0},
		// {"setpswd", setpswd_handler, "Set your password. Be able to set other's password if you are root.", "setpswd; setpswd <usrname>;", ROOT_PRIVILEGE_BIT},
		// {"setpriv", setpriv_handler, "Set user's privilege.", "setpriv <usrname> <database_name>;", ROOT_PRIVILEGE_BIT},
		// {"addusr", addusr_handler, "Add user.", "addusr <usrname>;", ROOT_PRIVILEGE_BIT},
		// {"delusr", delusr_handler, "Del user.", "delusr <usrname>;", ROOT_PRIVILEGE_BIT},
		// {"showusrs", showusrs_handler, "Show all users and privilege for each.", "showusrs;", ROOT_PRIVILEGE_BIT},

		// other
		// {"cancel", 0, "Quit current input command.", "enter \"cancel;\" whenever you need to quit current input, remember the ;", 0}, // execute_line, check whether the line ends with cancel
		{"help", help_handler, "Display help msg. Enter 'help;' see more about usage.", "help [edit/usage/<command>];", 0},
		{"?", help_handler, "Synonym for \"help\".", "help [edit/usage/<command>];", 0},
		// {"settings", settings_handler, "Display settings.", "settings [<conf_name>];", 0},
		{"version", version_handler, "Display  core version.", "version;", 0},

		// linux shell cmd
		{"pwd", pwd_handler, "Print name of current/working directory.", "pwd;", 0},
		{"clear", clear_handler, "Clear screen.", "clear;", 0},

		// raw_sparql
		{"raw_sparql", 0, "Support enter sparql query directedly in gconsole.",
		 "Begin with SELECT, INSERT, DELETE, PREFIX or BASE. For more about SPARQL, see https://www.w3.org/TR/sparql11-query/ ", QUERY_PRIVILEGE_BIT},
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
	cout << "Copyright (c) 2016, 2024, pkumod and topgraph and/or its affiliates." << endl;                                               \
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
	cout << "Copyright (c) 2016, 2024, pkumod and topgraph and/or its affiliates." << endl;
#define CHECK_CURRENT_DB_LOADED                                                                                    \
	if (_current_database.empty())                                                                                     \
	{                                                                                                              \
		cout << "Current database not selected. Please select it first, through \"USE <database_name>\"." << endl; \
		return -1;                                                                                                 \
	}
#define CHECK_CURRENT_DB_NOT_SYSDB                                                                                      \
	if (_current_database == Util::system_db)                                                                                 \
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
string root_username, root_password;
std::string _server_port;
std::string _current_database;
// global
int main(int argc, char **argv)
{
	Util util;
	//  read conf from conf.ini: version, root_name, root_pswd
	_server_port = util.getConfigureValue("port");
	_db_home = util.getConfigureValue("db_home");
	_db_suffix = util.getConfigureValue("db_suffix");
	
	root_username = util.getConfigureValue("root_username");
	product_version = util.getConfigureValue("version");
	product_name = util.getConfigureValue("product_name");
	product_name_lower = product_name;
	product_name_lower[0] = std::tolower(product_name_lower[0]);

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
	/* welcome and work */
	cout << endl;
	cout << product_name<<" Console , an interactive shell based utility to communicate with "<< product_name_lower <<" repositories." << endl;
	PRINT_VERSION
	cout << "" << endl;
	cout << "Welcome to the "<<product_name<<" Console." << endl;
	cout << "Commands end with ;. Cross line input is allowed." << endl;
	cout << "Comment start with #. Redirect (> and >>) is supported." << endl;
	cout << "CTRL+C to quit current command. CTRL+D to exit this console." << endl;
	cout << "Type 'help;' for help. " << endl
		 << endl;

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
	// strline = rm_comment(std::move(strline));
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
	Util::create_dirs("bin/.gconsole_history");
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
	httpentities::LoginRequest login_request(usrname, password);
	httpentities::BaseResponse login_response = HttpUtil::login(API_URL, login_request);
	if(login_response.StatusCode == CURLE_COULDNT_CONNECT)
	{
		cout << "Could not connect to server. Please check server status" << endl;
		exit(0);
	}
	return login_response.success();
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
	return 0;
}

// return priv bitset of usr on db_name
// check db_name exist or not
unsigned get_priv(string usr, string db_name)
{
	// TODO
	return 0;
}

// usrname has request_priv on db_name: return 0, else return -1;
// db_name doesn't exist: return -1
int check_priv(string db_name, unsigned request_priv)
{
	// TODO
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
	return 0;
}

// ofp is set to output, and output need to be closed outer
// query success:return 0; failed:return -1
int raw_sparql_handler(string sparql)
{
	CHECK_CURRENT_DB_LOADED
	string query_url;
	if (_current_database == Util::system_db)
	{
		query_url = BASE_URL + "/sys/query";
	}
	else
	{
		query_url = API_URL;
	}
	httpentities::QueryRequest query_request(_current_database, sparql, "n-triple");
	httpentities::QueryResponse query_response = HttpUtil::query(query_url, true, query_request);
	if (!query_response.success())
	{
		std::cout << "Query failed: " << query_response.StatusMsg << std::endl;
		return -1;
	}
	else if (!query_response.head.empty())
	{
		Util::printConsole(query_response.head, query_response.results);
		std::cout << "Query ans num " << query_response.ansNum << ", use " << query_response.queryTime << " ms." << std::endl;
	} 
	else
	{
		std::cout << "Update ans num " << query_response.ansNum << ", use " << query_response.queryTime << " ms." << std::endl;
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
		cout << product_name << " Console , an interactive shell based utility to communicate with " << product_name_lower << " repositories." << endl;
		cout << "" << endl;
		cout << "For information about " << product_name_lower << " products and services, visit:" << endl;
		cout << "   http://www.gstore.cn/" << endl;
		cout << "For developer information, including the " << product_name_lower << " Reference Manual, visit:" << endl;
		cout << "   http://www.gstore.cn/pcsite/index.html#/documentation" << endl;
		cout << "" << endl;
		cout << "Commands end with ;. Cross line input is allowed." << endl;
		cout << "Comment start with #." << endl;
		cout << "CTRL+C to quit current command. CTRL+D to exit this console." << endl;
		cout << "List of all console commands:" << endl;
		std::vector<std::string> headers = {"name", "description"};
		std::vector<std::vector<std::string>> rows;
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
		std::vector<std::string> headers = {"name", "description", "usage"};
		std::vector<std::vector<std::string>> rows;
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
		std::vector<std::string> headers = {"name", "description"};
		std::vector<std::vector<std::string>> rows;
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
	CHECK_CURRENT_DB_LOADED
	// monitor
	httpentities::MonitorRequest monitor_request(_current_database);
	httpentities::MonitorResponse monitor_response = HttpUtil::monitor(API_URL, true, monitor_request);
	if (!monitor_response.success())
	{
		cout << "Failed to monitor database: " << monitor_response.getStatusMsg() << endl;
		return -1;
	}
	
	std::vector<std::string> header = {"name", "value"};
	std::vector<std::vector<std::string>> rows;
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
	httpentities::ShowRequest show_request;
	httpentities::ShowResponse show_response = HttpUtil::show(API_URL, true, show_request);
	std::vector<std::string> headers = {"database", "creater", "builtTime", "status"};
	std::vector<std::vector<std::string>> rows;
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

	string db_name = args[0];
	string db_path = args[1];
	if (db_name == Util::system_db)
	{
		cout << "Your db name can NOT be \"system\"." << endl;
		return -1;
	}
	httpentities::BuildRequest build_request(db_name, db_path);	
	httpentities::BuildResponse build_response = HttpUtil::build(API_URL, true, build_request);
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
	cout << "Build RDF database " << db_name << " successfully!" << endl;
	return 0;
}

int drop_handler(const vector<string> &args)
{
	CHECK_ARGC(1, 1)
	string db_name = args[0];
	if (db_name == Util::system_db)
	{
		cout << "You can NOT drop system database. " << endl;
		return -1;
	}
	httpentities::DropRequest drop_request(db_name, "0");
	httpentities::BaseResponse drop_response = HttpUtil::drop(API_URL, true, drop_request);
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
	// TODO
	CHECK_CURRENT_DB_LOADED
	CHECK_CURRENT_DB_NOT_SYSDB
	cout << "Database " << _current_database << " exported successfully." << endl;
	return 0;
}

int backup_handler(const vector<string> &args)
{
	CHECK_ARGC(2, 0, 1)
	CHECK_CURRENT_DB_LOADED
	CHECK_CURRENT_DB_NOT_SYSDB
	// TODO
	std::string backup_path;
	cout << "Backup path: " << backup_path << endl;
	cout << "Database " << _current_database << " backup successfully." << endl;
	return 0;
}

int restore_handler(const vector<string> &args)
{
	CHECK_ARGC(1, 2)
	CHECK_CURRENT_DB_LOADED
	CHECK_CURRENT_DB_NOT_SYSDB
	// TODO
	cout << "Database " << _current_database << " restored successfully." << endl;
	return 0;
}

int use_handler(const vector<string> &args)
{
	CHECK_ARGC(1, 1)
	string new_db_name = args[0];
	if (new_db_name == Util::system_db)
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
	httpentities::LoadRequest load_request(new_db_name, "0");
	httpentities::LoadResponse load_response = HttpUtil::load(API_URL, true, load_request);
	if (!load_response.success())
	{
		cout << "Load database " << new_db_name << " failed: " << load_response.StatusMsg << endl;
		return -1;
	}
	_current_database = new_db_name;
	cout << "Current database switch to " << new_db_name << " successfully." << endl;
	return 0;
}

int unload_handler(const std::vector<std::string> &args)
{
	CHECK_CURRENT_DB_NOT_SYSDB
	if (_current_database.empty())
	{
		cout << "Use no database!";
		return -1;
	}
	httpentities::UnloadRequest unload_request(_current_database);
	httpentities::BaseResponse unload_response = HttpUtil::unload(API_URL, true, unload_request);
	if (!unload_response.success())
	{
		cout << "Unload database " << _current_database << " failed: " << unload_response.StatusMsg << endl;
		return -1;
	}
	_current_database = "";
	cout << "Unload database " << _current_database <<" successfully." << endl;
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
	// TODO
	cout << "Password set successfully." << endl;
	return 0;
}

int setpriv_handler(const vector<string> &args)
{
	CHECK_ARGC(1, 2)
	// TODO
	cout << "Privilege set successfully." << endl;
	return 0;
}

// add or del succeed: return 0 ;failed: return -1
int adddelusr_handler(int add, string usr)
{
	// TODO
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

	std::vector<std::string> headers = {"user", "privilege"};
	std::vector<std::vector<std::string>> rows;
	rows.push_back({root_username, "all privilege on all db"});

	// TODO
	Util::printConsole(headers, rows);
	return 0;
}

int init_handler(const vector<string> &args)
{
	CHECK_ARGC(1, 1)
	string db_names = args[0];
	if (db_names.find(Util::system_db) != std::string::npos)
	{
		cout << "You can NOT init system database. " << endl;
		return -1;
	}
	httpentities::InitRequest init_request(db_names);
	httpentities::InitResponse init_response = HttpUtil::init(API_URL, true, init_request);
	if (!init_response.success())
	{
		cout << "Init database " << db_names << " failed: " << init_response.StatusMsg << endl;
		return -1;
	}
	cout << "Init database result: " << endl;
	std::vector<std::string> headers = {"db_name", "status", "msg"};
	std::vector<std::vector<std::string>> rows;
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
	httpentities::RefreshconfRequest refresh_request;
	httpentities::BaseResponse refresh_response = HttpUtil::refreshConf(API_URL, true, refresh_request);
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
	string dir_path;
	if (Util::is_dir(file_path)) 
	{
		dir_path = file_path;
		file_path = "";
	}
	if (!file_path.empty() && !Util::file_exist(file_path))
	{
		cout << "File " << file_path << " does not exist." << endl;
		return -1;
	}
	if (!dir_path.empty() && !Util::dir_exist(dir_path))
	{
		cout << "Dir " << dir_path << " does not exist." << endl;
		return -1;
	}
	httpentities::BatchInsertRequest insert_request(_current_database, file_path, dir_path);
	long duration_time = Util::get_cur_time();
	httpentities::BatchInsertResponse insert_response = HttpUtil::batchInsert(API_URL, true, insert_request);
	duration_time = Util::get_cur_time() - duration_time;
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
	if (!Util::file_exist(file_path))
	{
		cout << "File " << file_path << " does not exist." << endl;
		return -1;
	}
	httpentities::BatchRemoveRequest remove_request(_current_database, file_path);
	long duration_time = Util::get_cur_time();
	httpentities::BatchRemoveResponse remove_response = HttpUtil::batchRemove(API_URL, true, remove_request);
	duration_time = Util::get_cur_time() - duration_time;
	if (!remove_response.success())
	{
		cout << "Delete the " << _current_database << " data failed: " << remove_response.StatusMsg << endl;
		return -1;
	}
	cout << "After removed triples num " << remove_response.successNum << ",failed num " << remove_response.failedNum <<",used " << duration_time << " ms" << endl;
	return 0;
}