/**
* @file  Server.cpp
* @author  suxunbin
* @date  12-AUG-2021
* @brief  a gStore socket server
*/

#include "Server.h"


using namespace std;

bool _stop = false; /**< A stopServer flag. */

Server::Server(int _port)
{
	this->connectionPort = _port;
	this->connectionMaxNum = Socket::MAX_CONNECTIONS;
	this->databaseMaxNum = 10;
	this->db_home = GlobalTypedef::db_home();
	this->db_suffix = GlobalTypedef::db_suffix();
}

Server::~Server()
{

}

bool
Server::createConnection()
{
	bool flag;

	flag = this->socket.create();
	if (!flag)
	{
		cerr << gs::TimeUtil::now(NORM_DATETIME_PATTERN) << "Cannot create socket. @Server::createConnection" << endl;
		return false;
	}

	flag = this->socket.bind(this->connectionPort);
	if (!flag)
	{
		cerr << gs::TimeUtil::now(NORM_DATETIME_PATTERN) << "Cannot bind to port " << this->connectionPort << ". @Server::createConnection" << endl;
		return false;
	}

	flag = this->socket.listen();
	if (!flag)
	{
		cerr << gs::TimeUtil::now(NORM_DATETIME_PATTERN) << "Cannot listen to port" << this->connectionPort << ". @Server::createConnection" << endl;
		return false;
	}

	return true;
}

bool
Server::deleteConnection()
{
	bool flag = this->socket.close();

	return flag;
}

bool
Server::response(int _code, std::string _msg, Socket& _socket)
{
	std::string resJson = "{\"code\":" + std::to_string(_code) + ",\"msg\":\"" + _msg + "\"}";
	bool flag = _socket.send(resJson);
	return flag;
}

/**
* @brief A socket thread class
*/
class sockThread
{
public:
	std::thread TD;
	int tid; /**< A thread id. */
	static int Threadnum; /**< A thread counter. */
	Socket socket; /**< A client socket. */
	Server* server;
	/**
	* @brief A constructor taking an argument.
	* @param[in]  _socket : A client socket
	*/
	sockThread(Socket& _socket);
	/** @brief A default destructor. */
	~sockThread();
	/**
	* @brief Get the thread id.
	* @return The thread id.
	*/
	int GetThreadID();
	/**
	* @brief A thread handling function.
	*/
	void run();
	/**
	* @brief Start the thread.
	*/
	void start();
};

int sockThread::Threadnum = 0;

sockThread::sockThread(Socket& _socket)
{
	Threadnum++;
	tid = Threadnum;
	socket = _socket;
}
sockThread::~sockThread()
{

}
int sockThread::GetThreadID()
{
	return tid;
}
void sockThread::run()
{
	cout << gs::TimeUtil::now(NORM_DATETIME_PATTERN) << "Thread:" << tid << " run\n";
	server->handler(socket);
}
void sockThread::start()
{
	TD = std::thread(&sockThread::run, this);
	TD.detach();
}

void
Server::handler(Socket& _socket)
{
	int repeated_num = 0;
	while (true)
	{
		if (repeated_num > 10)
			break;

		/**
		* @brief Receive the command message from the client.
		*/
		std::string recv_cmd;
		bool recv_return = _socket.recv(recv_cmd);
		if (!recv_return)
		{
			cerr << gs::TimeUtil::now(NORM_DATETIME_PATTERN) << "Receive command from client error. @Server::listen" << endl;
			repeated_num++;
			continue;
		}

		cout << gs::TimeUtil::now(NORM_DATETIME_PATTERN) << "Received msg: " << recv_cmd << endl;

		/**
		* @brief Parse the command message and construct an operation.
		*/
		Operation operation;
		bool parser_return = this->parser(recv_cmd, operation);
		cout << gs::TimeUtil::now(NORM_DATETIME_PATTERN) << "Parser_return=" << parser_return << endl; //debug
		if (!parser_return)
		{
			cout << gs::TimeUtil::now(NORM_DATETIME_PATTERN) << "Parser command error. @Server::listen" << endl;
			std::string error = "Invalid command.";
			this->response(1001, error, _socket);
			repeated_num++;
			continue;
		}

		/**
		* @brief Execute the specific command function.
		*/
		std::string ret_msg;
		CommandType cmd_type = operation.getCommand();
		bool _close = false;
		switch (cmd_type)
		{
		case CMD_TEST:
		{
			ret_msg = "OK";
			break;
		}
		case CMD_LOGIN:
		{
			std::string username = operation.getParameter("username");
			std::string password = operation.getParameter("password");
			this->login(username, password, _socket);
			break;
		}
		case CMD_LOAD:
		{
			std::string db_name = operation.getParameter("db_name");
			if (operation.getParameter("csr") == "1") {
				this->load(db_name, _socket, true);
			}
			else
				this->load(db_name, _socket, false);
			break;
		}
		case CMD_UNLOAD:
		{
			std::string db_name = operation.getParameter("db_name");
			this->unload(db_name, _socket);
			break;
		}

		case CMD_BUILD:
		{
			std::string db_name = operation.getParameter("db_name");
			std::string db_path = operation.getParameter("db_path");
			this->build(db_name, db_path, _socket);
			break;
		}
		case CMD_DROP:
		{
			std::string db_name = operation.getParameter("db_name");
			this->drop(db_name, _socket);
			break;
		}

		case CMD_QUERY:
		{
			std::string db_name = operation.getParameter("db_name");
			std::string sparql = operation.getParameter("sparql");
			std::string format = "json";
			if (operation.getParameter("format") == "file") {
				format = "file";
			}
			else if (operation.getParameter("format") == "json+file") {
				format = "json+file";
			}

			size_t pos = 0;
			while ((pos = sparql.find("\\r\\n", pos)) != std::string::npos) {
				sparql.replace(pos, 4, " ");
				pos += 1;
			}

			pos = 0;
			while ((pos = sparql.find("\\n", pos)) != std::string::npos) {
				sparql.replace(pos, 2, " ");
				pos += 1;
			}

			pos = 0;
			while ((pos = sparql.find("\\'", pos)) != std::string::npos) {
				sparql.replace(pos, 2, "\"");
				pos += 1;
			}

			pthread_t timer = Server::start_timer();
			if (timer == 0) {
				cerr << gs::TimeUtil::now(NORM_DATETIME_PATTERN) << "Failed to start timer." << endl;
			}
			this->query(db_name, sparql, format, _socket);
			if (timer != 0 && !Server::stop_timer(timer)) {
				cerr << gs::TimeUtil::now(NORM_DATETIME_PATTERN) << "Failed to stop timer." << endl;
			}
			break;
		}

		case CMD_SHOW:
		{
			this->show(_socket);
			break;
		}
		case CMD_STOP:
		{
			this->stopServer(_socket);
			_stop = true;
			_close = true;
			break;
		}
		case CMD_CLOSE:
		{
			this->closeConnection(_socket);
			_close = true;
			break;
		}

		default:
		{
			cerr << gs::TimeUtil::now(NORM_DATETIME_PATTERN) << "This command is not supported by now. @Server::listen" << endl;
			std::string error = "Invalid command.";
			this->response(1001, error, _socket);
		}
		}
		if (_close)
			break;
		repeated_num = 0;
	}
	/**
	* @brief Disconnect from the client.
	*/
	if (logins.find(_socket.username) != logins.end())
		logins.erase(_socket.username);
	_socket.username = "";
	_socket.password = "";
	_socket.close();
	/**
	* @brief Stop the server.
	*/
	if (_stop)
		kill(getpid(), SIGTERM);
}

void
Server::init()
{
	// TODO: init the server
}

void
Server::listen()
{
	this->init();
	Socket soc[this->connectionMaxNum];
	int i = 0;
	Socket new_server_socket;
	while (true)
	{
		/**
		* @brief Receive the stopServer signal.
		*/
		signal(SIGTERM, Server::stop_sigterm_handler);

		cout << gs::TimeUtil::now(NORM_DATETIME_PATTERN) << "Wait for connection..." << endl;

		this->socket.accept(new_server_socket);

		cout << gs::TimeUtil::now(NORM_DATETIME_PATTERN) << "Accept a new socket connection." << endl;

		/**
		* @brief Create a thread for a client socket.
		*/
		soc[i] = new_server_socket;
		// memcpy(&soc[i], &new_server_socket, sizeof(Socket));
		sockThread* tid = new sockThread(soc[i++]);
		tid->server = this;
		tid->start();
	}
}

std::string Server::checkparamValue(std::string param, std::string value)
{
	std::string result = "";
	if (value.empty())
	{
		result = "The value of " + param + " can not be empty!";
		return result;
	}
	if (param == "db_name")
	{
		std::string database = value;
		if (database == GlobalTypedef::system_db)
		{
			result = "You can not operate the system database.";
			return result;
		}
		size_t len_suffix = db_suffix.length();
		if (database.length() > len_suffix && database.substr(database.length() - len_suffix, len_suffix) == db_suffix)
		{
			result = "Your db_name to be built should not end with \""+db_suffix+"\".";
			return result;
		}
	}
	if (param == "db_path")
	{
		std::string path = value;
		if (path == SYSTEM_PATH)
		{
			result = "You can not operate the system files.";
			return result;
		}
	}
	return "";
}

bool Server::checkdbexist(std::string _db_name)
{
	bool result = true;
	std::map<std::string, int>::iterator it = localDBs.find(_db_name);
	if (it == localDBs.end())
		result = false;
	return result;
}

bool Server::checkdbload(std::string _db_name)
{
	bool result = true;
	std::map<std::string, Database*>::iterator it = databases.find(_db_name);
	if (it == databases.end())
		result = false;
	return result;
}

bool
Server::parser(std::string _raw_cmd, Operation& _ret_oprt)
{
	/**
	* @brief Check if the command is a valid JSON string.
	*/
	if (JsonUtil::accept(_raw_cmd) == false)
		return false;

	/**
	* @brief Delete the extra space.
	*/
	int para_start_pos = 0;
	int raw_len = (int)_raw_cmd.size();

	for (int i = 0; i < raw_len; i++) {
		if (_raw_cmd[i] == '\n') {
			_raw_cmd[i] = ' ';
		}
	}

	for (int i = 0; i < raw_len-1; i++) {
		if (_raw_cmd[i] == '\\' && _raw_cmd[i+1] == '"') {
			_raw_cmd[i] = '\\';
			_raw_cmd[i + 1] = '\'';
		}
	}

	while (para_start_pos < raw_len && _raw_cmd[para_start_pos] == ' ') {
		para_start_pos++;
	}
	if (para_start_pos == raw_len)
		return false;

	std::unordered_map<std::string, std::string> paras;
	int para_end_pos;
	std::vector<std::string> para_vec;

	/**
	* @brief Get all parameters.
	*/
	while (true)
	{
		if (_raw_cmd[para_start_pos] == '"')
		{
			para_start_pos++;
			para_end_pos = para_start_pos;
			while (true)
			{
				if (_raw_cmd[para_end_pos] == '"')
					break;
				para_end_pos++;
			}
			std::string para = _raw_cmd.substr(para_start_pos, para_end_pos - para_start_pos);
			para_vec.push_back(para);
			para_start_pos = para_end_pos;
		}
		para_start_pos++;
		if (_raw_cmd[para_start_pos] == '}')
			break;
	}
	if (para_vec.size() % 2 == 1)
		return false;
	std::string cmd = "";
	for (unsigned i = 0; i < para_vec.size(); i += 2)
	{
		if (para_vec[i] == "op")
			cmd = para_vec[i + 1];
		paras.insert(pair<std::string, std::string>(para_vec[i], para_vec[i + 1]));
	}

	/**
	* @brief Check if the parameters are valid.
	*/
	if (cmd == "")
		return false;
	int para_num = paras.size();

	if (cmd == "test") {
		_ret_oprt.setCommand(CMD_TEST);
	}
	if (cmd == "login") {
		_ret_oprt.setCommand(CMD_LOGIN);
		if (para_num != 3)
			return false;
		if ((paras.find("username") == paras.end()) || (paras.find("password") == paras.end()))
			return false;
	}
	else if (cmd == "build") {
		_ret_oprt.setCommand(CMD_BUILD);
		if (para_num != 3)
			return false;
		if ((paras.find("db_name") == paras.end()) || (paras.find("db_path") == paras.end()))
			return false;
	}
	else if (cmd == "load") {
		_ret_oprt.setCommand(CMD_LOAD);
		if (para_num == 3) {
			if ((paras.find("db_name") == paras.end())||(paras.find("csr") == paras.end()))
				return false;
		}
		else if (para_num == 2) {
			if (paras.find("db_name") == paras.end())
				return false;
			paras["csr"] = "0";
			_ret_oprt.setParameter(paras);
		}
		else
			return false;
	}
	else if (cmd == "unload") {
		_ret_oprt.setCommand(CMD_UNLOAD);
		if (para_num != 2)
			return false;
		if (paras.find("db_name") == paras.end())
			return false;
	}
	else if (cmd == "query") {
		_ret_oprt.setCommand(CMD_QUERY);
		if (para_num == 4) {
			if ((paras.find("db_name") == paras.end()) || (paras.find("sparql") == paras.end()) || (paras.find("format") == paras.end()))
				return false;
		}
		else if (para_num == 3) {
			if ((paras.find("db_name") == paras.end()) || (paras.find("sparql") == paras.end()))
				return false;
			paras["format"] = "json";
			_ret_oprt.setParameter(paras);
		}
		else
			return false;
	}
	else if (cmd == "show") {
		_ret_oprt.setCommand(CMD_SHOW);
		if (para_num != 1)
			return false;
	}
	else if (cmd == "stop") {
		_ret_oprt.setCommand(CMD_STOP);
		if (para_num != 1)
			return false;
	}
	else if (cmd == "close") {
		_ret_oprt.setCommand(CMD_CLOSE);
		if (para_num != 1)
			return false;
	}
	else if (cmd == "drop") {
		_ret_oprt.setCommand(CMD_DROP);
		if (para_num != 2)
			return false;
		if (paras.find("db_name") == paras.end())
			return false;
	}
	else {
		return false;
	}
	_ret_oprt.setParameter(paras);

	return true;
}

bool
Server::drop(std::string _db_name, Socket& _socket)
{
	// TODO: call ApiProvider
	string msg = "success";
	_socket.send(msg);
	return true;
}

bool
Server::login(std::string _username, std::string _password, Socket& _socket)
{
	// TODO: call ApiProvider
	string msg = "success";
	_socket.send(msg);
	return true;
}

bool
Server::load(std::string _db_name, Socket& _socket, bool load_csr)
{
	// TODO: call ApiProvider
	string msg = "success";
	_socket.send(msg);
	return true;
}

bool
Server::unload(std::string _db_name, Socket& _socket)
{
	// TODO: call ApiProvider
	string msg = "success";
	_socket.send(msg);
	return true;
}

bool
Server::build(std::string _db_name, std::string _db_path, Socket& _socket)
{
	// TODO: call ApiProvider
	string msg = "success";
	_socket.send(msg);
	return true;
}

bool
Server::query(std::string _db_name, std::string _sparql, std::string format, Socket& _socket)
{
	// TODO: call ApiProvider
	string msg = "success";
	_socket.send(msg);
	return true;
}

bool
Server::show(Socket& _socket)
{
	// TODO: call ApiProvider
	string msg = "success";
	_socket.send(msg);
	return true;
}

bool Server::stopServer(Socket& _socket)
{
	/**
	* @brief Check if the client logins.
	*/
	if (logins.find(_socket.username) == logins.end())
	{
		std::string error = "Need to login first.";
		this->response(1001, error, _socket);
		return false;
	}

	/**
	* @brief Check if the client is the root user.
	*/
	if (_socket.username != "root")
	{
		std::string error = "You have no rights to stop the server.";
		this->response(1002, error, _socket);
		return false;
	}

	std::map<std::string, Database*>::iterator iter;
	for (iter = databases.begin(); iter != databases.end(); iter++)
	{
		delete iter->second;
		iter->second = NULL;
	}
	databases.clear();
	users.clear();
	localDBs.clear();
	logins.clear();

	std::string success = "Server stopped.";
	this->response(0, success, _socket);

	return true;
}

bool Server::closeConnection(Socket& _socket) {
	std::string success = "Connection disconnected.";
		this->response(0, success, _socket);
	return true;
}

pthread_t Server::start_timer() {
	pthread_t timer_thread;
	if (pthread_create(&timer_thread, NULL, Server::timer, NULL) == 0) {
		return timer_thread;
	}
	return 0;
}

bool Server::stop_timer(pthread_t _timer) {
	return pthread_kill(_timer, SIGTERM) == 0;
}

void* Server::timer(void* _args) {
	/**
	* @brief Receive the stop timer signal.
	*/
	signal(SIGTERM, Server::timer_sigterm_handler);
	sleep(Util::gserver_query_timeout);
	cerr << gs::TimeUtil::now(NORM_DATETIME_PATTERN) << "Query out of time." << endl;
	abort();
}

void Server::timer_sigterm_handler(int _signal_num) {
	pthread_exit(0);
}

void Server::stop_sigterm_handler(int _signal_num) {
	cout << gs::TimeUtil::now(NORM_DATETIME_PATTERN) << "Server stopped." << endl;
	exit(_signal_num);
}

void Server::dirTraversal(const char* _dir_name, std::vector<std::string>& _filename)
{
	if (_dir_name == NULL)
	{
		std::cout << "dir_name is NULL ! " << std::endl;
		return;
	}
	struct stat s;
	lstat(_dir_name, &s);
	if (!S_ISDIR(s.st_mode))
	{
		std::cout << "dir_name is not a valid directory ! " << std::endl;
		return;
	}
	struct dirent* filename;
	DIR* dir;
	dir = opendir(_dir_name);
	if (dir == NULL)
	{
		std::cout << "Can not open directory " << _dir_name << std::endl;
		return;
	}
	while ((filename = readdir(dir)) != NULL)
	{
		if (strcmp(filename->d_name, ".") == 0 ||
			strcmp(filename->d_name, "..") == 0)
			continue;
		_filename.push_back(filename->d_name);
	}
	return;
}