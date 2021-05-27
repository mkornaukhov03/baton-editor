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
namespace {
static const auto &parentheses() {
  static QVector<QPair<QString, QString>> p = {
      {"(", ")"}, {"{", "}"}, {"[", "]"}, {"\"", "\""}, {"'", "'"}};
  return p;
}

}  // namespace

Editor::Editor(std::size_t fontSize, QWidget *parent)
    : QPlainTextEdit(parent),

      curIndent(0),

      newLine(true),

      fontSize(fontSize),
      highlighter(new Highlighter(this->document())) {
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
  });

  updateExtraSelection();
  connect(this, &Editor::transferCompletion, this, &Editor::resolveCompletion);
  connect(this, &QPlainTextEdit::cursorPositionChanged, this,
          &Editor::updateExtraSelection);
  updateLineNumberAreaWidth(0);

  QFont font;
  font.setFamily("Courier");
  font.setFixedPitch(true);
  font.setPointSize(fontSize);
  setFont(font);
}

void Editor::resolveCompletion(const std::string &compl_item) {
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

  const int INITIAL_WIDTH = 3;
  int space = INITIAL_WIDTH +
              fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

  return space;
}

void Editor::updateLineNumberAreaWidth(int) {
  const int LEFT = 0;
  const int RIGHT = 0;
  const int BOTTOM = 0;
  setViewportMargins(lineNumberAreaWidth(), LEFT, RIGHT, BOTTOM);
}

void Editor::updateLineNumberArea(const QRect &rect, int dy) {
  const int DX = 0;
  if (dy)
    lineNumberArea->scroll(DX, dy);
  else
    lineNumberArea->update(DX, rect.y(), lineNumberArea->width(),
                           rect.height());

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

    QColor lineColor = QColor(96, 100, 36, 50);  // transparent yellow
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

  const int ROW = 0;
  const int COL = 0;

  if (completionPrefix != completer()->completionPrefix()) {
    completer()->setCompletionPrefix(completionPrefix);
    completer()->popup()->setCurrentIndex(
        completer()->completionModel()->index(ROW, COL));
  }

  auto cursRect = cursorRect();
  cursRect.setWidth(
      completer()->popup()->sizeHintForColumn(0) +
      completer()->popup()->verticalScrollBar()->sizeHint().width());
  completer()->complete(cursRect);
}

void Editor::keyPressEvent(QKeyEvent *e) {
  const int defaultIndent =
      tabStopDistance() / fontMetrics().averageCharWidth();

  auto completerSkip = procCompleterStart(e);

  if (!completerSkip) {
    if (e->key() == Qt::Key_Tab && e->modifiers() == Qt::NoModifier) {
      insertPlainText(tab_replace_);
      return;
    }

    // Auto indentation
    int indentationLevel = getIndentationSpaces();

    // Have Qt Edior like behaviour, if {|} and enter is pressed indent the two
    // parenthesis
    if ((e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) &&
        charUnderCursor() == '}' && charUnderCursor(-1) == '{') {
      int charsBack = 0;
      insertPlainText("\n");

      insertPlainText(QString(indentationLevel + defaultIndent, ' '));

      insertPlainText("\n");
      charsBack++;

      insertPlainText(QString(indentationLevel, ' '));
      charsBack += indentationLevel;

      while (charsBack--) moveCursor(QTextCursor::MoveOperation::Left);
      return;
    }

    // Shortcut for moving line to left
    if (e->key() == Qt::Key_Backtab) {
      indentationLevel = std::min(indentationLevel, tab_replace_.size());

      auto cursor = textCursor();

      cursor.movePosition(QTextCursor::MoveOperation::StartOfLine);
      cursor.movePosition(QTextCursor::MoveOperation::Right,
                          QTextCursor::MoveMode::KeepAnchor, indentationLevel);

      cursor.removeSelectedText();
      return;
    }

    QPlainTextEdit::keyPressEvent(e);

    if ((e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)) {
      insertPlainText(QString(indentationLevel, ' '));
    }

    for (auto &&el : parentheses()) {
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

  procCompleterFinish(e);
}

void Editor::highlightParenthesis(
    QList<QTextEdit::ExtraSelection> *extraSelection) {
  auto currentSymbol = charUnderCursor();
  auto prevSymbol = charUnderCursor(-1);

  for (const auto &pair : parentheses()) {
    int direction;

    QChar counterSymbol;
    QChar activeSymbol;
    auto position = textCursor().position();

    constexpr int RIGHT = 1;
    constexpr int LEFT = -1;
    if (pair.first == currentSymbol) {
      direction = RIGHT;
      counterSymbol = pair.second[0];
      activeSymbol = currentSymbol;
    } else if (pair.second == prevSymbol) {
      direction = LEFT;
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
    format.setForeground(Qt::red);

    auto darkSeaGreen = QColor(180, 238, 180);
    format.setBackground(darkSeaGreen);

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

      constexpr int MOVE_TIMES = 1;

      selection.cursor.movePosition(QTextCursor::MoveOperation::Right,
                                    QTextCursor::MoveMode::KeepAnchor,
                                    MOVE_TIMES);

      extraSelection->append(selection);

      selection.cursor = textCursor();
      selection.cursor.clearSelection();
      selection.cursor.movePosition(
          directionEnum, QTextCursor::MoveMode::KeepAnchor, MOVE_TIMES);

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
      indentationLevel += tabStopDistance() / fontMetrics().averageCharWidth();
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
