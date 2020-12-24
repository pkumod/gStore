#include "../Util/Util.h"
#include "../Server/Server.h"

using namespace std;

bool send_backup_msg(unsigned short _port, long _time);

int main(int argc, char* argv[])
{

  //#ifdef DEBUG
  Util util;
  //#endif

  if (argc != 3) {
    return -1;
  }

  if (!Util::isValidPort(string(argv[1]))) {
    return -1;
  }

  unsigned short port;
  {
    stringstream ss(argv[1]);
    ss >> port;
  }

  long backup_time;
  {
    stringstream ss(argv[2]);
    ss >> backup_time;
  }

  while (true) {
    time_t cur_time = time(NULL);
    while (cur_time >= backup_time) {
      backup_time += Util::gserver_backup_interval;
    }
    sleep(backup_time - cur_time);
    if (!send_backup_msg(port, backup_time)) {
      return -1;
    }
  }

  return 0;
}

bool send_backup_msg(unsigned short _port, long _time)
{
  stringstream ss;
  ss << "backup " << _time;
  Socket socket;
  if (!socket.create() || !socket.connect("127.0.0.1", _port) || !socket.send(ss.str())) {
    return false;
  }
  string recv_msg;
  socket.recv(recv_msg);
  socket.close();
  if (recv_msg != "done") {
    return false;
  }
  return true;
}
