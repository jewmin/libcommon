#include "logger.h"

#ifdef _MSC_VER
#define TRED    FOREGROUND_RED | FOREGROUND_INTENSITY                       // ºì
#define TGREEN  FOREGROUND_GREEN | FOREGROUND_INTENSITY                     // ÂÌ
#define TYELLOW FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY    // »Æ
#define TNORMAL FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE         // ÆÕÍ¨
#define TWHITE  TNORMAL | FOREGROUND_INTENSITY                              // °×
#define TBLUE   FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY   // À¶
#else
#define TRED    1 // ºì
#define TGREEN  2 // ÂÌ
#define TYELLOW 3 // »Æ
#define TNORMAL 4 // ÆÕÍ¨
#define TWHITE  5 // °×
#define TBLUE   6 // À¶

static const char * ColorStrings[TBLUE + 1] =
{
    "",
    "\033[22;31m",
    "\033[22;32m",
    "\033[01;33m",
    "\033[0m",
    "\033[01;37m",
    "\033[22;34m",
};
#endif

uint16_t GetLogColor(Logger::LogLevel log_level)
{
    switch (log_level)
    {
        case Logger::Trace: return TBLUE;
        case Logger::Debug: return TBLUE;
        case Logger::Info:  return TGREEN;
        case Logger::Warn:  return TYELLOW;
        case Logger::Error: return TRED;
        case Logger::Fatal: return TRED;
        default:            return TNORMAL;
    }
}

void SetColor(Logger::LogLevel log_level)
{
#ifdef _MSC_VER
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GetLogColor(log_level));
#else
    fprintf(stdout, "%s", ColorStrings[GetLogColor(log_level)]);
#endif
}

Logger::Logger()
{
    log_level_ = Info;
}

Logger::~Logger()
{
    log_list_.Flush();
    allocator_.ReleaseList(static_cast<Packet * *>(log_list_), log_list_.Count());
    log_list_.Clear();
}

int Logger::InitLogger(LogLevel log_level)
{
    log_level_ = log_level;

    return Start();
}

void Logger::DeInitLogger()
{
    Stop();
}

void Logger::LogStub(LogLevel log_level, const char * file, const char * func, const int line, const char * fmt, ...)
{
    if (log_level_ <= log_level)
    {
        va_list ap; 
        va_start(ap, fmt);
        LogStub2(log_level, file, func, line, fmt, ap);
        va_end(ap);
    }
}

void Logger::LogStub2(LogLevel log_level, const char * file, const char * func, const int line, const char * fmt, va_list ap)
{
    thread_local static char msg[2048] = {0};
    thread_local static int offset = 0;
    thread_local static int len = 0;

    offset = 0;
    len = sizeof(msg) - 1;
    int ret = snprintf(msg, len, "%s[%s:%s:%d] ", GetLogPrefix(log_level), file, func, line);
    if (ret < 0) return;

    offset += ret;
    len -= ret;
    ret = vsnprintf(msg + offset, len, fmt, ap);
    if (ret < 0) return;
    else if (ret > len) offset += len;
    else offset += ret;
    msg[offset] = 0;

    allocator_lock_.Lock();
    Packet * packet = allocator_.Allocate();
    allocator_lock_.Unlock();

    packet->SetLength(0);
    packet->WriteAtom<uint8_t>(log_level);
    packet->WriteString(msg, offset);
    log_list_.Push(packet);
}

const char * Logger::GetLogPrefix(const LogLevel log_level)
{
    switch (log_level)
    {
        case Trace: return "[TRACE]";
        case Debug: return "[DEBUG]";
        case Info:  return "[INFO]";
        case Warn:  return "[WARN]";
        case Error: return "[ERROR]";
        case Fatal: return "[FATAL]";
        default:    return "";
    }
}

void Logger::SingleRun()
{
    if (log_list_.AppendCount() > 0)
    {
        static char time_string[256];
        time_t raw_time;
        struct tm * time_info;
        time(&raw_time);
        time_info = localtime(&raw_time);
        strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S ", time_info);

        Update(time_info);

        log_list_.Flush();
        int count = log_list_.Count();
        for (int i = 0; i < count; ++i)
        {
            Packet * packet = log_list_[i];
            packet->SetPosition(0);
            LogLevel log_level = static_cast<LogLevel>(packet->ReadAtom<uint8_t>());
            const char * msg = packet->ReadString();
            if (msg) Log(log_level, time_string, msg);
        }

        allocator_lock_.Lock();
        allocator_.ReleaseList(static_cast<Packet * *>(log_list_), count);
        log_list_.Clear();
        allocator_lock_.Unlock();
    }
}

void Logger::Log(LogLevel log_level, const char * time_string, const char * msg)
{
    SetColor(log_level);
    fprintf(stdout, time_string);
    fprintf(stdout, msg);
    fprintf(stdout, "\n");
    fflush(stdout);
}

void Logger::Run()
{
    while (!IsTerminated())
    {
        clock_t begin = clock();
        SingleRun();
        clock_t end = clock();
        int duration = static_cast<int>(static_cast<double>(end - begin) / CLOCKS_PER_SEC * 1000);
        if (duration < 16) jc_sleep(16 - duration);
    }
}

void Logger::OnTerminated()
{
    SingleRun();
}