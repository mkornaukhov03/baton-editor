#ifndef BATON_INTERFACE_H
#define BATON_INTERFACE_H

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <string>
#include <vector>

// class to parse from json to C++/Qt containers

class LSPHandler : public QObject {
  Q_OBJECT

 public:
  LSPHandler(LSPHandler &&) = delete;
  LSPHandler(LSPHandler &) = delete;

  LSPHandler &operator=(LSPHandler &&) = delete;
  LSPHandler &operator=(LSPHandler &) = delete;

  ~LSPHandler() final = default;

 signals:
  void completion(const std::vector<std::string> &);
  void diagnostic(const std::vector<std::string> &);

 public slots:
  void OnNotify(const QString &, const QJsonObject &){};
  void OnResponse(const QJsonObject &, const QJsonObject &){};
  void OnRequest(const QString &, const QJsonObject &,
                 const QJsonObject &){};
  void OnError(const QJsonObject &, const QJsonObject &){};
  void OnServerError(QProcess::ProcessError ){};
  void OnServerFinished(int , QProcess::ExitStatus){};

  void textChanged(){};
  void requestDiagonistics(){};

 private:
  void set_connections(){};
};

#endif
