#ifndef BATON_CLIENTS_H
#define BATON_CLIENTS_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QProcess>
#include <memory>
#include <vector>

#include "enums.h"
#include "lsp_basic.h"
#include "string_view"

namespace lsp {
class Client : public QObject {
  Q_OBJECT

  using RequestType = std::string;

 public:
  Client(const QString &path, const QStringList &args);
  Client(Client &&) = delete;
  Client(const Client &) = delete;
  Client &operator=(Client &&) = delete;
  Client &operator=(const Client &) = delete;
  ~Client() final;

  // common(more highly abstract than general notificator) notification messages
  // specified by LSP-protocol to comm
  void Exit();
  void Initialized();
  void DidOpen(DocumentUri uri, std::string_view code);
  void DidClose(DocumentUri uri){};
  void DidChange(DocumentUri uri,
                 const std::vector<TextDocumentContentChangeEvent> &changes,
                 std::optional<bool> wantDiagnostics = false){};

  // general notificator and requester
  void SendNotification(std::string_view method, json json_doc);
  RequestType SendRequest(std::string_view method, json json_doc);

 private:
  std::unique_ptr<QProcess> process_;
  std::vector<std::string> send_to_server_buffer_;
  bool has_initialized_ = false;

  void WriteToServer(std::string);
  void NotifyImpl(std::string_view, json params);
  void RequestImpl(std::string_view method, json params, RequestType type);
};
}  // namespace lsp

#endif
