// Here is definition of methods and friend functions, declared in "lsp_basic.h"
#include "lsp_basic.h"

namespace lsp {

URIForFile::URIForFile(const std::string &filename)
    : file_name_("file:///" + Encode(filename)) {}
[[nodiscard]] std::string_view URIForFile::str() const { return file_name_; }

bool URIForFile::operator==(const URIForFile &oth) const {
  return file_name_ == oth.file_name_;
}
bool URIForFile::operator!=(const URIForFile &oth) const {
  return !(*this == oth);
}
void URIForFile::set_filename(const std::string &filename) {
  file_name_ = "file:///" + Encode(filename);
}

void URIForFile::set_from_encoded(const std::string &filename) {
  file_name_ = filename;
}

std::string URIForFile::Encode(std::string_view str) {
  static auto to_hex = [](uint8_t ch) -> uint8_t {
    if (ch > 9) {
      return ch + 'A';
    }
    return ch + '0';
  };
  static std::string unreserved_special = "._~-";
  std::string response;
  for (uint8_t ch : str) {
    if (ch == '\\') {  // for Windows
      ch = '/';
    }
    if (std::isalnum(ch) || unreserved_special.find(ch) != std::string::npos) {
      response.push_back(ch);
    } else {
      response.push_back('%');
      response.push_back(to_hex(ch >> 4));              // first 4 bits
      response.push_back(to_hex(ch & ((1 << 4) - 1)));  // last 4 bits
    }
  }

  return response;
}

LSPError::LSPError(std::string msg, ErrorCode code)
    : message_(std::move(msg)), error_code_(code) {}

bool Position::operator==(const Position &oth) const {
  return std::tie(line, character) == std::tie(oth.line, oth.character);
}

bool Position::operator!=(const Position &oth) const { return !(*this == oth); }

bool Position::operator<(const Position &oth) const {
  return std::tie(line, character) < std::tie(oth.line, oth.character);
}

bool Position::operator<=(const Position &oth) const {
  return *this < oth || *this == oth;
}

bool Range::operator==(const Range &oth) const {
  return std::tie(start, end) == std::tie(oth.start, oth.end);
}
bool Range::operator!=(const Range &oth) const { return !(*this == oth); }
bool Range::operator<(const Range &oth) const {
  return std::tie(start, end) < std::tie(oth.start, oth.end);
}
[[nodiscard]] bool Range::contains(const Position &Pos) const {
  return start <= Pos && Pos < end;
}
[[nodiscard]] bool Range::contains(const Range &Rng) const {
  return start <= Rng.start && Rng.end <= end;
}

bool Location::operator==(const Location &oth) const {
  return uri == oth.uri && range == oth.range;
}
bool Location::operator!=(const Location &oth) const { return !(*this == oth); }
bool Location::operator<(const Location &oth) const {
  return std::tie(uri, range) < std::tie(oth.uri, oth.range);
}

ClientCapabilities::ClientCapabilities() {
  for (int i = 1; i <= 26; ++i) {
    WorkspaceSymbolKinds.push_back((SymbolKind)i);
  }
  for (int i = 0; i <= 25; ++i) {
    CompletionItemKinds.push_back((CompletionItemKind)i);
  }
}

}  // namespace lsp
