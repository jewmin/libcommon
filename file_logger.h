#ifndef __LIBCOMMON_FILE_LOGGGER_H__
#define __LIBCOMMON_FILE_LOGGGER_H__

#include "logger.h"

class FileLogger : public Logger {
public:
    FileLogger(const char * file_name);
    virtual ~FileLogger();

protected:
    inline void OpenFile() {
        CloseFile();

        char fn[MAX_PATH];
        snprintf(fn, sizeof(fn), "%s.%d-%d-%d", file_name_, last_time_.tm_year + 1900, last_time_.tm_mon + 1, last_time_.tm_mday);
        fn[MAX_PATH - 1] = 0;

        file_ = fopen(fn, "a+");
        if (!file_) {
            fprintf(stderr, "could not open log file %s", fn);
        }
    }

    inline void CloseFile() {
        if (file_) {
            fclose(file_);
            file_ = nullptr;
        }
    }

    void Update(struct tm * time_info) override;
    void Log(LogLevel log_level, const char * time_string, const char * msg) override;

private:
    FILE * file_;
    char file_name_[MAX_PATH];
    struct tm last_time_;
};

#endif