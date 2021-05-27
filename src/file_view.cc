#include "file_view.h"

#include <QDir>
#include <algorithm>
#include <iterator>

#include "editor.h"
#include "handler.h"

FileView::FileView(const std::string& filename, QWidget* parent)
    : QWidget(parent),
      handler_(QDir::currentPath().toStdString(), filename, ""),
      content_(""),
      carriage_line_(0),
      carriage_col_(0),
      completion_required_(false),
      valid_cpp_(true) {
  connect(&handler_, &lsp::LSPHandler::DoneCompletion, this,
          &FileView::GetCompletion);

  connect(&handler_, &lsp::LSPHandler::DoneDiagnostic, this,
          &FileView::GetDiagnostic);
}
FileView::~FileView() {}

void FileView::GetCompletion(const std::vector<std::string>& compls) {
  emit DoneCompletion(compls);
}
void FileView::GetDiagnostic(
    const std::vector<lsp::DiagnosticsResponse>& diagns) {
  emit DoneDiagnostic(diagns);
}

void FileView::SetValidity(bool val) { valid_cpp_ = val; }
void FileView::UploadContent(const std::string& new_content) {
  if (!valid_cpp_) {
    return;
  }
  content_ = new_content;
  if (content_.back() != '\n') {
    content_ += '\n';
  }
  handler_.FileChanged(content_);
}

void FileView::ChangeCursor(int new_line, int new_col) {
  if (!valid_cpp_) {
    return;
  }
  carriage_line_ = new_line;
  carriage_col_ = new_col;
  [[maybe_unused]] bool cond = true;

  // searching for next charachter after cursor
  auto it = content_.begin();
  for (int line = 0; line < carriage_line_; ++line) {
    it = std::find(it, content_.end(), '\n');
    it++;
  }
  char next_char =
      content_[(std::distance(content_.begin(), it) + carriage_col_)];

  static char allowable_for_completion[] = {'\n', '\0', '\t', ' ', '}', ')'};

  cond = std::find(std::begin(allowable_for_completion),
                   std::end(allowable_for_completion),
                   next_char) != std::end(allowable_for_completion);

  completion_required_ = cond;

  if (completion_required_) {
    handler_.RequestCompletion(carriage_line_, carriage_col_);
  }
}
