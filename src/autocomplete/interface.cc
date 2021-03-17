#include "interface.h"

#include <QObject>
#include <QString>

namespace lsp {
LSPHandler::LSPHandler(const std::string& root, const std::string& file_name)
    : root_uri_("file://" + root),
      file_uri_("file://" + file_name),
      client_(QString("clangd"), {}) {
  set_connections();
}
void LSPHandler::set_connections() {
  connect(&client_, SIGNAL(OnNotify(const std::string&, json)), this,
          SLOT(GetNotify(const std::string&, json)));
  connect(&client_, SIGNAL(OnResponse(json, json)), this,
          SLOT(GetResponse(json, json)));
  connect(&client_, SIGNAL(OnRequest(const std::string&, json, json)), this,
          SLOT(GetRequest(const std::string&, json, json)));
  connect(&client_, SIGNAL(OnError(json, json)), this,
          SLOT(GetError(json, json)));
  connect(&client_, SIGNAL(OnServerError(QProcess::ProcessError)), this,
          SLOT(GetServerError(QProcess::ProcessError)));
  connect(&client_, SIGNAL(OnServerFinished(int, QProcess::ExitStatus)), this,
          SLOT(GetServerFinished(int, QProcess::ExitStatus)));
  connect(&client_, SIGNAL(NewStderr(const std::string& content)), this,
          SLOT(GetStderrOutput(const std::string&)));
}
}  // namespace lsp
