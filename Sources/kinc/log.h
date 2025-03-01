#pragma once

#include <kinc/global.h>

#include <stdarg.h>

/*! \file log.h
    \brief Contains basic logging functionality.

    Logging functionality is similar to plain printf but provides some system-specific bonuses.
*/

#ifdef __cplusplus
extern "C" {
#endif

/// <summary>
/// Pass this to kinc_log or kinc_log_args
/// </summary>
/// <remarks>
/// When used on Android the log level is converted to the equivalent
/// Android logging level. It is currently ignored on all other targets.
/// </remarks>
typedef enum { KINC_LOG_LEVEL_INFO, KINC_LOG_LEVEL_WARNING, KINC_LOG_LEVEL_ERROR } kinc_log_level_t;

/// <summary>
/// Logging function similar to printf including some system-specific bonuses
/// </summary>
/// <remarks>
/// On most systems this is equivalent to printf.
/// On Windows it works with utf-8 strings (like printf does on any other target)
/// and also prints to the debug console in IDEs.
/// On Android this uses the android logging functions and also passes the logging level.
/// </remarks>
/// <param name="log_level">
/// The logLevel is ignored on all targets but Android where it is converted
/// to the equivalent Android log level
/// </param>
/// <param name="format">The parameter is equivalent to the first printf parameter.</param>
/// <param name="...">The parameter is equivalent to the second printf parameter.</param>
KINC_FUNC void kinc_log(kinc_log_level_t log_level, const char *format, ...);

/// <summary>
/// Equivalent to kinc_log but uses a va_list parameter
/// </summary>
/// <remarks>
/// You will need this if you want to log parameters using va_start/va_end.
/// </remarks>
/// <param name="log_level">
/// The logLevel is ignored on all targets but Android where it is converted
/// to the equivalent Android log level
/// </param>
/// <param name="format">The parameter is equivalent to the first vprintf parameter.</param>
/// <param name="args">The parameter is equivalent to the second vprintf parameter.</param>
KINC_FUNC void kinc_log_args(kinc_log_level_t log_level, const char *format, va_list args);

#ifdef KINC_IMPLEMENTATION_ROOT
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

#include <stdio.h>
#include <string.h>

#ifdef KINC_IMPLEMENTATION_ROOT
#undef KINC_IMPLEMENTATION
#endif
#include <kinc/string.h>
#include <kinc/system.h>
#ifdef KINC_IMPLEMENTATION_ROOT
#define KINC_IMPLEMENTATION
#endif

#ifdef KORE_MICROSOFT
#include <Windows.h>
#include <kinc/backend/SystemMicrosoft.h>
#endif

#ifdef KORE_ANDROID
#include <android/log.h>
#endif

void kinc_log(kinc_log_level_t level, const char *format, ...) {
	va_list args;
	va_start(args, format);
	kinc_log_args(level, format, args);
	va_end(args);
}

#define UTF8

void kinc_log_args(kinc_log_level_t level, const char *format, va_list args) {
#ifdef KORE_MICROSOFT
#ifdef UTF8
	wchar_t buffer[4096];
	kinc_microsoft_format(format, args, buffer);
	kinc_wstring_append(buffer, L"\r\n");
	OutputDebugString(buffer);
#ifdef KORE_WINDOWS
	DWORD written;
	WriteConsole(GetStdHandle(level == KINC_LOG_LEVEL_INFO ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE), buffer, (DWORD)kinc_wstring_length(buffer), &written, NULL);
#endif
#else
	char buffer[4096];
	vsnprintf(buffer, 4090, format, args);
	kinc_string_append(buffer, "\r\n");
	OutputDebugStringA(buffer);
#ifdef KORE_WINDOWS
	DWORD written;
	WriteConsoleA(GetStdHandle(level == KINC_LOG_LEVEL_INFO ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE), buffer, (DWORD)kinc_string_length(buffer), &written, NULL);
#endif
#endif
#else
	char buffer[4096];
	vsnprintf(buffer, 4090, format, args);
	kinc_string_append(buffer, "\n");
	fprintf(level == KINC_LOG_LEVEL_INFO ? stdout : stderr, "%s", buffer);
#endif

#ifdef KORE_ANDROID
	switch (level) {
	case KINC_LOG_LEVEL_INFO:
		__android_log_vprint(ANDROID_LOG_INFO, "Kinc", format, args);
		break;
	case KINC_LOG_LEVEL_WARNING:
		__android_log_vprint(ANDROID_LOG_WARN, "Kinc", format, args);
		break;
	case KINC_LOG_LEVEL_ERROR:
		__android_log_vprint(ANDROID_LOG_ERROR, "Kinc", format, args);
		break;
	}
#endif
}

#endif

#ifdef __cplusplus
}
#endif
