#ifndef Editor_H
#define Editor_H
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
  explicit Editor(QWidget *parent = nullptr);

  void lineNumberAreaPaintEvent(QPaintEvent *event);
  int lineNumberAreaWidth();
  QString curFile;
  int curIndent;
  bool newLine;

  virtual ~Editor() {}

 protected:
  void resizeEvent(QResizeEvent *event) override;
  void keyPressEvent(QKeyEvent *e) override {
    QPlainTextEdit::keyPressEvent(e);
    if (newLine && e->key() == Qt::Key_Space) {
      //      std::cerr << '\n' << "Space +1" << '\n';
      curIndent += 1;
    }
    if (newLine && e->key() == Qt::Key_Tab) {
      //      std::cerr << '\n' << "Tab +1" << '\n';
      curIndent += 4;
    }
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
      //      std::cerr << '\n';
      //      std::cerr << "indent = " << curIndent << '\n';
      newLine = true;
      for (int i = 0; i < curIndent; i++) {
        insertPlainText(" ");
      }
    } else if (e->key() != Qt::Key_Space && e->key() != Qt::Key_Tab &&
               e->key() != Qt::Key_Enter && e->key() != Qt::Key_Return) {
      newLine = false;
    }
  }

 signals:
  void changeCursor(int new_line, int new_col);
  void changeContent(const std::string &new_cont);
  void transferCompletion(const std::string &compl_item);
 private slots:
  void updateLineNumberAreaWidth(int newBlockCount);
  void highlightCurrentLine();
  void updateLineNumberArea(const QRect &rect, int dy);
  void resolveCompletion(const std::string &compl_item);

 private:
  Highlighter *highlighter;
  QWidget *lineNumberArea;
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
