/**********************************************************************
* Copyright (C) 2021 -  - All Rights Reserved
*
* 文件名称:        slog.cpp
* 摘    要:        日志包裹类,日志模块的接口
*
* 作    者:       liwenjie
* 修    改:       查看文件最下方.
*
***********************************************************************/

#include "Slog.h"

#include <stdio.h>
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/config.hxx>
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/helpers/socket.h>
#include <log4cplus/spi/loggerimpl.h>
#include <log4cplus/spi/loggingevent.h>

using namespace log4cplus;
using namespace log4cplus::helpers;

Slog::Slog()
{

}

Slog::~Slog()
{
	log4cplus::Logger _logger = log4cplus::Logger::getInstance("global");
	//LOG4CPLUS_INFO(_logger, "Slog System Stop Finish...");
	_logger.shutdown();
}



#define DO_LOGGER(logLevel, pFormat, bufSize)\
    log4cplus::Logger _logger = log4cplus::Logger::getInstance("global");\
    \
    if(_logger.isEnabledFor(logLevel))\
{                \
    va_list args;            \
    va_start(args, pFormat);        \
    char buf[bufSize] = {0};        \
    VSPRINTF(buf, sizeof(buf), pFormat, args);    \
    va_end(args);           \
    _logger.forcedLog(logLevel, buf); \
}

void Slog::Debug(const char* pFormat, ...)
{
	DO_LOGGER(log4cplus::DEBUG_LOG_LEVEL, pFormat, 1024);
}

void Slog::Error(const char* pFormat, ...)
{
	DO_LOGGER(log4cplus::ERROR_LOG_LEVEL, pFormat, 256);
}

void Slog::Fatal(const char* pFormat, ...)
{
	DO_LOGGER(log4cplus::FATAL_LOG_LEVEL, pFormat, 256);
}

void Slog::Info(const char* pFormat, ...)
{
	DO_LOGGER(log4cplus::INFO_LOG_LEVEL, pFormat, 512);
	
}

void Slog::Warn(const char* pFormat, ...)
{
	DO_LOGGER(log4cplus::WARN_LOG_LEVEL, pFormat, 256);
}

void Slog::Trace(const char* pFormat, ...)
{
	DO_LOGGER(log4cplus::TRACE_LOG_LEVEL, pFormat, 1024);
}

void Slog::init(const char* properties_filename)
{
	if (properties_filename == NULL) return;
	log4cplus::initialize();
	log4cplus::PropertyConfigurator::doConfigure(properties_filename);
	log4cplus::Logger _logger = log4cplus::Logger::getInstance("global");
	//LOG4CPLUS_INFO(_logger, "Logger system start finish.");
}
