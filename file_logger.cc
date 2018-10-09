#include "file_logger.h"

FileLogger::FileLogger(const char * file_name) {
    strncpy(file_name_, file_name, sizeof(file_name_));
    file_name_[MAX_PATH - 1] = 0;

    file_ = nullptr;
    memset(&last_time_, 0, sizeof(last_time_));
}

FileLogger::~FileLogger() {
    CloseFile();
}

void FileLogger::Update(struct tm * time_info) {
    if (last_time_.tm_year != time_info->tm_year || last_time_.tm_mon != time_info->tm_mon || last_time_.tm_mday != time_info->tm_mday) {
        memcpy(&last_time_, time_info, sizeof(last_time_));
        OpenFile();
    }
}

void FileLogger::Log(LogLevel log_level, const char * time_string, const char * msg) {
    if (file_) {
        fprintf(file_, time_string);
        fprintf(file_, msg);
        fprintf(file_, "\n");
        fflush(file_);
    }

    Logger::Log(log_level, time_string, msg);
}