#include "file_view.h"

#include <QDir>

#include "editor.h"

namespace lsp {
FileView::FileView(const std::string& filename, Editor* editor, QWidget* parent)
    : QWidget(parent),
      handler_(QDir::currentPath().toStdString(), filename, ""),
      content_(""),
      carriage_line_(0),
      carriage_col_(0),
      completion_required_(false),
      editor_(editor) {
  // set connections for handler
}
FileView::~FileView() {}

// not ready
void FileView::update() {
  editor_->setReadOnly(true);

  carriage_line_ = editor_->textCursor().blockNumber();
  carriage_col_ = editor_->textCursor().columnNumber();
  content_ = editor_->toPlainText().toStdString();

  handler_.FileChanged(content_);
  if (completion_required_) {
    handler_.RequestCompletion(carriage_line_, carriage_col_);
  }

  editor_->setReadOnly(false);
}

}  // namespace lsp
