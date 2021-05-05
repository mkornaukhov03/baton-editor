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
}
FileView::~FileView() {}

void FileView::uploadContent(const std::string& new_content) {
  content_ = new_content;
}

void FileView::changeCursor(int new_line, int new_col) {
  carriage_line_ = new_line;
  carriage_col_ = new_col;
}

// not ready
void FileView::update() {
  handler_.FileChanged(content_);
  if (completion_required_) {
    handler_.RequestCompletion(carriage_line_, carriage_col_);
  }
}

}  // namespace lsp
