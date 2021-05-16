#include "editor.h"

#include <QAbstractItemView>
#include <QFont>
#include <QPainter>
#include <QRegularExpression>
#include <QScrollBar>
#include <QSyntaxHighlighter>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTextDocumentFragment>

#include "syntax_highlighter.h"

static QVector<QPair<QString, QString>> parentheses = {
    {"(", ")"}, {"{", "}"}, {"[", "]"}, {"\"", "\""}, {"'", "'"}};

Editor::Editor(QWidget *parent)
    : QPlainTextEdit(parent),
      highlighter(new Highlighter(this->document())),
      curIndent(0),
      newLine(true),
      m_autoIndentation(true) {
  lineNumberArea = new LineNumberArea(this);

  connect(this, &Editor::blockCountChanged, this,
          &Editor::updateLineNumberAreaWidth);
  connect(this, &Editor::updateRequest, this, &Editor::updateLineNumberArea);
  connect(this, &Editor::cursorPositionChanged, this,
          &Editor::highlightCurrentLine);

  connect(this, &Editor::cursorPositionChanged, this, [&]() {
    emit changeCursor(this->textCursor().blockNumber(),
                      this->textCursor().columnNumber());
  });
  connect(this, &Editor::textChanged, this, [&]() {
    emit changeContent(this->toPlainText().toUtf8().toStdString());
  });

  connect(this, &Editor::transferCompletion, this, &Editor::resolveCompletion);

  updateLineNumberAreaWidth(0);
  highlightCurrentLine();
  QFont font;
  font.setFamily("Courier");
  font.setFixedPitch(true);
  font.setPointSize(10);
  setFont(font);

  //  QTextDocument *document = this->document();
  //  highlighter = new Highlighter(document);
}

void Editor::resolveCompletion(const std::string &compl_item) {
  std::cerr << "SELECTION:\n"
            << this->textCursor().selection().toPlainText().toStdString()
            << std::endl;

  auto cursor = this->textCursor();
  cursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
  cursor.removeSelectedText();
  cursor.insertText(QString::fromStdString(compl_item));
  cursor.clearSelection();
  this->setTextCursor(cursor);

  //  this->textCursor().insertText(QString::fromStdString(compl_item));
  //  this->textCursor().movePosition(QTextCursor::NextWord);

  //  this->textCursor().endEditBlock();
  //  this->undo();

  //  this->textCursor().insertText("ZHOPA");

  //  auto curs = this->textCursor();
  //  curs.movePosition(QTextCursor::StartOfWord, QTextCursor::KeepAnchor);

  //  std::cerr << "AFTER RESOLVE:\n"
  //            << this->toPlainText().toStdString() << std::endl;
  //  this->setTextCursor(curs);

  //  this->textCursor().removeSelectedText();
  //  this->textCursor().clearSelection();
  //  this->setFocus();
  //  emit cursorPositionChanged();
  //  emit changeContent(this->toPlainText().toStdString());
  //  textCursor().clearSelection();
  //  setFocus();
}

int Editor::lineNumberAreaWidth() {
  int digits = 1;
  int max = qMax(1, blockCount());
  while (max >= 10) {
    max /= 10;
    ++digits;
  }

  int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

  return space;
}

void Editor::updateLineNumberAreaWidth(int) {
  setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void Editor::updateLineNumberArea(const QRect &rect, int dy) {
  if (dy)
    lineNumberArea->scroll(0, dy);
  else
    lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

  if (rect.contains(viewport()->rect())) updateLineNumberAreaWidth(0);
}

void Editor::resizeEvent(QResizeEvent *e) {
  QPlainTextEdit::resizeEvent(e);

  QRect cr = contentsRect();
  lineNumberArea->setGeometry(
      QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void Editor::highlightCurrentLine() {
  QList<QTextEdit::ExtraSelection> extraSelections;

  if (!isReadOnly()) {
    QTextEdit::ExtraSelection selection;

    QColor lineColor = QColor(173, 216, 230);
    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    extraSelections.append(selection);
  }

  setExtraSelections(extraSelections);
}

QString Editor::wordUnderCursor() const {
  auto tc = textCursor();
  tc.select(QTextCursor::WordUnderCursor);
  return tc.selectedText();
}

bool Editor::procCompleterStart(QKeyEvent *e) {
  if (completer() && completer()->popup()->isVisible()) {
    switch (e->key()) {
      case Qt::Key_Enter:
      case Qt::Key_Return:
      case Qt::Key_Escape:
      case Qt::Key_Tab:
      case Qt::Key_Backtab:
        e->ignore();
        return true;  // let the completer do default behavior
      default:
        break;
    }
  }

  // todo: Replace with modifiable QShortcut
  auto isShortcut =
      ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_Space);

  return !(!completer() || !isShortcut);
}

void Editor::procCompleterFinish(QKeyEvent *e) {
  auto ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);

  if (!completer() || (ctrlOrShift && e->text().isEmpty()) ||
      e->key() == Qt::Key_Delete) {
    return;
  }

  static QString eow(R"(~!@#$%^&*()_+{}|:"<>?,./;'[]\-=)");

  auto isShortcut =
      ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_Space);
  auto completionPrefix = wordUnderCursor();

  if (!isShortcut && (e->text().isEmpty() || completionPrefix.length() < 2 ||
                      eow.contains(e->text().right(1)))) {
    completer()->popup()->hide();
    return;
  }

  if (completionPrefix != completer()->completionPrefix()) {
    completer()->setCompletionPrefix(completionPrefix);
    completer()->popup()->setCurrentIndex(
        completer()->completionModel()->index(0, 0));
  }

  auto cursRect = cursorRect();
  cursRect.setWidth(
      completer()->popup()->sizeHintForColumn(0) +
      completer()->popup()->verticalScrollBar()->sizeHint().width());

  completer()->complete(cursRect);
}

void Editor::keyPressEvent(QKeyEvent *e) {
#if QT_VERSION >= 0x050A00
  const int defaultIndent =
      tabStopDistance() / fontMetrics().averageCharWidth();
#else
  const int defaultIndent = tabStopWidth() / fontMetrics().averageCharWidth();
#endif

  auto completerSkip = procCompleterStart(e);

  if (!completerSkip) {
    if (m_replaceTab && e->key() == Qt::Key_Tab &&
        e->modifiers() == Qt::NoModifier) {
      insertPlainText(m_tabReplace);
      return;
    }

    // Auto indentation
    int indentationLevel = getIndentationSpaces();

#if QT_VERSION >= 0x050A00
    int tabCounts =
        indentationLevel * fontMetrics().averageCharWidth() / tabStopDistance();
#else
    int tabCounts =
        indentationLevel * fontMetrics().averageCharWidth() / tabStopWidth();
#endif

    // Have Qt Edior like behaviour, if {|} and enter is pressed indent the two
    // parenthesis
    if (m_autoIndentation &&
        (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) &&
        charUnderCursor() == '}' && charUnderCursor(-1) == '{') {
      int charsBack = 0;
      insertPlainText("\n");

      if (m_replaceTab)
        insertPlainText(QString(indentationLevel + defaultIndent, ' '));
      else
        insertPlainText(QString(tabCounts + 1, '\t'));

      insertPlainText("\n");
      charsBack++;

      if (m_replaceTab) {
        insertPlainText(QString(indentationLevel, ' '));
        charsBack += indentationLevel;
      } else {
        insertPlainText(QString(tabCounts, '\t'));
        charsBack += tabCounts;
      }

      while (charsBack--) moveCursor(QTextCursor::MoveOperation::Left);
      return;
    }

    // Shortcut for moving line to left
    if (m_replaceTab && e->key() == Qt::Key_Backtab) {
      indentationLevel = std::min(indentationLevel, m_tabReplace.size());

      auto cursor = textCursor();

      cursor.movePosition(QTextCursor::MoveOperation::StartOfLine);
      cursor.movePosition(QTextCursor::MoveOperation::Right,
                          QTextCursor::MoveMode::KeepAnchor, indentationLevel);

      cursor.removeSelectedText();
      return;
    }

    QPlainTextEdit::keyPressEvent(e);

    if (m_autoIndentation &&
        (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)) {
      if (m_replaceTab)
        insertPlainText(QString(indentationLevel, ' '));
      else
        insertPlainText(QString(tabCounts, '\t'));
    }

    if (m_autoParentheses) {
      for (auto &&el : parentheses) {
        // Inserting closed brace
        if (el.first == e->text()) {
          insertPlainText(el.second);
          moveCursor(QTextCursor::MoveOperation::Left);
          break;
        }

        // If it's close brace - check parentheses
        if (el.second == e->text()) {
          auto symbol = charUnderCursor();

          if (symbol == el.second) {
            textCursor().deletePreviousChar();
            moveCursor(QTextCursor::MoveOperation::Right);
          }

          break;
        }
      }
    }
  }

  procCompleterFinish(e);
}

// void Editor::keyPressEvent(QKeyEvent *e) {
//  if (c && c->popup()->isVisible()) {
//    switch (e->key()) {
//      case Qt::Key_Enter:
//      case Qt::Key_Return:
//      case Qt::Key_Escape:
//      case Qt::Key_Tab:
//      case Qt::Key_Backtab:
//        e->ignore();
//        return;
//      default:
//        break;
//    }
//  }

//  const bool isShortcut = (e->modifiers().testFlag(Qt::ControlModifier) &&
//                           e->key() == Qt::Key_E);  // CTRL+E
//  if (!c || !isShortcut) {
//    if (newLine && e->key() == Qt::Key_Space) {
//      //                std::cerr << '\n' << "Space +1" << '\n';
//      curIndent += 1;
//    }
//    if (newLine && e->key() == Qt::Key_Tab) {
//      //      std::cerr << '\n' << "Tab +1" << '\n';
//      curIndent += 4;
//    }
//    if (e->text() == '{') {
//      //      //        blockCount++;
//      appendPlainText("}");
//      moveCursor(QTextCursor::MoveOperation::Left);
//    }
//    //    if (e->text() == '}') {
//    //        blockCount--;
//    //    }
//    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
//      //      std::cerr << '\n';
//      //      std::cerr << "indent = " << curIndent << '\n';
//      newLine = true;
//      for (std::size_t i = 0; i < curIndent; i++) {
//        insertPlainText(" ");
//      }

//    }

//    QPlainTextEdit::keyPressEvent(e);
//  }

//  const bool ctrlOrShift = e->modifiers().testFlag(Qt::ControlModifier) ||
//                           e->modifiers().testFlag(Qt::ShiftModifier);
//  if (!c || (ctrlOrShift && e->text().isEmpty())) return;

//  static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-=");  // end of word
//  const bool hasModifier = (e->modifiers() != Qt::NoModifier) &&
//  !ctrlOrShift; QString completionPrefix = textUnderCursor();

//  if (!isShortcut &&
//      (hasModifier || e->text().isEmpty() || completionPrefix.length() < 3
//      ||
//       eow.contains(e->text().right(1)))) {
//    c->popup()->hide();
//    return;
//  }

//  if (completionPrefix != c->completionPrefix()) {
//    c->setCompletionPrefix(completionPrefix);
//    c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
//  }
//  QRect cr = cursorRect();
//  cr.setWidth(c->popup()->sizeHintForColumn(0) +
//              c->popup()->verticalScrollBar()->sizeHint().width());
//  c->complete(cr);  // popup it up!
//}

void Editor::lineNumberAreaPaintEvent(QPaintEvent *event) {
  QPainter painter(lineNumberArea);
  painter.fillRect(event->rect(), Qt::lightGray);

  QTextBlock block = firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top =
      qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
  int bottom = top + qRound(blockBoundingRect(block).height());

  while (block.isValid() && top <= event->rect().bottom()) {
    if (block.isVisible() && bottom >= event->rect().top()) {
      QString number = QString::number(blockNumber + 1);
      painter.setPen(Qt::black);
      painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                       Qt::AlignRight, number);
    }

    block = block.next();
    top = bottom;
    bottom = top + qRound(blockBoundingRect(block).height());
    ++blockNumber;
  }
}

void Editor::setCompleter(QCompleter *completer) {
  if (c) c->disconnect(this);

  c = completer;

  if (!c) return;

  c->setWidget(this);
  c->setCompletionMode(QCompleter::PopupCompletion);
  c->setCaseSensitivity(Qt::CaseInsensitive);
  QObject::connect(c, QOverload<const QString &>::of(&QCompleter::activated),
                   this, &Editor::insertCompletion);
}

QCompleter *Editor::completer() const { return c; }

void Editor::insertCompletion(const QString &completion) {
  if (c->widget() != this) return;
  QTextCursor tc = textCursor();
  int extra = completion.length() - c->completionPrefix().length();
  tc.movePosition(QTextCursor::Left);
  tc.movePosition(QTextCursor::EndOfWord);
  tc.insertText(completion.right(extra));
  setTextCursor(tc);
}

QString Editor::textUnderCursor() const {
  QTextCursor tc = textCursor();
  tc.select(QTextCursor::WordUnderCursor);
  return tc.selectedText();
}

void Editor::focusInEvent(QFocusEvent *e) {
  if (c) c->setWidget(this);
  QPlainTextEdit::focusInEvent(e);
}

int Editor::getIndentationSpaces() const {
  auto blockText = textCursor().block().text();

  int indentationLevel = 0;

  for (auto i = 0;
       i < blockText.size() && QString("\t ").contains(blockText[i]); ++i) {
    if (blockText[i] == ' ') {
      indentationLevel++;
    } else {
#if QT_VERSION >= 0x050A00
      indentationLevel += tabStopDistance() / fontMetrics().averageCharWidth();
#else
      indentationLevel += tabStopWidth() / fontMetrics().averageCharWidth();
#endif
    }
  }

  return indentationLevel;
}

QChar Editor::charUnderCursor(int offset) const {
  auto block = textCursor().blockNumber();
  auto index = textCursor().positionInBlock();
  auto text = document()->findBlockByNumber(block).text();

  index += offset;

  if (index < 0 || index >= text.size()) {
    return {};
  }

  return text[index];
}
