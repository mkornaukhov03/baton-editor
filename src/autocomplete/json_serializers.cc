#include <memory>
#include <string>

#include "enums.h"
#include "lsp_basic.h"
#include "json_serializers.h"

 // namespace nlohmann
namespace lsp {

// Serializing enums from enums.h to json using built-in nlohmann macro

NLOHMANN_JSON_SERIALIZE_ENUM(OffsetEncoding,
                             {
                                 {OffsetEncoding::UnsupportedEncoding,
                                  "unsupported"},
                                 {OffsetEncoding::UTF8, "utf-8"},
                                 {OffsetEncoding::UTF16, "utf-16"},
                                 {OffsetEncoding::UTF32, "utf-32"},
                             })
NLOHMANN_JSON_SERIALIZE_ENUM(MarkupKind,
                             {
                                 {MarkupKind::PlainText, "plaintext"},
                                 {MarkupKind::Markdown, "markdown"},
                             })
NLOHMANN_JSON_SERIALIZE_ENUM(ResourceOperationKind,
                             {{ResourceOperationKind::Create, "create"},
                              {ResourceOperationKind::Rename, "rename"},
                              {ResourceOperationKind::Delete, "dename"}})
NLOHMANN_JSON_SERIALIZE_ENUM(
    FailureHandlingKind,
    {{FailureHandlingKind::Abort, "abort"},
     {FailureHandlingKind::Transactional, "transactional"},
     {FailureHandlingKind::Undo, "undo"},
     {FailureHandlingKind::TextOnlyTransactional, "textOnlyTransactional"}})
NLOHMANN_JSON_SERIALIZE_ENUM(FoldingRangeKind,
                             {{FoldingRangeKind::Comment, "comment"},
                              {FoldingRangeKind::Imports, "imports"},
                              {FoldingRangeKind::Region, "region"}})

void to_json(json &j, const lsp::URIForFile &uri) { j = uri.str(); }
void from_json(const json &j, lsp::URIForFile &uri) {
  uri.set_from_encoded(j.get<std::string>());
}

void to_json(json &j, const TextDocumentIdentifier &value) {
  j = {{"uri", value.uri}};
}
void from_json(const json &, TextDocumentIdentifier &) {}

void to_json(json &j, const VersionedTextDocumentIdentifier &value) {
  j = {{"uri", value.uri}, {"version", value.version}};
}
void from_json(const json &, VersionedTextDocumentIdentifier &) {}

void to_json(json &j, const Position &value) {
  j = {{"line", value.line}, {"character", value.character}};
}
void from_json(const json &j, Position &value) {
  if (j.contains("line")) j.at("line").get_to(value.line);
  if (j.contains("character")) j.at("character").get_to(value.character);
}

void to_json(json &j, const Range &value) {
  j = {{"start", value.start}, {"end", value.end}};
}
void from_json(const json &j, Range &value) {
  if (j.contains("start")) j.at("start").get_to(value.start);

  if (j.contains("end")) j.at("end").get_to(value.end);
}

void to_json(json &j, const Location &value) {
  j = {{"uri", value.uri}, {"range", value.range}};
}

void from_json(const json &j, Location &value) {
  if (j.contains("uri")) j.at("uri").get_to(value.uri);
  if (j.contains("range")) j.at("range").get_to(value.range);
}

void to_json(json &j, const TextEdit &value) {
  j = {{"range", value.range}, {"newText", value.newText}};
}
void from_json(const json &j, TextEdit &value) {
  if (j.contains("range")) j.at("range").get_to(value.range);
  if (j.contains("newText")) j.at("newText").get_to(value.newText);
}

void to_json(json &j, const TextDocumentItem &value) {
  j = {{"uri", value.uri},
       {"languageId", value.languageId},
       {"version", value.version},
       {"text", value.text}};
}
void from_json(const json &, TextDocumentItem &) {}

void to_json(json &j, const ClientCapabilities &value) {
  j = {{"textDocument",
        {{"publishDiagnostics",
          {
              {"categorySupport", value.DiagnosticCategory},
              {"codeActionsInline", value.DiagnosticFixes},
              {"relatedInformation", value.DiagnosticRelatedInformation},
          }},
         {"completion",
          {{"completionItem",
            {{"snippetSupport", value.CompletionSnippets},
             {"deprecatedSupport", value.CompletionDeprecated}}},
           {"completionItemKind", {{"valueSet", value.CompletionItemKinds}}},
           {"editsNearCursor", value.CompletionFixes}}},
         {"codeAction",
          {{"codeActionLiteralSupport", value.CodeActionStructure}}},
         {"documentSymbol",
          {{"hierarchicalDocumentSymbolSupport",
            value.HierarchicalDocumentSymbol}}},
         {"hover", {{"contentFormat", value.HoverContentFormat}}},
         {"signatureHelp",
          {{"signatureInformation",
            {{"parameterInformation",
              {{"labelOffsetSupport", value.OffsetsInSignatureHelp}}}}}}}}},
       {"workspace",
        {{"symbol",
          {{"symbolKind", {{"valueSet", value.WorkspaceSymbolKinds}}}}},
         {"applyEdit", value.ApplyEdit},
         {"workspaceEdit", {{"documentChanges", value.DocumentChanges}}}}},
       {"offsetEncoding", value.offsetEncoding}};
}
void from_json(const json &, ClientCapabilities &) {}

void to_json(json &j, const ClangdCompileCommand &value) {
  j = {{"workingDirectory", value.workingDirectory},
       {"compilationCommand", value.compilationCommand}};
}
void from_json(const json &, ClangdCompileCommand &) {}

void to_json(json &j, const ConfigurationSettings &value) {
  j = {{"compilationDatabaseChanges", value.compilationDatabaseChanges}};
}
void from_json(const json &, ConfigurationSettings &) {}

void to_json(json &j, const InitializationOptions &value) {
  j = {{"configSettings", value.configSettings},
       {"compilationDatabasePath", value.compilationDatabasePath},
       {"fallbackFlags", value.fallbackFlags},
       {"clangdFileStatus", value.clangdFileStatus}};
}
void from_json(const json &, InitializationOptions &) {}

void to_json(json &j, const InitializeParams &value) {
  j = {{"processId", value.processId},
       {"capabilities", value.capabilities},
       {"rootUri", value.rootUri.value()},
       {"initializationOptions", value.initializationOptions},
       {"rootPath", value.rootPath.value()}};
}
void from_json(const json &, InitializeParams &) {}
void to_json(json &, const ShowMessageParams &) {}
void from_json(const json &j, ShowMessageParams &value) {
  if (j.contains("type")) j.at("type").get_to(value.type);
  if (j.contains("message")) j.at("message").get_to(value.message);
}

void to_json(json &j, const Registration &value) {
  j = {{"id", value.id}, {"method", value.method}};
}
void from_json(const json &, Registration &) {}

void to_json(json &j, const RegistrationParams &value) {
  j = {{"registrations", value.registrations}};
}
void from_json(const json &, RegistrationParams &) {}

void to_json(json &j, const UnregistrationParams &value) {
  j = {{"unregisterations", value.unregisterations}};
}
void from_json(const json &, UnregistrationParams &) {}

void to_json(json &j, const DidOpenTextDocumentParams &value) {
  j = {{"textDocument", value.textDocument}};
}
void from_json(const json &, DidOpenTextDocumentParams &) {}

void to_json(json &j, const DidCloseTextDocumentParams &value) {
  j = {{"textDocument", value.textDocument}};
}
void from_json(const json &, DidCloseTextDocumentParams &) {}

void to_json(json &j, const TextDocumentContentChangeEvent &value) {
  j = {{"range", value.range},
       {"rangeLength", value.rangeLength},
       {"text", value.text}};
}
void from_json(const json &, TextDocumentContentChangeEvent &) {}

void to_json(json &j, const DidChangeTextDocumentParams &value) {
  j = {{"textDocument", value.textDocument},
       {"contentChanges", value.contentChanges},
       {"wantDiagnostics", value.wantDiagnostics}};
}
void from_json(const json &, DidChangeTextDocumentParams &) {}

void to_json(json &j, const FileEvent &value) {
  j = {{"uri", value.uri}, {"type", value.type}};
}
void from_json(const json &, FileEvent &) {}

void to_json(json &j, const DidChangeWatchedFilesParams &value) {
  j = {{"changes", value.changes}};
}
void from_json(const json &, DidChangeWatchedFilesParams &) {}

void to_json(json &j, const DidChangeConfigurationParams &value) {
  j = {{"settings", value.settings}};
}
void from_json(const json &, DidChangeConfigurationParams &) {}

void to_json(json &j, const DocumentRangeFormattingParams &value) {
  j = {{"textDocument", value.textDocument}, {"range", value.range}};
}
void from_json(const json &, DocumentRangeFormattingParams &) {}

void to_json(json &j, const DocumentOnTypeFormattingParams &value) {
  j = {{"textDocument", value.textDocument},
       {"position", value.position},
       {"ch", value.ch}};
}
void from_json(const json &, DocumentOnTypeFormattingParams &) {}

void to_json(json &j, const FoldingRangeParams &value) {
  j = {{"textDocument", value.textDocument}};
}
void from_json(const json &, FoldingRangeParams &) {}

void to_json(json &j, const SelectionRangeParams &value) {
  j = {{"textDocument", value.textDocument}, {"positions", value.positions}};
}
void from_json(const json &, SelectionRangeParams &) {}

void to_json(json &, const SelectionRange &) {}
void from_json(const json &j, SelectionRange &value) {
  if (j.contains("range")) j.at("range").get_to(value.range);

  if (j.contains("parent")) {
    value.parent = std::make_unique<SelectionRange>();
    j.at("parent").get_to(*value.parent);
  }
}

void to_json(json &j, const DocumentFormattingParams &value) {
  j = {{"textDocument", value.textDocument}};
}
void from_json(const json &, DocumentFormattingParams &) {}

void to_json(json &j, const DocumentSymbolParams &value) {
  j = {{"textDocument", value.textDocument}};
}
void from_json(const json &, DocumentSymbolParams &) {}

void to_json(json &j, const DiagnosticRelatedInformation &value) {
  j = {{"location", value.location}, {"message", value.message}};
}
void from_json(const json &j, DiagnosticRelatedInformation &value) {
  if (j.contains("location")) {
    j.at("location").get_to(value.location);
  }
  if (j.contains("message")) j.at("message").get_to(value.message);
}

void to_json(json &j, const Diagnostic &value) {
  j = {{"range", value.range},
       {"code", value.code},
       {"source", value.source},
       {"message", value.message},
       {"relatedInformation", value.relatedInformation},
       {"category", value.category},
       {"codeActions", value.codeActions}};
}
void from_json(const json &j, Diagnostic &value) {
  if (j.contains("range")) {
    j.at("range").get_to(value.range);
  }
  if (j.contains("code")) {
    j.at("code").get_to(value.code);
  }
  if (j.contains("source")) j.at("source").get_to(value.source);

  if (j.contains("message")) j.at("message").get_to(value.message);

  if (j.contains("relatedInformation"))
    j.at("relatedInformation").get_to(value.relatedInformation);

  if (j.contains("category")) j.at("category").get_to(value.category);

  if (j.contains("codeActions")) j.at("codeActions").get_to(value.codeActions);
}

void to_json(json &, const PublishDiagnosticsParams &) {}
void from_json(const json &j, PublishDiagnosticsParams &value) {
  if (j.contains("uri")) {
    j.at("uri").get_to(value.uri);
  }
  if (j.contains("diagnostics")) {
    j.at("diagnostics").get_to(value.diagnostics);
  }
}

void to_json(json &j, const CodeActionContext &value) {
  j = {{"diagnostics", value.diagnostics}};
}
void from_json(const json &, CodeActionContext &) {}

void to_json(json &j, const CodeActionParams &value) {
  j = {{"textDocument", value.textDocument},
       {"range", value.range},
       {"context", value.context}};
}
void from_json(const json &, CodeActionParams &) {}
void to_json(json &j, const WorkspaceEdit &value) {
  j = {{"changes", value.changes}};
}
void from_json(const json &j, WorkspaceEdit &value) {
  if (j.contains("changes")) j.at("changes").get_to(value.changes);
}

void to_json(json &j, const TweakArgs &value) {
  j = {{"file", value.file},
       {"selection", value.selection},
       {"tweakID", value.tweakID}};
}
void from_json(const json &j, TweakArgs &value) {
  if (j.contains("file")) j.at("file").get_to(value.file);

  if (j.contains("selection")) j.at("selection").get_to(value.selection);

  if (j.contains("tweakID")) j.at("tweakID").get_to(value.tweakID);
}

void to_json(json &j, const ExecuteCommandParams &value) {
  j = {{"command", value.command},
       {"workspaceEdit", value.workspaceEdit},
       {"tweakArgs", value.tweakArgs}};
}
void from_json(const json &, ExecuteCommandParams &) {}

void to_json(json &j, const LspCommand &value) {
  j = {{"command", value.command},
       {"workspaceEdit", value.workspaceEdit},
       {"tweakArgs", value.tweakArgs},
       {"title", value.title}};
}
void from_json(const json &j, LspCommand &value) {
  if (j.contains("command")) j.at("command").get_to(value.command);

  if (j.contains("workspaceEdit"))
    j.at("workspaceEdit").get_to(value.workspaceEdit);

  if (j.contains("tweakArgs")) j.at("tweakArgs").get_to(value.tweakArgs);

  if (j.contains("title")) j.at("title").get_to(value.title);
}

void to_json(json &j, const CodeAction &value) {
  j = {{"title", value.title},
       {"kind", value.kind},
       {"diagnostics", value.diagnostics},
       {"edit", value.edit},
       {"command", value.command}};
}
void from_json(const json &j, CodeAction &value) {
  if (j.contains("title")) j.at("title").get_to(value.title);

  if (j.contains("kind")) j.at("kind").get_to(value.kind);

  if (j.contains("diagnostics")) j.at("diagnostics").get_to(value.diagnostics);

  if (j.contains("edit")) j.at("edit").get_to(value.edit);

  if (j.contains("command")) j.at("command").get_to(value.command);
}

void to_json(json &j, const SymbolInformation &value) {
  j = {{"name", value.name},
       {"kind", value.kind},
       {"location", value.location},
       {"containerName", value.containerName}};
}
void from_json(const json &j, SymbolInformation &value) {
  if (j.contains("name")) j.at("name").get_to(value.name);

  if (j.contains("kind")) j.at("kind").get_to(value.kind);

  if (j.contains("location")) j.at("location").get_to(value.location);

  if (j.contains("containerName"))
    j.at("containerName").get_to(value.containerName);
}

void to_json(json &j, const WorkspaceSymbolParams &value) {
  j = {{"query", value.query}};
}
void from_json(const json &, WorkspaceSymbolParams &) {}

void to_json(json &j, const ApplyWorkspaceEditParams &value) {
  j = {{"edit", value.edit}};
}
void from_json(const json &, ApplyWorkspaceEditParams &) {}

void to_json(json &j, const TextDocumentPositionParams &value) {
  j = {{"textDocument", value.textDocument}, {"position", value.position}};
}
void from_json(const json &, TextDocumentPositionParams &) {}

void to_json(json &j, const CompletionContext &value) {
  j = {{"triggerKind", value.triggerKind},
       {"triggerCharacter", value.triggerCharacter}};
}
void from_json(const json &, CompletionContext &) {}

void to_json(json &j, const CompletionParams &value) {
  j = {{"context", value.context},
       {"textDocument", value.textDocument},
       {"position", value.position}};
}
void from_json(const json &, CompletionParams &) {}

void to_json(json &, const MarkupContent &) {}
void from_json(const json &j, MarkupContent &value) {
  if (j.contains("kind")) j.at("kind").get_to(value.kind);

  if (j.contains("value")) j.at("value").get_to(value.value);
}

void to_json(json &, const Hover &) {}
void from_json(const json &j, Hover &value) {
  if (j.contains("contents")) j.at("contents").get_to(value.contents);

  if (j.contains("range")) j.at("range").get_to(value.range);
}

void to_json(json &, const CompletionItem &) {}
void from_json(const json &j, CompletionItem &value) {
  if (j.contains("label")) j.at("label").get_to(value.label);

  if (j.contains("kind")) j.at("kind").get_to(value.kind);

  if (j.contains("detail")) j.at("detail").get_to(value.detail);

  if (j.contains("documentation"))
    j.at("documentation").get_to(value.documentation);

  if (j.contains("sortText")) j.at("sortText").get_to(value.sortText);

  if (j.contains("filterText")) j.at("filterText").get_to(value.filterText);

  if (j.contains("insertText")) j.at("insertText").get_to(value.insertText);

  if (j.contains("insertTextFormat"))
    j.at("insertTextFormat").get_to(value.insertTextFormat);

  if (j.contains("textEdit")) j.at("textEdit").get_to(value.textEdit);

  if (j.contains("additionalTextEdits"))
    j.at("additionalTextEdits").get_to(value.additionalTextEdits);
}

void to_json(json &, const CompletionList &) {}
void from_json(const json &j, CompletionList &value) {
  if (j.contains("isIncomplete"))
    j.at("isIncomplete").get_to(value.isIncomplete);

  if (j.contains("items")) j.at("items").get_to(value.items);
}
void to_json(json &, const ParameterInformation &) {}
void from_json(const json &j, ParameterInformation &value) {
  if (j.contains("labelString")) j.at("labelString").get_to(value.labelString);

  if (j.contains("labelOffsets"))
    j.at("labelOffsets").get_to(value.labelOffsets);

  if (j.contains("documentation"))
    j.at("documentation").get_to(value.documentation);
}

void to_json(json &, const SignatureInformation &) {}
void from_json(const json &j, SignatureInformation &value) {
  if (j.contains("label")) j.at("label").get_to(value.label);

  if (j.contains("documentation"))
    j.at("documentation").get_to(value.documentation);

  if (j.contains("parameters")) j.at("parameters").get_to(value.parameters);
}

void to_json(json &, const SignatureHelp &) {}
void from_json(const json &j, SignatureHelp &value) {
  if (j.contains("signatures")) j.at("signatures").get_to(value.signatures);

  if (j.contains("activeParameter"))
    j.at("activeParameter").get_to(value.activeParameter);

  if (j.contains("argListStart"))
    j.at("argListStart").get_to(value.argListStart);
}

void to_json(json &j, const RenameParams &value) {
  j = {{"textDocument", value.textDocument},
       {"position", value.position},
       {"newName", value.newName}};
}
void from_json(const json &, RenameParams &) {}

void to_json(json &j, const TypeHierarchyParams &value) {
  j = {{"resolve", value.resolve},
       {"direction", value.direction},
       {"textDocument", value.textDocument},
       {"position", value.position}};
}
void from_json(const json &, TypeHierarchyParams &) {}

QT_BEGIN_NAMESPACE
void to_json(nlohmann::json &j, const QByteArray &qba) {
  j = nlohmann::json{qba.toStdString()};
}

void from_json(const nlohmann::json &j, QByteArray &qba) {
  qba = QByteArray::fromStdString(j.get<std::string>());
}
QT_END_NAMESPACE

}  // namespace lsp
