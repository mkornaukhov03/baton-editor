#ifndef Editor_H
#define Editor_H

#include <QMap>
#include <QPlainTextEdit>
#include <QPointer>
#include <QToolBar>

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

  virtual ~Editor(){};

 protected:
  void resizeEvent(QResizeEvent *event) override;

 private slots:
  void updateLineNumberAreaWidth(int newBlockCount);
  void highlightCurrentLine();
  void updateLineNumberArea(const QRect &rect, int dy);

 private:
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
