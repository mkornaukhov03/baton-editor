#include "editor.h"

#include <QAbstractItemView>
#include <QFont>
#include <QPainter>
#include <QRegularExpression>
#include <QScrollBar>
#include <QStringListModel>
#include <QSyntaxHighlighter>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTextDocumentFragment>

#include "syntax_highlighter.h"

static QVector<QPair<QString, QString>> parentheses = {
    {"(", ")"}, {"{", "}"}, {"[", "]"}, {"\"", "\""}, {"'", "'"}};

Editor::Editor(std::size_t fontSize, QWidget *parent)
    : QPlainTextEdit(parent),
      highlighter(new Highlighter(this->document())),
      curIndent(0),
      newLine(true),
      m_autoIndentation(true),
      fontSize(fontSize) {
  lineNumberArea = new LineNumberArea(this);

  connect(this, &Editor::blockCountChanged, this,
          &Editor::updateLineNumberAreaWidth);
  connect(this, &Editor::updateRequest, this, &Editor::updateLineNumberArea);

  connect(this, &Editor::cursorPositionChanged, this, [&]() {
    emit changeCursor(this->textCursor().blockNumber(),
                      this->textCursor().columnNumber());
  });
  connect(this, &Editor::textChanged, this, [&]() {
    emit changeContent(this->toPlainText().toUtf8().toStdString());

    connect(this, &QPlainTextEdit::cursorPositionChanged, this,
            &Editor::updateExtraSelection);
    updateExtraSelection();
  });

  connect(this, &Editor::transferCompletion, this, &Editor::resolveCompletion);

  updateLineNumberAreaWidth(0);
  QFont font;
  font.setFamily("Courier");
  font.setFixedPitch(true);
  font.setPointSize(fontSize);
  setFont(font);
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
}

int Editor::lineNumberAreaWidth() {
  int digits = 1;
  int max = qMax(1, blockCount());
  while (max >= 10) {
    max /= 10;
    ++digits;
  }
  std::cerr << "FONT METRIX: "
            << fontMetrics().horizontalAdvance(QLatin1Char('9')) << '\n';

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

void Editor::highlightCurrentLine(
    QList<QTextEdit::ExtraSelection> *extraSelection) {
  if (!isReadOnly()) {
    QTextEdit::ExtraSelection selection;

    QColor lineColor = QColor(96, 100, 36, 50);
    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    extraSelection->append(selection);
  }
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
  std::cerr << "Completter triggered!!!!!!!!!!!!!!" << std::endl;
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

void Editor::highlightParenthesis(
    QList<QTextEdit::ExtraSelection> *extraSelection) {
  auto currentSymbol = charUnderCursor();
  auto prevSymbol = charUnderCursor(-1);

  for (auto &pair : parentheses) {
    int direction;

    QChar counterSymbol;
    QChar activeSymbol;
    auto position = textCursor().position();

    if (pair.first == currentSymbol) {
      direction = 1;
      counterSymbol = pair.second[0];
      activeSymbol = currentSymbol;
    } else if (pair.second == prevSymbol) {
      direction = -1;
      counterSymbol = pair.first[0];
      activeSymbol = prevSymbol;
      position--;
    } else {
      continue;
    }

    auto counter = 1;

    while (counter != 0 && position > 0 &&
           position < (document()->characterCount() - 1)) {
      // Moving position
      position += direction;

      auto character = document()->characterAt(position);
      // Checking symbol under position
      if (character == activeSymbol) {
        ++counter;
      } else if (character == counterSymbol) {
        --counter;
      }
    }

    QTextCharFormat format;
    format.setForeground(QColor("#ff0000"));
    format.setBackground(QColor("#b4eeb4"));

    // Found
    if (counter == 0) {
      QTextEdit::ExtraSelection selection{};

      auto directionEnum = direction < 0 ? QTextCursor::MoveOperation::Left
                                         : QTextCursor::MoveOperation::Right;

      selection.format = format;
      selection.cursor = textCursor();
      selection.cursor.clearSelection();
      selection.cursor.movePosition(
          directionEnum, QTextCursor::MoveMode::MoveAnchor,
          std::abs(textCursor().position() - position));

      selection.cursor.movePosition(QTextCursor::MoveOperation::Right,
                                    QTextCursor::MoveMode::KeepAnchor, 1);

      extraSelection->append(selection);

      selection.cursor = textCursor();
      selection.cursor.clearSelection();
      selection.cursor.movePosition(directionEnum,
                                    QTextCursor::MoveMode::KeepAnchor, 1);

      extraSelection->append(selection);
    }

    break;
  }
}

void Editor::lineNumberAreaPaintEvent(QPaintEvent *event) {
  QPainter painter(lineNumberArea);
  painter.fillRect(event->rect(), Qt::lightGray);

  QFont fnt = painter.font();
  fnt.setPointSize(fontSize);
  painter.setFont(fnt);
  setFont(fnt);
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

void Editor::updateExtraSelection() {
  QList<QTextEdit::ExtraSelection> extra;
  highlightParenthesis(&extra);
  highlightCurrentLine(&extra);
  setExtraSelections(extra);
}
