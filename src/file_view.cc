#include "file_view.h"

#include <QDir>

#include "editor.h"

namespace lsp {
FileView::FileView(const std::string& filename, QWidget* parent)
    : QWidget(parent),
      handler_(QDir::currentPath().toStdString(), filename, ""),
      content_(""),
      carriage_line_(0),
      carriage_col_(0),
      completion_required_(false) {
  // set connections for handler
  connect(&handler_, SIGNAL(DoneCompletion(const std::vector<std::string>&)),
          this, SLOT(GetCompletion(const std::vector<std::string>&)));

  connect(&handler_,
          SIGNAL(DoneDiagnostic(const std::vector<lsp::DiagnosticsResponse>&)),
          this,
          SLOT(GetDiagnostic(const std::vector<lsp::DiagnosticsResponse>&)));
}
FileView::~FileView() {}

void FileView::GetCompletion(const std::vector<std::string>& compls) {
  emit DoneCompletion(compls);
}
void FileView::GetDiagnostic(
    const std::vector<lsp::DiagnosticsResponse>& diagns) {
  emit DoneDiagnostic(diagns);
}

void FileView::UploadContent(const std::string& new_content) {
  content_ = new_content;
  update();
}

void FileView::ChangeCursor(int new_line, int new_col) {
  carriage_line_ = new_line;
  carriage_col_ = new_col;
  [[maybe_unused]] bool cond = true;  // TODO: check the next symbol
  if (cond) {
    completion_required_ = true;
  }
  update();
}

// not ready
void FileView::Update() {
  handler_.FileChanged(content_);
  if (completion_required_) {
    handler_.RequestCompletion(carriage_line_, carriage_col_);
  }
}

}  // namespace lsp
