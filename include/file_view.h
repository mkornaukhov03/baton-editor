#ifndef FILE_VIEW_H
#define FILE_VIEW_H

#include <QWidget>
#include <string>
#include <vector>

#include "editor.h"
#include "handler.h"
#include "lsp_basic.h"

class FileView : public QWidget {
  Q_OBJECT

 public:
  explicit FileView(const std::string& filename, QWidget* parent = nullptr);
  virtual ~FileView();
  void SetValidity(bool);
 signals:
  void DoneCompletion(const std::vector<std::string>&);
  void DoneDiagnostic(const std::vector<lsp::DiagnosticsResponse>&);
 public slots:
  void UploadContent(const std::string& s);
  void ChangeCursor(int new_line, int new_col);

 private slots:
  void GetCompletion(const std::vector<std::string>&);
  void GetDiagnostic(const std::vector<lsp::DiagnosticsResponse>&);

 private:
  lsp::LSPHandler handler_;
  std::string content_;
  int carriage_line_;
  int carriage_col_;
  bool completion_required_;
  bool valid_cpp_;
};

#endif  // FILE_VIEW_H
