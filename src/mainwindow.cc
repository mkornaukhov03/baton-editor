#include "mainwindow.h"

#include <interface.h>

#include <QComboBox>
#include <QDir>
#include <QLayout>
#include <QtWidgets>
#include <iostream>  // for debugging/logging
#include <utility>

#include "directory_tree.h"
#include "editor.h"
#include "syntax_highlighter.h"
#include "terminal.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      textEdit(new Editor),
      splittedTextEdit(new Editor),
      splitted(false) {
  ui->setupUi(this);
  // Directory_tree *directory_tree = new Directory_tree(this);
  //  Terminal *terminal = new Terminal;
  lbl = new Suggest_label(nullptr);
  createActions();

  connect(textEdit->document(), &QTextDocument::contentsChanged, this,
          &MainWindow::documentWasModified);

  setCurrentFile(QString());

  createStatusBar();
  central_widget = new QWidget();
  grid_layout = new QGridLayout(central_widget);
  //  grid_layout->addWidget(lbl, 1, 1, 1, 1);
  grid_layout->addWidget(&directory_tree, 0, 0, 1, 1);
  grid_layout->addWidget(textEdit, 0, 3);
  grid_layout->setColumnStretch(0, 1);
  grid_layout->setColumnStretch(3, 5);
  //  grid_layout->addWidget(terminal, 1, 0, 2, 3);
  //  grid_layout->setRowStretch(0, 4);
  //  grid_layout->setRowStretch(1, 1);
  central_widget->setLayout(grid_layout);
  setCentralWidget(central_widget);
  connect(textEdit, SIGNAL(cursorPositionChanged()), this,
          SLOT(showCursorPosition()));
  connect(splittedTextEdit, SIGNAL(cursorPositionChanged()), this,
          SLOT(showCursorPositionOnSplitted()));
  connect(&directory_tree.tree, SIGNAL(clicked(QModelIndex)), this,
          SLOT(tree_clicked(const QModelIndex &)));
  lsp_handler =
      new lsp::LSPHandler(QDir::currentPath().toStdString(), "kek.cpp", "");
  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(update_autocomplete()));

  const int TIMER_PERIOD = 300;

  timer->start(TIMER_PERIOD);
  connect(lsp_handler, SIGNAL(DoneCompletion(const std::vector<std::string> &)),
          this,
          SLOT(set_autocomplete_to_label(const std::vector<std::string> &)));
  connect(
      lsp_handler,
      SIGNAL(DoneDiagnostic(const std::vector<lsp::DiagnosticsResponse> &)),
      this,
      SLOT(display_diagnostics(const std::vector<lsp::DiagnosticsResponse> &)));
}

void MainWindow::closeEvent(QCloseEvent *event) {
  if (maybeSave()) {
    event->accept();
  } else {
    event->ignore();
  }
}

void MainWindow::newFile() {
  if (maybeSave()) {
    textEdit->clear();
    setCurrentFile(QString());
  }
}

void MainWindow::open() {
  if (maybeSave()) {
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) loadFile(fileName);
  }
}

bool MainWindow::save() {
  if (curFile.isEmpty()) {
    return saveAs();
  } else {
    return saveFile(curFile);
  }
}

void MainWindow::split() {
  if (!splitted) {
    splitted = true;
    //    splittedTextEdit = new Editor;
    grid_layout->setColumnStretch(3, 2);
    grid_layout->addWidget(splittedTextEdit, 0, 3);
    grid_layout->addWidget(textEdit, 0, 5);
    grid_layout->setColumnStretch(3, 2);
    grid_layout->setColumnStretch(5, 2);
  } else {
    splitted = false;
    //    grid_layout->removeWidget(grid_layout->itemAt(1)->widget());
    for (int i = 0; i < 2; i++) {
      auto it1 = grid_layout->itemAt(1);
      if (it1) {
        delete it1->widget();
        //        delete it1;
      }
    }
    //    grid_layout->addWidget(textEdit, 0, 3);
    //    grid_layout->setColumnStretch(3, 5);
  }
}

void MainWindow::textSize(const QString &p) {
  qreal pointSize = p.toFloat();
  if (p.toFloat() > 0) {
    QTextCharFormat fmt;
    fmt.setFontPointSize(pointSize);
    mergeFormatOnWordOrSelection(fmt);
  }
}

void MainWindow::mergeFormatOnWordOrSelection(const QTextCharFormat &format) {
  QTextCursor cursor = textEdit->textCursor();
  if (!cursor.hasSelection()) cursor.select(QTextCursor::WordUnderCursor);
  cursor.mergeCharFormat(format);
  textEdit->mergeCurrentCharFormat(format);
}

bool MainWindow::saveAs() {
  QFileDialog dialog(this);
  dialog.setWindowModality(Qt::WindowModal);
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  if (dialog.exec() != QDialog::Accepted) return false;
  return saveFile(dialog.selectedFiles().first());
}

void MainWindow::documentWasModified() {
  setWindowTitle(tr("MainWindow[*]"));
  setWindowModified(textEdit->document()->isModified());
}

void MainWindow::createActions() {
  QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
  QAction *newAct = new QAction(tr("&New"), this);
  newAct->setShortcuts(QKeySequence::New);
  newAct->setStatusTip(tr("Create a new file"));
  connect(newAct, &QAction::triggered, this, &MainWindow::newFile);
  fileMenu->addAction(newAct);

  QAction *openAct = new QAction(tr("&Open..."), this);
  openAct->setShortcuts(QKeySequence::Open);
  openAct->setStatusTip(tr("Open an existing file"));
  connect(openAct, &QAction::triggered, this, &MainWindow::open);
  fileMenu->addAction(openAct);

  QAction *saveAsAct =
      fileMenu->addAction(tr("Save &As..."), this, &MainWindow::saveAs);
  saveAsAct->setShortcuts(QKeySequence::SaveAs);
  saveAsAct->setStatusTip(tr("Save the document under a new name"));

  tb = addToolBar(tr("Format Actions"));
  tb->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
  addToolBarBreak(Qt::TopToolBarArea);
  addToolBar(tb);

  comboSize = new QComboBox(tb);
  comboSize->setObjectName("comboSize");
  tb->addWidget(comboSize);

  comboSize->setEditable(true);

  const QList<int> standardSizes = QFontDatabase::standardSizes();
  for (int size : standardSizes) comboSize->addItem(QString::number(size));
  comboSize->setCurrentIndex(
      standardSizes.indexOf(QApplication::font().pointSize()));

  connect(comboSize, &QComboBox::textActivated, this, &MainWindow::textSize);

  const QIcon splitIcon =
      QIcon::fromTheme("edit-copy", QIcon(":/images/copy.png"));
  QAction *splitAct = new QAction(splitIcon, tr("Split"), this);
  splitAct->setStatusTip("Split right");
  connect(splitAct, &QAction::triggered, this, &MainWindow::split);
  tb->addAction(splitAct);
  tb->addWidget(lbl);
}

MainWindow::~MainWindow() {
  delete ui;
  delete lsp_handler;
}

bool MainWindow::maybeSave() {
  if (!textEdit->document()->isModified()) return true;
  const QMessageBox::StandardButton ret = QMessageBox::warning(
      this, tr("Application"),
      tr("В документе есть не сохранённые изменения.\n"
         "Сохранить?"),
      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
  switch (ret) {
    case QMessageBox::Save:
      return save();
    case QMessageBox::Cancel:
      return false;
    default:
      break;
  }
  return true;
}

void MainWindow::loadFile(const QString &fileName) {
  QFile file(fileName);
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    QMessageBox::warning(
        this, tr("Application"),
        tr("Cannot read file %1:\n%2.")
            .arg(QDir::toNativeSeparators(fileName), file.errorString()));
    return;
  }
  QTextStream in(&file);
#ifndef QT_NO_CURSOR
  QGuiApplication::setOverrideCursor(Qt::WaitCursor);
#endif
  textEdit->setPlainText(in.readAll());
#ifndef QT_NO_CURSOR
  QGuiApplication::restoreOverrideCursor();
#endif

  setCurrentFile(fileName);
  statusBar()->showMessage(tr("File loaded"), 2000);
}

void MainWindow::tree_clicked(const QModelIndex &index) {
  QFileInfo file_info = directory_tree.model.fileInfo(index);
  if (file_info.isFile()) {
    MainWindow::loadFile(file_info.filePath());
    return;
  }
}
void MainWindow::createStatusBar() { statusBar()->showMessage(tr("Ready")); }

bool MainWindow::saveFile(const QString &fileName) {
  QString errorMessage;

  QGuiApplication::setOverrideCursor(Qt::WaitCursor);
  QSaveFile file(fileName);
  if (file.open(QFile::WriteOnly | QFile::Text)) {
    QTextStream out(&file);
    out << textEdit->toPlainText();
    if (!file.commit()) {
      errorMessage =
          tr("Cannot write file %1:\n%2.")
              .arg(QDir::toNativeSeparators(fileName), file.errorString());
    }
  } else {
    errorMessage =
        tr("Cannot open file %1 for writing:\n%2.")
            .arg(QDir::toNativeSeparators(fileName), file.errorString());
  }
  QGuiApplication::restoreOverrideCursor();

  if (!errorMessage.isEmpty()) {
    QMessageBox::warning(this, tr("Application"), errorMessage);
    return false;
  }

  setCurrentFile(fileName);
  return true;
}

void MainWindow::setCurrentFile(const QString &fileName) {
  curFile = fileName;
  setWindowTitle(tr("MainWindow[*]"));
  textEdit->document()->setModified(false);
  setWindowModified(false);

  QString shownName = curFile;
  if (curFile.isEmpty()) shownName = "untitled.txt";
  setWindowFilePath(shownName);
}

void MainWindow::currentCharFormatChanged(const QTextCharFormat &format) {
  fontChanged(format.font());
}

void MainWindow::fontChanged(const QFont &f) {
  comboSize->setCurrentIndex(
      comboSize->findText(QString::number(f.pointSize())));
}

QString MainWindow::strippedName(const QString &fullFileName) {
  return QFileInfo(fullFileName).fileName();
}

void MainWindow::showCursorPosition() {
  int line = textEdit->textCursor().blockNumber() + 1;
  int column = textEdit->textCursor().columnNumber() + 1;
  statusBar()->showMessage(QString("Line %1  Column %2").arg(line).arg(column));
}

void MainWindow::showCursorPositionOnSplitted() {
  int line = splittedTextEdit->textCursor().blockNumber() + 1;
  int column = splittedTextEdit->textCursor().columnNumber() + 1;
  statusBar()->showMessage(QString("Line %1  Column %2").arg(line).arg(column));
}

void MainWindow::update_autocomplete() {
  static int cur_line = 0;
  static int cur_col = 0;

  static std::string content = "";
  textEdit->setReadOnly(true);

  if (content != textEdit->toPlainText().toUtf8().toStdString()) {
    content = textEdit->toPlainText().toUtf8().toStdString();
    lsp_handler->FileChanged(content);
  }
  if (cur_line != textEdit->textCursor().blockNumber() ||
      cur_col != textEdit->textCursor().columnNumber()) {
    cur_line = textEdit->textCursor().blockNumber();
    cur_col = textEdit->textCursor().columnNumber();
    lsp_handler->RequestCompletion(cur_line, cur_col);
  }

  textEdit->setReadOnly(false);
}

void MainWindow::set_autocomplete_to_label(
    const std::vector<std::string> &vec) {
  // only first
  std::cerr << "***** INSIDE SET AUTO COMPLETE TO LABEL ***** " << std::endl;
  if (vec.size() == 0) return;
  for (const auto &item : vec) {
    std::cerr << item << '\n';
  }
  lbl->setText(QString::fromStdString(vec[0]));
}

void MainWindow::display_diagnostics(
    const std::vector<lsp::DiagnosticsResponse> &resp) {
  std::cerr << "New diagnostics:\n";
  for (auto &[ctgry, msg, _] : resp) {
    std::cerr << "category: " << ctgry << '\n';
    std::cerr << "message: " << msg << std::endl;
  }
  std::cerr << "-----" << std::endl;
}
