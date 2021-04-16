#include "client.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <cstring>
#include <iostream>
#include <memory>

#include "json_serializers.h"
#include "nlohmann/json.hpp"

namespace lsp {
Client::Client(const QString &path, const QStringList &args)
    : process_(new QProcess()) {
  process_->setProgram(path);
  process_->setArguments(args);
  SetConnections();

  process_->start();
  process_->waitForStarted();
}

Client::~Client() {
  if (process_) {
    process_->kill();
  }
}

void Client::SetConnections() {
  connect(process_.get(), SIGNAL(errorOccurred(QProcess::ProcessError)), this,
          SLOT(OnClientError(QProcess::ProcessError)));
  connect(process_.get(), SIGNAL(readyReadStandardOutput()), this,
          SLOT(OnClientReadyReadStdout()));
  connect(process_.get(), SIGNAL(readyReadStandardError()), this,
          SLOT(OnClientReadyReadStderr()));
  connect(process_.get(), SIGNAL(finished(int, QProcess::ExitStatus)), this,
          SLOT(OnClientFinished(int, QProcess::ExitStatus)));
  // connect(process_.get(), SIGNAL(started()), this, SLOT(OnStarted()));
}

// private slots
void Client::OnClientReadyReadStdout() {
  // FIXME should create new thread and use waitForReadyRead

  QByteArray buffer = process_->readAllStandardOutput();
  int msg_start =
      buffer.indexOf("\r\n\r\n") + static_cast<int>(std::strlen("\r\n\r\n"));
  int len_start = buffer.indexOf("Content-Length: ") +
                  static_cast<int>(std::strlen("Content-Length: "));
  int len_end = buffer.indexOf("\r\n");
  assert(buffer.indexOf("\r\n\r\n") != -1 &&
         "Server answer does not requre protocol");
  assert(buffer.indexOf("Content-Length: ") != -1 &&
         "Server answer does not requre protocol");
  bool ok = false;


  [[maybe_unused]]int content_length = buffer.mid(len_start, len_end - len_start).toInt(&ok);


  assert(ok && "Server answer does not requre protocol");

  QByteArray payload = buffer.mid(msg_start);

  QJsonParseError error{};
  auto msg2 = QJsonDocument::fromJson(payload, &error);
  assert(payload.size() == content_length && "Not full message!");
  //   try {
  json msg(msg2.toJson().toStdString());  // possibly raise an exception
  auto rpc = json::parse(json::parse(msg.dump()).get<std::string>());
  std::cerr << rpc << '\n';
  msg = rpc;
  if (msg.contains("id")) {
    if (msg.contains("method")) {
      emit OnRequest(msg["method"].get<std::string>(), msg["params"],
                     msg["id"]);
    } else if (msg.contains("result")) {
      std::cerr << "Emitting OnResponse!\n";
      emit OnResponse(msg["id"], msg["result"]);
    } else if (msg.contains("error")) {
      emit OnError(msg["id"], msg["error"]);
    }
  } else if (msg.contains("method")) {
    if (msg.contains("params")) {
      emit OnNotify(msg["method"].get<std::string>(), msg["params"]);
    }
  } else {
    std::cerr << "Nothing emitted!\n";
  }

  //   } catch (const std::exception &ex) {
  //     std::cerr << ex.what();
  //     return;
  //   }
}

void Client::OnClientReadyReadStderr() {
  std::string content = QString(process_->readAllStandardError()).toStdString();
  if (!content.empty()) emit NewStderr(content);
}

void Client::OnClientError(QProcess::ProcessError error) {
  emit OnServerError(error);
}

void Client::OnClientFinished(int exit_code, QProcess::ExitStatus status) {
  emit OnServerFinished(exit_code, status);
}

// common request messages

Client::RequestType Client::Initialize(DocumentUri root_uri) {
  if (is_initialized_) {
    return "[Already initialized!]\n";
  }

  is_initialized_ = true;
  InitializeParams params{};
  params.processId = static_cast<uinteger>(QCoreApplication::applicationPid());
  params.rootUri = root_uri;
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

void Client::DidOpen(DocumentUri uri, std::string code) {
  DidOpenTextDocumentParams params;
  params.textDocument.uri = uri;
  params.textDocument.languageId = "cpp";
  params.textDocument.text = code;
  SendNotification("textDocument/didOpen", params);
}

void Client::DidClose(DocumentUri uri) {
  SendNotification("textDocument/didClose", DidCloseTextDocumentParams{uri});
}

void Client::DidChange(DocumentUri uri,
                       std::vector<TextDocumentContentChangeEvent> changes,
                       bool wantDiagnostics) {
  SendNotification(
      "textDocument/didChange",
      DidChangeTextDocumentParams{uri, std::move(changes), wantDiagnostics});
}

// general notificator and requester

void Client::SendNotification(std::string method, json json_doc) {
  NotifyImpl(method, std::move(json_doc));
}

Client::RequestType Client::SendRequest(std::string method, json json_doc) {
  RequestImpl(method, std::move(json_doc), RequestType(method));
  return RequestType(method);
}

// private helpers
void Client::WriteToServer(std::string dump) {
  send_to_server_buffer_.push_back(
      "Content-Length: " + std::to_string(dump.length()) + "\r\n");
  send_to_server_buffer_.push_back("\r\n");
  send_to_server_buffer_.push_back(std::move(dump));
  //     assert(process_-> state() == QProcess::Running);
  if (process_ != nullptr && process_->state() == QProcess::Running) {
    for (auto row : send_to_server_buffer_) {
      process_->write(row.c_str());
    }
    send_to_server_buffer_.clear();
  }
  process_->waitForReadyRead(100);
}

void Client::NotifyImpl(std::string method, json params) {
  json notification = {
      {"jsonrpc", "2.0"}, {"method", method}, {"params", params}};
  WriteToServer(notification.dump());
}

void Client::RequestImpl(std::string method, json params, RequestType type) {
  // in json request, id is type of desirable request
  json request = {
      {"jsonrpc", "2.0"}, {"id", type}, {"method", method}, {"params", params}};
  WriteToServer(request.dump());
}

}  // namespace lsp
