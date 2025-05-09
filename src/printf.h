#pragma once

#include <optional>
#include <regex>
#include <utility>

#include "printf_format_types.h"
#include "types.h"

namespace bpftrace {

static std::string generate_pattern_string()
{
  std::string pattern = "%-?[0-9]*(\\.[0-9]+)?(";
  for (const auto& e : printf_format_types) {
    pattern += e.first + "|";
  }
  pattern.pop_back(); // for the last "|"
  pattern += ")";
  return pattern;
}

const std::regex format_specifier_re(generate_pattern_string());

struct Field;

enum class ArgumentType {
  UNKNOWN = 0,
  CHAR,
  SHORT,
  INT,
  LONG,
  LONG_LONG,
  INTMAX_T,
  SIZE_T,
  PTRDIFF_T,
  POINTER,
};

class IPrintable {
public:
  virtual ~IPrintable() = default;
  virtual int print(char* buf,
                    size_t size,
                    const char* fmt,
                    Type token,
                    ArgumentType expected_type = ArgumentType::UNKNOWN) = 0;
};

class PrintableString : public virtual IPrintable {
public:
  PrintableString(std::string value,
                  std::optional<size_t> buffer_size = std::nullopt,
                  const char* trunc_trailer = nullptr);
  int print(char* buf,
            size_t size,
            const char* fmt,
            Type /*token*/,
            ArgumentType /*expected_type*/) override;

private:
  std::string value_;
};

class PrintableBuffer : public virtual IPrintable {
public:
  PrintableBuffer(char* buffer, size_t size)
      : value_(std::vector<char>(buffer, buffer + size))
  {
  }
  int print(char* buf,
            size_t size,
            const char* fmt,
            Type /*token*/,
            ArgumentType /*expected_type*/) override;
  void keep_ascii(bool value);
  void escape_hex(bool value);

private:
  std::vector<char> value_;
  bool keep_ascii_ = true;
  bool escape_hex_ = true;
};

class PrintableCString : public virtual IPrintable {
public:
  PrintableCString(char* value) : value_(value)
  {
  }
  int print(char* buf,
            size_t size,
            const char* fmt,
            Type /*token*/,
            ArgumentType /*expected_type*/) override;

private:
  char* value_;
};

class PrintableInt : public virtual IPrintable {
public:
  PrintableInt(uint64_t value) : value_(value)
  {
  }
  int print(char* buf,
            size_t size,
            const char* fmt,
            Type token,
            ArgumentType expected_type) override;

private:
  uint64_t value_;
};

class PrintableSInt : public virtual IPrintable {
public:
  PrintableSInt(int64_t value) : value_(value)
  {
  }
  int print(char* buf,
            size_t size,
            const char* fmt,
            Type token,
            ArgumentType expected_type) override;

private:
  int64_t value_;
};

class PrintableEnum : public virtual IPrintable {
public:
  PrintableEnum(uint64_t value, std::string name)
      : name_(std::move(name)), value_(value)
  {
  }
  int print(char* buf,
            size_t size,
            const char* fmt,
            Type token,
            ArgumentType expected_type) override;

private:
  std::string name_;
  uint64_t value_;
};

} // namespace bpftrace
