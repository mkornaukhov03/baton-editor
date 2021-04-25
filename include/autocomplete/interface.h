#ifndef BATON_INTERFACE_H
#define BATON_INTERFACE_H

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <string>
#include <vector>

#include <iostream> // debug

#include "client.h"
// class to parse from json to C++/Qt containers

namespace lsp {

class LSPHandler final: public  QObject  {
  Q_OBJECT

 public:
  LSPHandler(LSPHandler &&) = delete;
  LSPHandler(LSPHandler &) = delete;

  LSPHandler &operator=(LSPHandler &&) = delete;
  LSPHandler &operator=(LSPHandler &) = delete;

  LSPHandler(const std::string& root, const std::string& file_name, const std::string & content);

  ~LSPHandler() final;

 signals:
  void DoneCompletion(const std::vector<std::string> &);
  void DoneDiagnostic(const std::vector<std::string> &);

 public slots:
  // from LSP client, connected inside
  void GetNotify(const std::string &, json) { }
  void GetResponse(json, json);
  void GetRequest(const std::string &, json, json) { }
  void GetError(json, json) { }
  void GetServerError(QProcess::ProcessError) { }
  void GetServerFinished(int, QProcess::ExitStatus) { }
  void GetStderrOutput(const std::string & s) {
    std::cerr << "Inside LSPHandler::GetStderrrOutput\n";
    std::cerr << s << std::endl;
  }

  // from user
  void RequestCompletion(std::size_t, std::size_t);
  void FileChanged(const std::string & new_content, std::size_t, std::size_t);

 private:
  std::string root_;
  std::string file_;
  Client client_;
  void set_connections();
};
}// namespace lsp
#endif
