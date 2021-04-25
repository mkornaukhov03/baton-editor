#include "terminal.h"

#include "ui_terminal.h"

Terminal::Terminal(QWidget *parent)
    : QWidget(parent), ui(new Ui::Terminal), process(new QProcess) {
  ui->setupUi(this);
  connect(process, SIGNAL(readyReadStandardError()), this, SLOT(setStdout()));
  connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(setStdout()));
  connect(ui->lineEdit, SIGNAL(returnPressed()), this, SLOT(command()));
}

Terminal::~Terminal() { delete ui; }

void Terminal::setStdout() {
  if (QSysInfo::productType() == "windows") {
    QTextCodec *codec = QTextCodec::codecForName("IBM 866");
    ui->textBrowser->append(codec->toUnicode(process->readAllStandardOutput()));
  } else {
    ui->textBrowser->append(process->readAllStandardOutput());
    //    ui->textBrowser->append(process->readAllStandardError());
  }
}

void Terminal::command() {
  QString strCommand;
  if (QSysInfo::productType() == "windows") {
    strCommand = "cmd /C";
  }
  strCommand += ui->lineEdit->text();
  ui->lineEdit->setText("");
  process->start(strCommand);
}
