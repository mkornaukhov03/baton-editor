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
  virtual ~Terminal();

 public slots:
  void setStdout();
  void command();

 private:
  Ui::Terminal *ui;
  QProcess *process;
};

#endif  // TERMINAL_H
