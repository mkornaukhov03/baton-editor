#include "interface.h"

#include <QObject>
#include <QString>

#include <iostream> // debug

namespace lsp {
LSPHandler::LSPHandler(const std::string& root, const std::string& file_name,
                       const std::string& content)
    : root_(root), file_(file_name), client_(QString("clangd"), {}) {
  client_.Initialize("file://" + root_);
  client_.DidOpen("file:///" + file_, content);
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
  connect(&client_, SIGNAL(NewStderr(const std::string& )), this,
          SLOT(GetStderrOutput(const std::string&)));
}

void LSPHandler::GetResponse(json id, json result) {
  std::cerr << "==== INSIDE GetResponse() ====" << std::endl;
  std::string id_str = id.get<std::string>();
  if (id_str == "textDocument/completion") {
    std::vector<std::string> resp;
    for (auto item : result["items"]) {
      resp.push_back(item["insertText"].get<std::string>());
    }
    emit DoneCompletion(resp);
  }
  else {
    std::cerr << "NOT COMPLETION!!\n";
  }
}


void LSPHandler::RequestCompletion(std::size_t line, std::size_t col) {
  std::cerr << "Root: " << root_ << '\n';
  std::cerr << "File: " << file_ << '\n';
  client_.Completion("file:///" + file_, Position{line, col});
}

void LSPHandler::FileChanged(const std::string& new_content,
                             std::size_t last_line, std::size_t last_col) {
  client_.DidChange(
      "file:///" + file_,
      std::vector<lsp::TextDocumentContentChangeEvent>{
          {Range{{0, 0}, {last_line, last_col}}, std::nullopt, new_content}}, true);
}

LSPHandler::~LSPHandler() {
  client_.DidClose("file:///" + file_);
  client_.Shutdown();
  client_.Exit();
}

}  // namespace lsp
