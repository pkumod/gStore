/*=============================================================================
# Filename:		gconsole.cpp
# Author: Bookug Lobert, modified by Wang Libo
# Mail: 1181955272@qq.com
# Last Modified:	2016-07-20 19:28
# Description:
This is a console integrating all commands in Gstore System and others. It
provides completion of command names, line editing features, and access to the
history list.
NOTICE: no separators required in the end of your commands, and please just type
one command at a time. If there are many instructions to execute, please write
them in a file like test.sql, and tell the gconsole to use this file
=============================================================================*/

#include "../Database/Database.h"
#include "../Util/Util.h"
#include "GstoreConnector.h"

using namespace std;

//NOTICE: not imitate the usage of gload/gquery/gclient/gserver in command line
//but need to support the query scripts(so support parameters indirectly)

//The names of functions that actually do the manipulation.
//common commands
int help_handler(const vector<string>&);
int source_handler(const vector<string>&);
int quit_handler(const vector<string>&);
//C/S commands
int connect_handler(const vector<string>&);
int disconnect_handler(const vector<string>&);
// server commands
int start_handler(const vector<string>&);
int stop_handler(const vector<string>&);
int restart_handler(const vector<string>&);
int port_handler(const vector<string>&);
int printport_handler(const vector<string>&);
//remote commands
int build_handler(const vector<string>&);
int drop_handler(const vector<string>&);
int load_handler(const vector<string>&);
int unload_handler(const vector<string>&);
int query_handler(const vector<string>&);
int show_handler(const vector<string>&);
int add_handler(const vector<string>&);
int sub_handler(const vector<string>&);
//backup commands
int backup_handler(const vector<string>&);
int restore_handler(const vector<string>&);

//A structure which contains information on the commands this program can understand.
typedef struct {
  const char* name;                   // User printable name of the function
  int (*func)(const vector<string>&); // Function to call to do the job
  const char* doc;                    // Documentation for this function
} COMMAND;
//
COMMAND native_commands[] = {
  { "help", help_handler, "\tDisplay this text." },
  { "?", help_handler, "\tSynonym for \"help\"." },
  { "source", source_handler, "\tUse a file containing SPARQL queries." },
  { "quit", quit_handler, "\tQuit this console." },
  { "connect", connect_handler, "\tConnect to a server running Gstore." },
  { "show", show_handler, "\tShow the database name which is used now." },
  { "build", build_handler, "\tBuild a database from a dataset." },
  { "drop", drop_handler, "\tDrop a database according to the given path." },
  { "load", load_handler, "\tLoad an existing database." },
  { "unload", unload_handler, "\tUnload the current used database." },
  { "query", query_handler, "\tAnswer a SPARQL query." },
  { "add", add_handler, "\tInsert triples to an existing database." },
  { "sub", sub_handler, "\tRemove triples from an existing database." },
  { "start", start_handler, "\tStart local server." },
  { "stop", stop_handler, "\tStop local server." },
  { "restart", restart_handler, "\tRestart local server." },
  { "port", port_handler, "\tChange port of local server." },
  { "printport", printport_handler, "Print local server's port configuration." },
  { "backup", backup_handler, "\tBackup current database." },
  { "restore", restore_handler, "\tRestore a database backup." },
  { NULL, NULL, NULL }
};
//
COMMAND remote_commands[] = {
  { "help", help_handler, "\tDisplay this text." },
  { "?", help_handler, "\tSynonym for \"help\"." },
  { "source", source_handler, "\tUse a file containing SPARQL queries." },
  { "show", show_handler, "\tShow the database name which is used now." },
  { "build", build_handler, "\tBuild a database from a dataset." },
  { "drop", drop_handler, "\tDrop a database according to the given path." },
  { "load", load_handler, "\tLoad a existing database." },
  { "unload", unload_handler, "\tUnload the current used database." },
  { "query", query_handler, "\tAnswer a SPARQL query." },
  { "disconnect", disconnect_handler, "Disconnect the current server connection." },
  { NULL, NULL, NULL }
};
COMMAND* current_commands = native_commands; //according to gc ?= NULL

char* dupstr(const char*);
char* stripwhite(char*);
COMMAND* find_command(char*);
void initialize_readline();
int execute_line(char*);
int deal_with_script(char*);
bool parse_arguments(char*, vector<string>&);
int save_history();
int load_history();

// Global variables
//The name of this program, as taken from argv[0].
char* progname;
//
//When true, this global means the user is done using this program.
bool done = false;          //still running
                            //server-client mode or local engine mode
GstoreConnector* gc = NULL; //local mode by default
                            //redirect mechanism, only useful for query
FILE* output = stdout;
//current using database in local
Database* current_database = NULL;

string db_home = ".";

int
main(int argc, char** argv)
{
  //NOTICE:this is needed to ensure the file path is the work path
  //chdir(dirname(argv[0]));
  //NOTICE:this is needed to set several debug files
  //#ifdef DEBUG
  Util util;
  //#endif

  db_home = Util::global_config["db_home"];

  char* line, *s;
  progname = argv[0];

  //NOTICE+DEBUG: the code segment below is used to change directory if not in gStore root, but here exists some bugs
  //long cur_path_len;
  //char* cur_work_dir;
  //if ((cur_path_len = pathconf(".", _PC_PATH_MAX)) == -1) {
  //cerr << "Couldn't get current working path length" << endl;
  //return 1;
  //}
  //if ((cur_work_dir = (char*)malloc(cur_path_len + 1 + strlen(progname))) == NULL) {
  //cerr << "Couldn't allocate memory for the pathname" << endl;
  //return 1;
  //}
  //if (getcwd(cur_work_dir, cur_path_len) == NULL) {
  //cerr << "Couldn't get current working directory!" << endl;
  //return 1;
  //}
  //strcat(cur_work_dir, "/");
  //strcat(cur_work_dir, progname);
  //char* dir_name = dirname(cur_work_dir);
  //free(cur_work_dir);
  //if (dir_name == NULL) {
  //cerr << "Couldn't get current directory name!" << endl;
  //return 1;
  //}
  //string root(dir_name);
  //root += "/..";
  //if (chdir(root.c_str()) == -1) {
  //cerr << "Couldn't change current directory!" << endl;
  //return 1;
  //}

  system("clear");

  //the info to be printed
  cout << endl;
  cout << "Gstore Console(gconsole), an interactive shell based utility to communicate with gStore repositories." << endl;
  cout << "usage: start-gconsole [OPTION]" << endl;
  cout << " -h, --help              print this help" << endl;
  cout << " -s, --source            source the SPARQL script" << endl;
  cout << "For bug reports and suggestions, see https://github.com/pkumod/gStore" << endl << endl << endl;

  if (argc > 1) {
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
      cout << "Type \"?\" or \"help\" in the console to see info of all commands" << endl;
      if (argc > 2) {
        cerr << "Nonsense to add more parameters!" << endl;
      }
      return 0;
    } else if (strcmp(argv[1], "-s") == 0 || strcmp(argv[1], "--source") == 0) {
      if (argc != 3) {
        cerr << "You should just add one script file to be sourced!" << endl;
        return 1;
      }
      return deal_with_script(argv[2]);
    } else {
      cerr << "Wrong option used, please see the help info first!" << endl;
      return 1;
    }
  }

  cout << "Notice that commands are a little different between native mode and remote mode." << endl;
  cout << "Now is in native mode, please type your commands." << endl;
  cout << "Please do not use any separators in the end." << endl << endl;

  initialize_readline(); //Bind our completer
  using_history();
  load_history();

  //Loop reading and executing lines until the user quits.
  while (!done) {
    if (gc == NULL) {
      line = readline("gstore>");
    } else {
      line = readline("server>");
    }

    //BETTER:multi lines input in alignment?need separators like ';' in gclient.cpp
    //For simplicity, we do not use this feature here.

    if (line == NULL) //EOF or Ctrl-D
    {
      if (current_database != NULL) {
        cerr << endl << "Please unload your database before quiting!" << endl << endl;
        continue;
      }
      cout << endl << endl;
      break;
    }

    //Remove leading and trailing whitespace from the line.
    //Then, if there is anything left, add it to the history
    //list and execute it.
    s = stripwhite(line);

    if (*s) {
      add_history(s);
      execute_line(s);
    }

    free(line);
  }
  save_history();
  exit(0);
}

/* **************************************************************** */
/*                                                                  */
/*                  Some Utilities                                  */
/*                                                                  */
/* **************************************************************** */

char*
dupstr(const char* s)
{
  char* r;
  int len = strlen(s) + 1;
  r = (char*)malloc(len); //BETTER:xmalloc?
  memset(r, 0, sizeof(char) * (len));
  strcpy(r, s);
  return r;
}

// Execute a command line
int
execute_line(char* line)
{
  int i;
  COMMAND* cmd;
  char* word = NULL;

  //to find if redirected
  bool is_redirected = false;
  int j = strlen(line) - 1;
  while (j > -1) {
    if (line[j] == '"')
      break;
    else if (line[j] == '>') {
      is_redirected = true;
      i = j;
      break;
    } else
      j--;
  }
  if (is_redirected) {
    j++;
    while (line[j] && whitespace(line[j])) {
      j++;
    }
    cout << "The file is: " << (line + j) << endl;
    output = fopen(line + j, "w+");
    if (output == NULL) {
      cout << "Failed to open " << (line + j) << endl;
      output = stdout;
    }
    line[i] = '\0';
  }
  //BETTER: how about >> ?

  //Isolate the command word.
  i = 0;
  while (line[i] && whitespace(line[i]))
    i++;
  word = line + i;

  while (line[i] && !whitespace(line[i]))
    i++;

  if (line[i])
    line[i++] = '\0';

  cmd = find_command(word);

  if (!cmd) {
    if (gc == NULL) {
      cout << "Now is in native mode!" << endl;
    } else {
      cout << "Now is in remote mode!" << endl;
    }
    cout << word << ": No such command for gconsole" << endl << endl;
    if (output != stdout) {
      fclose(output);
      output = stdout;
    }
    return -1;
  }

  //Get argument to command, if any.
  while (line[i] && whitespace(line[i]))
    i++;
  word = line + i;
  //cout << word << endl; //debug

  vector<string> args;
  int ret;
  if (parse_arguments(word, args)) {
    ret = cmd->func(args);
  } else {
    ret = -1;
  }
#ifdef DEBUG_PRECISE
  msg << "All done, now to close the file." << endl;
#endif
  if (output != stdout) {
    fclose(output);
    output = stdout;
  }
  cout << endl;
  return ret;
}

// Look up NAME as the name of a command, and return a pointer to that
// command.  Return a NULL pointer if NAME isn't a command name.
COMMAND*
find_command(char* name)
{
  int i;

  for (i = 0; current_commands[i].name; i++) {
    if (strcmp(name, current_commands[i].name) == 0)
      return &current_commands[i];
  }

  return (COMMAND*)NULL;
}

// Strip whitespace from the start and end of STRING. Return a pointer into STRING.
char*
stripwhite(char* string)
{
  char* s, *t;

  for (s = string; whitespace(*s); s++)
    ;

  if (*s == 0)
    return s;

  t = s + strlen(s) - 1;
  while (t > s && (whitespace(*t) || *t == '\r' || *t == '\n')) {
    t--;
  }
  *++t = '\0';

  return s;
}

//support commands scripts
//QUERY:source another file again(how about exactly this script twice or more)
int deal_with_script(char* file)
{
  FILE* fp = NULL;
  if ((fp = fopen(file, "r")) == NULL) {
    cerr << "Open error: " << file << endl;
    return -1;
  }

  //WARN:the length of each line in the script should not exceed 500
  char line[505], *s = NULL;

  while ((fgets(line, 501, fp)) != NULL) {
    //NOTICE:empty line here also contains '\n'
    if (strlen(line) == 1)
      continue;
    s = stripwhite(line);
    //To support comments that begin with "#"
    if (*s && *s != '#') {
      execute_line(s);
    }
  }

  //end of file
  if (current_database != NULL) {
    cerr << endl << "Please unload your database before quitting!" << endl << endl;
    //TODO
  }
  if (gc != NULL) {
    cerr << endl << "Please return to native mode before quitting!" << endl << endl;
    //TODO
  }

  return 0;
}

// Parse arguments
bool parse_arguments(char* word, vector<string>& args)
{
  if (word == NULL) {
    return true;
  }

  while (*word) {
    int i = 0;

    if (*word == '\"') {
      i++;
      while (word[i] && word[i] != '\"') {
        i++;
      }
      char tmp = word[i + 1];
      if (word[i] == '\"' && (whitespace(tmp) || tmp == '\0')) {
        word[++i] = '\0';
        args.push_back(string(word));
        //cout << args.size() - 1 << '\t' << args.back() << endl; //debug
        word[i] = tmp;
        while (word[i] && whitespace(word[i])) {
          i++;
        }
        word += i;
        continue;
      } else {
        cerr << "Invalid arguments!" << endl;
        return false;
      }
    }

    while (word[i] && !whitespace(word[i])) {
      i++;
    }
    char tmp = word[i];
    word[i] = '\0';
    args.push_back(string(word));
    //cout << args.size() - 1 << '\t' << args.back() << endl; //debug
    word[i] = tmp;
    while (word[i] && whitespace(word[i])) {
      i++;
    }
    word += i;
  }
  return true;
}

// Save command history
// Notice that only by quitting gconsole normally can command history be saved
int save_history()
{
  //Limit the number of history to save
  static const int max_history = 1024;
  stifle_history(max_history);

  HIST_ENTRY** pHisList = history_list();

  if (pHisList == NULL) {
    return 0;
  }

  ofstream fout("bin/.gconsole_history", ios::out);

  if (!fout) {
    return -1;
  }

  while (pHisList[0] != NULL) {
    fout << string(pHisList[0]->line) << endl;
    pHisList++;
  }

  fout.close();

  return 0;
}

// Load command history
int load_history()
{
  ifstream fin("bin/.gconsole_history", ios::in);

  if (!fin) {
    return -1;
  }

  const int line_length = 1024;
  char line[line_length];

  while (fin.getline(line, line_length)) {
    add_history(line);
  }

  fin.close();

  return 0;
}

/* **************************************************************** */
/*                                                                  */
/*                  Interface to Readline Completion                */
/*                                                                  */
/* **************************************************************** */

char* command_generator(const char*, int);
char** gconsole_completion(const char*, int, int);

/* Tell the GNU Readline library how to complete.  We want to try to complete
on command names if this is the first word in the line, or on filenames
if not. */
void
initialize_readline()
{
  /* Allow conditional parsing of the ~/.inputrc file. */
  rl_readline_name = "gconsole";

  /* Tell the completer that we want a crack first. */
  rl_attempted_completion_function = gconsole_completion;
}

/* Attempt to complete on the contents of TEXT. START and END bound the
region of rl_line_buffer that contains the word to complete. TEXT is
the word to complete. We can use the entire contents of rl_line_buffer
in case we want to do some simple parsing. Return the array of matches,
or NULL if there aren't any. */
char**
gconsole_completion(const char* text, int start, int end)
{
  char** matches;

  matches = (char**)NULL;

  //If this word is at the start of the line, then it is a command
  //to complete. Otherwise it is the name of a file in the current directory.
  if (start == 0) {
    matches = rl_completion_matches(text, command_generator);
  }

  return matches;
}

/* Generator function for command completion. STATE lets us know whether
to start from scratch; without any state(i.e. STATE == 0), then we
start at the top of the list. */
char*
command_generator(const char* text, int state)
{
  static int list_index, len;
  const char* name;

  /* If this is a new word to complete, initialize now.  This includes
	saving the length of TEXT for efficiency, and initializing the index
	variable to 0. */
  if (!state) {
    list_index = 0;
    len = strlen(text);
  }

  /* Return the next name which partially matches from the command list. */
  while ((name = current_commands[list_index].name) != NULL) {
    list_index++;

    if (strncmp(name, text, len) == 0)
      return dupstr(name);
  }

  /* If no names matched, then return NULL. */
  return (char*)NULL;
}

/* **************************************************************** */
/*                                                                  */
/*                       gconsole commands                          */
/*                                                                  */
/* **************************************************************** */

//Print out help for ARG, or for all of the commands if ARG is not present.
int help_handler(const vector<string>& args)
{
  switch (args.size()) {
  case 0: {
    int i;
    for (i = 0; current_commands[i].name; i++) {
      cout << '\t' << current_commands[i].name << '\t' << current_commands[i].doc << endl;
    }
    break;
  }
  case 1: {
    int i;
    int printed = 0;
    for (i = 0; current_commands[i].name; i++) {
      if (args[0] == current_commands[i].name) {
        cout << '\t' << current_commands[i].name << '\t' << current_commands[i].doc << endl;
        printed++;
        break;
      }
    }

    if (printed == 0) {
      cerr << "No commands match \"" << args[0] << "\". Possibilities are:" << endl;

      for (i = 0; current_commands[i].name; i++) {
        //Print in six columns.
        if (printed == 6) {
          printed = 0;
          cout << endl;
        }

        cout << current_commands[i].name << '\t';
        printed++;
      }

      if (printed) {
        cout << endl;
      }
    }
    break;
  }
  default: {
    cerr << "Too many arguments!" << endl;
    return -1;
  }
  }
  return 0;
}

//NOTICE:the SPARQL file to be used should be placed in the local machine even when in remote mode
int source_handler(const vector<string>& args)
{
  if (args.size() != 1) {
    cerr << "Exactly 1 argument required!" << endl;
    return -1;
  }

  char* str = dupstr(args[0].c_str());
  int ret = deal_with_script(str);
  free(str);
  return ret;
}

int quit_handler(const vector<string>& args)
{
  if (!args.empty()) {
    cerr << "Too many arguments!" << endl;
    return -1;
  }

  if (gc != NULL) {
    cerr << "This command cannot be used when in remote mode." << endl;
    return -1;
  }

  if (current_database != NULL) {
    cerr << "Please unload your database before quitting." << endl;
    return -1;
  }

  done = true;
  return 0;
}

int connect_handler(const vector<string>& args)
{
  if (args.size() > 2) {
    cerr << "Too many arguments!" << endl;
    return -1;
  }

  if (gc != NULL) {
    cerr << "This command cannot be used when in remote mode." << endl;
    return -1;
  }

  if (current_database != NULL) {
    cerr << "Please unload your database before entering remote mode." << endl;
    return -1;
  }

  unsigned short port = GstoreConnector::defaultServerPort;
  string ip = GstoreConnector::defaultServerIP;

  if (args.size() == 2) {
    if (!Util::isValidIP(args[0])) {
      cerr << "Invalid IP: " << args[0] << endl;
      return -1;
    }

    if (!Util::isValidPort(args[1])) {
      cerr << "Invalid Port: " << args[1] << endl;
      return -1;
    }

    ip = args[0];
    stringstream(args[1]) >> port;
  } else if (args.size() == 1) {
    if (Util::isValidIP(args[0])) {
      ip = args[0];
    } else if (Util::isValidPort(args[0])) {
      stringstream(args[0]) >> port;
    } else {
      cerr << "Invalid argument, neither IP nor port: " << args[0] << endl;
      return -1;
    }
  }

  //initialize the GStore server's IP address and port.
  gc = new GstoreConnector(ip, port);

  if (!gc->test()) {
    cerr << "Failed to connect to server at " << ip << ':' << port << endl;
    delete gc;
    gc = NULL;
    return -1;
  }

  current_commands = remote_commands;
  cout << "Now is in remote mode, please type your commands." << endl;

  return 0;
}

int disconnect_handler(const vector<string>& args)
{
  if (!args.empty()) {
    cerr << "Too many arguments!" << endl;
    return -1;
  }

  if (gc == NULL) {
    cerr << "This command cannot be used when in native mode." << endl;
    return -1;
  }

  string show_ret = gc->show();
  if (show_ret != "connect to server error"
      && show_ret != "send show command error."
      && show_ret != "\n[empty]\n") {
    cerr << "Please unload your server database before entering native mode." << endl;
    return -1;
  }

  delete gc;
  gc = NULL;
  current_commands = native_commands;
  cout << "Now is in native mode, please type your commands." << endl;

  return 0;
}

int show_handler(const vector<string>& args)
{
  if (args.size() > 1) {
    cerr << "Too many arguments!" << endl;
    return -1;
  }

  bool flag = false;
  if (args.size() == 1) {
    if (args[0] == "all") {
      flag = true;
    } else {
      cerr << "Invalid argument: " << args[0] << endl;
      return -1;
    }
  }

  if (gc != NULL) {
    string database = gc->show(flag);
    cout << database << endl;
    return 0;
  }

  //native mode
  if (flag) {
    //showing all avaliable databases
    string database = Util::getItemsFromDir(db_home);
    if (database.empty()) {
      database = "No databases.";
    }
    cout << database << endl;
    return 0;
  }

  //below is to show current using database
  if (current_database == NULL) {
    cout << "No database used now." << endl;
  } else {
    cout << current_database->getName() << endl;
  }

  return 0;
}

//NOTICE: for build() and load(), always keep database in the root of gStore
int build_handler(const vector<string>& args)
{
  if (args.size() != 2) {
    cerr << "Exactly 2 arguments required!" << endl;
    return -1;
  }

  string database = args[0];
  //WARN:user better not end with ".db" by themselves!!!
  if (database.length() > 3 && database.substr(database.length() - 3, 3) == ".db") {
    cerr << "Your db name to be built should not end with \".db\"." << endl;
    return -1;
  }

  //NOTICE: not add .db here, better to use db_suffix
  //database += ".db";

  //NOTICE: when in remote mode, the dataset should be placed in the server! And the exact path can only be got in the server
  //we can deal with it in Database
  string dataset = args[1];

  //remote mode
  if (gc != NULL) {
    // QUERY:how to interact:string?dict(0:string)?json(service_id, service_args)?
    // Here uses the Cpp API Wrapper
    // build a new database by a RDF file.
    // note that the relative path is related to gserver.
    if (gc->build(database, dataset)) {
      return 0;
    } else {
      return -1;
    }
  }

  if (current_database != NULL) {
    cerr << "Please unload your database first." << endl;
    return -1;
  }

  cout << "Import dataset to build database..." << endl;
  cout << "DB_store: " << database << "\tRDF_data: " << dataset << endl;
  int len = database.length();

  if (database.substr(len - 3, 3) == ".db") {
    cerr << "your database can not end with .db" << endl;
    return -1;
  }

  current_database = new Database(database);
  bool flag = current_database->build(dataset);
  delete current_database;
  current_database = NULL;

  if (!flag) {
    cerr << "Import RDF file to database failed." << endl;
    string cmd = "rm -rf " + database;
    system(cmd.c_str());
    return -1;
  }

  cout << "Import RDF file to database done." << endl;
  return 0;
}

int drop_handler(const vector<string>& args)
{
  if (args.size() != 1) {
    cerr << "Exactly 1 argument required!" << endl;
    return -1;
  }

  //only drop when *.db, avoid other files be removed
  string database = args[0];
  if (database.length() > 3 && database.substr(database.length() - 3, 3) == ".db") {
    cerr << "You should use exactly the same db name as building, which should not end with \".db\"" << endl;
    return -1;
  }
  //database += ".db";

  //remote mode
  if (gc != NULL) {
    if (gc->drop(database)) {
      return 0;
    } else {
      return -1;
    }
  }

  if (current_database != NULL) {
    cerr << "Please do not use this command when you are using a database." << endl;
    return -1;
  }

  string cmd = string("rm -rf ") + database;
  int ret = system(cmd.c_str());
  cout << database << " dropped." << endl;
  return ret;
}

//NOTICE+WARN:
//generally, datasets are very large while a query file cannot be too large.
//So, when in remote mode, we expect that datasets in the server are used, while
//queries in local machine are used(transformed to string and passed to server).

int load_handler(const vector<string>& args)
{
  if (args.size() != 1) {
    cerr << "Exactly 1 argument is required!" << endl;
    return -1;
  }

  string database = args[0];
  if (database.length() > 3 && database.substr(database.length() - 3, 3) == ".db") {
    cerr << "You should use exactly the same db name as building, which should not end with \".db\"" << endl;
    return -1;
  }

  //NOTICE: not add .db here, better to use db_suffix
  //database += ".db";

  //remote mode
  if (gc != NULL) {
    if (gc->load(database)) {
      return 0;
    } else {
      return -1;
    }
  }

  if (current_database != NULL) {
    cerr << "Please unload your database first!" << endl;
    return -1;
  }

  current_database = new Database(database);
  bool flag = current_database->load();
  if (!flag) {
    cerr << "Failed to load the database." << endl;
    delete current_database;
    current_database = NULL;
    return -1;
  }

  cout << "Database loaded successfully." << endl;
  return 0;
}

int unload_handler(const vector<string>& args)
{
  if (!args.empty()) {
    cerr << "Too many arguments!" << endl;
    return -1;
  }

  //remote mode
  if (gc != NULL) {
    string database = gc->show();
    if (database == "\n[empty]\n") {
      cerr << "No database used now." << endl;
      return -1;
    }
    if (gc->unload(database.substr(1, database.length() - 2))) {
      return 0;
    } else {
      return -1;
    }
  }

  if (current_database == NULL) {
    cerr << "No database used now." << endl;
    return -1;
  }

  delete current_database;
  current_database = NULL;
  cout << "Database unloaded." << endl;

  return 0;
}

int query_handler(const vector<string>& args)
{
  if (gc == NULL) {
    if (args.size() != 1) {
      cerr << "Exactly 1 argument required!" << endl;
      return -1;
    }
    if (current_database == NULL) {
      cerr << "No database in use!" << endl;
      return -1;
    }
  } else if (args.size() != 1 && !(args.size() == 3 && args[1] == "-r")) {
    cerr << "Invalid arguments!" << endl;
    return -1;
  }

  string sparql;

  if (args[0][0] == '\"') { //query quoted in string
    sparql = args[0].substr(1, args[0].length() - 2);
  } else { //query in file indicated by this path
           //NOTICE:the query is native, not in server!
    string ret = Util::getExactPath(args[0].c_str());
    const char* path = ret.c_str();
    if (path == NULL) {
      cerr << "Invalid path of query." << endl;
      return -1;
    }
#ifdef DEBUG
    cout << path << endl;
#endif
    sparql = Util::getQueryFromFile(path);
  }

  if (sparql.empty()) {
    cerr << "Empty SPARQL." << endl;
    return -1;
  }

#ifdef DEBUG
  cout << sparql << endl;
#endif

  //remote mode
  if (gc != NULL) {
    //QUERY:how to use query path in the server
    //execute SPARQL query on this database.
    string answer;
    if (args.size() == 3) {
      answer = gc->query(sparql, args[2]);
    } else {
      answer = gc->query(sparql);
    }
    fprintf(output, "%s\n", answer.c_str());
    return 0;
  }

  ResultSet rs;
  bool ret = current_database->query(sparql, rs, output);
  current_database->save();
  if (ret) {
#ifdef DEBUG_PRECISE
    cout << "query() returns true!" << endl;
#endif
    return 0;
  } else {
#ifdef DEBUG
    cout << "query() returns false!" << endl;
#endif
    return -1;
  }
}

int add_handler(const vector<string>& args)
{
  if (args.size() != 2) {
    cerr << "Exactly 2 arguments required!" << endl;
    return -1;
  }
  if (current_database != NULL) {
    cerr << "Please unload your database first!" << endl;
    return -1;
  }
  string database = args[0];
  if (database.length() > 3 && database.substr(database.length() - 3, 3) == ".db") {
    cerr << "You should use exactly the same db name as building, which should not end with \".db\"" << endl;
    return -1;
  }
  //database += ".db";
  Database _db(database);
  if (!_db.insert(args[1])) {
    cerr << "Failed to insert!" << endl;
    return -1;
  }
  return 0;
}

int sub_handler(const vector<string>& args)
{
  if (args.size() != 2) {
    cerr << "Exactly 2 arguments required!" << endl;
    return -1;
  }
  if (current_database != NULL) {
    cerr << "Please unload your database first!" << endl;
    return -1;
  }
  string database = args[0];
  if (database.length() > 3 && database.substr(database.length() - 3, 3) == ".db") {
    cerr << "You should use exactly the same db name as building, which should not end with \".db\"" << endl;
    return -1;
  }
  //database += ".db";
  Database _db(database);
  if (!_db.remove(args[1])) {
    cerr << "Failed to remove!" << endl;
    return -1;
  }
  return 0;
}

int start_handler(const vector<string>& args)
{
  if (!args.empty()) {
    cerr << "Too many arguments!" << endl;
    return -1;
  }
  return system("bin/gserver -s");
}

int stop_handler(const vector<string>& args)
{
  if (!args.empty()) {
    cerr << "Too many arguments!" << endl;
    return -1;
  }
  return system("bin/gserver -t");
}

int restart_handler(const vector<string>& args)
{
  if (!args.empty()) {
    cerr << "Too many arguments!" << endl;
    return -1;
  }
  return system("bin/gserver -r");
}

int port_handler(const vector<string>& args)
{
  if (args.size() > 1) {
    cerr << "Too many arguments!" << endl;
    return -1;
  }
  if (args.empty()) {
    return system("bin/gserver -p");
  }
  if (!Util::isValidPort(args[0])) {
    cerr << "Invalid port: " << args[0] << endl;
    return -1;
  }
  string cmd = "bin/gserver -p " + args[0];
  return system(cmd.c_str());
}

int printport_handler(const vector<string>& args)
{
  if (!args.empty()) {
    cerr << "Too many arguments!" << endl;
    return -1;
  }
  return system("bin/gserver -P");
}

int backup_handler(const vector<string>& args)
{
  if (!args.empty()) {
    cerr << "Too many arguments!" << endl;
    return -1;
  }
  if (current_database == NULL) {
    cerr << "No database loaded!" << endl;
    return -1;
  }
  current_database->backup();
  return 0;
}

int restore_handler(const vector<string>& args)
{
  if (args.size() != 1) {
    cerr << "Exactly 1 argument required!" << endl;
    return -1;
  }
  if (current_database != NULL) {
    cerr << "Please unload your database first!" << endl;
    return -1;
  }
  string database = args[0];
  if (database.length() > 3 && database.substr(database.length() - 3, 3) == ".db") {
    cerr << "You should use exactly the same db name as building, which should not end with \".db\"" << endl;
    return -1;
  }
  //database += ".db";

  Database db(database);
  if (!db.restore()) {
    return -1;
  }
  return 0;
}
