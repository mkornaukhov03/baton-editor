#ifndef Editor_H
#define Editor_H
#include <QCompleter>
#include <QFontMetrics>
#include <QMap>
#include <QPlainTextEdit>
#include <QPointer>
#include <QToolBar>
#include <iostream>
#include <string>

#include "syntax_highlighter.h"

QT_BEGIN_NAMESPACE
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class QTextCharFormat;
class QComboBox;
class QAction;
class QMenu;
QT_END_NAMESPACE
class LineNumberArea;

class Editor : public QPlainTextEdit {
  Q_OBJECT

 public:
  explicit Editor(std::size_t fontSize = DEFAULT_FONT_SIZE,
                  QWidget *parent = nullptr);

  void lineNumberAreaPaintEvent(QPaintEvent *event);
  int lineNumberAreaWidth();
  QString curFile;
  std::size_t curIndent;
  bool newLine;
  std::size_t fontSize;
  void setCompleter(QCompleter *c);
  QCompleter *completer() const;

  virtual ~Editor() {}

 protected:
  void resizeEvent(QResizeEvent *event) override;
  void keyPressEvent(QKeyEvent *e) override;
  void focusInEvent(QFocusEvent *e) override;

 signals:
  void changeCursor(int new_line, int new_col);
  void changeContent(const std::string &new_cont);
 private slots:
  void updateLineNumberAreaWidth(int newBlockCount);
  void highlightCurrentLine(QList<QTextEdit::ExtraSelection> *extraSelection);
  void updateLineNumberArea(const QRect &rect, int dy);
  void insertCompletion(const QString &completion);

 private:
  Highlighter *highlighter;
  QWidget *lineNumberArea;
  QCompleter *c = nullptr;

  QString textUnderCursor() const;
  int getIndentationSpaces() const;

  QString tab_replace_ = "    ";
  QChar charUnderCursor(int offset = 0) const;
  QString wordUnderCursor() const;
  bool procCompleterStart(QKeyEvent *e);
  void procCompleterFinish(QKeyEvent *e);
  void highlightParenthesis(QList<QTextEdit::ExtraSelection> *extraSelection);
  void updateExtraSelection();

  static constexpr int DEFAULT_FONT_SIZE = 11;
};

class LineNumberArea : public QWidget {
 public:
  explicit LineNumberArea(Editor *editor) : QWidget(editor), editor(editor) {}

  QSize sizeHint() const override {
    const int HEIGHT = 0;
    return QSize(editor->lineNumberAreaWidth(), HEIGHT);
  }

 protected:
  void paintEvent(QPaintEvent *event) override {
    editor->lineNumberAreaPaintEvent(event);
  }

 private:
  Editor *editor;
};

#endif
