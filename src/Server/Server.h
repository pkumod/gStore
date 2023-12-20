/**
* @file  Server.h
* @author  suxunbin
* @date  12-AUG-2021
* @brief  a gStore socket server
*/

#ifndef _SERVER_SERVER_H
#define _SERVER_SERVER_H

#include "../Util/Util.h"
#include "../Database/Database.h"
#include "Socket.h"
#include "Operation.h"

#define SYSTEM_PATH "data/system/system.nt"

class Server
{
public:
	/**
	* @brief A constructor taking an argument.
	* @param[in]  _port : A port
	*/
	Server(int _port = Socket::DEFAULT_CONNECT_PORT);

	/** @brief A default destructor. */
	~Server();

	/**
	* @brief Create a server socket.
	* @return
	*	true : Create the socket successfully. \n
	*	false : Create the socket failed. \n
	*/
	bool createConnection();

	/**
	* @brief Close the server socket.
	* @return
	*	true : Close the socket successfully. \n
	*	false : Close the socket failed. \n
	*/
	bool deleteConnection();

	/**
	* @brief Handle all kinds of command types.
	* @param[in]  _socket : A client socket
	*/
	void handler(Socket& _socket);

	/**
	* @brief The server listens connection requests from clients.
	*/
	void listen();

	/**
	* @brief Import the system database to initialize.
	*/
	void init();

	/**
	* @brief Send messages to the client.
	* @param[in]  _code : StatusCode value
	* @param[in]  _msg : StatusMsg value
	* @param[in]  _socket : A client socket
	* @return
	*	true : Send the message successfully. \n
	*	false : Send the message failed. \n
	*/
	bool response(int _code, std::string _msg, Socket& _socket);

	/**
	* @brief Check if the param is legal or not.
	* @param[in] param: The param name
	* @param[in] value: The value of param.
	* @return A status message.
	*/
	std::string checkparamValue(std::string param, std::string value);

	/**
	* @brief Check if the database exists or not.
	* @param[in] _db_name: A database name.
	* @return
	*	true : The database exists. \n
	*	false : The database does not exist. \n
	*/
	bool checkdbexist(std::string _db_name);

	/**
	* @brief Check if the database is loaded or not.
	* @param[in] _db_name: A database name.
	* @return
	*	true : The database is loaded. \n
	*	false : The database is not loaded. \n
	*/
	bool checkdbload(std::string _db_name);

	/**
	* @brief Parse the command from the client.
	* @param[in]  _raw_cmd : A raw command from the client
	* @param[out]  _ret_oprt : An operation
	* @return
	*	true : Parse the command successfully. \n
	*	false : Parse the command failed. \n
	*/
	bool parser(std::string _raw_cmd, Operation& _ret_oprt);

	/**
	* @brief Drop the database named [db_name].
	* @param[in]  _db_name : A database name
	* @param[in]  socket : A client socket
	* @return
	*	true : Drop the database successfully. \n
	*	false : Drop the database failed. \n
	*/
	bool drop(std::string _db_name, Socket& socket);

	/**
	* @brief A client logins the server.
	* @param[in]  _username : A client username
	* @param[in]  _password : A client password
	* @param[in]  socket : A client socket
	* @return
	*	true : Login successfully. \n
	*	false : Login failed. \n
	*/
	bool login(std::string _username, std::string _password, Socket& socket);

	/**
	* @brief Load the database named [db_name].
	* @param[in]  _db_name : A database name
	* @param[in]  socket : A client socket
	* @return
	*	true : Load the database successfully. \n
	*	false : Load the database failed. \n
	*/
	bool load(std::string _db_name, Socket& _socket, bool load_csr);

	/**
	* @brief Unload the database named [db_name].
	* @param[in]  _db_name : A database name
	* @param[in]  socket : A client socket
	* @return
	*	true : Unload the database successfully. \n
	*	false : Unload the database failed. \n
	*/
	bool unload(std::string _db_name, Socket& socket);

	/**
	* @brief Show all the databases.
	* @param[in]  socket : A client socket
	* @return
	*	true : Show the databases successfully. \n
	*	false : Show the databases failed. \n
	*/
	bool show(Socket& socket);

	/**
	* @brief Build a database named [db_name].
	* @param[in]  _db_name : A database name
	* @param[in]  _db_path : The rdf file path
	* @param[in]  socket : A client socket
	* @return
	*	true : Build the database successfully. \n
	*	false : Build the database failed. \n
	*/
	bool build(std::string _db_name, std::string _db_path, Socket& socket);

	/**
	* @brief Query a sparql in a database named [db_name].
	* @param[in]  _db_name : A database name
	* @param[in]  _sparql : A sparql query
	* @param[in]  socket : A client socket
	* @return
	*	true : Query the sparql successfully. \n
	*	false : Query the sparql failed. \n
	*/
	bool query(std::string _db_name, std::string _sparql, std::string format, Socket& _socket);

	/**
	* @brief Handle before stopping the server.
	* @param[in]  socket : A client socket
	* @return
	*	true : Stop the server successfully. \n
	*	false : Stop the server failed. \n
	*/
	bool stopServer(Socket& socket);

	/**
	* @brief Close a client socket.
	* @param[in]  socket : A client socket
	* @return
	*	true : Close the socket successfully. \n
	*	false : Close the socket failed. \n
	*/
	bool closeConnection(Socket& socket);

private:
	int connectionPort; /**< The port for clients to connect. */
	int connectionMaxNum; /**< The maximum connections of the server. */
	int databaseMaxNum; /**< The maximum loaded database number of the server. */
	Socket socket; /**< The server socket. */
	std::map<std::string, int> localDBs; /**< The list of all built databases. */
	std::map<std::string, std::string> users; /**< The list of all users. */
	std::map<std::string, Database*> databases; /**< The list of all loaded databases. */
	std::map<std::string, int> logins; /**< The list of all users who login. */
	std::string db_home; /**< The root directory. */
	std::string db_suffix; /**< The database suffix. */
	Database* system_database; /**< The sytsem database. */
	std::string CoreVersion; /**< The version of gStore core. */
	std::string APIVersion; /**< The version of API interface. */

	/**
	* @brief Create a timer pthread.
	* @return A timer pthread.
	*/
	static pthread_t start_timer();

	/**
	* @brief Stop the timer pthread.
	* @param[in]  _timer : The timer pthread
	* @return
	*	true : Stop the pthread successfully. \n
	*	false : Stop the pthread failed. \n
	*/
	static bool stop_timer(pthread_t _timer);

	/**
	* @brief A query timeout handler.
	* @param[in]  _args : The thread arguments
	*/
	static void* timer(void* _args);

	/**
	* @brief Handle a stop timer signal.
	* @param[in]  _signal_num : A signal
	*/
	static void timer_sigterm_handler(int _signal_num);

	/**
	* @brief Handle a stopServer signal.
	* @param[in]  _signal_num : A signal
	*/
	static void stop_sigterm_handler(int _signal_num);

	/**
	* @brief Traverse a directory and get all filenames.
	* @param[in]  _dir_name : A directory name
	* @param[out]  _filename : A filename list of the directory
	*/
	static void dirTraversal(const char* _dir_name, std::vector<std::string>& _filename);

	/**
	* @brief Query the system database.
	* @param[in]  _sparql : A sparql query
	* @param[out]  _res : Query results
	* @return
	*	true : Query the system database successfully. \n
	*	false : Query the system database failed. \n
	*/
	bool querySys(std::string _sparql, std::string& _res);

	/**
	* @brief Import the built database list and the user list from sytsem.db.
	*/
	void importSys();

	/**
	* @brief Create a response in a JSON format.
	* @param[in]  StatusCode
	* @param[in]  StatusMsg
	* @param[in]  _body_flag
	* @param[in]  ResponseBody : Response context
	* @return A response in a JSON format.
	*/
	std::string CreateJson(int StatusCode, std::string StatusMsg, bool _body_flag, std::string ResponseBody = "response");
};

#endif /* _SERVER_SERVER_H */