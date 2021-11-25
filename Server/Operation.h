/**
* @file  Operation.h
* @author  suxunbin
* @date  10-AUG-2021
* @brief  an operation class used to store a command and its parameter list
*/

#ifndef OPERATION_H_
#define OPERATION_H_

#include "../Util/Util.h"

/** @brief  A enum of different command types */
enum CommandType {
	CMD_OTHER, /**< enum value other. */
	CMD_TEST, /**< enum value test. */
	CMD_LOGIN, /**< enum value login. */
	CMD_BUILD, /**< enum value build. */
	CMD_LOAD, /**< enum value load. */
	CMD_UNLOAD, /**< enum value unload. */
	CMD_QUERY, /**< enum value query. */
	CMD_SHOW, /**< enum value show. */
	CMD_STOP, /**< enum value stop. */
	CMD_DROP, /**< enum value drop. */
	CMD_CLOSE /**< enum value close. */
};

class Operation
{
public:
	/** @brief A default constructor. */
	Operation();

	/**
	* @brief A constructor taking two arguments.
	* @param[in]  _cmd : The command type
	* @param[in]  _para : The parameter list
	*/
	Operation(CommandType _cmd, const std::unordered_map<std::string, std::string>& _para);

	/** @brief A default destructor. */
	~Operation();

	/**
	* @brief Get the command type.
	*/
	CommandType getCommand();

	/**
	* @brief Get the parameter value of a parameter.
	* @param[in]  _para_name : A parameter name
	*/
	std::string getParameter(std::string _para_name);

	/**
	* @brief Set the command type.
	* @param[in] _cmd : A command type
	*/
	void setCommand(CommandType _cmd);

	/**
	* @brief Set the parameter list.
	* @param[in] _para : A parameter list
	*/
	void setParameter(const std::unordered_map<std::string, std::string>& _para);


private:
	CommandType command; /**< The command type. */
	std::unordered_map<std::string, std::string> parameters; /**<  The parameter list. */
};

#endif /* OPERATION_H_ */