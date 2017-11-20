#ifndef __COMMON_LOG_H__
#define __COMMON_LOG_H__

typedef enum {
	INFO    = (1<<0),
	WARNING = (1<<1),
	ERROR   = (1<<2),
	DEBUG   = (1<<3),
} LOG_LEVEL;

void LogFormat  (LOG_LEVEL l, const char* fmt, ...);
void LogSetLevel(LOG_LEVEL l);
#endif
