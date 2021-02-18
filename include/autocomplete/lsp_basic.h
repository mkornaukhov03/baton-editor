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
enum class TraceLevel {
  Off = 0,
  Messages = 1,
  Verbose = 2,
};
enum class TextDocumentSyncKind {
  // Documents should not be synced at all.
  None = 0,

  // Documents are synced by always sending the full content of the document.
  Full = 1,

  // Documents are synced by sending the full content on open.  After that
  // only incremental updates to the document are send.
  Incremental = 2,
};
enum class CompletionItemKind {
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
enum class SymbolKind {
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
enum class OffsetEncoding {
  // Any string is legal on the wire. Unrecognized encodings parse as this.
  UnsupportedEncoding,
  // Length counts code units of UTF-16 encoded text. (Standard LSP behavior).
  UTF16,
  // Length counts bytes of UTF-8 encoded text. (Clangd extension).
  UTF8,
  // Length counts codepoints in unicode text. (Clangd extension).
  UTF32,
};
enum class MarkupKind {
  PlainText,
  Markdown,
};
enum class ResourceOperationKind { Create, Rename, Delete };
enum class FailureHandlingKind {
  Abort,
  Transactional,
  Undo,
  TextOnlyTransactional
};
enum class MessageType {
  Error = 1,
  Warning = 2,
  Info = 3,
  Log = 4,
};
enum class FileChangeType { Created = 1, Changed = 2, Deleted = 3 };
enum class FoldingRangeKind {
  Comment,
  Imports,
  Region,
};
enum class CompletionTriggerKind {
  Invoked = 1,
  TriggerCharacter = 2,
  TriggerTriggerForIncompleteCompletions = 3
};
enum class InsertTextFormat {
  Missing = 0,
  PlainText = 1,
  Snippet = 2,
};
enum class DocumentHighlightKind { Text = 1, Read = 2, Write = 3 };
enum class TypeHierarchyDirection { Children = 0, Parents = 1, Both = 2 };

template <class T>
using optional = std::optional<T>;
using uinteger = uint32_t;
using DocumentUri = std::string_view;
using TextType = std::string_view;
class URIForFile {
  // Implementation according to https://tools.ietf.org/html/rfc3986#section-2
 public:
  URIForFile(const std::string &filename, bool is_absolute)
      : is_absolute_(is_absolute), file_name_(Encode(filename)) {
    if (is_absolute_) {
      file_name_ = "file://" + file_name_;
    } else {
      file_name_ = "file://" + file_name_;
    }
  }
  [[nodiscard]] std::string_view str() const { return file_name_; }

  [[nodiscard]] bool is_absolute() const { return is_absolute_; }

  friend bool operator==(const URIForFile &lhs, const URIForFile &rhs) {
    return lhs.file_name_ == rhs.file_name_;
  }
  friend bool operator!=(const URIForFile &lhs, const URIForFile &rhs) {
    return lhs.file_name_ != rhs.file_name_;
  }

  void set_filename(const std::string &filename, bool is_absolute) {
    is_absolute_ = is_absolute;
    file_name_ = Encode(filename);
    if (is_absolute_) {
      file_name_ = "file://" + file_name_;
    } else {
      file_name_ = "file://" + file_name_;
    }
  }

  void set_from_encoded(const std::string &filename) {
    is_absolute_ = filename.find("file://") != std::string::npos;
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
  uinteger version{};
};

struct Position {
  // zero-based
  uinteger line{};
  uinteger character{};

  friend bool operator==(const Position &lhs, const Position &rhs) {
    return std::tie(lhs.line, lhs.character) ==
           std::tie(rhs.line, rhs.character);
  }
  friend bool operator!=(const Position &lhs, const Position &rhs) {
    return !(lhs == rhs);
  }
  friend bool operator<(const Position &lhs, const Position &rhs) {
    return std::tie(lhs.line, lhs.character) <
           std::tie(rhs.line, rhs.character);
  }
  friend bool operator<=(const Position &lhs, const Position &rhs) {
    return std::tie(lhs.line, lhs.character) <=
           std::tie(rhs.line, rhs.character);
  }
};

struct Range {
  // [start; end)
  Position start;
  Position end;

  friend bool operator==(const Range &lhs, const Range &rhs) {
    return std::tie(lhs.start, lhs.end) == std::tie(rhs.start, rhs.end);
  }
  friend bool operator!=(const Range &lhs, const Range &rhs) {
    return !(lhs == rhs);
  }
  friend bool operator<(const Range &lhs, const Range &rhs) {
    return std::tie(lhs.start, lhs.end) < std::tie(rhs.start, rhs.end);
  }
  [[nodiscard]] bool contains(Position Pos) const {
    return start <= Pos && Pos < end;
  }
  [[nodiscard]] bool contains(Range Rng) const {
    return start <= Rng.start && Rng.end <= end;
  }
};

struct Location {
  DocumentUri uri;
  Range range;

  friend bool operator==(const Location &LHS, const Location &RHS) {
    return LHS.uri == RHS.uri && LHS.range == RHS.range;
  }
  friend bool operator!=(const Location &LHS, const Location &RHS) {
    return !(LHS == RHS);
  }
  friend bool operator<(const Location &LHS, const Location &RHS) {
    return std::tie(LHS.uri, LHS.range) < std::tie(RHS.uri, RHS.range);
  }
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
  // The supported set of SymbolKinds for workspace/symbol.
  // workspace.symbol.symbolKind.valueSet
  std::vector<SymbolKind> WorkspaceSymbolKinds;
  // Whether the client accepts diagnostics with codeActions attached inline.
  // textDocument.publishDiagnostics.codeActionsInline.
  bool DiagnosticFixes = true;

  // Whether the client accepts diagnostics with related locations.
  // textDocument.publishDiagnostics.relatedInformation.
  bool DiagnosticRelatedInformation = true;

  // Whether the client accepts diagnostics with category attached to it
  // using the "category" extension.
  // textDocument.publishDiagnostics.categorySupport
  bool DiagnosticCategory = true;

  // Client supports snippets as insert text.
  // textDocument.completion.completionItem.snippetSupport
  bool CompletionSnippets = true;

  bool CompletionDeprecated = true;

  // Client supports completions with additionalTextEdit near the cursor.
  // This is a clangd extension. (LSP says this is for unrelated text only).
  // textDocument.completion.editsNearCursor
  bool CompletionFixes = true;

  // Client supports hierarchical document symbols.
  bool HierarchicalDocumentSymbol = true;

  // Client supports processing label offsets instead of a simple label string.
  bool OffsetsInSignatureHelp = true;

  // The supported set of CompletionItemKinds for textDocument/completion.
  // textDocument.completion.completionItemKind.valueSet
  std::vector<CompletionItemKind> CompletionItemKinds;

  // Client supports CodeAction return value for textDocument/codeAction.
  // textDocument.codeAction.codeActionLiteralSupport.
  bool CodeActionStructure = true;
  // Supported encodings for LSP character offsets. (clangd extension).
  std::vector<OffsetEncoding> offsetEncoding = {OffsetEncoding::UTF8};
  // The content format that should be used for Hover requests.
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
  // What we can change throught the didChangeConfiguration request, we can
  // also set through the initialize request (initializationOptions field).
  ConfigurationSettings configSettings;

  optional<TextType> compilationDatabasePath;
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
  optional<DocumentUri> rootUri;
  optional<TextType> rootPath;
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
  optional<Range> range;

  // The length of the range that got replaced.
  optional<uinteger> rangeLength;
  // The new text of the range/document.
  std::string text;
};
struct DidChangeTextDocumentParams {
  TextDocumentIdentifier textDocument;
  std::vector<TextDocumentContentChangeEvent> contentChanges;
  optional<bool> wantDiagnostics;
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
  optional<std::vector<DiagnosticRelatedInformation>> relatedInformation;

  optional<std::string> category;

  // clagd extension
  optional<std::vector<CodeAction>> codeActions;
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
  optional<std::map<std::string, std::vector<TextEdit>>> changes;
};
struct TweakArgs {
  std::string file;
  Range selection;
  std::string tweakID;
};
struct ExecuteCommandParams {
  std::string command;
  optional<WorkspaceEdit> workspaceEdit;
  optional<TweakArgs> tweakArgs;
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
  optional<std::string> kind;
  // The diagnostics that this code action resolves.
  optional<std::vector<Diagnostic>> diagnostics;

  // The workspace edit this code action performs.
  optional<WorkspaceEdit> edit;

  // A command this code action executes. If a code action provides an edit
  // and a command, first the edit is executed and then the command.
  optional<LspCommand> command;
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
  optional<TextType> ID;
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
  optional<TextType> triggerCharacter;
};
struct CompletionParams : TextDocumentPositionParams {
  optional<CompletionContext> context;
};
struct MarkupContent {
  MarkupKind kind = MarkupKind::PlainText;
  std::string value;
};
struct Hover {
  MarkupContent contents;
  optional<Range> range;
};
struct CompletionItem {
  // what to paste
  std::string label;

  CompletionItemKind kind = CompletionItemKind::Missing;

  std::string detail;

  std::string documentation;

  // A string that should be used when comparing this item with other items.
  // When `falsy` the label is used.
  std::string sortText;

  // A string that should be used when filtering a set of completion items.
  std::string filterText;

  // A string that should be inserted to a document when selecting this
  std::string insertText;

  // The format of the insert text. The format applies to both the `insertText`
  // property and the `newText` property of a provided `textEdit`.
  InsertTextFormat insertTextFormat = InsertTextFormat::Missing;

  // An edit which is applied to a document when selecting this completion.
  // When an edit is provided `insertText` is ignored.
  TextEdit textEdit;

  std::vector<TextEdit> additionalTextEdits;

  // Indicates if this item is deprecated.
  bool deprecated = false;
};
struct CompletionList {
  bool isIncomplete = false;
  std::vector<CompletionItem> items;
};
struct ParameterInformation {
  std::string labelString;
  optional<std::pair<unsigned, unsigned>> labelOffsets;  // clang specific
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

  optional<std::string> detail;
  SymbolKind kind;
  bool deprecated;
  DocumentUri uri;
  Range range;
  Range selectionRange;
  optional<std::vector<TypeHierarchyItem>> parents;

  optional<std::vector<TypeHierarchyItem>> children;
};
struct FileStatus {
  DocumentUri uri;
  TextType state;
};
}  // namespace lsp

#endif  // BATON_LSP_BASIC_H
