#ifndef STRINGPIECE_H
#define STRINGPIECE_H

#include "Types.h"

//#include <iosfwd>

namespace ouge {

class StringArg {
  public:
    StringArg(const char* str);
    StringArg(const std::string& str);

    const char* c_str() const;

  private:
    const char* str_;
};

class StringPiece {
  public:
    StringPiece();
    StringPiece(const char*);
    StringPiece(const unsigned char*);
    StringPiece(const std::string&);
    StringPiece(const char* offset, int len);

    const char* data() const;
    int         size() const;
    bool        empty() const;
    const char* begin() const;
    const char* end() const;
    void        clear();
    void set(const char* buffer, int len);
    void set(const char* str);
    void set(const void* buffer, int len);
    char operator[](int i) const;
    void remove_prefix(int n);
    void remove_suffix(int n);
    bool operator==(const StringPiece& x) const;
    bool operator!=(const StringPiece& x) const;
    int         compare(const StringPiece&) const;
    std::string as_string() const;
    void CopyToString(std::string* target) const;
    bool starts_with(const StringPiece&) const;

  private:
    const char* ptr_;
    int         length_;
};

std::ostream& operator<<(std::ostream& o, const ouge::StringPiece& piece);
}

#endif /* STRINGPIECE_H */
