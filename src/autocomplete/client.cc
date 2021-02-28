#include "client.h"

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <memory>

#include "json_serializers.h"

namespace lsp {
Client::Client(const QString &path, const QStringList &args)
    : process_(new QProcess()) {
  process_->setProgram(path);
  process_->setArguments(args);
}
Client::~Client() {
  if (process_) {
    process_->kill();
  }
}

// common request messages

Client::RequestType Client::Initialize(DocumentUri root) {
  if (is_initialized_) {
    return "[Already initialized!]\n";
  }

  is_initialized_ = true;
  InitializeParams params;
  params.processId = static_cast<uinteger>(QCoreApplication::applicationPid());
  params.rootUri = root;
  return SendRequest("initialize", params);
}

Client::RequestType Client::Shutdown() {
  is_initialized_ = false;
  return SendRequest("shutdown", {});
}

Client::RequestType Client::RangeFormatting(DocumentUri uri, Range range) {
  return SendRequest("textDocument/RangeFormatting",
                     DocumentRangeFormattingParams{uri, range});
}

Client::RequestType Client::FoldingRange(DocumentUri uri) {
  return SendRequest("textDocument/foldingRange", FoldingRangeParams{uri});
}

Client::RequestType Client::SelectionRange(DocumentUri uri,
                                           std::vector<Position> positions) {
  return SendRequest("textDocument/selectionRange",
                     SelectionRangeParams{uri, std::move(positions)});
}

Client::RequestType Client::Formatting(DocumentUri uri) {
  return SendRequest("textDocument/formatting", DocumentFormattingParams{uri});
}

Client::RequestType Client::CodeAction(DocumentUri uri, Range range,
                                       CodeActionContext context) {
  return SendRequest("textDocument/codeAction",
                     CodeActionParams{uri, range, std::move(context)});
}

Client::RequestType Client::Completion(DocumentUri uri, Position position,
                                       CompletionContext context) {
  //                            CompletionParams params;
  return SendRequest("textDocument/completion",
                     CompletionParams{uri, position, context});
}

// common notification messages

void Client::Exit() { SendNotification("exit", {}); }

void Client::Initialized() { SendNotification("initialized", {}); }

void Client::DidOpen(DocumentUri uri, std::string_view code) {
  SendNotification("textDocument/didOpen",
                   DidOpenTextDocumentParams{uri, code, 0, "cpp"});
}

void Client::DidClose(DocumentUri uri) {
  SendNotification("testDocument/didClose", DidCloseTextDocumentParams{uri});
}

void Client::DidChange(DocumentUri uri,
                       std::vector<TextDocumentContentChangeEvent> changes,
                       bool wantDiagnostics) {
  SendNotification(
      "textDocument/didChange",
      DidChangeTextDocumentParams{uri, std::move(changes), wantDiagnostics});
}

// general notificator and requester

void Client::SendNotification(std::string_view method, json json_doc) {
  NotifyImpl(method, std::move(json_doc));
}

Client::RequestType Client::SendRequest(std::string_view method,
                                        json json_doc) {
  RequestImpl(method, std::move(json_doc), RequestType(method));
  return RequestType(method);
}

// private helpers
void Client::WriteToServer(std::string dump) {
  send_to_server_buffer_.push_back(
      "Content-Length: " + std::to_string(dump.length()) + "\r\n\r\n");
  send_to_server_buffer_.push_back(std::move(dump));
}

void Client::NotifyImpl(std::string_view method, json params) {
  json notification = {
      {"jsonrpc", "2.0"}, {"method", method}, {"params", params}};
  WriteToServer(notification.dump());
}

void Client::RequestImpl(std::string_view method, json params,
                         RequestType type) {
  // in json request, id is type of desirable request
  json request = {
      {"jsonrpc", "2.0"}, {"id", type}, {"method", method}, {"params", params}};
  WriteToServer(request.dump());
}

}  // namespace lsp
