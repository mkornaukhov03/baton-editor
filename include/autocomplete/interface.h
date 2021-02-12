#ifndef BATON_INTERFACE_H
#define BATON_INTERFACE_H

#include <QObject>
//#include <QStringList>
#include <QTimer>
#include <string>
#include <vector>

//  1) Need a RO access to file + position of cursor

class LSPHandler : public QObject {
  Q_OBJECT

 public:
  LSPHandler(LSPHandler &&) = delete;
  LSPHandler(LSPHandler &) = delete;

  LSPHandler &operator=(LSPHandler &&) = delete;
  LSPHandler &operator=(LSPHandler &) = delete;

  ~LSPHandler() final;

 signals:
  void completion(std::vector<string>);
  void diagnostic(std::vector<string>);

 public slots:
  void OnNotify(QString method, QJsonObject param);
  void OnResponse(QJsonObject id, QJsonObject response);
  void OnRequest(QString method, QJsonObject param, QJsonObject id);
  void OnError(QJsonObject id, QJsonObject error);
  void OnServerError(QProcess::ProcessError error);
  void OnServerFinished(int exitCode, QProcess::ExitStatus status);

  void textChanged();
  void requestDiagonistics();

 private:
  // LSPClient * lsp_;
  // QPlainTextEdit * code_;
  // QTimer timer_completion_, timer_diagnonstic;

  void set_connections();
};

#endif
