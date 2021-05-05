#ifndef FILE_VIEW_H
#define FILE_VIEW_H

#include <QWidget>
#include <string>

#include "editor.h"
#include "interface.h"
#include "lsp_basic.h"

namespace lsp {
class FileView : public QWidget {
  Q_OBJECT

 public:
  explicit FileView(const std::string& filename, QWidget* parent = nullptr);
  virtual ~FileView();

  void update();

 public slots:
  void uploadContent(const std::string& s);
  void changeCursor(int new_line, int new_col);

 private:
  LSPHandler handler_;
  std::string content_;
  int carriage_line_;
  int carriage_col_;
  bool completion_required_;
};
}  // namespace lsp

#endif  // FILE_VIEW_H
