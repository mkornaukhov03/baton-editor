#ifndef BATON_LSP_BASIC_H
#define BATON_LSP_BASIC_H

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

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
enum class TraceLevel
{
  Off = 0,
  Messages = 1,
  Verbose = 2,
};
enum class TextDocumentSyncKind
{
  /// Documents should not be synced at all.
  None = 0,

  /// Documents are synced by always sending the full content of the document.
  Full = 1,

  /// Documents are synced by sending the full content on open.  After that
  /// only incremental updates to the document are send.
  Incremental = 2,
};
enum class CompletionItemKind
{
  Missing = 0,
  Text = 1,
  Method = 2,
  Function = 3,
  Constructor = 4,
  Field = 5,
  Variable = 6,
  Class = 7,
  Interface = 8,
  Module = 9,
  Property = 10,
  Unit = 11,
  Value = 12,
  Enum = 13,
  Keyword = 14,
  Snippet = 15,
  Color = 16,
  File = 17,
  Reference = 18,
  Folder = 19,
  EnumMember = 20,
  Constant = 21,
  Struct = 22,
  Event = 23,
  Operator = 24,
  TypeParameter = 25,
};
enum class SymbolKind
{
  File = 1,
  Module = 2,
  Namespace = 3,
  Package = 4,
  Class = 5,
  Method = 6,
  Property = 7,
  Field = 8,
  Constructor = 9,
  Enum = 10,
  Interface = 11,
  Function = 12,
  Variable = 13,
  Constant = 14,
  String = 15,
  Number = 16,
  Boolean = 17,
  Array = 18,
  Object = 19,
  Key = 20,
  Null = 21,
  EnumMember = 22,
  Struct = 23,
  Event = 24,
  Operator = 25,
  TypeParameter = 26
};
enum class OffsetEncoding
{
  // Any string is legal on the wire. Unrecognized encodings parse as this.
  UnsupportedEncoding,
  // Length counts code units of UTF-16 encoded text. (Standard LSP behavior).
  UTF16,
  // Length counts bytes of UTF-8 encoded text. (Clangd extension).
  UTF8,
  // Length counts codepoints in unicode text. (Clangd extension).
  UTF32,
};
enum class MarkupKind
{
  PlainText,
  Markdown,
};
enum class ResourceOperationKind
{
  Create,
  Rename,
  Delete
};
enum class FailureHandlingKind
{
  Abort,
  Transactional,
  Undo,
  TextOnlyTransactional
};


template <class T>
using optional = std::optional<T>;
using uinteger = uint32_t;
using DocumentUri = std::string_view;
using TextType = std::string_view;
class URIForFile {
  // Implementation according to https://tools.ietf.org/html/rfc3986#section-2
 public:
  URIForFile(const std::string& filename, bool is_absolute)
      : is_absolute_(is_absolute), file_name_(Encode(filename)) {
    if (is_absolute_) {
      file_name_ = "file://" + file_name_;
    } else {
      file_name_ = "file:///" + file_name_;
    }
  }
  [[nodiscard]] std::string_view str() const { return file_name_; }

  [[nodiscard]] bool is_absolute() const { return is_absolute_; }

  friend bool operator==(const URIForFile& lhs, const URIForFile& rhs) {
    return lhs.file_name_ == rhs.file_name_;
  }
  friend bool operator!=(const URIForFile& lhs, const URIForFile& rhs) {
    return lhs.file_name_ != rhs.file_name_;
  }

  void set_filename(const std::string& filename, bool is_absolute) {
    is_absolute_ = is_absolute;
    file_name_ = Encode(filename);
    if (is_absolute_) {
      file_name_ = "file://" + file_name_;
    } else {
      file_name_ = "file:///" + file_name_;
    }
  }

  void set_from_encoded(const std::string& filename) {
    is_absolute_ = filename.find("file:///") != std::string::npos;
    file_name_ = filename;
  }

 private:
  static std::string Encode(std::string_view str) {
    // must not be invoked at same filename twice or more times
    // because % symbol will encoded as %25 etc.

    static auto to_hex = [](uint8_t ch) -> uint8_t {
      if (ch > 9) {
        return ch + 65;  // 65 - ASCII code for 'A'
      }
      return ch + 48;  // 48 - ASCII code for '0'
    };
    static std::string unreserved_special = "._~-";
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

struct LSPError {
  LSPError(std::string msg, ErrorCode code)
      : message_(std::move(msg)), error_code_(code) {}

  std::string message_;
  ErrorCode error_code_{};
};



struct TextDocumentIdentifier {
  DocumentUri uri;
};

struct VersionedTextDocumentIdentifier : TextDocumentIdentifier {
  uinteger version {};
};

struct Position {
  // zero-based
  uinteger line {};
  uinteger character {};

  friend bool operator==(const Position &lhs, const Position &rhs)
  {
    return std::tie(lhs.line, lhs.character) == std::tie(rhs.line, rhs.character);
  }
  friend bool operator!=(const Position &lhs, const Position &rhs)
  {
    return !(lhs == rhs);
  }
  friend bool operator<(const Position &lhs, const Position &rhs)
  {
    return std::tie(lhs.line, lhs.character) < std::tie(rhs.line, rhs.character);
  }
  friend bool operator<=(const Position &lhs, const Position &rhs)
  {
    return std::tie(lhs.line, lhs.character) <= std::tie(rhs.line, rhs.character);
  }
};

struct Range {
  // [start; end)
  Position start;
  Position end;

  friend bool operator==(const Range &lhs, const Range &rhs)
  {
    return std::tie(lhs.start, lhs.end) == std::tie(rhs.start, rhs.end);
  }
  friend bool operator!=(const Range &lhs, const Range &rhs)
  {
    return !(lhs == rhs);
  }
  friend bool operator<(const Range &lhs, const Range &rhs)
  {
    return std::tie(lhs.start, lhs.end) < std::tie(rhs.start, rhs.end);
  }
  [[nodiscard]] bool contains(Position Pos) const
  {
    return start <= Pos && Pos < end;
  }
  [[nodiscard]] bool contains(Range Rng) const
  {
    return start <= Rng.start && Rng.end <= end;
  }
  
};

struct Location {
  DocumentUri uri;
  Range range;

  friend bool operator==(const Location &LHS, const Location &RHS)
  {
    return LHS.uri == RHS.uri && LHS.range == RHS.range;
  }
  friend bool operator!=(const Location &LHS, const Location &RHS)
  {
    return !(LHS == RHS);
  }
  friend bool operator<(const Location &LHS, const Location &RHS)
  {
    return std::tie(LHS.uri, LHS.range) < std::tie(RHS.uri, RHS.range);
  }

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
template <>
struct adl_serializer<lsp::URIForFile> {
  static void to_json(json& j, const lsp::URIForFile& uri) { j = uri.str(); }
  static void from_json(const json& j, lsp::URIForFile& uri) {
    uri.set_from_encoded(j.get<std::string>());
  }
};
}  // namespace nlohmann

#endif  // BATON_LSP_BASIC_H
