/*=============================================================================
# Filename: APIServer.h
# Author: wenjieli
# Mail: liwenjiehn@pku.edu.cn
# Last Modified: 2021-7-24
# Description: the http server based on the Sougou C++ workflow.
# Description: https://gitee.com/sogou/workflow
=============================================================================*/

#ifndef _APISERVER_APISERVER_H
#define _APISERVER_APISERVER_H

#include "string.h"
/*
 * 
 */

class APIServer
{
public:

    APIServer(unsigned short _port);
    void setPort(unsigned short _port);
    unsigned short getPort();
    ~APIServer();


private:
    //the listen port
    unsigned  short _port_;
    string _log_file_;
    //GstoreConnector *gc;
};

#endif // _APISERVER_APISERVER_H

