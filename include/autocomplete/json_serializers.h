#ifndef JSON_SERIALIZERS_H_
#define JSON_SERIALIZERS_H_
namespace nlohmann {
template <typename T>
struct adl_serializer<std::optional<T>> {
  static void to_json(json &j, const std::optional<T> &opt) {
    if (opt == std::nullopt) {
      j = nullptr;
    } else {
      j = *opt;
    }
  }

  static void from_json(const json &j, std::optional<T> &opt) {
    if (j.is_null()) {
      opt = std::nullopt;
    } else {
      opt = j.get<T>();
    }
  }
};
}  // namespace nlohmann

namespace lsp {
void to_json(json &j, const lsp::URIForFile &uri);
void from_json(const json &j, lsp::URIForFile &uri);

void to_json(json &j, const TextDocumentIdentifier &value);
void from_json(const json &, TextDocumentIdentifier &);

void to_json(json &j, const VersionedTextDocumentIdentifier &value);
void from_json(const json &, VersionedTextDocumentIdentifier &);

void to_json(json &j, const Position &value);
void from_json(const json &j, Position &value);

void to_json(json &j, const Range &value);
void from_json(const json &j, Range &value);

void to_json(json &j, const Location &value);

void from_json(const json &j, Location &value);

void to_json(json &j, const TextEdit &value);
void from_json(const json &j, TextEdit &value);

void to_json(json &j, const TextDocumentItem &value);
void from_json(const json &, TextDocumentItem &);

void to_json(json &j, const ClientCapabilities &value);
void from_json(const json &, ClientCapabilities &);

void to_json(json &j, const ClangdCompileCommand &value);
void from_json(const json &, ClangdCompileCommand &);

void to_json(json &j, const ConfigurationSettings &value);
void from_json(const json &, ConfigurationSettings &);

void to_json(json &j, const InitializationOptions &value);
void from_json(const json &, InitializationOptions &);

void to_json(json &j, const InitializeParams &value);
void from_json(const json &, InitializeParams &);

void to_json(json &, const ShowMessageParams &);
void from_json(const json &j, ShowMessageParams &value);

void to_json(json &j, const Registration &value);
void from_json(const json &, Registration &);

void to_json(json &j, const RegistrationParams &value);
void from_json(const json &, RegistrationParams &);

void to_json(json &j, const UnregistrationParams &value);
void from_json(const json &, UnregistrationParams &);

void to_json(json &j, const DidOpenTextDocumentParams &value);
void from_json(const json &, DidOpenTextDocumentParams &);

void to_json(json &j, const DidCloseTextDocumentParams &value);
void from_json(const json &, DidCloseTextDocumentParams &);

void to_json(json &j, const TextDocumentContentChangeEvent &value);
void from_json(const json &, TextDocumentContentChangeEvent &);

void to_json(json &j, const DidChangeTextDocumentParams &value);
void from_json(const json &, DidChangeTextDocumentParams &);

void to_json(json &j, const FileEvent &value);
void from_json(const json &, FileEvent &);

void to_json(json &j, const DidChangeWatchedFilesParams &value);
void from_json(const json &, DidChangeWatchedFilesParams &);

void to_json(json &j, const DidChangeConfigurationParams &value);
void from_json(const json &, DidChangeConfigurationParams &);

void to_json(json &j, const DocumentRangeFormattingParams &value);
void from_json(const json &, DocumentRangeFormattingParams &);

void to_json(json &j, const DocumentOnTypeFormattingParams &value);
void from_json(const json &, DocumentOnTypeFormattingParams &);

void to_json(json &j, const FoldingRangeParams &value);
void from_json(const json &, FoldingRangeParams &);

void to_json(json &j, const SelectionRangeParams &value);
void from_json(const json &, SelectionRangeParams &);

void to_json(json &, const SelectionRange &);
void from_json(const json &j, SelectionRange &value);

void to_json(json &j, const DocumentFormattingParams &value);
void from_json(const json &, DocumentFormattingParams &);

void to_json(json &j, const DocumentSymbolParams &value);
void from_json(const json &, DocumentSymbolParams &);

void to_json(json &j, const DiagnosticRelatedInformation &value);
void from_json(const json &j, DiagnosticRelatedInformation &value);

void to_json(json &j, const Diagnostic &value);
void from_json(const json &j, Diagnostic &value);

void to_json(json &, const PublishDiagnosticsParams &);
void from_json(const json &j, PublishDiagnosticsParams &value);

void to_json(json &j, const CodeActionContext &value);
void from_json(const json &, CodeActionContext &);

void to_json(json &j, const CodeActionParams &value);
void from_json(const json &, CodeActionParams &);
void to_json(json &j, const WorkspaceEdit &value);
void from_json(const json &j, WorkspaceEdit &value);

void to_json(json &j, const TweakArgs &value);
void from_json(const json &j, TweakArgs &value);

void to_json(json &j, const ExecuteCommandParams &value);
void from_json(const json &, ExecuteCommandParams &);

void to_json(json &j, const LspCommand &value);
void from_json(const json &j, LspCommand &value);

void to_json(json &j, const CodeAction &value);
void from_json(const json &j, CodeAction &value);

void to_json(json &j, const SymbolInformation &value);
void from_json(const json &j, SymbolInformation &value);

void to_json(json &j, const WorkspaceSymbolParams &value);
void from_json(const json &, WorkspaceSymbolParams &);

void to_json(json &j, const ApplyWorkspaceEditParams &value);
void from_json(const json &, ApplyWorkspaceEditParams &);

void to_json(json &j, const TextDocumentPositionParams &value);
void from_json(const json &, TextDocumentPositionParams &);

void to_json(json &j, const CompletionContext &value);
void from_json(const json &, CompletionContext &);

void to_json(json &j, const CompletionParams &value);
void from_json(const json &, CompletionParams &);

void to_json(json &, const MarkupContent &);
void from_json(const json &j, MarkupContent &value);

void to_json(json &, const Hover &);
void from_json(const json &j, Hover &value);

void to_json(json &, const CompletionItem &);
void from_json(const json &j, CompletionItem &value);

void to_json(json &, const CompletionList &);
void from_json(const json &j, CompletionList &value);

void to_json(json &, const ParameterInformation &);
void from_json(const json &j, ParameterInformation &value);

void to_json(json &, const SignatureInformation &);
void from_json(const json &j, SignatureInformation &value);

void to_json(json &, const SignatureHelp &);
void from_json(const json &j, SignatureHelp &value);

void to_json(json &j, const RenameParams &value);
void from_json(const json &, RenameParams &);

void to_json(json &j, const TypeHierarchyParams &value);
void from_json(const json &, TypeHierarchyParams &);

}  // namespace lsp

#endif
