/*
* Operation.h
*
* Created on: 2014-10-16
* Author: hanshuo
*/

#ifndef OPERATION_H_
#define OPERATION_H_

#include "../Util/Util.h"
#include "../Util/Bstr.h"

//NOTICE:CMD_DROP is used to remove the database, and CMD_CREATE is not useful because
//we always need to import a dataset to create a gstore db
enum CommandType {
  CMD_CONNECT,
  CMD_TEST,
  CMD_LOAD,
  CMD_UNLOAD,
  CMD_CREATE,
  CMD_DROP,
  CMD_IMPORT,
  CMD_QUERY,
  CMD_SHOW,
  CMD_INSERT,
  CMD_STOP,
  CMD_BACKUP,
  CMD_OTHER
  //CMD_CONNECT, CMD_EXIT, CMD_TEST, CMD_LOAD, CMD_UNLOAD, CMD_CREATE, CMD_DROP,
  //CMD_IMPORT, CMD_QUERY, CMD_SHOW, CMD_INSERT, CMD_STOP, CMD_OTHER
}; // extend the operation command type here.

class Operation {
  public:
  Operation();
  Operation(std::string _usr, std::string _pwd, CommandType _cmd, const std::vector<std::string>& _para);
  Operation(CommandType _cmd, const std::vector<std::string>& _para);
  ~Operation();

  Bstr encrypt();
  Bstr decrypt();

  CommandType getCommand();
  std::string getParameter(int _idx);

  void setCommand(CommandType _cmd);
  void setParameter(const std::vector<std::string>& _para);

  private:
  /*
	* attention: the username and password is not used to verify permissions of connection by now...
	* this part of functions should be implemented later.
	*/
  std::string username;
  std::string password;
  CommandType command;
  std::vector<std::string> parameters;
};

#endif /* OPERATION_H_ */
