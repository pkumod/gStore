/**
* @file  Server.cpp
* @author  suxunbin
* @date  12-AUG-2021
* @brief  a gStore socket server
*/

#include "Server.h"

using namespace rapidjson;
using namespace std;

bool _stop = false; /**< A stopServer flag. */

Server::Server(int _port)
{
	this->connectionPort = _port;
	this->connectionMaxNum = Socket::MAX_CONNECTIONS;
	this->databaseMaxNum = 10;
	this->db_home = Util::global_config["db_home"];
	this->db_suffix = Util::global_config["db_suffix"];
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
		cerr << Util::getTimeString() << "Cannot create socket. @Server::createConnection" << endl;
		return false;
	}

	flag = this->socket.bind(this->connectionPort);
	if (!flag)
	{
		cerr << Util::getTimeString() << "Cannot bind to port " << this->connectionPort << ". @Server::createConnection" << endl;
		return false;
	}

	flag = this->socket.listen();
	if (!flag)
	{
		cerr << Util::getTimeString() << "Cannot listen to port" << this->connectionPort << ". @Server::createConnection" << endl;
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
	std::string resJson = CreateJson(_code, _msg, 0);
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
	cout << Util::getTimeString() << "Thread:" << tid << " run\n";
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
			cerr << Util::getTimeString() << "Receive command from client error. @Server::listen" << endl;
			repeated_num++;
			continue;
		}

		cout << Util::getTimeString() << "Received msg: " << recv_cmd << endl;

		/**
		* @brief Parse the command message and construct an operation.
		*/
		Operation operation;
		bool parser_return = this->parser(recv_cmd, operation);
		cout << Util::getTimeString() << "Parser_return=" << parser_return << endl; //debug
		if (!parser_return)
		{
			cout << Util::getTimeString() << "Parser command error. @Server::listen" << endl;
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
				cerr << Util::getTimeString() << "Failed to start timer." << endl;
			}
			this->query(db_name, sparql, format, _socket);
			if (timer != 0 && !Server::stop_timer(timer)) {
				cerr << Util::getTimeString() << "Failed to stop timer." << endl;
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
			cerr << Util::getTimeString() << "This command is not supported by now. @Server::listen" << endl;
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
	/**
	* @brief Load the system database.
	*/
	if (access((db_home + "/system" + db_suffix).c_str(), 00) != 0)
	{
		cerr << Util::getTimeString() << "Can not find system"+db_suffix+"." << endl;
		return;
	}
	localDBs.insert(pair<std::string, int>("system", 1));
	system_database = new Database("system");
	bool flag = system_database->load();
	if (!flag)
	{
		cerr << Util::getTimeString() << "Failed to load the database system.db." << endl;
		delete system_database;
		system_database = NULL;
		return;
	}
	databases.insert(pair<std::string, Database*>("system", system_database));

	importSys();
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

		cout << Util::getTimeString() << "Wait for connection..." << endl;

		this->socket.accept(new_server_socket);

		cout << Util::getTimeString() << "Accept a new socket connection." << endl;

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
		if (database == "system")
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
	Document document;
	document.Parse(_raw_cmd.c_str());
	if (document.HasParseError())
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
	* @brief Check if the database name is legal.
	*/
	std::string result = checkparamValue("db_name", _db_name);
	if (result.empty() == false)
	{
		this->response(1003, result, _socket);
		return false;
	}

	/**
	* @brief Check if database named [db_name] exists.
	*/
	if (!this->checkdbexist(_db_name))
	{
		std::string error = "Database not built yet.";
		this->response(1004, error, _socket);
		return false;
	}

	/**
	* @brief Check if database named [db_name] is already loaded.
	*/
	if (this->checkdbload(_db_name))
	{
		std::string error = "Need to unload database first.";
		this->response(1004, error, _socket);
		return false;
	}

	std::string cmd = "rm -rf " + db_home + "/" + _db_name + db_suffix;
	int ret = system(cmd.c_str());
	if (ret == 0) {
		localDBs.erase(_db_name);
		std::string success = "Drop database done.";
		this->response(0, success, _socket);
		return true;
	}
	else {
		std::string error = "Drop database failed.";
		this->response(1005, error, _socket);
		return false;
	}
}

bool
Server::login(std::string _username, std::string _password, Socket& _socket)
{
	/**
	* @brief Check if the client's username and password is right.
	*/
	std::map<std::string, std::string>::iterator iter = users.find(_username);
	if (iter == users.end())
	{
		std::string error = "username not find.";
		this->response(903, error, _socket);
		return false;
	}
	else if (iter->second != _password)
	{
		std::string error = "wrong password.";
		this->response(902, error, _socket);
		return false;
	}
	logins.insert(pair<std::string, int>(_username, 1));

	std::string success = "Login successfully.";
	this->response(0, success, _socket);
	_socket.username = _username;
	_socket.password = _password;

	return true;
}

bool
Server::load(std::string _db_name, Socket& _socket, bool load_csr)
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
	* @brief Check if the database name is legal.
	*/
	std::string result = checkparamValue("db_name", _db_name);
	if (result.empty() == false)
	{
		this->response(1003, result, _socket);
		return false;
	}

	/**
	* @brief Check if database named [db_name] exists.
	*/
	if (!this->checkdbexist(_db_name))
	{
		std::string error = "Database not built yet.";
		this->response(1004, error, _socket);
		return false;
	}

	/**
	* @brief Check if database named [db_name] is already loaded.
	*/
	if (this->checkdbload(_db_name))
	{
		std::string error = "Database already load.";
		this->response(0, error, _socket);
		return false;
	}

	Database* database = new Database(_db_name);
	bool flag = database->load(load_csr);

	if (!flag)
	{
		std::string error = "Failed to load the database.";
		this->response(1005, error, _socket);
		delete database;
		database = NULL;
		return false;
	}

	databases.insert(pair<std::string, Database*>(_db_name, database));

	Document resDoc;
	Document::AllocatorType& allocator = resDoc.GetAllocator();
	resDoc.SetObject();
	resDoc.AddMember("StatusCode", 0, allocator);
	resDoc.AddMember("StatusMsg", "Load database successfully.", allocator);
	if (load_csr) {
		resDoc.AddMember("csr", "1", allocator);
	}
	else {
		resDoc.AddMember("csr", "0", allocator);
	}
	StringBuffer resBuffer;
	PrettyWriter<StringBuffer> resWriter(resBuffer);
	resDoc.Accept(resWriter);
	std::string resJson = resBuffer.GetString();
	_socket.send(resJson);

	return true;
}

bool
Server::unload(std::string _db_name, Socket& _socket)
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
	* @brief Check if the database name is legal.
	*/
	std::string result = checkparamValue("db_name", _db_name);
	if (result.empty() == false)
	{
		this->response(1003, result, _socket);
		return false;
	}

	/**
	* @brief Check if database named [db_name] exists.
	*/
	if (!this->checkdbexist(_db_name))
	{
		std::string error = "Database not built yet.";
		this->response(1004, error, _socket);
		return false;
	}

	/**
	* @brief Check if database named [db_name] is already unloaded.
	*/
	std::map<std::string, Database*>::iterator iter = databases.find(_db_name);
	if (iter == databases.end())
	{
		std::string error = "Database: " + _db_name + " is not loaded yet.";
		this->response(0, error, _socket);
		return false;
	}

	Database* database = iter->second;
	delete database;
	database = NULL;
	databases.erase(_db_name);;

	std::string success = "Unload database done.";
	this->response(0, success, _socket);

	return true;
}

bool
Server::build(std::string _db_name, std::string _db_path, Socket& _socket)
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
	* @brief Check if the database name is legal.
	*/
	std::string result = checkparamValue("db_name", _db_name);
	if (result.empty() == false)
	{
		this->response(1003, result, _socket);
		return false;
	}

	/**
	* @brief Check if the rdf file path is legal.
	*/
	result = checkparamValue("db_path", _db_path);
	if (result.empty() == false)
	{
		this->response(1003, result, _socket);
		return false;
	}

	/**
	* @brief Check if database named [db_name] is already built.
	*/
	if (this->checkdbexist(_db_name))
	{
		std::string error = "Database already built.";
		this->response(1004, error, _socket);
		return false;
	}

	cout << "Import dataset to build database..." << endl;
	cout << "DB_store: " << _db_name << "\tRDF_data: " << _db_path << endl;

	Database* database = new Database(_db_name);
	bool flag = database->build(_db_path, _socket);
	delete database;
	database = NULL;

	/**
	* @brief Build the database failed.
	*/
	if (!flag)
	{
		std::string error = "Import RDF file to database failed.";
		this->response(1005, error, _socket);
		std::string cmd = "rm -rf " + db_home + "/" + _db_name + db_suffix;
		system(cmd.c_str());
		return false;
	}

	/**
	* @brief Create a success flag file.
	*/
	ofstream fsuc;
	fsuc.open(db_home + "/" + _db_name + db_suffix + "/success.txt");
	fsuc.close();

	localDBs.insert(pair<std::string, int>(_db_name, 1));
	std::string success = "Import RDF file to database done.";
	this->response(0, success, _socket);

	return true;
}

bool
Server::query(std::string _db_name, std::string _sparql, std::string format, Socket& _socket)
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
	* @brief Check if the database name is legal.
	*/
	std::string result = checkparamValue("db_name", _db_name);
	if (result.empty() == false)
	{
		this->response(1003, result, _socket);
		return false;
	}

	/**
	* @brief Check if the sparql query is legal.
	*/
	result = checkparamValue("sparql", _sparql);
	if (result.empty() == false)
	{
		this->response(1003, result, _socket);
		return false;
	}

	/**
	* @brief Check if database named [db_name] is already loaded.
	*/
	std::map<std::string, Database*>::iterator iter = databases.find(_db_name);
	if (iter == databases.end())
	{
		std::string error = "Need to load database first.";
		this->response(1004, error, _socket);
		return false;
	}

	Database* database = iter->second;

	FILE* output = NULL;
	ResultSet res_set;
	std::string _ret_msg;
	int ret_val = database->query(_sparql, res_set, output);
	if (output != NULL)
		fclose(output);

	/**
	* @brief Select query.
	*/
	if (ret_val < -1)
	{
		if (ret_val == -100)
		{
// #ifdef SERVER_SEND_JSON
			if (format == "json") {
				_ret_msg = res_set.to_JSON();
				Document resDoc;
				Document::AllocatorType& allocator = resDoc.GetAllocator();
				resDoc.Parse(_ret_msg.c_str());
				resDoc.AddMember("StatusCode", 0, allocator);
				resDoc.AddMember("StatusMsg", "success", allocator);
				StringBuffer resBuffer;
				PrettyWriter<StringBuffer> resWriter(resBuffer);
				resDoc.Accept(resWriter);
				std::string resJson = resBuffer.GetString();
				_socket.send(resJson);
			}
// #else
			else if (format == "file") {
				_ret_msg = res_set.to_str();
				time_t now = time(0);
				tm* ltm = localtime(&now);
				char digit[15];
				strftime(digit, sizeof(digit), "%Y%m%d%H%M%S", ltm);
				std::string filename = digit;
				filename += ".txt";
				std::string folderPath = "./query_result/";
				if (0 != access(folderPath.c_str(), 0)) {
					mkdir(folderPath.c_str(), 0777);
				}
				std::ofstream file(folderPath + filename);
				if (file.is_open()) {
					file << _ret_msg;
					file.close();
				}
				else {
					std::string error = "Open file failed.";
					this->response(1001, error, _socket);
					return false;
				}
				Document resDoc;
				Document::AllocatorType& allocator = resDoc.GetAllocator();
				resDoc.SetObject();
				resDoc.AddMember("StatusCode", 0, allocator);
				resDoc.AddMember("StatusMsg", "success", allocator);
				resDoc.AddMember("FileName", rapidjson::Value(filename.c_str(), allocator), allocator);
				StringBuffer resBuffer;
				PrettyWriter<StringBuffer> resWriter(resBuffer);
				resDoc.Accept(resWriter);
				std::string resJson = resBuffer.GetString();
				_socket.send(resJson);
			}
// #endif
			else if (format == "json+file"||format == "file+json") {
				_ret_msg = res_set.to_JSON();
				Document resDoc;
				Document::AllocatorType& allocator = resDoc.GetAllocator();
				resDoc.Parse(_ret_msg.c_str());

				_ret_msg = res_set.to_str();
				time_t now = time(0);
				tm* ltm = localtime(&now);
				char digit[15];
				strftime(digit, sizeof(digit), "%Y%m%d%H%M%S", ltm);
				std::string filename = digit;
				filename += ".txt";
				std::string folderPath = "./query_result/";
				if (0 != access(folderPath.c_str(), 0)) {
					mkdir(folderPath.c_str(), 0777);
				}
				std::ofstream file(folderPath + filename);
				if (file.is_open()) {
					file << _ret_msg;
					file.close();
				}
				else {
					std::string error = "Open file failed.";
					this->response(1001, error, _socket);
					return false;
				}

				resDoc.AddMember("StatusCode", 0, allocator);
				resDoc.AddMember("StatusMsg", "success", allocator);
				resDoc.AddMember("FileName", rapidjson::Value(filename.c_str(), allocator), allocator);
				StringBuffer resBuffer;
				PrettyWriter<StringBuffer> resWriter(resBuffer);
				resDoc.Accept(resWriter);
				std::string resJson = resBuffer.GetString();
				_socket.send(resJson);
			}
			return true;
		}
		else /**< Query error. */
		{
			std::string error = "Query failed.";
			this->response(1005, error, _socket);
			return false;
		}
	}
	else /**< Update query. */
	{
		if (ret_val >= 0)
		{
			std::string responsebody = "Update num: " + Util::int2string(ret_val);
			std::string success = "success";
			std::string resJson = CreateJson(0, success, true, responsebody);
			_socket.send(resJson);
			return true;
		}
		else /**< Update error. */
		{
			std::string error = "Update failed.";
			this->response(1005, error, _socket);
			return false;
		}
	}
}

bool
Server::show(Socket& _socket)
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

	std::map<std::string, int>::iterator iter;
	Document resDoc;
	resDoc.SetObject();
	Document::AllocatorType& allocator = resDoc.GetAllocator();
	Value jsonArray(kArrayType);
	for (iter = localDBs.begin(); iter != localDBs.end(); iter++)
	{
		if (iter->first == "system")
			continue;
		Value obj(kObjectType);
		Value db_name;
		db_name.SetString(iter->first.c_str(), iter->first.length(), allocator);
		if (databases.find(iter->first) == databases.end())
			obj.AddMember(db_name, "unloaded", allocator);
		else
			obj.AddMember(db_name, "loaded", allocator);
		jsonArray.PushBack(obj, allocator);
	}
	resDoc.AddMember("ResponseBody", jsonArray, allocator);
	resDoc.AddMember("StatusCode", 0, allocator);
	resDoc.AddMember("StatusMsg", "success", allocator);
	StringBuffer resBuffer;
	PrettyWriter<StringBuffer> resWriter(resBuffer);
	resDoc.Accept(resWriter);
	string resJson = resBuffer.GetString();
	_socket.send(resJson);
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
	cerr << Util::getTimeString() << "Query out of time." << endl;
	abort();
}

void Server::timer_sigterm_handler(int _signal_num) {
	pthread_exit(0);
}

void Server::stop_sigterm_handler(int _signal_num) {
	cout << Util::getTimeString() << "Server stopped." << endl;
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

bool Server::querySys(std::string _sparql, std::string& _res)
{
	FILE* output = NULL;
	ResultSet res_set;
	int ret_val = system_database->query(_sparql, res_set, output);

	/**
	* @brief Select query.
	*/
	if (ret_val < -1)
	{
		if (ret_val == -100)
		{
#ifdef SERVER_SEND_JSON
			_res = res_set.to_JSON();
#else
			_res = res_set.to_str();
#endif
			return true;
		}
		else /**< Query error. */
		{
			std::string error = "Query failed.";
			cerr << Util::getTimeString() << error << endl;
			return false;
		}
	}
	else /**< Update query. */
	{
		if (ret_val >= 0)
		{
			_res = "Update num: " + Util::int2string(ret_val) + "\n";
			return true;
		}
		else /**< Update error. */
		{
			std::string error = "Update failed.\n";
			cerr << Util::getTimeString() << error << endl;
			return false;
		}
	}
}

void Server::importSys()
{
	/**
	* @brief Query the system.db and get all users.
	*/
	std::string sparql = "select ?x ?y where{?x <has_password> ?y.}";
	std::string strJson;
	querySys(sparql, strJson);

	Document document;
	document.Parse(strJson.c_str());
	Value& p1 = document["results"];
	Value& p2 = p1["bindings"];

	for (unsigned i = 0; i < p2.Size(); i++)
	{
		Value& pp = p2[i];
		Value& pp1 = pp["x"];
		Value& pp2 = pp["y"];
		std::string username = pp1["value"].GetString();
		std::string password = pp2["value"].GetString();
		users.insert(pair<std::string, std::string>(username, password));
	}

	/**
	* @brief Query the system.db and get all databases.
	*/
	sparql = "select ?x where{?x <database_status> \"already_built\".}";
	querySys(sparql, strJson);
	document.Parse(strJson.c_str());
	p1 = document["results"];
	p2 = p1["bindings"];

	for (unsigned i = 0; i < p2.Size(); i++)
	{
		Value& pp = p2[i];
		Value& pp1 = pp["x"];
		std::string db_name = pp1["value"].GetString();

		localDBs.insert(pair<std::string, int>(db_name, 1));
	}

	/**
	* @brief Query the system.db and get the core version.
	*/
	sparql = "select ?x where{<CoreVersion> <value> ?x.}";
	querySys(sparql, strJson);
	document.Parse(strJson.c_str());
	p1 = document["results"];
	p2 = p1["bindings"];
	for (unsigned i = 0; i < p2.Size(); i++)
	{
		Value& pp = p2[i];
		Value& pp1 = pp["x"];
		CoreVersion = pp1["value"].GetString();
	}

	/**
	* @brief Query the system.db and get the API version.
	*/
	sparql = "select ?x where{<APIVersion> <value> ?x.}";
	querySys(sparql, strJson);
	document.Parse(strJson.c_str());
	p1 = document["results"];
	p2 = p1["bindings"];
	for (unsigned i = 0; i < p2.Size(); i++)
	{
		Value& pp = p2[i];
		Value& pp1 = pp["x"];
		APIVersion = pp1["value"].GetString();
	}
}

std::string Server::CreateJson(int StatusCode, std::string StatusMsg, bool _body_flag, std::string ResponseBody)
{
	StringBuffer s;
	PrettyWriter<StringBuffer> writer(s);
	writer.StartObject();
	if (_body_flag)
	{
		writer.Key("ResponseBody");
		writer.String(StringRef(ResponseBody.c_str()));
	}
	writer.Key("StatusCode");
	writer.Uint(StatusCode);
	writer.Key("StatusMsg");
	writer.String(StringRef(StatusMsg.c_str()));
	writer.EndObject();
	std::string res = s.GetString();
	return res;
}