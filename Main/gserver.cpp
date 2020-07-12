/*=================================================================================
# Filename: gserver.cpp
# Author: Bookug Lobert
# Mail: 1181955272@qq.com
# Last Modified: 2016-02-26 19:15
# Description: first written by hanshuo, modified by zengli, improved by Wang Libo
=================================================================================*/

#include "../Server/Server.h"
#include "../Util/Util.h"

using namespace std;

//#define GSERVER_PORT_FILE "bin/.gserver_port"
//#define GSERVER_PORT_SWAP "bin/.gserver_port.swap"
//#define GSERVER_LOG "logs/gserver.log"

bool isOnlyProcess(const char* argv0);
void checkSwap();
bool startServer(bool _debug);
bool stopServer();

int main(int argc, char* argv[])
{
  //#ifdef DEBUG
  Util util;
  //#endif

  string mode;
  if (argc == 1) {
    mode = "-h";
  } else {
    mode = argv[1];
  }

  if (argc > 3 || (argc == 3 && mode != "-p" && mode != "--port")) {
    cout << "Invalid arguments! Input \"bin/gserver -h\" for help." << endl;
    return -1;
  } else if (mode == "-h" || mode == "--help") {
    cout << endl;
    cout << "gStore Server (gServer)" << endl;
    cout << endl;
    cout << "Usage:\tbin/gserver [option]" << endl;
    cout << endl;
    cout << "Options:" << endl;
    cout << "\t-h,--help\t\tDisplay this message." << endl;
    cout << "\t-s,--start\t\tStart gServer." << endl;
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
      } else {
        stringstream ss(argv[2]);
        ss >> port;
      }
    }
    if (!isOnlyProcess(argv[0])) {
      //ofstream out(GSERVER_PORT_SWAP, ios::out);
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
    //ofstream out(GSERVER_PORT_FILE, ios::out);
    ofstream out(Util::gserver_port_file.c_str());
    if (!out) {
      cerr << "Failed to change port!" << endl;
      return -1;
    }
    out << port;
    out.close();
    cout << "Port changed to " << port << '.' << endl;
    return 0;
  } else if (mode == "-s" || mode == "--start") {
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
    } else {
      return -1;
    }
  } else if (mode == "-t" || mode == "--stop") {
    if (isOnlyProcess(argv[0])) {
      cerr << "gServer not running!" << endl;
      return -1;
    }
    if (stopServer()) {
      return 0;
    } else {
      return -1;
    }
  } else if (mode == "-r" || mode == "--restart") {
    if (isOnlyProcess(argv[0])) {
      cerr << "gServer not running!" << endl;
      return -1;
    }
    if (!stopServer()) {
      return -1;
    }
    if (startServer(false)) {
      sleep(1);
      if (isOnlyProcess(argv[0])) {
        cerr << "Server stopped unexpectedly. Check for port conflicts!" << endl;
        return -1;
      }
      return 0;
    } else {
      return -1;
    }
    return 0;
  } else if (mode == "-P" || mode == "--printport") {
    unsigned short port = Socket::DEFAULT_CONNECT_PORT;
    //ifstream in(GSERVER_PORT_FILE);
    ifstream in(Util::gserver_port_file.c_str());
    if (in) {
      in >> port;
      in.close();
    }
    cout << "Current connection port is " << port << '.' << endl;
    unsigned short portSwap = 0;
    //ifstream inSwap(GSERVER_PORT_SWAP);
    ifstream inSwap(Util::gserver_port_swap.c_str());
    if (inSwap) {
      inSwap >> portSwap;
      inSwap.close();
    }
    if (portSwap != 0) {
      cout << "Connection port to be changed to " << portSwap << '.' << endl;
    }
    return 0;
  } else if (mode == "-k" || mode == "--kill") {
    if (isOnlyProcess(argv[0])) {
      cerr << "No process to kill!" << endl;
      return -1;
    }
    execl("/usr/bin/killall", "killall", Util::getExactPath(argv[0]).c_str(), NULL);
    return 0;
  } else {
    cerr << "Invalid arguments! Type \"bin/gserver -h\" for help." << endl;
    return -1;
  }
}

bool isOnlyProcess(const char* argv0)
{
  return Util::getSystemOutput("pidof " + Util::getExactPath(argv0)) == Util::int2string(getpid());
}

void checkSwap()
{
  //if (access(GSERVER_PORT_SWAP, 00) != 0) {
  if (access(Util::gserver_port_swap.c_str(), 00) != 0) {
    return;
  }
  //ifstream in(GSERVER_PORT_SWAP, ios::in);
  ifstream in(Util::gserver_port_swap.c_str());
  if (!in) {
    cerr << "Failed in checkSwap(), port may not be changed." << endl;
    return;
  }
  unsigned short port;
  in >> port;
  in.close();
  //ofstream out(GSERVER_PORT_FILE, ios::out);
  ofstream out(Util::gserver_port_file.c_str());
  if (!out) {
    cerr << "Failed in checkSwap(), port may not be changed." << endl;
    return;
  }
  out << port;
  out.close();
  //chmod(GSERVER_PORT_FILE, 0644);
  chmod(Util::gserver_port_file.c_str(), 0644);
  //string cmd = string("rm ") + GSERVER_PORT_SWAP;
  string cmd = string("rm ") + Util::gserver_port_swap;
  system(cmd.c_str());
}

bool startServer(bool _debug)
{
  unsigned short port = Socket::DEFAULT_CONNECT_PORT;
  //ifstream in(GSERVER_PORT_FILE, ios::in);
  ifstream in(Util::gserver_port_file.c_str());
  if (!in) {
    //ofstream out(GSERVER_PORT_FILE, ios::out);
    ofstream out(Util::gserver_port_file.c_str());
    if (out) {
      out << port;
      out.close();
      //chmod(GSERVER_PORT_FILE, 0644);
      chmod(Util::gserver_port_file.c_str(), 0644);
    }
  } else {
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

bool stopServer()
{
  unsigned short port = Socket::DEFAULT_CONNECT_PORT;
  //ifstream in(GSERVER_PORT_FILE, ios::in);
  ifstream in(Util::gserver_port_file.c_str());
  if (in) {
    in >> port;
    in.close();
  }
  Socket socket;
  if (!socket.create() || !socket.connect("127.0.0.1", port) || !socket.send("stop")) {
    cerr << "Failed to stop server at port " << port << '.' << endl;
    return false;
  }
  string recv_msg;
  socket.recv(recv_msg);
  socket.close();
  if (recv_msg != "server stopped.") {
    cerr << "Failed to stop server at port " << port << '.' << endl;
    return false;
  }
  cout << "Server stopped at port " << port << '.' << endl;
  checkSwap();
  return true;
}
