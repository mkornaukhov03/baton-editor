#include <memory>
#include <string>

#include "enums.h"
#include "lsp_basic.h"

namespace nlohmann {
using namespace lsp;
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

// Use common approach to serialize user-defined structures:
// specialization of nlohmann::adl_serizalizer

template <typename T>
struct adl_serializer<std::optional<T>> {
  static void to_json(json &j, const std::optional<T> &opt) {
    if (opt) {
      j = opt.value();
    } else {
      j = nullptr;
    }
  }
  static void from_json(const json &j, std::optional<T> &opt) {
    if (j.is_null()) {
      opt = std::optional<T>();
    } else {
      opt = std::optional<T>(j.get<T>());
    }
  }
};
template <>
struct adl_serializer<lsp::URIForFile> {
  static void to_json(json &j, const lsp::URIForFile &uri) { j = uri.str(); }
  static void from_json(const json &j, lsp::URIForFile &uri) {
    uri.set_from_encoded(j.get<std::string>());
  }
};

template <>
struct adl_serializer<TextDocumentIdentifier> {
  static void to_json(json &j, const TextDocumentIdentifier &value) {
    j = {{"uri", value.uri}};
  }
  static void from_json(const json &, TextDocumentIdentifier &) {}
};

template <>
struct adl_serializer<VersionedTextDocumentIdentifier> {
  static void to_json(json &j, const VersionedTextDocumentIdentifier &value) {
    j = {{"uri", value.uri}, {"version", value.version}};
  }
  static void from_json(const json &, VersionedTextDocumentIdentifier &) {}
};

template <>
struct adl_serializer<Position> {
  static void to_json(json &j, const Position &value) {
    j = {{"line", value.line}, {"character", value.character}};
  }
  static void from_json(const json &j, Position &value) {
    if (j.contains("line")) j.at("line").get_to(value.line);
    if (j.contains("character")) j.at("character").get_to(value.character);
  }
};

template <>
struct adl_serializer<Range> {
  static void to_json(json &j, const Range &value) {
    j = {{"start", value.start}, {"end", value.end}};
  }
  static void from_json(const json &j, Range &value) {
    if (j.contains("start")) j.at("start").get_to(value.start);

    if (j.contains("end")) j.at("end").get_to(value.end);
  }
};

template <>
struct adl_serializer<Location> {
  static void to_json(json &j, const Location &value) {
    j = {{"uri", value.uri}, {"range", value.range}};
  }

  static void from_json(const json &j, Location &value) {
    if (j.contains("uri")) j.at("uri").get_to(value.uri);
    if (j.contains("range")) j.at("range").get_to(value.range);
  }
};

template <>
struct adl_serializer<TextEdit> {
  static void to_json(json &j, const TextEdit &value) {
    j = {{"range", value.range}, {"newText", value.newText}};
  }
  static void from_json(const json &j, TextEdit &value) {
    if (j.contains("range")) j.at("range").get_to(value.range);
    if (j.contains("newText")) j.at("newText").get_to(value.newText);
  }
};

template <>
struct adl_serializer<TextDocumentItem> {
  static void to_json(json &j, const TextDocumentItem &value) {
    j = {{"uri", value.uri},
         {"languageId", value.languageId},
         {"version", value.version},
         {"text", value.text}};
  }
  static void from_json(const json &, TextDocumentItem &) {}
};

template <>
struct adl_serializer<ClientCapabilities> {
  static void to_json(json &j, const ClientCapabilities &value) {
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
  static void from_json(const json &, ClientCapabilities &) {}
};

template <>
struct adl_serializer<ClangdCompileCommand> {
  static void to_json(json &j, const ClangdCompileCommand &value) {
    j = {{"workingDirectory", value.workingDirectory},
         {"compilationCommand", value.compilationCommand}};
  }
  static void from_json(const json &, ClangdCompileCommand &) {}
};

template <>
struct adl_serializer<ConfigurationSettings> {
  static void to_json(json &j, const ConfigurationSettings &value) {
    j = {{"compilationDatabaseChanges", value.compilationDatabaseChanges}};
  }
  static void from_json(const json &, ConfigurationSettings &) {}
};

template <>
struct adl_serializer<InitializationOptions> {
  static void to_json(json &j, const InitializationOptions &value) {
    j = {{"configSettings", value.configSettings},
         {"compilationDatabasePath", value.compilationDatabasePath},
         {"fallbackFlags", value.fallbackFlags},
         {"clangdFileStatus", value.clangdFileStatus}};
  }
  static void from_json(const json &, InitializationOptions &) {}
};

template <>
struct adl_serializer<InitializeParams> {
  static void to_json(json &j, const InitializeParams &value) {
    j = {{"processId", value.processId},
         {"capabilities", value.capabilities},
         {"rootUri", value.rootUri.value()},
         {"initializationOptions", value.initializationOptions},
         {"rootPath", value.rootPath.value()}};
  }
  static void from_json(const json &, InitializeParams &) {}
};
template <>
struct adl_serializer<ShowMessageParams> {
  static void to_json(json &, const ShowMessageParams &) {}
  static void from_json(const json &j, ShowMessageParams &value) {
    if (j.contains("type")) j.at("type").get_to(value.type);
    if (j.contains("message")) j.at("message").get_to(value.message);
  }
};

template <>
struct adl_serializer<Registration> {
  static void to_json(json &j, const Registration &value) {
    j = {{"id", value.id}, {"method", value.method}};
  }
  static void from_json(const json &, Registration &) {}
};

template <>
struct adl_serializer<RegistrationParams> {
  static void to_json(json &j, const RegistrationParams &value) {
    j = {{"registrations", value.registrations}};
  }
  static void from_json(const json &, RegistrationParams &) {}
};

template <>
struct adl_serializer<UnregistrationParams> {
  static void to_json(json &j, const UnregistrationParams &value) {
    j = {{"unregisterations", value.unregisterations}};
  }
  static void from_json(const json &, UnregistrationParams &) {}
};

template <>
struct adl_serializer<DidOpenTextDocumentParams> {
  static void to_json(json &j, const DidOpenTextDocumentParams &value) {
    j = {{"textDocument", value.textDocument}};
  }
  static void from_json(const json &, DidOpenTextDocumentParams &) {}
};

template <>
struct adl_serializer<DidCloseTextDocumentParams> {
  static void to_json(json &j, const DidCloseTextDocumentParams &value) {
    j = {{"textDocument", value.textDocument}};
  }
  static void from_json(const json &, DidCloseTextDocumentParams &) {}
};

template <>
struct adl_serializer<TextDocumentContentChangeEvent> {
  static void to_json(json &j, const TextDocumentContentChangeEvent &value) {
    j = {{"range", value.range},
         {"rangeLength", value.rangeLength},
         {"text", value.text}};
  }
  static void from_json(const json &, TextDocumentContentChangeEvent &) {}
};

template <>
struct adl_serializer<DidChangeTextDocumentParams> {
  static void to_json(json &j, const DidChangeTextDocumentParams &value) {
    j = {{"textDocument", value.textDocument},
         {"contentChanges", value.contentChanges},
         {"wantDiagnostics", value.wantDiagnostics}};
  }
  static void from_json(const json &, DidChangeTextDocumentParams &) {}
};

template <>
struct adl_serializer<FileEvent> {
  static void to_json(json &j, const FileEvent &value) {
    j = {{"uri", value.uri}, {"type", value.type}};
  }
  static void from_json(const json &, FileEvent &) {}
};

template <>
struct adl_serializer<DidChangeWatchedFilesParams> {
  static void to_json(json &j, const DidChangeWatchedFilesParams &value) {
    j = {{"changes", value.changes}};
  }
  static void from_json(const json &, DidChangeWatchedFilesParams &) {}
};

template <>
struct adl_serializer<DidChangeConfigurationParams> {
  static void to_json(json &j, const DidChangeConfigurationParams &value) {
    j = {{"settings", value.settings}};
  }
  static void from_json(const json &, DidChangeConfigurationParams &) {}
};

template <>
struct adl_serializer<DocumentRangeFormattingParams> {
  static void to_json(json &j, const DocumentRangeFormattingParams &value) {
    j = {{"textDocument", value.textDocument}, {"range", value.range}};
  }
  static void from_json(const json &, DocumentRangeFormattingParams &) {}
};

template <>
struct adl_serializer<DocumentOnTypeFormattingParams> {
  static void to_json(json &j, const DocumentOnTypeFormattingParams &value) {
    j = {{"textDocument", value.textDocument},
         {"position", value.position},
         {"ch", value.ch}};
  }
  static void from_json(const json &, DocumentOnTypeFormattingParams &) {}
};

template <>
struct adl_serializer<FoldingRangeParams> {
  static void to_json(json &j, const FoldingRangeParams &value) {
    j = {{"textDocument", value.textDocument}};
  }
  static void from_json(const json &, FoldingRangeParams &) {}
};

template <>
struct adl_serializer<SelectionRangeParams> {
  static void to_json(json &j, const SelectionRangeParams &value) {
    j = {{"textDocument", value.textDocument}, {"positions", value.positions}};
  }
  static void from_json(const json &, SelectionRangeParams &) {}
};

template <>
struct adl_serializer<SelectionRange> {
  static void to_json(json &, const SelectionRange &) {}
  static void from_json(const json &j, SelectionRange &value) {
    if (j.contains("range")) j.at("range").get_to(value.range);

    if (j.contains("parent")) {
      value.parent = std::make_unique<SelectionRange>();
      j.at("parent").get_to(*value.parent);
    }
  }
};

template <>
struct adl_serializer<DocumentFormattingParams> {
  static void to_json(json &j, const DocumentFormattingParams &value) {
    j = {{"textDocument", value.textDocument}};
  }
  static void from_json(const json &, DocumentFormattingParams &) {}
};

template <>
struct adl_serializer<DocumentSymbolParams> {
  static void to_json(json &j, const DocumentSymbolParams &value) {
    j = {{"textDocument", value.textDocument}};
  }
  static void from_json(const json &, DocumentSymbolParams &) {}
};

template <>
struct adl_serializer<DiagnosticRelatedInformation> {
  static void to_json(json &j, const DiagnosticRelatedInformation &value) {
    j = {{"location", value.location}, {"message", value.message}};
  }
  static void from_json(const json &j, DiagnosticRelatedInformation &value) {
    if (j.contains("location")) {
      j.at("location").get_to(value.location);
    }
    if (j.contains("message")) j.at("message").get_to(value.message);
  }
};

template <>
struct adl_serializer<Diagnostic> {
  static void to_json(json &j, const Diagnostic &value) {
    j = {{"range", value.range},
         {"code", value.code},
         {"source", value.source},
         {"message", value.message},
         {"relatedInformation", value.relatedInformation},
         {"category", value.category},
         {"codeActions", value.codeActions}};
  }
  static void from_json(const json &j, Diagnostic &value) {
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

    if (j.contains("codeActions"))
      j.at("codeActions").get_to(value.codeActions);
  }
};

template <>
struct adl_serializer<PublishDiagnosticsParams> {
  static void to_json(json &, const PublishDiagnosticsParams &) {}
  static void from_json(const json &j, PublishDiagnosticsParams &value) {
    if (j.contains("uri")) {
      j.at("uri").get_to(value.uri);
    }
    if (j.contains("diagnostics")) {
      j.at("diagnostics").get_to(value.diagnostics);
    }
  }
};

template <>
struct adl_serializer<CodeActionContext> {
  static void to_json(json &j, const CodeActionContext &value) {
    j = {{"diagnostics", value.diagnostics}};
  }
  static void from_json(const json &, CodeActionContext &) {}
};

template <>
struct adl_serializer<CodeActionParams> {
  static void to_json(json &j, const CodeActionParams &value) {
    j = {{"textDocument", value.textDocument},
         {"range", value.range},
         {"context", value.context}};
  }
  static void from_json(const json &, CodeActionParams &) {}
};

template <>
struct adl_serializer<WorkspaceEdit> {
  static void to_json(json &j, const WorkspaceEdit &value) {
    j = {{"changes", value.changes}};
  }
  static void from_json(const json &j, WorkspaceEdit &value) {
    if (j.contains("changes")) j.at("changes").get_to(value.changes);
  }
};

template <>
struct adl_serializer<TweakArgs> {
  static void to_json(json &j, const TweakArgs &value) {
    j = {{"file", value.file},
         {"selection", value.selection},
         {"tweakID", value.tweakID}};
  }
  static void from_json(const json &j, TweakArgs &value) {
    if (j.contains("file")) j.at("file").get_to(value.file);

    if (j.contains("selection")) j.at("selection").get_to(value.selection);

    if (j.contains("tweakID")) j.at("tweakID").get_to(value.tweakID);
  }
};

template <>
struct adl_serializer<ExecuteCommandParams> {
  static void to_json(json &j, const ExecuteCommandParams &value) {
    j = {{"command", value.command},
         {"workspaceEdit", value.workspaceEdit},
         {"tweakArgs", value.tweakArgs}};
  }
  static void from_json(const json &, ExecuteCommandParams &) {}
};

template <>
struct adl_serializer<LspCommand> {
  static void to_json(json &j, const LspCommand &value) {
    j = {{"command", value.command},
         {"workspaceEdit", value.workspaceEdit},
         {"tweakArgs", value.tweakArgs},
         {"title", value.title}};
  }
  static void from_json(const json &j, LspCommand &value) {
    if (j.contains("command")) j.at("command").get_to(value.command);

    if (j.contains("workspaceEdit"))
      j.at("workspaceEdit").get_to(value.workspaceEdit);

    if (j.contains("tweakArgs")) j.at("tweakArgs").get_to(value.tweakArgs);

    if (j.contains("title")) j.at("title").get_to(value.title);
  }
};

template <>
struct adl_serializer<CodeAction> {
  static void to_json(json &j, const CodeAction &value) {
    j = {{"title", value.title},
         {"kind", value.kind},
         {"diagnostics", value.diagnostics},
         {"edit", value.edit},
         {"command", value.command}};
  }
  static void from_json(const json &j, CodeAction &value) {
    if (j.contains("title")) j.at("title").get_to(value.title);

    if (j.contains("kind")) j.at("kind").get_to(value.kind);

    if (j.contains("diagnostics"))
      j.at("diagnostics").get_to(value.diagnostics);

    if (j.contains("edit")) j.at("edit").get_to(value.edit);

    if (j.contains("command")) j.at("command").get_to(value.command);
  }
};

template <>
struct adl_serializer<SymbolInformation> {
  static void to_json(json &j, const SymbolInformation &value) {
    j = {{"name", value.name},
         {"kind", value.kind},
         {"location", value.location},
         {"containerName", value.containerName}};
  }
  static void from_json(const json &j, SymbolInformation &value) {
    if (j.contains("name")) j.at("name").get_to(value.name);

    if (j.contains("kind")) j.at("kind").get_to(value.kind);

    if (j.contains("location")) j.at("location").get_to(value.location);

    if (j.contains("containerName"))
      j.at("containerName").get_to(value.containerName);
  }
};

template <>
struct adl_serializer<WorkspaceSymbolParams> {
  static void to_json(json &j, const WorkspaceSymbolParams &value) {
    j = {{"query", value.query}};
  }
  static void from_json(const json &, WorkspaceSymbolParams &) {}
};

template <>
struct adl_serializer<ApplyWorkspaceEditParams> {
  static void to_json(json &j, const ApplyWorkspaceEditParams &value) {
    j = {{"edit", value.edit}};
  }
  static void from_json(const json &, ApplyWorkspaceEditParams &) {}
};

template <>
struct adl_serializer<TextDocumentPositionParams> {
  static void to_json(json &j, const TextDocumentPositionParams &value) {
    j = {{"textDocument", value.textDocument}, {"position", value.position}};
  }
  static void from_json(const json &, TextDocumentPositionParams &) {}
};

template <>
struct adl_serializer<CompletionContext> {
  static void to_json(json &j, const CompletionContext &value) {
    j = {{"triggerKind", value.triggerKind},
         {"triggerCharacter", value.triggerCharacter}};
  }
  static void from_json(const json &, CompletionContext &) {}
};

template <>
struct adl_serializer<CompletionParams> {
  static void to_json(json &j, const CompletionParams &value) {
    j = {{"context", value.context},
         {"textDocument", value.textDocument},
         {"position", value.position}};
  }
  static void from_json(const json &, CompletionParams &) {}
};

template <>
struct adl_serializer<MarkupContent> {
  static void to_json(json &, const MarkupContent &) {}
  static void from_json(const json &j, MarkupContent &value) {
    if (j.contains("kind")) j.at("kind").get_to(value.kind);

    if (j.contains("value")) j.at("value").get_to(value.value);
  }
};

template <>
struct adl_serializer<Hover> {
  static void to_json(json &, const Hover &) {}
  static void from_json(const json &j, Hover &value) {
    if (j.contains("contents")) j.at("contents").get_to(value.contents);

    if (j.contains("range")) j.at("range").get_to(value.range);
  }
};

template <>
struct adl_serializer<CompletionItem> {
  static void to_json(json &, const CompletionItem &) {}
  static void from_json(const json &j, CompletionItem &value) {
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
};

template <>
struct adl_serializer<CompletionList> {
  static void to_json(json &, const CompletionList &) {}
  static void from_json(const json &j, CompletionList &value) {
    if (j.contains("isIncomplete"))
      j.at("isIncomplete").get_to(value.isIncomplete);

    if (j.contains("items")) j.at("items").get_to(value.items);
  }
};

template <>
struct adl_serializer<ParameterInformation> {
  static void to_json(json &, const ParameterInformation &) {}
  static void from_json(const json &j, ParameterInformation &value) {
    if (j.contains("labelString"))
      j.at("labelString").get_to(value.labelString);

    if (j.contains("labelOffsets"))
      j.at("labelOffsets").get_to(value.labelOffsets);

    if (j.contains("documentation"))
      j.at("documentation").get_to(value.documentation);
  }
};

template <>
struct adl_serializer<SignatureInformation> {
  static void to_json(json &, const SignatureInformation &) {}
  static void from_json(const json &j, SignatureInformation &value) {
    if (j.contains("label")) j.at("label").get_to(value.label);

    if (j.contains("documentation"))
      j.at("documentation").get_to(value.documentation);

    if (j.contains("parameters")) j.at("parameters").get_to(value.parameters);
  }
};

template <>
struct adl_serializer<SignatureHelp> {
  static void to_json(json &, const SignatureHelp &) {}
  static void from_json(const json &j, SignatureHelp &value) {
    if (j.contains("signatures")) j.at("signatures").get_to(value.signatures);

    if (j.contains("activeParameter"))
      j.at("activeParameter").get_to(value.activeParameter);

    if (j.contains("argListStart"))
      j.at("argListStart").get_to(value.argListStart);
  }
};

template <>
struct adl_serializer<RenameParams> {
  static void to_json(json &j, const RenameParams &value) {
    j = {{"textDocument", value.textDocument},
         {"position", value.position},
         {"newName", value.newName}};
  }
  static void from_json(const json &, RenameParams &) {}
};

template <>
struct adl_serializer<TypeHierarchyParams> {
  static void to_json(json &j, const TypeHierarchyParams &value) {
    j = {{"resolve", value.resolve},
         {"direction", value.direction},
         {"textDocument", value.textDocument},
         {"position", value.position}};
  }
  static void from_json(const json &, TypeHierarchyParams &) {}
};

}  // namespace nlohmann
