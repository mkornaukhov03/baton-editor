#include "client.h"

#include <memory>

namespace lsp {
Client::Client(const QString &path, const QStringList &args)
    : process_(new QProcess()) {
  process_->setProgram(path);
  process_->setArguments(args);
}
Client::~Client() {
  if (process_) {
    process_->kill();
  }
}

void Client::WriteToServer(std::string dump) {
  send_to_server_buffer_.push_back(
      "Content-Length: " + std::to_string(dump.length()) + "\r\n\r\n");
  send_to_server_buffer_.push_back(std::move(dump));
}

void Client::RequestImpl(std::string_view method, json params,
                         RequestType type) {
  // in json request, id is type of desirable request
  json request = {
      {"jsonrpc", "2.0"}, {"id", type}, {"method", method}, {"params", params}};
  WriteToServer(request.dump());
}

}  // namespace lsp
