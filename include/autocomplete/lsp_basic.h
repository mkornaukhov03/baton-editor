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

using uinteger = uint64_t;
using DocumentUri = std::string_view;
using TextType = std::string_view;

// Implementation of types, specified by LSP and LLVM to connect and communicate
// with clangd c++ language server

class URIForFile {
 public:
  explicit URIForFile(const std::string &filename);
  [[nodiscard]] std::string_view str() const;

  bool operator==(const URIForFile &oth) const;
  bool operator!=(const URIForFile &oth) const;

  void set_filename(const std::string &filename);

  void set_from_encoded(const std::string &filename);

 private:
  // must not be invoked at same filename twice or more times
  // because % symbol will encoded as %25 etc.
  static std::string Encode(std::string_view str);
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
  ClientCapabilities();
};
struct ClangdCompileCommand {
  TextType workingDirectory;
  std::vector<TextType> compilationCommand;
};
struct ConfigurationSettings {
  std::map<std::string, ClangdCompileCommand> compilationDatabaseChanges;
};
struct InitializationOptions {
  ConfigurationSettings configSettings;

  std::optional<TextType> compilationDatabasePath;
  std::vector<TextType> fallbackFlags;

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
  MessageType type = MessageType::Info;
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
  TextDocumentItem textDocument;
};
struct DidCloseTextDocumentParams {
  TextDocumentIdentifier textDocument;
};
struct TextDocumentContentChangeEvent {
  std::optional<Range> range;

  std::optional<uinteger> rangeLength;
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
struct LspCommand : public ExecuteCommandParams {
  std::string title;
};
struct CodeAction {
  std::string title;

  std::optional<std::string> kind;
  std::optional<std::vector<Diagnostic>> diagnostics;

  std::optional<WorkspaceEdit> edit;
  std::optional<LspCommand> command;
};
struct SymbolInformation {
  std::string name;
  SymbolKind kind = SymbolKind::Class;
  Location location;
  std::string containerName;
};
struct SymbolDetails {
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
  std::optional<std::pair<unsigned, unsigned>> labelOffsets;  // clangd specific
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
