#include "handler.h"

#include <json_serializers.h>

#include <QObject>
#include <QString>
#include <iostream>  // debug

namespace lsp {
LSPHandler::LSPHandler(const std::string& root, const std::string& file_name,
                       const std::string& content)
    : root_(root), file_(file_name), client_(QString("clangd"), {}) {
  client_.Initialize("file://" + root_);
  client_.DidOpen("file:///" + file_, content);
  set_connections();
}
void LSPHandler::set_connections() {
  connect(&client_, &Client::OnNotify, this, &LSPHandler::GetNotify);

  connect(&client_, &Client::OnResponse, this, &LSPHandler::GetResponse);

  connect(&client_, &Client::OnRequest, this, &LSPHandler::GetRequest);

  connect(&client_, &Client::OnError, this, &LSPHandler::GetError);

  connect(&client_, &Client::OnServerError, this, &LSPHandler::GetServerError);

  connect(&client_, &Client::OnServerFinished, this,
          &LSPHandler::GetServerFinished);

  connect(&client_, &Client::NewStderr, this, &LSPHandler::GetStderrOutput);
}

void LSPHandler::GetResponse(json id, json result) {
  std::string id_str = id.get<std::string>();

  const unsigned MAX_COMPLETION_ITEMS = 13;

  if (id_str == "textDocument/completion") {
    auto is_valid = [&](const std::string& s) {
      assert(s.size() > 0);
      if (s.size() == 0) return false;
      if (s.size() == 1) return true;
      bool resp =
          !(s[0] == '_' && (s[1] == '_' || (s[1] >= 'A' && s[1] <= 'Z')));
      const std::size_t sz = std::string("std::__").size();
      if (s.size() >= sz) {
        resp &= std::string(s.begin(), std::next(s.begin(), sz)) != "std::__";
      }
      return resp;
    };

    constexpr char stop_symbols[] = {'<', '(', '$', ' ', '{'};
    std::vector<std::string> resp;
    for (auto item : result["items"]) {
      std::string s = item["insertText"].get<std::string>();
      if (!is_valid(s)) continue;
      s = std::string(s.begin(), std::find_if(s.begin(), s.end(), [&](char ch) {
                        return std::find(std::begin(stop_symbols),
                                         std::end(stop_symbols),
                                         ch) != std::end(stop_symbols);
                      }));
      resp.push_back(s);
    }
    std::sort(resp.begin(), resp.end());
    resp.erase(std::unique(resp.begin(), resp.end()), resp.end());
    if (resp.size() > MAX_COMPLETION_ITEMS) resp.clear();
    emit DoneCompletion(resp);
  } else {
    std::cerr << "Response from server: not a completion\n" << std::endl;
  }
}

void LSPHandler::GetNotify(const std::string& id, json result) {
  if (id == "textDocument/publishDiagnostics") {
    std::vector<lsp::DiagnosticsResponse> resp;
    for (const auto& item : result["diagnostics"]) {
      Range rng;
      from_json(item["range"], rng);
      resp.emplace_back(
          lsp::DiagnosticsResponse{item["category"], item["message"], rng});
    }
    emit DoneDiagnostic(resp);
  } else {
    std::cerr << "Notification from server: not a diagnostics\n";
  }
}

void LSPHandler::RequestCompletion(std::size_t line, std::size_t col) {
  client_.Completion("file:///" + file_, Position{line, col});
}

void LSPHandler::FileChanged(const std::string& new_content) {
  client_.DidChange(
      "file:///" + file_,
      std::vector<lsp::TextDocumentContentChangeEvent>{{new_content}}, true);
}

LSPHandler::~LSPHandler() {
  client_.DidClose("file:///" + file_);
  client_.Shutdown();
  client_.Exit();
}

}  // namespace lsp
