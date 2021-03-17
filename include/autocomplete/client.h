#ifndef BATON_CLIENTS_H
#define BATON_CLIENTS_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QProcess>
#include <memory>
#include <vector>
#include <string>

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

    // request messages to send to server specified by lsp protocol

    RequestType Initialize(DocumentUri root = {});
    RequestType Shutdown();

    RequestType RangeFormatting(DocumentUri uri, Range range);
    RequestType FoldingRange(DocumentUri uri);
    RequestType SelectionRange(DocumentUri uri, std::vector<Position> positions);

    RequestType Formatting(DocumentUri uri);
    RequestType CodeAction(DocumentUri uri, Range range,
                           CodeActionContext context);
    RequestType Completion(DocumentUri uri, Position position,
                           CompletionContext context = {});

    // common(more highly abstract than general notificator) notification messages
    // specified by LSP-protocol
    void Exit();
    void Initialized();
    void DidOpen(DocumentUri uri, std::string code);
    void DidClose(DocumentUri uri);
    void DidChange(DocumentUri uri,
                   std::vector<TextDocumentContentChangeEvent> changes,
                   bool wantDiagnostics = false);

    void LogInfo();
    // general notificator and requester
    void SendNotification(std::string method, json json_doc);
    RequestType SendRequest(std::string method, json json_doc);

signals:
    void OnNotify(const std::string &method, json params);
    void OnResponse(json id, json params);
    void OnRequest(const std::string &method, json params, json id);
    void OnError(json id, json error);
    void OnServerError(QProcess::ProcessError error);
    void OnServerFinished(int exitCode, QProcess::ExitStatus status);
    void NewStderr(const std::string &content);

private slots:
    void OnClientReadyReadStdout();
    void OnClientReadyReadStderr();
    void OnClientError(QProcess::ProcessError error);
    void OnClientFinished(int exit_code, QProcess::ExitStatus status);

private:
    std::unique_ptr<QProcess> process_;
    std::vector<std::string> send_to_server_buffer_;
    bool is_initialized_ = false;

    void WriteToServer(std::string);
    void NotifyImpl(std::string, json params);
    void RequestImpl(std::string method, json params, RequestType type);
    void SetConnections();
};
}  // namespace lsp

#endif
