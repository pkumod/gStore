/*=============================================================================
# Filename:		gconsole.cpp
# Author: Bookug Lobert
# Mail: 1181955272@qq.com
# Last Modified:	2016-02-26 19:28
# Description:
This is a console integrating all commands in Gstore System and others. It
provides completion of command names, line editing features, and access to the
history list.
=============================================================================*/

#include "../Database/Database.h"
#include "../Util/Util.h"
#include "GstoreConnector.h"

using namespace std;

//extern char *xmalloc PARAMS((size_t));

//The names of functions that actually do the manipulation.
//common commands
int help_handler PARAMS((char *));
int quit_handler PARAMS((char *));
//C/S commands
int connect_handler PARAMS((char *));
int disconnect_handler PARAMS((char *));
//system commands
int list_handler PARAMS((char *));
int view_handler PARAMS((char *));
int rename_handler PARAMS((char *));
int stat_handler PARAMS((char *));
int pwd_handler PARAMS((char *));
int delete_handler PARAMS((char *));
int cd_handler PARAMS((char *));
//remote commands
int build_handler PARAMS((char *));
int drop_handler PARAMS((char *));
int load_handler PARAMS((char *));
int unload_handler PARAMS((char *));
int query_handler PARAMS((char *));
int show_handler PARAMS((char *));

//A structure which contains information on the commands this program can understand.
typedef struct {
    const char *name;			// User printable name of the function
    rl_icpfunc_t *func;		// Function to call to do the job
    const char *doc;			// Documentation for this function
} COMMAND;
//
COMMAND native_commands[] = {
    { "help", help_handler, "Display this text" },
    { "?", help_handler, "Synonym for `help'" },
    { "quit", quit_handler, "Quit this console" },
    { "connect", connect_handler, "Connect to a server running Gstore" },
    { "show", show_handler, "Show the database name which is used now" },
    { "build", build_handler, "Build a database from a dataset" },
    { "drop", drop_handler, "Drop a database according to the given path" },
    { "load", load_handler, "Load a existing database" },
    { "unload", unload_handler, "Unload the current used database" },
    { "query", query_handler, "Answer a SPARQL query" },

    //{ "cd", cd_handler, "Change to directory DIR" },
    //{ "delete", delete_handler, "Delete FILE" },
    //{ "list", list_handler, "List files in DIR" },
    //{ "ls", list_handler, "Synonym for `list'" },
    //{ "pwd", pwd_handler, "Print the current working directory" },
    //{ "rename", rename_handler, "Rename FILE to NEWNAME" },
    //{ "stat", stat_handler, "Print out statistics on FILE" },
    //{ "view", view_handler, "View the contents of FILE" },

    {NULL, NULL, NULL }
    //char* rl_icpfunc_t*, char*
};
//
COMMAND remote_commands[] = {
    { "help", help_handler, "Display this text" },
    { "?", help_handler, "Synonym for `help'" },
    { "show", show_handler, "Show the database name which is used now" },
    { "build", build_handler, "Build a database from a dataset" },
    { "drop", drop_handler, "Drop a database according to the given path" },
    { "load", load_handler, "Load a existing database" },
    { "unload", unload_handler, "Unload the current used database" },
    { "query", query_handler, "Answer a SPARQL query" },
    { "disconnect", disconnect_handler, "Disconnect the current server connection" },

    {NULL, NULL, NULL }
    //char* rl_icpfunc_t*, char*
};
COMMAND *current_commands = native_commands;  //according to gc ?= NULL



// Forward declarations.
//
char *dupstr(const char*);
//
char *stripwhite(char *);
//
COMMAND *find_command(char *);
//
void initialize_readline();
//
int execute_line(char *);
//
int valid_argument(char *, char *);
//
int too_dangerous(char *);



// Global variables
//The name of this program, as taken from argv[0].
char *progname;
//
//When true, this global means the user is done using this program.
bool done = false;   //still running
//
//server-client mode or local engine mode
GstoreConnector *gc = NULL;  //local mode by default
//redirect mechanism, only useful for query
FILE *output = stdout;
//current using database in local
Database *current_database = NULL;

//TODO:how to support commands scripts out or in console
int
main(int argc, char **argv)
{
#ifdef DEBUG
    Util util;
#endif
    char *line, *s;
    progname = argv[0];

	fprintf(stderr, "notice that commands are a little different between native mode and remote mode!\n");

    initialize_readline();	//Bind our completer

    //Loop reading and executing lines until the user quits.
    while(!done)
    {
        if(gc == NULL)
            line = readline("gstore>");
        else
            line = readline("server>");

        //BETTER:multi lines input in alignment?need separators like ';' in gclient.cpp

        if(line == NULL) //EOF or Ctrl-D
        {
			if(current_database != NULL)
			{
				fprintf(stderr, "\nplease unload your database before quiting!\n\n");
				continue;
			}
            printf("\n\n");
            break;
        }

        //Remove leading and trailing whitespace from the line.
        //Then, if there is anything left, add it to the history
        //list and execute it.
        s = stripwhite(line);

        if(*s)
        {
            add_history(s);
            execute_line(s);
        }

        free(line);
    }
    exit(0);
}


/* **************************************************************** */
/*                                                                  */
/*                  Some Utilities                                  */
/*                                                                  */
/* **************************************************************** */

char *
dupstr(const char *s)
{
    char *r;
    int len = strlen(s) + 1;
    r = (char *)malloc(len);  //BETTER:xmalloc?
    memset(r, 0, sizeof(char) * (len));
    strcpy(r, s);
    return(r);
}

// Execute a command line
int
execute_line(char *line)
{
    register int i;
    COMMAND *cmd;
    char *word = NULL;

    output = stdout;
    //to find if redirected
    bool is_redirected = false;
    int j = strlen(line) - 1;
    while(j > -1)
    {
        if(line[j] == '"')
            break;
        else if(line[j] == '>')
        {
            is_redirected = true;
            i = j;
            break;
        }
        else
            j--;
    }
    if(is_redirected)
    {
        j++;
        while(line[j] && whitespace(line[j]))
        {
            j++;
        }
		fprintf(stderr, "the file is: %s\n", line+j);
        if((output = fopen(line+j, "w+")) == NULL)
        {
            output = stdout;
            fprintf(stderr, "fail to open %s\n", line+j);
        }
		line[i] = '\0';
    }
    //BETTER: how about >> ?

    //Isolate the command word.
    i = 0;
	while(line[i] && whitespace(line[i]))
		i++;
    word = line + i;

    while(line[i] && !whitespace(line[i]))
        i++;

    if(line[i])
        line[i++] = '\0';

    //command = find_command(word);
    cmd = find_command(word);

    if(!cmd)
    {
        if(gc == NULL)
            fprintf(stderr, "now is in native mode!\n");
        else
            fprintf(stderr, "now is in remote mode!\n");
        fprintf(stderr, "%s: No such command for gconsole.\n", word);
		if(output != stdout)
			fclose(output);
        return(-1);
    }

    //Get argument to command, if any.
    while(line[i] && whitespace(line[i]))
        i++;
    word = line + i;

    int ret = cmd->func(word);
#ifdef DEBUG
	fprintf(stderr, "all done, now to close the file!\n");
#endif
    if(output != stdout)
        fclose(output);
	fprintf(stderr, "\n");
    return ret;
}

// Look up NAME as the name of a command, and return a pointer to that
// command.  Return a NULL pointer if NAME isn't a command name.
COMMAND *
find_command(char *name)
{
    register int i;

    for(i = 0; current_commands[i].name; i++)
	{
		//fprintf(stderr, "%s - %s\n", name, current_commands[i].name);
        if(strcmp(name, current_commands[i].name) == 0)
            return(&current_commands[i]);
	}

    return((COMMAND*)NULL);
}

// Strip whitespace from the start and end of STRING.  Return a pointer into STRING.
char *
stripwhite(char *string)
{
    register char *s, *t;

    for(s = string; whitespace(*s); s++)
        ;

    if(*s == 0)
        return(s);

    t = s + strlen(s) - 1;
    while(t > s && whitespace(*t))
        t--;
    *++t = '\0';

    return s;
}

//Function which tells you that you can't do this.
int
too_dangerous(char *caller)
{
    fprintf(stderr,
            "%s: Too dangerous for me to distribute.  Write it yourself.\n",
            caller);
    return 0;
}

// Return non-zero if ARG is a valid argument for CALLER, else print an error message and return zero.
int
valid_argument(char *caller, char *arg)
{
    if(!arg || !*arg)
    {
        fprintf(stderr, "%s: Argument required.\n", caller);
        return(0);
    }

    return(1);
}


/* **************************************************************** */
/*                                                                  */
/*                  Interface to Readline Completion                */
/*                                                                  */
/* **************************************************************** */

char *command_generator PARAMS((const char *, int));
char **gconsole_completion PARAMS((const char *, int, int));

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

/* Attempt to complete on the contents of TEXT.  START and END bound the
   region of rl_line_buffer that contains the word to complete.  TEXT is
   the word to complete.  We can use the entire contents of rl_line_buffer
   in case we want to do some simple parsing.  Return the array of matches,
   or NULL if there aren't any. */
char **
gconsole_completion(const char *text, int start, int end)
{
    char **matches;

    matches =(char **)NULL;

    //If this word is at the start of the line, then it is a command
    //to complete.  Otherwise it is the name of a file in the current directory.
    if(start == 0)
        matches = rl_completion_matches(text, command_generator);

    return(matches);
}

/* Generator function for command completion.  STATE lets us know whether
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
    if(!state)
    {
        list_index = 0;
        len = strlen(text);
    }

    /* Return the next name which partially matches from the command list. */
    while((name = current_commands[list_index].name) != NULL)
    {
        list_index++;

        if(strncmp(name, text, len) == 0)
            return(dupstr(name));
    }

    /* If no names matched, then return NULL. */
    return((char *)NULL);
}


/* **************************************************************** */
/*                                                                  */
/*                       gconsole commands                          */
/*                                                                  */
/* **************************************************************** */

// String to pass to system().  This is for the LIST, VIEW and RENAME commands.
static char syscom[1024];

//Print out help for ARG, or for all of the commands if ARG is not present.
int
help_handler(char *args)
{
    register int i;
    int printed = 0;

    for(i = 0; current_commands[i].name; i++)
    {
        if(args == NULL || !*args)
        {
            fprintf(stderr, "%s\t\t%s.\n", current_commands[i].name, current_commands[i].doc);
            printed++;
        }
        else if(strcmp(args, current_commands[i].name) == 0)
        {
            fprintf(stderr, "%s\t\t%s.\n", current_commands[i].name, current_commands[i].doc);
            printed++;
            break;
        }
    }

    if(printed == 0)
    {
        fprintf(stderr, "No commands match `%s'.  Possibilties are:\n", args);

        for(i = 0; current_commands[i].name; i++)
        {
            //Print in six columns.
            if(printed == 6)
            {
                printed = 0;
                printf("\n");
            }

            fprintf(stderr, "%s\t", current_commands[i].name);
            printed++;
        }

        if(printed)
            fprintf(stderr, "\n");
    }
    return(0);
}

int
quit_handler(char *args)
{
    if(gc != NULL)
    {
        fprintf(stderr, "this command cannot be used when in remote mode!\n");
        return -1;
    }
	if(current_database != NULL)
	{
		fprintf(stderr, "please unload your database before quiting!\n");
		return -1;
	}
    done = true;
    return(0);
}

int
connect_handler(char *args)
{
    if(gc != NULL)
    {
        fprintf(stderr, "this command cannot be used when in remote mode!\n");
        return -1;
    }
	if(current_database != NULL)
    {
        fprintf(stderr, "please unload your local database before entering remote mode!\n");
        return -1;
    }

    int port = 3305;
    string ip = "127.0.0.1";  //maybe "localhost"

    if(args != NULL && *args != '\0')
    {
		fprintf(stderr, "arguments specified!\n");
        //BETTER:a common parse module for string? separted with ' ', '\t' or '"'
		//BETTER:only port and only ip are supported
        int i = 0;
        while(args[i] && !whitespace(args[i]))
        {
            i++;
        }
        args[i++] = '\0';
        ip = string(args);
        while(args[i] && whitespace(args[i]))
        {
            i++;
        }
        port = atoi(args+i);
    }

    //initialize the GStore server's IP address and port.
    gc = new GstoreConnector(ip, port);
	//if(!gc->connect())
	//{
		//delete gc;
		//gc = NULL;
		//return -1;
	//}
    current_commands = remote_commands;

    return 0;
}

int
disconnect_handler(char *args)
{
    if(gc == NULL)
    {
        fprintf(stderr, "this command cannot be used when in native mode!\n");
        return -1;
    }

    if(gc->show() != "\n[empty]\n")
    {
        fprintf(stderr, "please unload your server database before entering native mode!\n");
        return -1;
    }
    delete gc;
    gc = NULL;
    current_commands = native_commands;

    return 0;
}

int
show_handler(char *args)
{
	//BETTER:show all or inuse, ls|grep "\.db" > ans.txt, as well as server
    if(gc != NULL)
    {
        string database = gc->show();
        fprintf(stderr, "%s", database.c_str());
        return 0;
    }

    //native mode
    if(current_database == NULL)
    {
        fprintf(stderr, "no database used now!\n");
    }
    else
    {
        fprintf(stderr, "%s\n", current_database->getName().c_str());
    }
    return 0;
}

int
build_handler(char *args)
{
    int i = 0;
    while(args[i] && !whitespace(args[i]))
    {
        i++;
    }
    args[i++] = '\0';
	//BETTER:the position is the root of Gstore by default
	//(or change to a specified folder later)
    string database = string(args) + string(".db");
	string dataset = string(args + i);
    while(args[i] && whitespace(args[i]))
    {
        i++;
    }

	//NOTICE: when in remote mode, the dataset should be placed in the server! And the exact path can only be got in the server
	//we can deal with it in Database
    if(gc != NULL) //remote mode
    {
        // QUERY:how to interact:string?dict(0:string)?json(service_id, service_args)?
        // Here uses the Cpp API Wrapper
        // build a new database by a RDF file.
        // note that the relative path is related to gserver.
        if(gc->build(database, dataset))
            return 0;
        else
            return -1;
    }

    //gc == NULL, native mode
	//string ret = Util::getExactPath(args + i);
	//const char *path = ret.c_str();
	//if(path == NULL)
	//{
		//fprintf(stderr, "invalid path of dataset!\n");
		//return -1;
	//}
//#ifdef DEBUG
	//fprintf(stderr, "%s;\n", path);
//#endif
    //dataset = string(path);
	//free(path);

    //system("clock");
    cout << "import dataset to build database..." << endl;
    fprintf(stderr, "DB_store: %s\tRDF_data: %s\n", database.c_str(), dataset.c_str());
    if(current_database != NULL)
	{
		delete current_database;
	}
	current_database = new Database(database);
    bool flag = current_database->build(dataset);
    //system("clock");
    if(flag)
    {
        fprintf(stderr, "import RDF file to database done.\n");
        return 0;
    }
    else
    {
        fprintf(stderr, "import RDF file to database fail.\n");
        return -1;
    }
}

int
drop_handler(char *args)
{
	//TODO:native and remote
    //NOTICE: not using databases, drop a given one at a time
    if(current_database != NULL) //how to judge when remote
    {
        fprintf(stderr, "please donot use this command when you are using a database!\n");
        return -1;
    }
	if(args == NULL || *args == '\0')
	{
		fprintf(stderr, "invalid arguments!\n");
		return -1;
	}

	char info[] = "drop";
	too_dangerous(info);
	//DEBUG:not works
    //return remove(args);
	//TODO:only drop when *.db, avoid other files be removed
	//string cmd = string("rm -rf ") + string(args);
	//fprintf(stderr, "%s\n", cmd.c_str());
	//return system(cmd.c_str());
	return 0;
}

int
load_handler(char *args)
{
    string database = string(args);
    if(gc != NULL)
    {
        gc->load(database);
    }
    else
    {
        if(current_database != NULL)
		{
            //current_database->unload();
			delete current_database;
		}
        current_database = new Database(database);
        current_database->load();
    }

    return 0;
}

int
unload_handler(char *args)
{
    //unload the using database, no args needed
    if(args != NULL && *args != '\0')
    {
        fprintf(stderr, "arguments invalid!\n");
        return -1;
    }

    if(gc != NULL)
    {
        // unload this database
        string database = gc->show();
        if(database == "\n[empty]\n")
        {
            fprintf(stderr, "no database used now!\n");
            return -1;
        }
        gc->unload(database.substr(1, database.length()-2));
    }

    //native mode
    if(current_database == NULL)
    {
        fprintf(stderr, "no database used now!\n");
    }
    else
    {
        //current_database->unload();
		delete current_database;
        current_database = NULL;
    }

    return 0;
}

int
query_handler(char *args)
{
    if(args == NULL || *args == '\0')
    {
        fprintf(stderr, "invalid arguments!\n");
        return -1;
    }
    string sparql = "";
    if(*args == '"') //query quoted in string
    {
        int i = strlen(args) - 1;
        if(i > 0 && args[i] == '"')
        {
            args[i] = '\0';
            sparql = string(args+1);
        }
        else
        {
            fprintf(stderr, "invalid arguments!\n");
            return -1;
        }
    }
    else //query in file indicated by this path
    {
		//NOTICE:the query is native, not in server!
		string ret = Util::getExactPath(args);
		const char *path = ret.c_str();
		if(path == NULL)
		{
			fprintf(stderr, "invalid path of query!\n");
			return -1;
		}
#ifdef DEBUG
		fprintf(stderr, "%s;\n", path);
#endif
        sparql = Util::getQueryFromFile(path);
		//free(path);
    }

    if(sparql.empty())
        return 0;

#ifdef DEBUG
	fprintf(stderr, "%s;\n", sparql.c_str());
#endif

    if(gc != NULL)
    {
		//QUERY:how to use query path in the server
        //execute SPARQL query on this database.
        string answer = gc->query(sparql);
        fprintf(output, "%s\n", answer.c_str());
        return 0;
    }
    else
    {
		if(current_database == NULL)
		{
			fprintf(stderr, "no database in use!\n");
			return -1;
		}
        ResultSet rs;
        bool ret = current_database->query(sparql, rs, output);
        if(ret)
		{
#ifdef DEBUG
			fprintf(stderr, "query() returns true!\n"); 
#endif
			return 0;
		}
        else
		{ 
#ifdef DEBUG
			fprintf(stderr, "query() returns false!\n"); 
#endif
            return -1;
		}
    }
}

int
list_handler(char *args)
{
	char info[] = "";
    if(!args)
        args = info;

    sprintf(syscom, "ls -FClg %s", args);
    return(system(syscom));
}

int
view_handler(char *args)
{
	char info[] = "view";
    if(!valid_argument(info, args))
        return 1;

#if defined(__MSDOS__)
    //more.com doesn't grok slashes in pathnames
    sprintf(syscom, "less %s", args);
#else
    sprintf(syscom, "more %s", args);
#endif
    return(system(syscom));
}

int
rename_handler(char *args)
{
	char info[] = "rename";
    too_dangerous(info);
    return(1);
}

int
stat_handler(char *args)
{
    struct stat finfo;
	char info[] = "stat";

    if(!valid_argument(info, args))
        return(1);

    if(stat(args, &finfo) == -1)
    {
        perror(args);
        return(1);
    }

    printf("Statistics for `%s':\n", args);

    printf("%s has %d link%s, and is %d byte%s in length.\n",
           args,
           (int)finfo.st_nlink,
           (finfo.st_nlink == 1) ? "" : "s",
           (int)finfo.st_size,
           (finfo.st_size == 1) ? "" : "s");
    printf("Inode Last Change at: %s", ctime(&finfo.st_ctime));
    printf("      Last access at: %s", ctime(&finfo.st_atime));
    printf("    Last modified at: %s", ctime(&finfo.st_mtime));
    return(0);
}

int
delete_handler(char *args)
{
	char info[] = "delete";
    too_dangerous(info);
    return(1);
}

int
cd_handler(char *args)
{
    if(chdir(args) == -1)
    {
        perror(args);
        return 1;
    }

	char info[] = "";
    pwd_handler(info);
    return(0);
}

int
pwd_handler(char *args)
{
    // no need for args
    if(args != NULL && *args != '\0')
    {
        fprintf(stderr, "invalid arguments!\n");
        return -1;
    }
    char dir[1024], *s;

    s = getcwd(dir, sizeof(dir) - 1);
    if(s == 0)
    {
        printf("Error getting pwd: %s\n", dir);
        return 1;
    }

    printf("Current directory is %s\n", dir);
    return 0;
}

