/*
 *
 * Spine OpenGL
 *
 * @author 	: keith@robot9.me
 * @date	: 2021/03/30
 *
 */

#ifndef SPINE_RENDER_LOGGER_H
#define SPINE_RENDER_LOGGER_H

#include <cstdio>
#include <cstring>
#include <cstdarg>

namespace SpineRender {

#define MAX_LOG_LENGTH 1024
#define LOG_SOURCE_LINE

#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)

#define LOG_INFO(...)       SpineRender::Logger::log(SpineRender::INFO,     __FILENAME__, __LINE__, __VA_ARGS__)
#define LOG_WARNING(...)    SpineRender::Logger::log(SpineRender::WARNING,  __FILENAME__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...)      SpineRender::Logger::log(SpineRender::ERROR,    __FILENAME__, __LINE__, __VA_ARGS__)
#define LOG_FATAL(...)      SpineRender::Logger::log(SpineRender::FATAL,    __FILENAME__, __LINE__, __VA_ARGS__)

enum LogLevel {
    INFO,
    WARNING,
    ERROR,
    FATAL
};

typedef void (*LogFunc)(void *context, int level, const char *msg);

class Logger {
public:
    static void setLogFunc(void *ctx, LogFunc func) {
        logContext = ctx;
        logFunc = func;
    }

    static void log(LogLevel level, const char *file, int line, const char *message, ...) {
        va_list argPtr;
        va_start(argPtr, message);
        vsnprintf(buf, MAX_LOG_LENGTH - 1, message, argPtr);
        va_end(argPtr);
        buf[MAX_LOG_LENGTH - 1] = '\0';

        if (logFunc != nullptr) {
            logFunc(logContext, level, buf);
            return;
        }

        switch (level) {
#ifdef LOG_SOURCE_LINE
            case INFO:      fprintf(stdout, "[SpineRender][INFO] %s:%d: %s\n", file, line, buf);    break;
            case WARNING:   fprintf(stdout, "[SpineRender][WARNING] %s:%d: %s\n", file, line, buf); break;
            case ERROR:     fprintf(stderr, "[SpineRender][ERROR] %s:%d: %s\n", file, line, buf);   break;
            case FATAL:     fprintf(stderr, "[SpineRender][FATAL] %s:%d: %s\n", file, line, buf);   break;
#else
            case INFO:      fprintf(stdout, "[SpineRender][INFO] : %s\n", buf);                     break;
            case WARNING:   fprintf(stdout, "[SpineRender][WARNING] : %s\n", buf);                  break;
            case ERROR:     fprintf(stderr, "[SpineRender][ERROR] : %s\n", buf);                    break;
            case FATAL:     fprintf(stderr, "[SpineRender][FATAL] : %s\n", buf);                    break;
#endif
        }
    }

private:
    static void *logContext;
    static LogFunc logFunc;

    static char buf[MAX_LOG_LENGTH];
};

}

#endif //SPINE_RENDER_LOGGER_H
