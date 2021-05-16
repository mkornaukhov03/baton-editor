#ifndef TERMINAL_H
#define TERMINAL_H

#include <QWidget>
#include <QtCore>

namespace Ui {
class Terminal;
}

class Terminal : public QWidget {
  Q_OBJECT

 public:
  explicit Terminal(QWidget *parent = nullptr);
  ~Terminal();

 public slots:
  void readStandardOutput();
  void readStandardError();
  void command();

 private:
  Ui::Terminal *ui;
  QProcess *process;
};

#endif  // TERMINAL_H
