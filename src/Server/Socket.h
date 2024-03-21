/**
* @file  Socket.h
* @author  suxunbin
* @date  11-AUG-2021
* @brief  a socket interface
*/

#ifndef _SERVER_SOCKET_H
#define _SERVER_SOCKET_H

#include "../Util/Util.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"  
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/error/en.h"

#define BUFFER_SIZE 131072 /**<  The socket send/recv buffer size. */

class Socket
{
public:
	/** @brief A default constructor. */
	Socket();

	Socket(const Socket &socket_src);

	/** @brief A default destructor. */
	~Socket();

	/**
	* @brief Create a socket and set sockopt.
	* @return
	*	true : Create the socket successfully. \n
	*	false : Create the socket failed. \n
	*/
	bool create();

	/**
	* @brief Close a socket.
	* @return
	*	true : Close the socket successfully. \n
	*	false : Close the socket failed. \n
	*/
	bool close();

	/**
	* @brief Bind a socket with a port.
	* @param[in]  _port : A port
	* @return
	*	true : Bind the socket successfully. \n
	*	false : Bind the socket failed. \n
	*/
	bool bind(const unsigned short _port);

	/**
	* @brief Start to listen on the socket.
	* @return
	*	true : Start to listen on the socket successfully. \n
	*	false : Start to listen on the socket failed. \n
	*/
	bool listen()const;

	/**
	* @brief Accept a new socket from a client.
	* @param[in]  _new_socket : A new socket from a client
	* @return
	*	true : Accept the socket successfully. \n
	*	false : Accept the socket failed. \n
	*/
	bool accept(Socket& _new_socket)const;

	/**
	* @brief Connect the socket with an IP address and a port.
	* @param[in]  _hostname : An IP address
	* @param[in]  _port : A port
	* @return
	*	true : Connect the socket successfully. \n
	*	false : Connect the socket failed. \n
	*/
	bool connect(const std::string _hostname, const unsigned short _port);

	/**
	* @brief Send messages.
	* @param[in]  _msg : a message
	* @return
	*	true : Send the message successfully. \n
	*	false : Send the message failed. \n
	*/
	bool send(const std::string& _msg)const;

	/**
	* @brief Receive messages.
	* @param[out]  _msg : the message received
	* @return
	*	<= 0 : Receive the message failed. \n
	*	> 0 : The received message length. \n
	*/
	int recv(std::string& _msg)const;

	/**
	* @brief Determine whether a socket is valid.
	* @return
	*	true : A valid socket. \n
	*	false : An invalid socket. \n
	*/
	bool isValid()const;

	static const int MAX_CONNECTIONS = 3000; /**<  The maximum connections of the server. */
	static const unsigned short DEFAULT_CONNECT_PORT = 9000; /**<  A default port. */
	static const std::string DEFAULT_SERVER_IP; /**<  A default IP address. */
	std::string username; /**<  The username to connect the server. */
	std::string password; /**<  The password to connect the server. */

private:
	int sock; /**<  A sockfd. */
	sockaddr_in addr; /**<  A socket address. */
};

#endif /* _SERVER_SOCKET_H */

