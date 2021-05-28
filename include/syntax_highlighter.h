#ifndef SYNTAX_HIGHLIGHTER_H
#define SYNTAX_HIGHLIGHTER_H

#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

class Highlighter : public QSyntaxHighlighter {
  Q_OBJECT

 public:
  explicit Highlighter(QTextDocument *parent = nullptr);
  virtual ~Highlighter() {}

 protected:
  void highlightBlock(const QString &text) override;

 private:
  struct HighlightingRule {
    QRegularExpression pattern;
    QTextCharFormat format;
  };
  QVector<HighlightingRule> highlightingRules;
  HighlightingRule rule;
  QRegularExpression commentStartExpression;
  QRegularExpression commentEndExpression;

  QTextCharFormat keywordFormat;
  QTextCharFormat singleLineCommentFormat;
  QTextCharFormat multiLineCommentFormat;
  QTextCharFormat quotationFormat;
  QTextCharFormat functionFormat;
  QTextCharFormat returnFormat;
  QTextCharFormat conditionalStatementsFormat;
  QTextCharFormat conditionalCyclesFormat;
  QTextCharFormat streamFormat;
  QTextCharFormat includeFormat;
  QTextCharFormat triangleBracketsFormat;
  QTextCharFormat numberFormat;
  QTextCharFormat defineFormat;
  void addRule(QTextCharFormat *format, QColor foreground, int font,
               QRegularExpression pattern);
};

#endif  // SYNTAX_HIGHLIGHTER_H
