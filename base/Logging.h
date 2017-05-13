#ifndef BASE_LOGGING_H
#define BASE_LOGGING_H

#include "base/LogStream.h"
#include "base/Timestamp.h"

namespace ouge {

class TimeZone;

class Logger {
  public:
    enum LogLevel {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LOG_LEVELS,
    };

    // 对文件名的封装，格式化文件名
    // TODO: 区别
    class SourceFile {
      public:
        // 可以根据数组的大小自动推断N
        template <int N>
        inline SourceFile(const char (&arr)[N]) : data_(arr), size_(N - 1) {
            const char* slash = strrchr(data_, '/');
            if (slash) {
                data_ = slash + 1;
                size_ -= static_cast<int>(data_ - arr);
            }
        }

        explicit SourceFile(const char* filename) : data_(filename) {
            const char* slash = strrchr(filename, '/');
            if (slash) {
                data_ = slash + 1;
            }
            size_ = static_cast<int>(strlen(data_));
        }

        const char* data_;
        int         size_;
    };

    Logger(SourceFile file, int line);
    Logger(SourceFile file, int line, LogLevel level);
    Logger(SourceFile file, int line, LogLevel level, const char* func);
    Logger(SourceFile file, int line, bool toAbort);
    ~Logger();

    LogStream& stream() { return impl_.stream_; }

    static LogLevel logLevel();
    static void setLogLevel(LogLevel level);

    using OutputFunc = void (*)(const char* msg, int len);
    using FlushFunc  = void (*)(void);

    static void setOutput(OutputFunc);
    static void setFlush(FlushFunc);
    static void setTimeZone(const TimeZone& tz);

  private:
    // 具体实现
    class Impl {
      public:
        using LogLevel = Logger::LogLevel;
        Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
        void formatTime();
        void finish();

        Timestamp  time_;
        LogStream  stream_;
        LogLevel   level_;
        int        line_;
        SourceFile basename_;
    };

    Impl impl_;
};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel
Logger::logLevel() {
    return g_logLevel;
}

#define LOG_TRACE                                                              \
    if (ouge::Logger::logLevel() <= ouge::Logger::TRACE)                       \
    ouge::Logger(__FILE__, __LINE__, ouge::Logger::TRACE, __func__).stream()
#define LOG_DEBUG                                                              \
    if (ouge::Logger::logLevel() <= ouge::Logger::DEBUG)                       \
    ouge::Logger(__FILE__, __LINE__, ouge::Logger::DEBUG, __func__).stream()
#define LOG_INFO                                                               \
    if (ouge::Logger::logLevel() <= ouge::Logger::INFO)                        \
    muduo::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN ouge::Logger(__FILE__, __LINE__, ouge::Logger::WARN).stream()
#define LOG_ERROR ouge::Logger(__FILE__, __LINE__, ouge::Logger::ERROR).stream()
#define LOG_FATAL ouge::Logger(__FILE__, __LINE__, ouge::Logger::FATAL).stream()
#define LOG_SYSERR ouge::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL ouge::Logger(__FILE__, __LINE__, true).stream()

const char* strerror_tl(int savedErrno);

#define CHECK_NOTNULL(val)                                                     \
    ::ouge::CheckNotNull(                                                      \
            __FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

template <typename T>
T*
CheckNotNull(Logger::SourceFile file, int line, const char* names, T* ptr) {
    if (ptr == NULL) {
        Logger(file, line, Logger::FATAL).stream() << names;
    }
    return ptr;
}
}

#endif