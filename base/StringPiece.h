#ifndef OUGE_BASE_STRINGPIECE_H
#define OUGE_BASE_STRINGPIECE_H
// TODO:delte this file

#include "base/Types.h"
#include "base/Copyable.h"

#include <cstring>

namespace ouge {
// 专门用于处理字符串参数,使得 char* 和string 具有统一性
class StringArg : public Copyable {
  public:
    // 可隐式转换
    StringArg(const char* str) : str_(str) {}
    // 可隐式转换
    StringArg(const std::string& str) : str_(str.c_str()) {}

    const char* c_str() const { return str_; }

  private:
    const char* str_;
};

// 字符串类,封装了一些便于使用的函数
class StringPiece : public Copyable {
  private:
    const char* ptr_;
    int         length_;

  public:
    StringPiece() : ptr_(NULL), length_(0) {}
    StringPiece(const char* str)
            : ptr_(str), length_(static_cast<int>(strlen(ptr_))) {}
    StringPiece(const unsigned char* str)
            : ptr_(reinterpret_cast<const char*>(str)),
              length_(static_cast<int>(strlen(ptr_))) {}
    StringPiece(const std::string& str)
            : ptr_(str.data()), length_(static_cast<int>(str.size())) {}
    StringPiece(const char* offset, int len) : ptr_(offset), length_(len) {}

    const char* data() const { return ptr_; }
    int         size() const { return length_; }
    bool        empty() const { return length_ == 0; }
    const char* begin() const { return ptr_; }
    const char* end() const { return ptr_ + length_; }

    void clear() {
        ptr_    = NULL;
        length_ = 0;
    }
    void set(const char* buffer, int len) {
        ptr_    = buffer;
        length_ = len;
    }
    void set(const char* str) {
        ptr_    = str;
        length_ = static_cast<int>(strlen(str));
    }
    void set(const void* buffer, int len) {
        ptr_    = reinterpret_cast<const char*>(buffer);
        length_ = len;
    }

    char operator[](int i) const { return ptr_[i]; }

    // 删除前 n 个字节
    void remove_prefix(int n) {
        ptr_ += n;
        length_ -= n;
    }

    // 删除后 n 个字节
    void remove_suffix(int n) { length_ -= n; }

    bool operator==(const StringPiece& x) const {
        return ((length_ == x.length_) && (memcmp(ptr_, x.ptr_, length_) == 0));
    }
    bool operator!=(const StringPiece& x) const { return !(*this == x); }

    bool operator<(const StringPiece& x) const {
        int r = memcmp(ptr_, x.ptr_, length_ < x.length_ ? length_ : x.length_);
        return ((r < 0) || ((r == 0) && (length_ < x.length_)));
    }
    bool operator<=(const StringPiece& x) const {
        int r = memcmp(ptr_, x.ptr_, length_ < x.length_ ? length_ : x.length_);
        return ((r < 0) || ((r == 0) && (length_ <= x.length_)));
    }
    bool operator>(const StringPiece& x) const { return !(*this <= x); }
    bool operator>=(const StringPiece& x) const { return !(*this < x); }

    int compare(const StringPiece& x) const {
        int r = memcmp(ptr_, x.ptr_, length_ < x.length_ ? length_ : x.length_);
        if (r == 0) {
            if (length_ < x.length_)
                r = -1;
            else if (length_ > x.length_)
                r = +1;
        }
        return r;
    }

    std::string as_string() const { return std::string(ptr_, length_); }

    void CopyToString(std::string* target) const {
        target->assign(ptr_, length_);
    }

    // 判断是不是以x开头
    bool starts_with(const StringPiece& x) const {
        return ((length_ >= x.length_)
                && (memcmp(ptr_, x.ptr_, x.length_) == 0));
    }
};

}    // namespace ouge

std::ostream& operator<<(std::ostream& o, const ouge::StringPiece& piece) {
    o << piece.as_string();
    return o;
}

#endif
