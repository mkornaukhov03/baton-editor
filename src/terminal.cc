#include "terminal.h"

#include <QLineEdit>

#include "ui_terminal.h"

Terminal::Terminal(QWidget *parent) : QWidget(parent), ui(new Ui::Terminal) {
  ui->setupUi(this);
  QPalette p = palette();
  p.setColor(QPalette::Base, Qt::black);
  p.setColor(QPalette::Text, Qt::green);
  setPalette(p);
  process = new QProcess;
  process->start("bash", {}, QIODevice::ReadWrite);
  process->waitForStarted();
  connect(process, &QProcess::readyReadStandardOutput, this,
          &Terminal::readStandardOutput);
  connect(process, &QProcess::readyReadStandardError, this,
          &Terminal::readStandardError);
  connect(ui->lineEdit, &QLineEdit::returnPressed, this, &Terminal::command);
}

Terminal::~Terminal() { delete ui; }

void Terminal::readStandardOutput() {
  ui->textBrowser->append(process->readAllStandardOutput());
  ui->textBrowser->append(process->readAllStandardError());
}

void Terminal::readStandardError() {
  QByteArray cmdout = process->readAllStandardError();
  if (!cmdout.isEmpty()) {
    ui->textBrowser->setTextColor(Qt::red);
    ui->textBrowser->append(QString::fromLocal8Bit(cmdout));
    ui->textBrowser->setTextColor(Qt::green);
  }
}

void Terminal::command() {
  process->write(ui->lineEdit->text().toLocal8Bit() + '\n');
  ui->lineEdit->clear();
}
