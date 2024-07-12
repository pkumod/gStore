#pragma once
#include "log4cplus/logger.h"
#include "log4cplus/fileappender.h"
#include "log4cplus/configurator.h"
#include "log4cplus/loggingmacros.h"
#include "log4cplus/layout.h"

using namespace log4cplus::thread;
using namespace log4cplus::helpers;


#if !defined(LOG4CPLUS_DISABLE_TRACE)
#define LOG4CPLUS_CORE_METHOD(logger, logEvent)                        \
    log4cplus::TraceLogger _log4cplus_trace_logger(logger, logEvent,    \
        LOG4CPLUS_MACRO_FILE (), __LINE__,                              \
        LOG4CPLUS_MACRO_FUNCTION ());
#define LOG4CPLUS_CORE(logger, logEvent)                               \
    LOG4CPLUS_MACRO_BODY (logger, logEvent, TRACE_LOG_LEVEL)
#define LOG4CPLUS_CORE_STR(logger, logEvent)                           \
    LOG4CPLUS_MACRO_STR_BODY (logger, logEvent, TRACE_LOG_LEVEL)
#define LOG4CPLUS_CORE_FMT(logger, ...)                                \
    LOG4CPLUS_MACRO_FMT_BODY (logger, TRACE_LOG_LEVEL, __VA_ARGS__)

#else
#define LOG4CPLUS_CORE_METHOD(logger, logEvent) LOG4CPLUS_DOWHILE_NOTHING()
#define LOG4CPLUS_CORE(logger, logEvent) LOG4CPLUS_DOWHILE_NOTHING()
#define LOG4CPLUS_CORE_STR(logger, logEvent) LOG4CPLUS_DOWHILE_NOTHING()
#define LOG4CPLUS_CORE_FMT(logger, logFmt, ...) LOG4CPLUS_DOWHILE_NOTHING()

#endif

#define SLOG_TRACE(logEvent)     LOG4CPLUS_TRACE(Slog::_logger, logEvent)
#define SLOG_DEBUG(logEvent)     LOG4CPLUS_DEBUG(Slog::_logger, logEvent)
#define SLOG_INFO(logEvent)      LOG4CPLUS_INFO(Slog::_logger, logEvent)
#define SLOG_WARN(logEvent)      LOG4CPLUS_WARN(Slog::_logger, logEvent)
#define SLOG_ERROR(logEvent)     LOG4CPLUS_ERROR(Slog::_logger, logEvent)
#define SLOG_FATAL(logEvent)     LOG4CPLUS_FATAL(Slog::_logger, logEvent)
#define SLOG_CORE(logEvent)      LOG4CPLUS_CORE(Slog::_logger, logEvent)
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
