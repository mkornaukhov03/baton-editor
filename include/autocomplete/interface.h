#ifndef BATON_INTERFACE_H
#define BATON_INTERFACE_H

#include <QProcess>
#include <QObject>
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
  void completion(const std::vector<std::string> &);
  void diagnostic(const std::vector<std::string> &);

 public slots:
  void OnNotify(const QString &method, const QJsonObject &param);
  void OnResponse(const QJsonObject &id, const QJsonObject &response);
  void OnRequest(const QString &method, const QJsonObject &param,
                 const QJsonObject &id);
  void OnError(const QJsonObject &id, const QJsonObject &error);
  void OnServerError(QProcess::ProcessError error);
  void OnServerFinished(int exitCode, QProcess::ExitStatus status);

  void textChanged();
  void requestDiagonistics();

 private:
  void set_connections();
};

#endif
