#ifndef BATON_INTERFACE_H
#define BATON_INTERFACE_H

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <string>
#include <vector>
#include "client.h"
// class to parse from json to C++/Qt containers

namespace lsp {

class LSPHandler : public QObject {
  Q_OBJECT

 public:
  LSPHandler(LSPHandler &&) = delete;
  LSPHandler(LSPHandler &) = delete;

  LSPHandler &operator=(LSPHandler &&) = delete;
  LSPHandler &operator=(LSPHandler &) = delete;

  LSPHandler(const std::string& root, const std::string& file_name);
  
  ~LSPHandler() final = default;

 signals:
  void completion(const std::vector<std::string> &);
  void diagnostic(const std::vector<std::string> &);

 public slots:
  // from LSP client, connected inside
  void GetNotify(const std::string &, json){ }
  void GetResponse(json, json){ }
  void GetRequest(const std::string &, json, json){ }
  void GetError(json, json){ }
  void GetServerError(QProcess::ProcessError){ }
  void GetServerFinished(int, QProcess::ExitStatus){ }
  void GetStderrOutput(const std::string &){ }

  // from user
  void textChanged(){ }
  void requestDiagonistics(){ }

 private:
  std::string root_uri_;
  std::string file_uri_;
  Client client_;
  void set_connections();
};
}// namespace lsp
#endif
