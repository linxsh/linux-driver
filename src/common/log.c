#include "common.h"
#include "log.h"

static LOG_LEVEL  level  = DEBUG;

void LogFormat(LOG_LEVEL l, const char* fmt, ...)
{
	va_list args;
	const char *lString  = NULL;

	switch (l & level) {
	case INFO:
		lString = "Driver INFO";
		break;
	case WARNING:
		lString = "Driver WARN";
		break;
	case ERROR:
		lString = "Driver ERROR";
		break;
	case DEBUG:
		lString = "Driver DEBUG";
		break;
	default:
		return;
	}

	dr_printf("[%s]: ", lString);
	va_start(args, fmt);
	dr_vprintf(fmt, args);
	va_end(args);
	return;
}

void LogSetLevel (LOG_LEVEL  l)
{
	level = l;
}
