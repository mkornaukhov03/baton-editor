#include "client.h"

#include <memory>

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

// common notification messages

void Client::Exit() { SendNotification("exit", {}); }

void Client::Initialized() { SendNotification("initialized", {}); }

void Client::DidOpen(DocumentUri uri, std::string_view code) {
  DidOpenTextDocumentParams params;
  params.textDocument.uri = uri;
  params.textDocument.text = code;
  params.textDocument.languageId = "cpp";
  json json_doc;
  //   nlohmann::to_json(json_doc, params);
  //   SendNotification("textDocument/didOpen", json_doc);
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
