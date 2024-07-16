/*
 * @Author: wangjian 2606583267@qq.com
 * @Date: 2022-08-16 23:54:34
 * @LastEditors: wangjian 2606583267@qq.com
 * @LastEditTime: 2022-08-17 01:02:59
 * @FilePath: /gstore/Util/Slog.cpp
 * @Description: Slog.cpp
 */

#include "Slog.h"

log4cplus::Logger Slog::_logger = log4cplus::Logger::getRoot();

Slog::Slog()
{

}

Slog::~Slog()
{
	_logger.shutdown();
}

Slog& Slog::getInstance(){
	static Slog _slog;
	return _slog; 
}

void Slog::init(const char* log_mode)
{
	std::ifstream log_config_file(log_mode);
	if (log_config_file.good())
	{
		log_config_file.close();
		log4cplus::PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT(log_mode));
	}
	else
	{
		std::cout << "log config file not exist, use default config" << std::endl;
		log4cplus::BasicConfigurator::doConfigure();
	}
}