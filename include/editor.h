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
// class SyntaxStyle;
// class StyleSyntaxHighlighter;

class Editor : public QPlainTextEdit {
  Q_OBJECT

 public:
  explicit Editor(std::size_t fontSize = 11, QWidget *parent = nullptr);

  void lineNumberAreaPaintEvent(QPaintEvent *event);
  int lineNumberAreaWidth();
  QString curFile;
  std::size_t curIndent;
  bool newLine;
  std::size_t fontSize;
  // std::size_t blockCount = 0;
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
  void transferCompletion(const std::string &compl_item);
 private slots:
  void updateLineNumberAreaWidth(int newBlockCount);
  void highlightCurrentLine();
  void updateLineNumberArea(const QRect &rect, int dy);
  void resolveCompletion(const std::string &compl_item);
  void insertCompletion(const QString &completion);

 private:
  Highlighter *highlighter;
  QWidget *lineNumberArea;
  QCompleter *c = nullptr;
  QString textUnderCursor() const;
  int getIndentationSpaces() const;
  bool m_autoIndentation = true;
  bool m_autoParentheses = true;
  bool m_replaceTab = true;

  //  StyleSyntaxHighlighter *m_highlighter;
  //  SyntaxStyle *m_syntaxStyle;
  QString m_tabReplace = "    ";
  QChar charUnderCursor(int offset = 0) const;
  QString wordUnderCursor() const;
  bool procCompleterStart(QKeyEvent *e);
  void procCompleterFinish(QKeyEvent *e);
  void highlightParenthesis(QList<QTextEdit::ExtraSelection> *extraSelection);
  void updateExtraSelection();
  //  void setSyntaxStyle(SyntaxStyle *style);
  //  void updateStyle();
};

class LineNumberArea : public QWidget {
 public:
  explicit LineNumberArea(Editor *editor) : QWidget(editor), editor(editor) {}

  QSize sizeHint() const override {
    return QSize(editor->lineNumberAreaWidth(), 0);
  }

 protected:
  void paintEvent(QPaintEvent *event) override {
    editor->lineNumberAreaPaintEvent(event);
  }

 private:
  Editor *editor;
};

#endif
