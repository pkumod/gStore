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
	if (std::string(log_mode) == "0")
	{
		log4cplus::PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT("./slog.stdout.properties"));
	}
	else
	{
		log4cplus::PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT("./slog.properties"));
	}
}