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
    const int WAITING_TIME = -1;
    process_->waitForFinished(WAITING_TIME);
  }
}

void Client::SetConnections() {
  connect(process_.get(), &QProcess::errorOccurred, this,
          &Client::OnClientError);
  connect(process_.get(), &QProcess::readyReadStandardOutput, this,
          &Client::OnClientReadyReadStdout);
  connect(process_.get(), &QProcess::readyReadStandardError, this,
          &Client::OnClientReadyReadStderr);
  connect(process_.get(),
          QMetaObject::normalizedSignature(
              SIGNAL(finished(int, QProcess::ExitStatus))),
          this,
          QMetaObject::normalizedSignature(
              SLOT(OnClientFinished(int, QProcess::ExitStatus))));
}

// private slots
void Client::OnClientReadyReadStdout() {
  static std::string buff{};
  static unsigned msg_size = 0;

  QByteArray cur_buffer = process_->readAllStandardOutput();

  try {
    auto process_message = [&]() {
      json msg = json::parse(buff);

      if (msg.contains("id")) {
        if (msg.contains("method")) {
          emit OnRequest(msg["method"].get<std::string>(), msg["params"],
                         msg["id"]);
        } else if (msg.contains("result")) {
          emit OnResponse(msg["id"], msg["result"]);
        } else if (msg.contains("error")) {
          emit OnError(msg["id"], msg["error"]);
        }
      } else if (msg.contains("method")) {
        if (msg.contains("params")) {
          emit OnNotify(msg["method"].get<std::string>(), msg["params"]);
        }
      } else {
      }

      buff.clear();
      msg_size = 0;
    };

    if (cur_buffer.indexOf("\r\n\r\n") == -1) {  // the middle of the message
      for (const auto ch : cur_buffer.toStdString()) {
        buff.push_back(ch);
      }
      if (buff.size() == msg_size) {
        process_message();
      }
    } else {  // the start of the message
      int msg_start = cur_buffer.indexOf("\r\n\r\n") +
                      static_cast<int>(std::strlen("\r\n\r\n"));
      int len_start = cur_buffer.indexOf("Content-Length: ") +
                      static_cast<int>(std::strlen("Content-Length: "));
      int len_end = cur_buffer.indexOf("\r\n");
      bool ok = false;
      [[maybe_unused]] int content_length =
          cur_buffer.mid(len_start, len_end - len_start).toInt(&ok);
      QByteArray payload = cur_buffer.mid(msg_start);
      msg_size = payload.size();
      buff = payload.toStdString();
      if (payload.size() == content_length) {
        process_message();
      }
    }
  } catch (...) {
    std::cerr << "ASSERT IN JSON FAULT!\n";
  }
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
  return SendRequest(
      "textDocument/RangeFormatting",
      DocumentRangeFormattingParams{{std::move(uri)}, std::move(range)});
}

Client::RequestType Client::FoldingRange(DocumentUri uri) {
  return SendRequest("textDocument/foldingRange",
                     FoldingRangeParams{std::move(uri)});
}

Client::RequestType Client::SelectionRange(DocumentUri uri,
                                           std::vector<Position> positions) {
  return SendRequest(
      "textDocument/selectionRange",
      SelectionRangeParams{{std::move(uri)}, std::move(positions)});
}

Client::RequestType Client::Formatting(DocumentUri uri) {
  return SendRequest("textDocument/formatting",
                     DocumentFormattingParams{std::move(uri)});
}

Client::RequestType Client::CodeAction(DocumentUri uri, Range range,
                                       CodeActionContext context) {
  return SendRequest("textDocument/codeAction",
                     CodeActionParams{{uri}, range, std::move(context)});
}

Client::RequestType Client::Completion(DocumentUri uri, Position position,
                                       CompletionContext context) {
  CompletionParams params;
  params.textDocument.uri = std::move(uri);
  params.position = std::move(position);
  params.context = std::move(context);
  return SendRequest("textDocument/completion", params);
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
      DidChangeTextDocumentParams{{uri}, std::move(changes), wantDiagnostics});
}

// general notificator and requester

void Client::SendNotification(std::string method, json json_doc) {
  NotifyImpl(std::move(method), std::move(json_doc));
}

Client::RequestType Client::SendRequest(std::string method, json json_doc) {
  RequestImpl(method, std::move(json_doc), RequestType(method));
  return RequestType(method);
}

// private helpers
void Client::WriteToServer(const std::string &dump) {
  send_to_server_buffer_.push_back(
      "Content-Length: " + std::to_string(dump.length()) + "\r\n");
  send_to_server_buffer_.push_back("\r\n");
  send_to_server_buffer_.push_back(std::move(dump));
  if (process_ != nullptr && process_->state() == QProcess::Running) {
    for (const auto &row : send_to_server_buffer_) {
      process_->write(row.c_str());
    }
    send_to_server_buffer_.clear();
  }
  process_->waitForReadyRead(10);
}

void Client::NotifyImpl(std::string method, json params) {
  json notification = {
      {"jsonrpc", "2.0"}, {"method", method}, {"params", params}};
  WriteToServer(notification.dump());
}

void Client::RequestImpl(std::string method, json params, RequestType type) {
  // in json request, id is type of desirable request
  json request = {{"jsonrpc", "2.0"},
                  {"id", std::move(type)},
                  {"method", std::move(method)},
                  {"params", std::move(params)}};
  WriteToServer(request.dump());
}

}  // namespace lsp
