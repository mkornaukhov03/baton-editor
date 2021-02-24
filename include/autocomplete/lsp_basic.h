#ifndef BATON_LSP_BASIC_H
#define BATON_LSP_BASIC_H

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "enums.h"
#include "nlohmann/json.hpp"

namespace lsp {

using json = nlohmann::json;

using uinteger = uint32_t;
using DocumentUri = std::string_view;
using TextType = std::string_view;

// Implementation of types, specified by LSP and LLVM to connect to communicate
// with clangd c++ language server

class URIForFile {
 public:
  explicit URIForFile(const std::string &filename)
      : file_name_(Encode(filename)) {
    file_name_ = "file:///" + file_name_;
  }
  [[nodiscard]] std::string_view str() const { return file_name_; }

  friend bool operator==(const URIForFile &lhs, const URIForFile &rhs) {
    return lhs.file_name_ == rhs.file_name_;
  }
  friend bool operator!=(const URIForFile &lhs, const URIForFile &rhs) {
    return lhs.file_name_ != rhs.file_name_;
  }

  void set_filename(const std::string &filename) {
    file_name_ = "file:///" + Encode(filename);
  }

  void set_from_encoded(const std::string &filename) { file_name_ = filename; }

 private:
  static std::string Encode(std::string_view str) {
    // must not be invoked at same filename twice or more times
    // because % symbol will encoded as %25 etc.

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
      if (std::isalnum(ch) ||
          unreserved_special.find(ch) != std::string::npos) {
        response.push_back(ch);
      } else {
        response.push_back('%');
        response.push_back(to_hex(ch >> 4));              // first 4 bits
        response.push_back(to_hex(ch & ((1 << 4) - 1)));  // last 4 bits
      }
    }

    return response;
  }
  std::string file_name_;
};

struct LSPError {
  LSPError(std::string msg, ErrorCode code);

  std::string message_;
  ErrorCode error_code_{};
};

struct TextDocumentIdentifier {
  DocumentUri uri;
};

struct VersionedTextDocumentIdentifier : TextDocumentIdentifier {
  uinteger version{};
};

struct Position {
  // zero-based
  uinteger line{};
  uinteger character{};

  bool operator==(const Position &oth) const;
  bool operator!=(const Position &oth) const;
  bool operator<(const Position &oth) const;
  bool operator<=(const Position &oth) const;
};

struct Range {
  // [start; end)
  Position start;
  Position end;

  bool operator==(const Range &oth) const;
  bool operator!=(const Range &oth) const;
  bool operator<(const Range &oth) const;
  [[nodiscard]] bool contains(const Position &Pos) const;
  [[nodiscard]] bool contains(const Range &Rng) const;
};

struct Location {
  DocumentUri uri;
  Range range;

  bool operator==(const Location &oth) const;
  bool operator!=(const Location &oth) const;
  bool operator<(const Location &oth) const;
};

struct TextEdit {
  Range range;
  std::string newText;
};

struct TextDocumentItem {
  DocumentUri uri;
  std::string_view languageId;
  uinteger version{};
  std::string_view text;
};
struct ClientCapabilities {
  std::vector<SymbolKind> WorkspaceSymbolKinds;
  bool DiagnosticFixes = true;

  bool DiagnosticRelatedInformation = true;

  bool DiagnosticCategory = true;

  bool CompletionSnippets = true;

  bool CompletionDeprecated = true;

  bool CompletionFixes = true;

  bool HierarchicalDocumentSymbol = true;

  bool OffsetsInSignatureHelp = true;

  std::vector<CompletionItemKind> CompletionItemKinds;

  bool CodeActionStructure = true;
  std::vector<OffsetEncoding> offsetEncoding = {OffsetEncoding::UTF8};
  std::vector<MarkupKind> HoverContentFormat = {MarkupKind::PlainText};

  bool ApplyEdit = false;
  bool DocumentChanges = false;
  ClientCapabilities() {
    for (int i = 1; i <= 26; ++i) {
      WorkspaceSymbolKinds.push_back((SymbolKind)i);
    }
    for (int i = 0; i <= 25; ++i) {
      CompletionItemKinds.push_back((CompletionItemKind)i);
    }
  }
};
struct ClangdCompileCommand {
  TextType workingDirectory;
  std::vector<TextType> compilationCommand;
};
struct ConfigurationSettings {
  std::map<std::string, ClangdCompileCommand>
      compilationDatabaseChanges;  // maybe hash map
};
struct InitializationOptions {
  ConfigurationSettings configSettings;

  std::optional<TextType> compilationDatabasePath;
  // Additional flags to be included in the "fallback command" used when
  // the compilation database doesn't describe an opened file.
  // The command used will be approximately `clang $FILE $fallbackFlags`.
  std::vector<TextType> fallbackFlags;

  // Clients supports show file status for textDocument/clangd.fileStatus.
  bool clangdFileStatus = false;
};
struct InitializeParams {
  unsigned processId = 0;
  ClientCapabilities capabilities;
  std::optional<DocumentUri> rootUri;
  std::optional<TextType> rootPath;
  InitializationOptions initializationOptions;
};
struct ShowMessageParams {
  // The message type.
  MessageType type = MessageType::Info;
  // The actual message.
  std::string message;
};
struct Registration {
  TextType id;
  TextType method;
};
struct RegistrationParams {
  std::vector<Registration> registrations;
};
struct UnregistrationParams {
  std::vector<Registration> unregisterations;
};
struct DidOpenTextDocumentParams {
  // The document that was opened.
  TextDocumentItem textDocument;
};
struct DidCloseTextDocumentParams {
  // The document that was closed.
  TextDocumentIdentifier textDocument;
};
struct TextDocumentContentChangeEvent {
  // The range of the document that changed.
  std::optional<Range> range;

  // The length of the range that got replaced.
  std::optional<uinteger> rangeLength;
  // The new text of the range/document.
  std::string text;
};
struct DidChangeTextDocumentParams {
  TextDocumentIdentifier textDocument;
  std::vector<TextDocumentContentChangeEvent> contentChanges;
  std::optional<bool> wantDiagnostics;
};
struct FileEvent {
  URIForFile uri;
  FileChangeType type = FileChangeType::Created;
};
struct DidChangeWatchedFilesParams {
  std::vector<FileEvent> changes;
};
struct DidChangeConfigurationParams {
  ConfigurationSettings settings;
};
struct DocumentRangeFormattingParams {
  TextDocumentIdentifier textDocument;
  Range range;
};
struct DocumentOnTypeFormattingParams {
  TextDocumentIdentifier textDocument;
  Position position;
  TextType ch;
};
struct FoldingRangeParams {
  TextDocumentIdentifier textDocument;
};
struct FoldingRange {
  // zero-based
  uinteger startLine;
  uinteger startCharacter;
  uinteger endLine;
  uinteger endCharacter;

  FoldingRangeKind kind;
};
struct SelectionRangeParams {
  TextDocumentIdentifier textDocument;
  std::vector<Position> positions;
};
struct SelectionRange {
  Range range;
  std::unique_ptr<SelectionRange> parent;
};
struct DocumentFormattingParams {
  TextDocumentIdentifier textDocument;
};
struct DocumentSymbolParams {
  TextDocumentIdentifier textDocument;
};
struct DiagnosticRelatedInformation {
  Location location;
  std::string message;
};
struct CodeAction;
struct Diagnostic {
  Range range;
  uinteger severity = 0;

  std::string code;

  std::string source;
  std::string message;
  std::optional<std::vector<DiagnosticRelatedInformation>> relatedInformation;

  std::optional<std::string> category;

  // clagd extension
  std::optional<std::vector<CodeAction>> codeActions;
};
struct PublishDiagnosticsParams {
  std::string uri;
  std::vector<Diagnostic> diagnostics;
};
struct CodeActionContext {
  std::vector<Diagnostic> diagnostics;
};
struct CodeActionParams {
  TextDocumentIdentifier textDocument;

  Range range;

  CodeActionContext context;
};
struct WorkspaceEdit {
  std::optional<std::map<std::string, std::vector<TextEdit>>> changes;
};
struct TweakArgs {
  std::string file;
  Range selection;
  std::string tweakID;
};
struct ExecuteCommandParams {
  std::string command;
  std::optional<WorkspaceEdit> workspaceEdit;
  std::optional<TweakArgs> tweakArgs;
};
struct LspCommand  // Command according to protocol
    : public ExecuteCommandParams {
  std::string title;
};
struct CodeAction {
  // A short, human-readable, title for this code action.
  std::string title;

  // The kind of the code action.
  // Used to filter code actions.
  std::optional<std::string> kind;
  // The diagnostics that this code action resolves.
  std::optional<std::vector<Diagnostic>> diagnostics;

  // The workspace edit this code action performs.
  std::optional<WorkspaceEdit> edit;

  // A command this code action executes. If a code action provides an edit
  // and a command, first the edit is executed and then the command.
  std::optional<LspCommand> command;
};
struct SymbolInformation {
  std::string name;
  SymbolKind kind = SymbolKind::Class;
  Location location;
  std::string containerName;
};
struct SymbolDetails {
  // clang tools feature
  TextType name;
  TextType containerName;
  TextType USR;
  std::optional<TextType> ID;
};
struct WorkspaceSymbolParams {
  TextType query;
};
struct ApplyWorkspaceEditParams {
  WorkspaceEdit edit;
};
struct TextDocumentPositionParams {
  TextDocumentIdentifier textDocument;
  Position position;
};
struct CompletionContext {
  CompletionTriggerKind triggerKind = CompletionTriggerKind::Invoked;
  std::optional<TextType> triggerCharacter;
};
struct CompletionParams : TextDocumentPositionParams {
  std::optional<CompletionContext> context;
};
struct MarkupContent {
  MarkupKind kind = MarkupKind::PlainText;
  std::string value;
};
struct Hover {
  MarkupContent contents;
  std::optional<Range> range;
};
struct CompletionItem {
  std::string label;

  CompletionItemKind kind = CompletionItemKind::Missing;

  std::string detail;

  std::string documentation;

  std::string sortText;

  std::string filterText;

  std::string insertText;

  InsertTextFormat insertTextFormat = InsertTextFormat::Missing;
  TextEdit textEdit;

  std::vector<TextEdit> additionalTextEdits;

  bool deprecated = false;
};
struct CompletionList {
  bool isIncomplete = false;
  std::vector<CompletionItem> items;
};
struct ParameterInformation {
  std::string labelString;
  std::optional<std::pair<unsigned, unsigned>> labelOffsets;  // clang specific
  std::string documentation;
};
struct SignatureInformation {
  std::string label;
  std::string documentation;
  std::vector<ParameterInformation> parameters;
};
struct SignatureHelp {
  std::vector<SignatureInformation> signatures;
  uinteger activeSignature = 0;
  uinteger activeParameter = 0;
  Position argListStart;  // clangd specific
};
struct RenameParams {
  TextDocumentIdentifier textDocument;
  Position position;
  std::string newName;
};
struct TypeHierarchyParams : public TextDocumentPositionParams {
  uinteger resolve = 0;
  TypeHierarchyDirection direction = TypeHierarchyDirection::Parents;
};
struct TypeHierarchyItem {
  std::string name;

  std::optional<std::string> detail;
  SymbolKind kind;
  bool deprecated;
  DocumentUri uri;
  Range range;
  Range selectionRange;
  std::optional<std::vector<TypeHierarchyItem>> parents;

  std::optional<std::vector<TypeHierarchyItem>> children;
};
struct FileStatus {
  DocumentUri uri;
  TextType state;
};
}  // namespace lsp

#endif  // BATON_LSP_BASIC_H
