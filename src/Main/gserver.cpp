/**
* @file  gserver.cpp
* @author  suxunbin
* @date  13-AUG-2021
* @brief  a gStore socket server
*/

#include "../Server/Server.h"
#include "../Util/Util.h"

using namespace rapidjson;
using namespace std;

//#define GSERVER_LOG "logs/gserver.log"

bool isOnlyProcess(const char* argv0);
void checkSwap();
bool startServer(bool _debug);
bool stopServer();
bool load_db(std::string dbname, bool load_csr);
int parseJson(std::string strJson);

int main(int argc, char* argv[])
{
	//#ifdef DEBUG
	Util util;
	//#endif

	string mode;
	if (argc == 1) {
		mode = "-h";
	}
	else {
		mode = argv[1];
	}

	if ((argc == 4 || argc ==6) && (mode == "-s" || mode == "--start")) {
		mode = argv[2];
		bool load_csr = false;
		if (mode != "-db" && mode != "--database") {
			cout << "Invalid arguments! Input \"bin/gserver -h\" for help." << endl;
			return -1;
		}
		else if (argc == 6) {
			mode = argv[4];
			if (mode != "-c" && mode != "--csr") {
				cout << "Invalid arguments! Input \"bin/gserver -h\" for help." << endl;
				return -1;
			}
			else {
				mode = argv[5];
				if (mode == "1") {
					load_csr = true;
				}
				else if (mode == "0") {
					load_csr = false;
				}
				else {
					cout << "Invalid arguments! Input \"bin/gserver -h\" for help." << endl;
					return -1;
				}
			}
		}
		if (!isOnlyProcess(argv[0])) {
			cerr << "gServer already running!" << endl;
			return -1;
		}
		if (startServer(false)) {
			sleep(1);
			if (isOnlyProcess(argv[0])) {
				cerr << "Server stopped unexpectedly. Check for port conflicts!" << endl;
				return -1;
			}
			if (!load_db(argv[3], load_csr)) {
				return -1;
			}
			return 0;
		}
		else {
			return -1;
		}
	}
	else if (argc > 3 || (argc == 3 && mode != "-p" && mode != "--port")) {
		cout << "Invalid arguments! Input \"bin/gserver -h\" for help." << endl;
		return -1;
	}

	else if (mode == "-h" || mode == "--help") {
		cout << endl;
		cout << "gStore Server (gServer)" << endl;
		cout << endl;
		cout << "Usage:\tbin/gserver [option]" << endl;
		cout << endl;
		cout << "Options:" << endl;
		cout << "\t-h,--help\t\tDisplay this message." << endl;
		cout << "\t-s,--start\t\tStart gServer." << endl;
		cout << "\t-db,--database[option]\tthe database name.Default value is empty.Notice that the name can not end with .db" << endl;
		cout << "\t-c,--csr[option]\tEnable CSR Struct or not. 9 denote that false, 1 denote that true. Default value is 0." << endl;
		cout << "\t-t,--stop\t\tStop gServer." << endl;
		cout << "\t-r,--restart\t\tRestart gServer." << endl;
		cout << "\t-p,--port [PORT=" << Socket::DEFAULT_CONNECT_PORT << "]\tChange connection port configuration, takes effect after restart if gServer running." << endl;
		cout << "\t-P,--printport\t\tDisplay current connection port configuration." << endl;
		cout << "\t-d,--debug\t\tStart gServer in debug mode (keep gServer in the foreground)." << endl;
		cout << "\t-k,--kill\t\tKill existing gServer process(es), ONLY use when out of normal procedures." << endl;
		cout << endl;
		return 0;
	}

	if (mode == "-p" || mode == "--port") {
		unsigned short port = Socket::DEFAULT_CONNECT_PORT;
		if (argc == 3) {
			if (!Util::isValidPort(string(argv[2]))) {
				cerr << "Invalid port: " << argv[2] << endl;
				return -1;
			}
			else {
				stringstream ss(argv[2]);
				ss >> port;
			}
		}
		if (!isOnlyProcess(argv[0])) {
			ofstream out(Util::gserver_port_swap.c_str());
			if (!out) {
				cerr << "Failed to change port!" << endl;
				return -1;
			}
			out << port;
			out.close();
			cout << "Port will be changed to " << port << " after the current server stops or restarts." << endl;
			return 0;
		}
		ofstream out(Util::gserver_port_file.c_str());
		if (!out) {
			cerr << "Failed to change port!" << endl;
			return -1;
		}
		out << port;
		out.close();
		cout << "Port changed to " << port << '.' << endl;
		return 0;
	}

	else if (mode == "-s" || mode == "--start") {
		if (!isOnlyProcess(argv[0])) {
			cerr << "gServer already running!" << endl;
			return -1;
		}
		if (startServer(false)) {
			sleep(1);
			if (isOnlyProcess(argv[0])) {
				cerr << "Server stopped unexpectedly. Check for port conflicts!" << endl;
				return -1;
			}
			return 0;
		}
		else {
			return -1;
		}
	}

	else if (mode == "-t" || mode == "--stop") {
		if (isOnlyProcess(argv[0])) {
			cerr << "gServer not running!" << endl;
			return -1;
		}
		if (stopServer()) {
			return 0;
		}
		else {
			return -1;
		}
	}

	else if (mode == "-r" || mode == "--restart") {
		if (isOnlyProcess(argv[0])) {
			cerr << "gServer not running!" << endl;
			return -1;
		}
		if (!stopServer()) {
			return -1;
		}
		sleep(10);
		if (startServer(false)) {
			sleep(1);
			if (isOnlyProcess(argv[0])) {
				cerr << "Server stopped unexpectedly. Check for port conflicts!" << endl;
				return -1;
			}
			return 0;
		}
		else {
			return -1;
		}
		return 0;
	}

	else if (mode == "-P" || mode == "--printport") {
		unsigned short port = Socket::DEFAULT_CONNECT_PORT;
		ifstream in(Util::gserver_port_file.c_str());
		if (in) {
			in >> port;
			in.close();
		}
		cout << "Current connection port is " << port << '.' << endl;
		unsigned short portSwap = 0;
		ifstream inSwap(Util::gserver_port_swap.c_str());
		if (inSwap) {
			inSwap >> portSwap;
			inSwap.close();
		}
		if (portSwap != 0) {
			cout << "Connection port to be changed to " << portSwap << '.' << endl;
		}
		return 0;
	}

	else if (mode == "-k" || mode == "--kill") {
		if (isOnlyProcess(argv[0])) {
			cerr << "No process to kill!" << endl;
			return -1;
		}
		execl("/usr/bin/killall", "killall", Util::getExactPath(argv[0]).c_str(), NULL);
		return 0;
	}

	else {
		cerr << "Invalid arguments! Type \"bin/gserver -h\" for help." << endl;
		return -1;
	}
}

bool isOnlyProcess(const char* argv0) {
	return Util::getSystemOutput("pidof " + Util::getExactPath(argv0)) == Util::int2string(getpid());
}

void checkSwap() {
	if (access(Util::gserver_port_swap.c_str(), 00) != 0) {
		return;
	}
	ifstream in(Util::gserver_port_swap.c_str());
	if (!in) {
		cerr << "Failed in checkSwap(), port may not be changed." << endl;
		return;
	}
	unsigned short port;
	in >> port;
	in.close();
	ofstream out(Util::gserver_port_file.c_str());
	if (!out) {
		cerr << "Failed in checkSwap(), port may not be changed." << endl;
		return;
	}
	out << port;
	out.close();
	chmod(Util::gserver_port_file.c_str(), 0644);
	string cmd = Util::gserver_port_swap;
	Util::remove_path(cmd);
}

bool startServer(bool _debug) {
	unsigned short port = Socket::DEFAULT_CONNECT_PORT;
	ifstream in(Util::gserver_port_file.c_str());
	if (!in) {
		ofstream out(Util::gserver_port_file.c_str());
		if (out) {
			out << port;
			out.close();
			chmod(Util::gserver_port_file.c_str(), 0644);
		}
	}
	else {
		in >> port;
		in.close();
	}

	if (_debug) {
		Server server(port);
		if (!server.createConnection()) {
			cerr << Util::getTimeString() << "Failed to create connection at port " << port << '.' << endl;
			return false;
		}
		cout << Util::getTimeString() << "Server started at port " << port << '.' << endl;
		server.listen();
		server.deleteConnection();
		return true;
	}

	pid_t fpid = fork();

	// child
	if (fpid == 0) {
		if (!Util::dir_exist("logs")) {
			Util::create_dir("logs");
		}
		freopen(Util::gserver_log.c_str(), "a", stdout);
		freopen(Util::gserver_log.c_str(), "a", stderr);

		int status;

		while (true) {
			fpid = fork();

			// child, main process
			if (fpid == 0) {
				Server server(port);
				if (!server.createConnection()) {
					cerr << Util::getTimeString() << "Failed to create connection at port " << port << '.' << endl;
					return false;
				}
				cout << Util::getTimeString() << "Server started at port " << port << '.' << endl;
				server.listen();
				server.deleteConnection();
				exit(0);
				return true;
			}

			// parent, deamon process
			else if (fpid > 0) {
				waitpid(fpid, &status, 0);
				if (WIFEXITED(status)) {
					exit(0);
					return true;
				}
				cerr << Util::getTimeString() << "Server stopped abnormally, restarting server..." << endl;
			}

			// fork failure
			else {
				cerr << Util::getTimeString() << "Failed to start server: deamon fork failure." << endl;
				return false;
			}
		}
	}

	// parent
	else if (fpid > 0) {
		cout << "Server started at port " << port << '.' << endl;
		return true;
	}

	// fork failure
	else {
		cerr << "Failed to start server at port " << port << '.' << endl;
		return false;
	}
}

bool stopServer() {
	/**
	* @brief Read the server port.
	*/
	unsigned short port = Socket::DEFAULT_CONNECT_PORT;
	ifstream in(Util::gserver_port_file.c_str());
	if (in) {
		in >> port;
		in.close();
	}

	/**
	* @brief Login the server using the root user.
	*/
	Socket socket;
	std::string cmd = "{\"op\":\"login\",\"username\":\"root\",\"password\":\"123456\"}";
	if (!socket.create() || !socket.connect("127.0.0.1", port) || !socket.send(cmd)) {
		cerr << "Failed to stop server at port " << port << '.' << endl;
		return false;
	}
	std::string recv_msg;
	socket.recv(recv_msg);

	/**
	* @brief Stop the server.
	*/
	cmd = "{\"op\":\"stop\"}";
	if (!socket.send(cmd)) {
		cerr << "Failed to stop server at port " << port << '.' << endl;
		return false;
	}
	socket.recv(recv_msg);
	socket.close();

	int StatusCode = parseJson(recv_msg);

	if (StatusCode != 0) {
		cerr << "Failed to stop server at port " << port << '.' << endl;
		return false;
	}
	cout << "Server stopped at port " << port << '.' << endl;
	checkSwap();
	return true;
}

bool load_db(std::string dbname, bool load_csr) {
	unsigned short port = Socket::DEFAULT_CONNECT_PORT;
	ifstream in(Util::gserver_port_file.c_str());
	if (in) {
		in >> port;
		in.close();
	}
	Socket socket;
	std::string cmd = "{\"op\":\"login\",\"username\":\"root\",\"password\":\"123456\"}";
	if (!socket.create() || !socket.connect("127.0.0.1", port) || !socket.send(cmd)) {
		cerr << "Failed to load database " << dbname << '.' << endl;
		return false;
	}
	std::string recv_msg;
	socket.recv(recv_msg);
	
	cmd = "{\"op\":\"load\",\"db_name\":\"";
	cmd += dbname;
	cmd += "\",";
	if (load_csr)
		cmd += "\"csr\":\"1\"}";
	else
		cmd += "\"csr\":\"0\"}";
	socket.send(cmd);
	socket.recv(recv_msg);
	if (recv_msg.find("Load database successfully.") != std::string::npos) {
		cout << "Load database successfully. csr=";
		if (load_csr)
			cout << 1 << endl;
		else
			cout << 0 << endl;
		return true;
	}
	else {
		cerr << "Failed to load database " << dbname << '.' << endl;
		return false;
	}
}

int parseJson(std::string strJson)
{
	Document document;
	document.Parse(strJson.c_str());
	Value& p = document["StatusCode"];
	int StatusCode = p.GetInt();
	return StatusCode;
}
