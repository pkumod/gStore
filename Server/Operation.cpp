/*
* Operation.cpp
*
*  Created on: 2014-10-16
*      Author: hanshuo
*/

#include "Operation.h"

Operation::Operation()
{
  this->command = CMD_OTHER;
}

Operation::Operation(CommandType _cmd, const std::vector<std::string>& _para)
{
  this->command = _cmd;
  this->parameters = _para;
}

Operation::Operation(std::string _usr, std::string _pwd, CommandType _cmd, const std::vector<std::string>& _para)
{
  this->username = _usr;
  this->password = _pwd;
  this->command = _cmd;
  this->parameters = _para;
}

Operation::~Operation()
{
}

Bstr Operation::encrypt()
{
  //TODO
  return Bstr(NULL, 0);
}

Bstr Operation::decrypt()
{
  //TODO
  return Bstr(NULL, 0);
}

CommandType Operation::getCommand()
{
  return this->command;
}

std::string Operation::getParameter(int _idx)
{
  if ((unsigned)_idx < this->parameters.size()) {
    return this->parameters[_idx];
  } else {
    std::cerr << "beyond index error. @Operation::getParameter" << std::endl;
    return *this->parameters.end();
  }
}

void Operation::setCommand(CommandType _cmd)
{
  this->command = _cmd;
}

void Operation::setParameter(const std::vector<std::string>& _para)
{
  this->parameters = _para;
}
