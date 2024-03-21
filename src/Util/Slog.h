#pragma once
#include "log4cplus/logger.h"
#include "log4cplus/fileappender.h"
#include "log4cplus/configurator.h"
#include "log4cplus/loggingmacros.h"
#include "log4cplus/layout.h"

using namespace log4cplus::thread;
using namespace log4cplus::helpers;

#define SLOG_TRACE(logEvent)     LOG4CPLUS_TRACE(Slog::_logger, logEvent)
#define SLOG_DEBUG(logEvent)     LOG4CPLUS_DEBUG(Slog::_logger, logEvent)
#define SLOG_INFO(logEvent)      LOG4CPLUS_INFO(Slog::_logger, logEvent)
#define SLOG_WARN(logEvent)      LOG4CPLUS_WARN(Slog::_logger, logEvent)
#define SLOG_ERROR(logEvent)     LOG4CPLUS_ERROR(Slog::_logger, logEvent)
#define SLOG_FATAL(logEvent)     LOG4CPLUS_FATAL(Slog::_logger, logEvent)

class Slog
{
public:
	static Slog& getInstance();
	static log4cplus::Logger _logger;
	void init(const char* log_mode);
private:
	Slog();
	virtual ~Slog();
};
