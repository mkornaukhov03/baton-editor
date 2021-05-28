#include "syntax_highlighter.h"

#include <QColor>
#include <QPalette>
#include <QTextDocument>
#include <cmath>
#include <iostream>
#include <string>

namespace {

// http://alienryderflex.com/hsp.html <-- here is explanation of constants
bool isLight(int r, int g, int b) {
  double brightness = std::sqrt(0.299 * r * r + 0.587 * g * g + 0.114 * b * b);
  return brightness > (255.0 / 2);
}

// Custom colors
const int boldFont = 75;
const int normalFont = 50;
const auto LIGHT_BLUE = QColor(99, 206, 255);
const auto BRIGHT_RED = QColor(255, 70, 70);     // bright red;
const auto BRIGHT_GREEN = QColor(118, 255, 61);  // bright green
const auto BRIGHT_BLUE = QColor(98, 93, 243);    // bright blue
const auto ORANGE = QColor(255, 166, 2);         // orange
const auto BRIGHT_PINK = QColor(255, 109, 246);
const auto LIGHT_PINK = QColor(244, 187, 255);

}  // namespace

Highlighter::Highlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {
  //  HighlightingRule rule;

  QColor color = QPalette().color(QPalette::Window);
  int r, g, b;
  color.getRgb(&r, &g, &b);
  bool is_light = isLight(r, g, b);

  QColor keywordColor = is_light ? Qt::darkBlue : LIGHT_BLUE;  // light blue
  QColor commentColor = is_light ? Qt::red : BRIGHT_RED;
  QColor quotationColor = is_light ? Qt::darkGreen : BRIGHT_GREEN;
  QColor functionColor = is_light ? Qt::blue : BRIGHT_BLUE;      // bright blue
  QColor returnColor = is_light ? Qt::darkMagenta : ORANGE;      // orange
  QColor condCycleColor = is_light ? Qt::magenta : BRIGHT_PINK;  // bright pink
  QColor streamColor =
      is_light ? Qt::darkGreen : LIGHT_PINK;  // very bright pink

  keywordFormat.setForeground(keywordColor);
  keywordFormat.setFontWeight(QFont::Bold);

  const QString keywordPatterns[] = {
      QStringLiteral("\\bchar\\b"),
      QStringLiteral("\\bclass\\b"),
      QStringLiteral("\\bconst\\b"),
      QStringLiteral("\\bdouble\\b"),
      QStringLiteral("\\benum\\b"),
      QStringLiteral("\\bexplicit\\b"),
      QStringLiteral("\\bfriend\\b"),
      QStringLiteral("\\binline\\b"),
      QStringLiteral("\\bint\\b"),
      QStringLiteral("\\blong\\b"),
      QStringLiteral("\\bnamespace\\b"),
      QStringLiteral("\\boperator\\b"),
      QStringLiteral("\\bprivate\\b"),
      QStringLiteral("\\bprotected\\b"),
      QStringLiteral("\\bpublic\\b"),
      QStringLiteral("\\bshort\\b"),
      QStringLiteral("\\bsignals\\b"),
      QStringLiteral("\\bsigned\\b"),
      QStringLiteral("\\bslots\\b"),
      QStringLiteral("\\bstatic\\b"),
      QStringLiteral("\\bstruct\\b"),
      QStringLiteral("\\btemplate\\b"),
      QStringLiteral("\\btypedef\\b"),
      QStringLiteral("\\btypename\\b"),
      QStringLiteral("\\bunion\\b"),
      QStringLiteral("\\bunsigned\\b"),
      QStringLiteral("\\bvirtual\\b"),
      QStringLiteral("\\bvoid\\b"),
      QStringLiteral("\\bvolatile\\b"),
      QStringLiteral("\\bbool\\b"),
      QStringLiteral("\\busing\\b"),
      QStringLiteral("\\bstring\\b"),
      QStringLiteral("\\bvector\\b"),
      QStringLiteral("\\bstd::vector\\b"),
      QStringLiteral("\\bstd::string\\b"),
      QStringLiteral("\\bunordered_map\\b"),
      QStringLiteral("\\bstd::unordered_map\\b"),
      QStringLiteral("\\bstack\\b"),
      QStringLiteral("\\bstd::stack\\b"),
      QStringLiteral("\\bmap\\b"),
      QStringLiteral("\\bstd::map\\b"),
      QStringLiteral("\\blist\\b"),
      QStringLiteral("\\bstd::list\\b"),
      QStringLiteral("\\bpriority_queue\\b"),
      QStringLiteral("\\bstd::priority_queue\\b"),
      QStringLiteral("\\bqueue\\b"),
      QStringLiteral("\\bstd::queue\\b"),
      QStringLiteral("\\bdeque\\b"),
      QStringLiteral("\\bstd::deque\\b"),
      QStringLiteral("\\bint32_t\\b"),
      QStringLiteral("\\bfunction\\b"),
      QStringLiteral("\\bstd::function\\b"),
      QStringLiteral("\\bfloat\\b"),
      QStringLiteral("\\buint32_t\\b"),
      QStringLiteral("\\bint64_t\\b"),
      QStringLiteral("\\buint64_t\\b"),
      QStringLiteral("\\bint16_t\\b"),
      QStringLiteral("\\buint16_t\\b"),
      QStringLiteral("\\bint8_t\\b"),
      QStringLiteral("\\buint8_t\\b"),
      QStringLiteral("\\bchar32_t\\b"),
      QStringLiteral("\\bchar16_t\\b"),
      QStringLiteral("\\breturn\\b"),
      QStringLiteral("\\bstd::stringstream\\b"),
      QStringLiteral("\\bstringstream\\b"),
      QStringLiteral("\\bstd::cerr\\b"),
      QStringLiteral("\\bcerr\\b"),
      QStringLiteral("\\bcout\\b"),
      QStringLiteral("\\bstd::cout\\b"),
      QStringLiteral("\\bcin\\b"),
      QStringLiteral("\\bstd::cin\\b"),
      QStringLiteral("\\bifstream\\b"),
      QStringLiteral("\\bstd::ifstream\\b"),
      QStringLiteral("\\bistream\\b"),
      QStringLiteral("\\bstd::istream\\b"),
      QStringLiteral("\\bostream\\b"),
      QStringLiteral("\\bofstream\\b"),
      QStringLiteral("\\bstd::ostream\\b"),
      QStringLiteral("\\bstd::ofstream\\b"),
      QStringLiteral("\\bprintf\\b"),
      QStringLiteral("\\bscanf\\b"),
      QStringLiteral("\\bfprintf\\b"),
      QStringLiteral("\\bfscanf\\b"),
      QStringLiteral("\\bset\\b"),
      QStringLiteral("\\bstd::set\\b"),
      QStringLiteral("\\bstd::pair\\b"),
      QStringLiteral("\\bpair\\b"),
      QStringLiteral("\\bunordered_set\\b"),
      QStringLiteral("\\bstd::unordered_set\\b"),
      QStringLiteral("\\bbitset\\b"),
      QStringLiteral("\\bstd::bitset\\b"),
      QStringLiteral("\\btrue\\b"),
      QStringLiteral("\\bfalse\\b"),
      QStringLiteral("\\bauto\\b"),
      QStringLiteral("\\bdecltype\\b"),
      QStringLiteral("\\btemplate\\b"),
      QStringLiteral("\\bsizeof\\b"),
      QStringLiteral("\\bsize\\b"),
      QStringLiteral("\\b.*::")};
  for (const QString &pattern : keywordPatterns) {
    rule.pattern = QRegularExpression(pattern);
    rule.format = keywordFormat;
    highlightingRules.append(rule);
  }
  multiLineCommentFormat.setForeground(commentColor);
  addRule(functionFormat, functionColor, boldFont,
          QRegularExpression(QStringLiteral("\\b[A-Za-z0-9_]+(?=\\()")));
  addRule(returnFormat, returnColor, normalFont,
          QRegularExpression(QStringLiteral("\\breturn\\b")));
  addRule(conditionalStatementsFormat, condCycleColor, boldFont,
          QRegularExpression(QStringLiteral("\\b(if|else)\\b")));
  addRule(conditionalCyclesFormat, condCycleColor, boldFont,
          QRegularExpression(QStringLiteral("\\b(while|for|switch|case)\\b")));
  addRule(includeFormat, streamColor, normalFont,
          QRegularExpression(QStringLiteral("#include")));
  addRule(defineFormat, streamColor, normalFont,
          QRegularExpression(QStringLiteral("#define")));
  addRule(numberFormat, functionColor, normalFont,
          QRegularExpression(QStringLiteral("\\b[0-9]*\\b")));
  addRule(triangleBracketsFormat, streamColor, normalFont,
          QRegularExpression(QStringLiteral("<.*>")));
  addRule(
      streamFormat, streamColor, boldFont,
      QRegularExpression(QStringLiteral(
          "\\b(std::stringstream|stringstream|std::cerr|std::cout|cerr|cout|"
          "cin|"
          "std::cin|ifstream|std::ifstream|istream|std::istream|ostream|std::"
          "ostream|ofstream|std::ofstream|printf|scanf|fprintf|fscanf)\\b")));
  addRule(singleLineCommentFormat, commentColor, normalFont,
          QRegularExpression(QStringLiteral("//[^\n]*")));
  addRule(quotationFormat, quotationColor, normalFont,
          QRegularExpression(QStringLiteral("\".*\"")));
  commentStartExpression = QRegularExpression(QStringLiteral("/\\*"));
  commentEndExpression = QRegularExpression(QStringLiteral("\\*/"));
}

void Highlighter::highlightBlock(const QString &text) {
  for (const HighlightingRule &rule : qAsConst(highlightingRules)) {
    QRegularExpressionMatchIterator matchIterator =
        rule.pattern.globalMatch(text);
    while (matchIterator.hasNext()) {
      QRegularExpressionMatch match = matchIterator.next();
      setFormat(match.capturedStart(), match.capturedLength(), rule.format);
    }
  }
  setCurrentBlockState(0);

  int startIndex = 0;
  if (previousBlockState() != 1)
    startIndex = text.indexOf(commentStartExpression);

  while (startIndex >= 0) {
    QRegularExpressionMatch match =
        commentEndExpression.match(text, startIndex);
    int endIndex = match.capturedStart();
    int commentLength = 0;
    if (endIndex == -1) {
      setCurrentBlockState(1);
      commentLength = text.length() - startIndex;
    } else {
      commentLength = endIndex - startIndex + match.capturedLength();
    }
    setFormat(startIndex, commentLength, multiLineCommentFormat);
    startIndex =
        text.indexOf(commentStartExpression, startIndex + commentLength);
  }
}

void Highlighter::addRule(QTextCharFormat &format, QColor foreground, int font,
                          QRegularExpression pattern) {
  format.setForeground(foreground);
  format.setFontWeight(font);
  rule.pattern = pattern;
  rule.format = format;
  highlightingRules.append(rule);
}
