#ifndef BATON_LSP_BASIC_H
#define BATON_LSP_BASIC_H

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include "nlohmann/json.hpp"

namespace lsp {
// Implementation of basic Language Server Protocol types

using json = nlohmann::json;
enum class ErrorCode {
  // Defined by JSON RPC.
  ParseError = -32700,
  InvalidRequest = -32600,
  MethodNotFound = -32601,
  InvalidParams = -32602,
  InternalError = -32603,

  ServerNotInitialized = -32002,
  UnknownErrorCode = -32001,

  // Defined by the protocol.
  RequestCancelled = -32800,
  ContentModified = -32801,
};
using string = std::string_view;
template <class T>
using optional = std::optional<T>;
using uinteger = uint32_t;
using DocumentUri = string;

// Implementation according to https://tools.ietf.org/html/rfc3986#section-2
class URIForFile {
 public:
  URIForFile(bool is_absolute, const std::string& filename)
      : is_absolute_(is_absolute), file_name_(Encode(filename)) {
    if (is_absolute_) {
      file_name_ = "file://" + file_name_;
    } else {
      file_name_ = "file:///" + file_name_;
    }
  }
  [[nodiscard]] string str() const { return file_name_; }

  [[nodiscard]] bool absolute() const { return is_absolute_; }

  friend bool operator==(const URIForFile& lhs, const URIForFile& rhs) {
    return lhs.file_name_ == rhs.file_name_;
  }
  friend bool operator!=(const URIForFile& lhs, const URIForFile& rhs) {
    return lhs.file_name_ != rhs.file_name_;
  }

  void set_filename(bool is_absolute, const std::string& filename) {
    is_absolute_ = is_absolute;
    file_name_ = Encode(filename);
    if (is_absolute_) {
      file_name_ = "file://" + file_name_;
    } else {
      file_name_ = "file:///" + file_name_;
    }
  }

 private:
  static std::string Encode(string str) {
    // must not be invoked at same filename twice or more times
    // because % symbol will encoded as %25 etc.

    static auto to_hex = [](uint8_t ch) -> uint8_t {
      if (ch > 9) {
        return ch + 65;  // 65 - ASCII code for 'A'
      }
      return ch + 48;  // 48 - ASCII code for '0'
    };
    // in https://github.com/cpeditor is "._-*/:"
    static std::string unreserved_special = "._~-/";
    std::string response;
    for (uint8_t ch : str) {
      if (ch == '\\') {  // for Windows
        ch = '/';
      }
      if (std::isalnum(ch) ||
          unreserved_special.find(ch) != std::string::npos) {
        // what if 'ma//in.cpp' ???
        response.push_back(ch);
      } else {
        response.push_back('%');
        response.push_back(to_hex(ch >> 4));              // first 4 bits
        response.push_back(to_hex(ch & ((1 << 4) - 1)));  // last 4 bits
      }
    }

    return response;
  }
  bool is_absolute_{};
  std::string file_name_;
};

}  // namespace lsp

namespace nlohmann {
template <typename T>
struct adl_serializer<lsp::optional<T>> {
  static void to_json(json& j, const lsp::optional<T>& opt) {
    if (opt) {
      j = opt.value();
    } else {
      j = nullptr;
    }
  }
  static void from_json(const json& j, lsp::optional<T>& opt) {
    if (j.is_null()) {
      opt = lsp::optional<T>();
    } else {
      opt = lsp::optional<T>(j.get<T>());
    }
  }
};
}  // namespace nlohmann

#endif  // BATON_LSP_BASIC_H
