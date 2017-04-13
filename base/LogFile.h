#ifndef LOGFILE_H
#define LOGFILE_H

#include "base/Mutex.h"

#include "base/Copyable.h"
#include <memory>
#include <string>

namespace ouge {
namespace FileUtil {

class AppendFile;
}

class LogFile : NonCopyable {
 public:
  LogFile(const std::string& basename, size_t rollSize, bool threadSafe = true,
          int flushInterval = 3, int checkEveryN = 1024);
  ~LogFile();

  void append(const char* logline, int len);
  void flush();
  bool roolFile();

 private:
  void append_unlocked(const char* logline, int len);
  static std::string getLogFileName(const std::string& basename, time_t* now);

  const std::string basename_;
  const size_t rollSize_;
  const int flushInterval_;
  const int checEveryN_;

  int count_;
  std::unique_ptr<MutexLock> mutex_;
  time_t startOfPeriod_;
  time_t lastRoll_;
  time_t lastFlush_;
  std::unique_ptr<FileUtil::AppendFile> file_;

  const static int kRoolPerSeconds_ = 60 * 60 * 24;
};
}

#endif /* LOGFILE_H */
