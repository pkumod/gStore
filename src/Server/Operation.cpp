/**
* @file  Operation.cpp
* @author  suxunbin
* @date  10-AUG-2021
* @brief  an operation class used to store a command and its parameter list
*/

#include "Operation.h"

Operation::Operation()
{
	this->command = CMD_OTHER;
}

Operation::Operation(CommandType _cmd, const std::unordered_map<std::string, std::string>& _para)
{
	this->command = _cmd;
	this->parameters = _para;
}

Operation::~Operation()
{

}

CommandType Operation::getCommand()
{
	return this->command;
}

std::string Operation::getParameter(std::string _para_name)
{
	std::unordered_map<std::string, std::string>::iterator it = this->parameters.find(_para_name);
	if (it == this->parameters.end())
		std::cerr << "beyond para_name error. @Operation::getParameter" << std::endl;
	return it->second;
}

void Operation::setCommand(CommandType _cmd)
{
	this->command = _cmd;
}

void Operation::setParameter(const std::unordered_map<std::string, std::string>& _para)
{
	this->parameters = _para;
}