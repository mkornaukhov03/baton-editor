#include "client.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <cstring>
#include <memory>

// for debug
#include <cassert>
#include <iostream>

#include "json_serializers.h"
#include "nlohmann/json.hpp"

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
  int content_length = buffer.mid(len_start, len_end - len_start).toInt(&ok);
  assert(ok && "Server answer does not requre protocol");

  QByteArray payload = buffer.mid(msg_start);
  assert(payload.size() != content_length && "Not full message!");

  /*
    if (obj.contains("id"))
    {
        if (obj.contains("method"))
        {
            emit onRequest(obj["method"].toString(), obj["param"].toObject(),
    obj["id"].toObject());
        }
        else if (obj.contains("result"))
        {
            emit onResponse(obj["id"].toObject(), obj["result"].toObject());
        }
        else if (obj.contains("error"))
        {
            emit onError(obj["id"].toObject(), obj["error"].toObject());
        }
    }
    else if (obj.contains("method"))
    {
        // notification
        if (obj.contains("params"))
        {
            emit onNotify(obj["method"].toString(), obj["params"].toObject());
        }
    }
  */

  nlohmann::json msg(payload);  // possibly raise an exception
  if (msg.contains("id")) {
    if (msg.contains("method")) {
      std::cerr << "request with method: " << msg["method"].get<std::string>()
                << '\n';
      emit OnRequest(msg["method"].get<std::string>(), msg["params"],
                     msg["id"]);
    } else if (msg.contains("result")) {
      std::cerr << "response with result: " << msg["result"].dump() << '\n';
      emit OnResponse(msg["id"], msg["result"]);
    } else if (msg.contains("error")) {
      std::cerr << "error ocured! " << msg["error"].dump() << '\n';
      emit OnError(msg["id"], msg["error"]);
    }
  } else if (msg.contains("method")) {
    if (msg.contains("params")) {
      emit OnNotify(msg["method"].get<std::string>(), msg["params"]);
    }
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
  return SendRequest("textDocument/RangeFormatting", {uri, range});
}

Client::RequestType Client::FoldingRange(DocumentUri uri) {
  return SendRequest("textDocument/foldingRange", FoldingRangeParams{uri});
}

Client::RequestType Client::SelectionRange(DocumentUri uri,
                                           std::vector<Position> positions) {
  return SendRequest("textDocument/selectionRange",
                     {uri, std::move(positions)});
}

Client::RequestType Client::Formatting(DocumentUri uri) {
  return SendRequest("textDocument/formatting", {uri});
}

Client::RequestType Client::CodeAction(DocumentUri uri, Range range,
                                       CodeActionContext context) {
  return SendRequest("textDocument/codeAction",
                     CodeActionParams{uri, range, std::move(context)});
}

Client::RequestType Client::Completion(DocumentUri uri, Position position,
                                       CompletionContext context) {
  //                            CompletionParams params;
  return SendRequest("textDocument/completion", {uri, position, context});
}

// common notification messages

void Client::Exit() { SendNotification("exit", {}); }

void Client::Initialized() { SendNotification("initialized", {}); }

void Client::DidOpen(DocumentUri uri, std::string_view code) {
  SendNotification("textDocument/didOpen", {uri, code, 0, "cpp"});
}

void Client::DidClose(DocumentUri uri) {
  SendNotification("testDocument/didClose", DidCloseTextDocumentParams{uri});
}

void Client::DidChange(DocumentUri uri,
                       std::vector<TextDocumentContentChangeEvent> changes,
                       bool wantDiagnostics) {
  SendNotification("textDocument/didChange",
                   {uri, std::move(changes), wantDiagnostics});
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
