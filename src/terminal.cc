#include "terminal.h"

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
  connect(process, SIGNAL(readyReadStandardOutput()), this,
          SLOT(readStandardOutput()));
  connect(process, SIGNAL(readyReadStandardError()), this,
          SLOT(readStandardError()));
  connect(ui->lineEdit, SIGNAL(returnPressed()), this, SLOT(command()));
}

Terminal::~Terminal() { delete ui; }

void Terminal::readStandardOutput() {
  if (QSysInfo::productType() == "windows") {
    QTextCodec *codec = QTextCodec::codecForName("IBM 866");
    ui->textBrowser->append(codec->toUnicode(process->readAllStandardOutput()));
  } else {
    ui->textBrowser->append(process->readAllStandardOutput());
    ui->textBrowser->append(process->readAllStandardError());
  }
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
  //  QString strCommand;
  //  if (QSysInfo::productType() == "windows") {
  //    strCommand = "cmd /C";
  //  }
  //  strCommand += ui->lineEdit->text();
  //  ui->textBrowser->append("Linux:~$ ");
  process->write(ui->lineEdit->text().toLocal8Bit() + '\n');
  ui->lineEdit->clear();
}
