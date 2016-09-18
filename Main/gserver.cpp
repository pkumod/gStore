/*=============================================================================
# Filename: gserver.cpp
# Author: Bookug Lobert
# Mail: 1181955272@qq.com
# Last Modified: 2016-02-26 19:15
# Description: first written by hanshuo, modified by zengli
=============================================================================*/

#include "../Server/Server.h"
#include "../Util/Util.h"

using namespace std;

int main(int argc, char * argv[])
{
#ifdef DEBUG
	Util util;
#endif

	unsigned short port = Socket::DEFAULT_CONNECT_PORT;

	if (argc > 1)
	{
		std::stringstream ss(argv[1]);
		ss >> port;
	}

	Server server(port);

	bool flag = server.createConnection();
	if (!flag) {
		cerr << "Failed to create connection." << endl;
		return -1;
	}

	cout << "Server started at port " << port << endl;
	server.listen();

	return 0;
}