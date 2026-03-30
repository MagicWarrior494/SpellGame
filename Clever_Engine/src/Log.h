#pragma once
#include <cstdio>
#include <cstdarg>

#ifndef CLEVER_ENGINE_API
    #ifdef _WIN32
        #define CLEVER_ENGINE_API __declspec(dllexport)
    #else
        #define CLEVER_ENGINE_API
    #endif
#endif

class CLEVER_ENGINE_API Log
{
public:
    static void Info(const char* fmt, ...)
    {
        printf("[INFO]  ");
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
        printf("\n");
        fflush(stdout);
    }

    static void Warn(const char* fmt, ...)
    {
        printf("[WARN]  ");
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
        printf("\n");
        fflush(stdout);
    }

    static void Error(const char* fmt, ...)
    {
        fprintf(stderr, "[ERROR] ");
        va_list args;
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);
        fprintf(stderr, "\n");
        fflush(stderr);
    }
};
