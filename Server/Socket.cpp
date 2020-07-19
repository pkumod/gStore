/*
* Socket.cpp
*
*  Created on: 2014-10-14
*      Author: hanshuo
*/

#include "Socket.h"

using namespace std;

const std::string Socket::DEFAULT_SERVER_IP = "127.0.0.1";

Socket::Socket()
{
  this->sock = -1;
  ::memset(&addr, 0, sizeof(addr));
}

Socket::~Socket()
{
  if (this->isValid()) {
    //::shutdown(this->sock, SHUT_RDWR);
    ::close(this->sock);
  }
}

bool Socket::create()
{
  this->sock = ::socket(AF_INET, SOCK_STREAM, 0);

  if (!this->isValid()) {
    std::cerr << "create socket failed. @Socket::create" << std::endl;
    return false;
  }

  // TIME_WAIT - arg
  int on = 1;
  int setsockopt_REUSEADDR_return = setsockopt(this->sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));

  // BUFFER_SIZE - arg
  int bufferSize = 128 * 1024;
  int setsockopt_RCVBUF_return = setsockopt(this->sock, SOL_SOCKET, SO_RCVBUF, (const char*)&bufferSize, sizeof(bufferSize));
  int setsockopt_SNDBUF_return = setsockopt(this->sock, SOL_SOCKET, SO_SNDBUF, (const char*)&bufferSize, sizeof(bufferSize));

  if (setsockopt_REUSEADDR_return == -1 || setsockopt_RCVBUF_return == -1 || setsockopt_SNDBUF_return == -1) {
    std::cerr << "set socket options failed. @Socket::create" << std::endl;
    return false;
  }

  return true;
}

bool Socket::close()
{
  if (!this->isValid()) {
    return false;
  }

  int shutdown_return = ::shutdown(this->sock, SHUT_RDWR);
  if (shutdown_return == -1) {
    return false;
  }

  this->sock = -1;
  ::memset(&addr, 0, sizeof(addr));

  return true;
}

bool Socket::bind(const unsigned short _port)
{

  if (!this->isValid()) {
    return false;
  }

  this->addr.sin_family = AF_INET;
  this->addr.sin_addr.s_addr = htons(INADDR_ANY);
  this->addr.sin_port = htons(_port);

  int bind_return = ::bind(this->sock, (struct sockaddr*)&this->addr, sizeof(this->addr));

  if (bind_return == -1) {
    return false;
  }

  return true;
}

bool Socket::listen() const
{
  if (!this->isValid()) {
    return false;
  }

  int listen_return = ::listen(this->sock, Socket::MAX_CONNECTIONS);
  if (listen_return == -1) {
    return false;
  }

  return true;
}

bool Socket::accept(Socket& _new_socket) const
{
  size_t addr_length = sizeof(this->addr);
  _new_socket.sock = ::accept(this->sock, (struct sockaddr*)&this->addr, (socklen_t*)&addr_length);

  if (_new_socket.sock < 0) {
    return false;
  }

  return true;
}

bool Socket::send(const std::string& _msg) const
{
  int msg_len = _msg.size() + 1;

  int send_return = ::send(this->sock, &msg_len, sizeof(msg_len), 0);
  if (send_return == -1) {
    std::cerr << "send message length error. @Socket::send" << std::endl;
    return false;
  }

  int send_len = _msg.size() + 1;
  char* buf = new char[send_len];
  ::strcpy(buf, _msg.c_str());

  send_return = ::send(this->sock, buf, send_len, 0);
  delete[] buf;

  if (send_return == -1) {
    std::cerr << "send message context error. @Socket::send" << std::endl;
    return false;
  }

  return true;
}

int Socket::recv(std::string& _msg) const
{
  _msg.clear();

  //receive the message's length first.
  int msg_len;
  int recv_return = ::recv(this->sock, &msg_len, sizeof(msg_len), 0);

  //    std::cout << "msg_len=" << msg_len << std::endl; //debug

  if (recv_return == -1) {
    std::cerr << "receive message length error, errno=" << errno << ".@Socket::recv" << std::endl;
    return 0;
  }

  // then allocate the buffer's space by message's length, and receive the message's context.
  if (msg_len == 0) {
    return 0;
  }

  char* buf = new char[msg_len];
  int recv_len = 0;
  do {
    int cur_len = ::recv(this->sock, buf + recv_len, msg_len - recv_len, 0);

    if (cur_len == -1) {
      std::cerr << "receive message context error, errno=" << errno << ".@Socket::recv" << std::endl;
      delete[] buf;

      return 0;
    }
    recv_len += cur_len;
  } while (recv_len < msg_len);

  //    std::cout << "recv_len=" << recv_len << std::endl; //debug

  _msg = buf;
  delete[] buf;

  return msg_len;
}

bool Socket::connect(const std::string _hostname, const unsigned short _port)
{
  if (!this->isValid()) {
    return false;
  }

  (this->addr).sin_family = AF_INET;
  (this->addr).sin_port = htons(_port);

  //int status = inet_pton(AF_INET, _hostname.c_str(), &(this->addr).sin_addr);
  int status = inet_aton(_hostname.c_str(), &(this->addr).sin_addr);
  if (status == 0) {
    std::cerr << "bad IP address. @Socket::connect" << std::endl;
    return false;
  }

  if (errno == EAFNOSUPPORT) {
    return false;
  }

  status = ::connect(this->sock, (sockaddr*)&this->addr, sizeof(this->addr));

  if (status == 0) {
    return true;
  } else {
    return false;
  }
}

bool Socket::isValid() const
{
  return (this->sock != -1);
}
