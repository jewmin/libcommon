#include "gtest/gtest.h"
#include "log_test.h"
#include "uv.h"

TEST(LogTest, use)
{
    Logger logger;
    logger.InitLogger(Logger::Info);
    logger.LogTrace("this is %s log", "trace");
    logger.LogDebug("this is %s log", "debug");
    logger.LogInfo("this is %s log", "info");
    logger.LogWarn("this is %s log", "warn");
    logger.LogError("this is %s log", "error");
    jc_sleep(32);
    logger.SetLogLevel(Logger::Trace);
    logger.LogFatal("this is %s log", "fatal");
    logger.LogError("this is %s log", "error");
    logger.LogWarn("this is %s log", "warn");
    logger.LogInfo("this is %s log", "info");
    logger.LogDebug("this is %s log", "debug");
    logger.LogTrace("this is %s log", "trace");
    logger.DeInitLogger();
}

void thread_cb(void * logger)
{
    FileLogger * pLogger = reinterpret_cast<FileLogger *>(logger);
    pLogger->LogTrace("this is %p thread, log trace!!!", uv_thread_self());
    pLogger->LogDebug("this is %p thread, log debug!!!", uv_thread_self());
    pLogger->LogInfo("this is %p thread, log info!!!", uv_thread_self());
    pLogger->LogWarn("this is %p thread, log warn!!!", uv_thread_self());
    pLogger->LogError("this is %p thread, log error!!!", uv_thread_self());
    pLogger->LogFatal("this is %p thread, log fatal!!!", uv_thread_self());
}

TEST(LogTest, multi_thread)
{
    FileLogger logger("unit_test.log");
    logger.InitLogger(Logger::Trace);
    uv_thread_t tid[10];
    uv_thread_create(&tid[0], thread_cb, &logger);
    uv_thread_create(&tid[1], thread_cb, &logger);
    uv_thread_create(&tid[2], thread_cb, &logger);
    uv_thread_create(&tid[3], thread_cb, &logger);
    uv_thread_create(&tid[4], thread_cb, &logger);
    uv_thread_create(&tid[5], thread_cb, &logger);
    uv_thread_create(&tid[6], thread_cb, &logger);
    uv_thread_create(&tid[7], thread_cb, &logger);
    uv_thread_create(&tid[8], thread_cb, &logger);
    uv_thread_create(&tid[9], thread_cb, &logger);
    for (int i = 0; i < 10; ++i)
        uv_thread_join(&tid[i]);
    logger.DeInitLogger();
}

TEST(LogTest, no_start_thread)
{
    Logger logger;
    logger.LogTrace("this is %s log", "trace");
    logger.LogDebug("this is %s log", "debug");
    logger.LogInfo("this is %s log", "info");
    logger.LogWarn("this is %s log", "warn");
    logger.LogError("this is %s log", "error");
}