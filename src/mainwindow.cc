#include "mainwindow.h"

#include <handler.h>
#include <stdio.h>
#include <stdlib.h>

#include <QComboBox>
#include <QDir>
#include <QLayout>
#include <QMenuBar>
#include <QSplitter>
#include <QString>
#include <QtWidgets>
#include <iostream>  // for debugging/logging
#include <list>
#include <utility>

#include "directory_tree.h"
#include "editor.h"
#include "syntax_highlighter.h"
#include "terminal.h"
namespace {
const int tabStop = 4;
struct WidgetPlacer {
  int row, col, row_span, col_span;
};
}  // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      textEdit(new Editor),
      terminal(new Terminal),
      splitted(false),
      display_failure_log(new QPlainTextEdit),
      font(new QFont) {
  ui->setupUi(this);
  font->setFamily("Courier");
  font->setStyleHint(QFont::Monospace);
  font->setFixedPitch(true);
  font->setPointSize(11);

  textEdit->setFont(*font);
  metrics = new QFontMetrics(*font);
  textEdit->setTabStopDistance(tabStop * metrics->horizontalAdvance(' '));
  disp = new autocompleteDisplay(nullptr);
  fv = new FileView("kek.cpp", this);

  createStatusBar();
  createActions();

  connect(textEdit->document(), &QTextDocument::contentsChanged, this,
          &MainWindow::documentWasModified);

  setCurrentFile(QString(), textEdit);

  central_widget = new QWidget();
  grid_layout = new QGridLayout(central_widget);


  WidgetPlacer dir_tr = {0, 0, 1, 2};
  WidgetPlacer disp = {1, 6, 3, 7};
  WidgetPlacer term = {1, 0, 3, 6};
  grid_layout->addWidget(&directory_tree.tree, dir_tr.row, dir_tr.col,
                         dir_tr.row_span, dir_tr.col_span);
  grid_layout->addWidget(display_failure_log, disp.row, disp.col, disp.row_span,
                         disp.col_span);
  grid_layout->addWidget(terminal, term.row, term.col, term.row_span,
                         term.col_span);

  std::vector<int> stretch_for_col = {4, 1};
  for (std::size_t i = 0; i < stretch_for_col.size(); ++i)
    grid_layout->setRowStretch(i, stretch_for_col[i]);

  splitter = new QSplitter(centralWidget());
  splitter->addWidget(textEdit);
  stretch_for_col = {0, 10};
  for (std::size_t i = 0; i < stretch_for_col.size(); ++i)
    splitter->setStretchFactor(i, stretch_for_col[i]);

  WidgetPlacer splt = {0, 2, 1, 11};
  grid_layout->addWidget(splitter, splt.row, splt.col, splt.row_span,
                         splt.col_span);
  central_widget->setLayout(grid_layout);
  setCentralWidget(central_widget);

  connect(textEdit, &Editor::cursorPositionChanged, this,
          &MainWindow::showCursorPosition);
  connect(&directory_tree.tree, &QTreeView::clicked, this,
          &MainWindow::tree_clicked);

  display_failure_log->setReadOnly(true);
  QStringList stringList;
  stringList << "m0"
             << "m1"
             << "m2";
  QStringListModel *model = new QStringListModel(stringList);
  completer = new QCompleter(model, this);
  completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
  completer->setCaseSensitivity(Qt::CaseInsensitive);
  completer->setWrapAround(false);
  textEdit->setCompleter(completer);

  connect(textEdit, &Editor::changeContent, fv, &FileView::UploadContent);

  connect(textEdit, &Editor::changeCursor, fv, &FileView::ChangeCursor);


  connect(fv, &FileView::DoneDiagnostic, this, &MainWindow::display_failure);

  connect(fv, &FileView::DoneCompletion, this,
          &MainWindow::displayAutocompleteOptions);

  textEdit->setFocus();
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
    setCurrentFile(QString(), textEdit);
  }
}

void MainWindow::open() {
  if (maybeSave()) {
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) loadFile(fileName);
  }
}

void MainWindow::choose_directory() {
  directory_tree.dir_name = QFileDialog::getExistingDirectory(this);
  directory_tree.set_root_path();
}

bool MainWindow::save() {
  QString filename =
      (textEdit->curFile.isEmpty()) ? "untitled.cpp" : textEdit->curFile;
  QString splittedFilename = (splittedTextEdit->curFile.isEmpty())
                                 ? "untitled.cpp"
                                 : splittedTextEdit->curFile;
  QMessageBox msgBox;
  msgBox.setText(tr("Which file would you like to save?"));
  QAbstractButton *pButtonEdit = msgBox.addButton(
      tr(filename.toStdString().c_str()), QMessageBox::YesRole);
  msgBox.addButton(tr(splittedFilename.toStdString().c_str()),
                   QMessageBox::NoRole);
  msgBox.exec();
  if (msgBox.clickedButton() == pButtonEdit) {
    if (textEdit->curFile.isEmpty()) {
      return saveAs();
    } else {
      return saveFile(textEdit->curFile, textEdit);
    }
  } else {
    if (splittedTextEdit->curFile.isEmpty()) {
      return saveAs();
    } else {
      return saveFile(splittedTextEdit->curFile, splittedTextEdit);
    }
  }
}

void MainWindow::split() {
  if (!splitted) {
    splitted = true;
    splittedTextEdit = new Editor(textEdit->fontSize);
    splittedTextEdit->setCompleter(completer);
    splitter->addWidget(splittedTextEdit);

    const int IND = 2;
    const int STRETCH_FACTOR = 1;
    splitter->setStretchFactor(IND, STRETCH_FACTOR);
    fv_split = new FileView("lol.cpp", this);

    connect(splittedTextEdit, &Editor::changeContent, fv_split,
            &FileView::UploadContent);

    connect(splittedTextEdit, &Editor::changeCursor, fv_split,
            &FileView::ChangeCursor);

    connect(fv_split, &FileView::DoneCompletion, this,
            &MainWindow::displayAutocompleteOptions);

    connect(fv_split, &FileView::DoneDiagnostic, this,
            &MainWindow::display_failure);

    splittedTextEdit->setTabStopDistance(tabStop *
                                         metrics->horizontalAdvance(' '));

    connect(splittedTextEdit, &Editor::cursorPositionChanged, this,
            &MainWindow::showCursorPositionOnSplitted);

    splittedTextEdit->setFont(*font);
  } else {
    disconnect(splittedTextEdit, &Editor::changeContent, fv_split,
               &FileView::UploadContent);

    disconnect(splittedTextEdit, &Editor::changeCursor, fv_split,
               &FileView::ChangeCursor);

    disconnect(fv_split, &FileView::DoneDiagnostic, this,
               &MainWindow::display_failure);

    delete fv_split;
    splitted = false;
    delete splitter->widget(1);
  }
}

void MainWindow::textSize(const QString &p) {
  qreal pointSize = p.toFloat();
  if (p.toFloat() > 0) {
    QTextCharFormat fmt;
    fmt.setFontPointSize(pointSize);
    textEdit->fontSize = pointSize;
    textEdit->repaint();
    if (splitted) {
      splittedTextEdit->fontSize = pointSize;
      splittedTextEdit->repaint();
    }
    mergeFormatOnWordOrSelection(fmt);
  }
}

void MainWindow::mergeFormatOnWordOrSelection(const QTextCharFormat &format) {
  QTextCursor cursor = textEdit->textCursor();
  textEdit->selectAll();
  cursor.select(QTextCursor::WordUnderCursor);
  cursor.mergeCharFormat(format);
  textEdit->mergeCurrentCharFormat(format);
  cursor.movePosition(QTextCursor::End);
  textEdit->setTextCursor(cursor);

  if (splitted) {
    QTextCursor splitCursor = splittedTextEdit->textCursor();
    splittedTextEdit->repaint();
    splittedTextEdit->selectAll();
    splitCursor.mergeCharFormat(format);
    splittedTextEdit->mergeCurrentCharFormat(format);
    splitCursor.movePosition(QTextCursor::End);
    splittedTextEdit->setTextCursor(splitCursor);
  }
}

bool MainWindow::saveAs() {
  if (splitted) {
    QString filename = (textEdit->curFile.isEmpty()) ? "untitled(left).cpp"
                                                     : textEdit->curFile;
    QString splittedFilename = (splittedTextEdit->curFile.isEmpty())
                                   ? "untitled(right).cpp"
                                   : splittedTextEdit->curFile;
    QMessageBox msgBox;
    msgBox.setText(tr("Which file would you like to save?"));
    QAbstractButton *pButtonEdit = msgBox.addButton(
        tr(filename.toStdString().c_str()), QMessageBox::YesRole);
    msgBox.addButton(tr(splittedFilename.toStdString().c_str()),
                     QMessageBox::NoRole);
    msgBox.exec();
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec() != QDialog::Accepted) return false;
    if (msgBox.clickedButton() != pButtonEdit) {
      return saveFile(dialog.selectedFiles().first(), splittedTextEdit);
    } else {
      return saveFile(dialog.selectedFiles().first(), textEdit);
    }
  }
  QFileDialog dialog(this);
  dialog.setWindowModality(Qt::WindowModal);
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  if (dialog.exec() != QDialog::Accepted) return false;
  return saveFile(dialog.selectedFiles().first(), textEdit);
}

void MainWindow::documentWasModified() {
  setWindowTitle(tr("Baton Editor[*]"));
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

  QAction *set_root_directory = fileMenu->addAction(
      tr("&Set root directory..."), this, &MainWindow::choose_directory);
  set_root_directory->setStatusTip(
      tr("Choose the directory which will be shown in directory tree"));

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
}

MainWindow::~MainWindow() { delete ui; }

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
  static QString good_suf[] = {".h", ".c", ".cpp", ".hpp", ".cc"};
  if (std::none_of(
          std::begin(good_suf), std::end(good_suf),
          [&fileName](const auto &str) { return fileName.contains(str); })) {
    std::cerr << "CONTAINS BAD SUFFIX" << std::endl;
    fv->SetValidity(false);
  } else {
    fv->SetValidity(true);
  }

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

  setCurrentFile(fileName, textEdit);
  const int TIME_OUT_MS = 2000;
  statusBar()->showMessage(tr("File loaded"), TIME_OUT_MS);
  std::cerr << "FILENAME = " << fileName.toStdString() << std::endl;
}

void MainWindow::tree_clicked(const QModelIndex &index) {
  QFileInfo file_info = directory_tree.model.fileInfo(index);
  if (file_info.isFile()) {
    MainWindow::loadFile(file_info.filePath());
    return;
  }
}
void MainWindow::createStatusBar() { statusBar()->showMessage(tr("Ready")); }

bool MainWindow::saveFile(const QString &fileName, Editor *editArea) {
  QString errorMessage;

  QGuiApplication::setOverrideCursor(Qt::WaitCursor);
  QSaveFile file(fileName);
  if (file.open(QFile::WriteOnly | QFile::Text)) {
    QTextStream out(&file);
    out << editArea->toPlainText();
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

  setCurrentFile(fileName, editArea);
  return true;
}

void MainWindow::setCurrentFile(const QString &fileName, Editor *editArea) {
  editArea->curFile = fileName;
  setWindowTitle(tr("Baton Editor[*]"));
  editArea->document()->setModified(false);
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

void MainWindow::displayAutocompleteOptions(
    const std::vector<std::string> &vec) {
  disp->clear();
  std::cerr << "______AUTOCOMPLETE DISPLAY________" << std::endl;
  if (vec.size() == 0) return;
  QStringListModel *model =
      reinterpret_cast<QStringListModel *>(completer->model());
  QStringList stringList;

  for (const auto &item : vec) {
    std::cerr << item << '\n';
    disp->appendText(item);
    stringList << QString::fromStdString(item);
  }
  model->setStringList(stringList);
}

void MainWindow::display_failure(
    const std::vector<lsp::DiagnosticsResponse> &resp) {
  if (resp.empty()) {
    display_failure_log->clear();
    return;
  }
  std::string failure_log;
  for (auto &[ctgry, msg, range] : resp) {
    failure_log += msg;
    failure_log += " in the ";
    failure_log += std::to_string(range.start.line + 1);
    if (range.start.line + 1 == 1) {
      failure_log += "st line, ";
    } else if (range.start.line + 1 == 2) {
      failure_log += "nd line, ";
    } else if (range.start.line + 1 == 3) {
      failure_log += "rd line, ";
    } else {
      failure_log += "th line, ";
    }
    failure_log += std::to_string(range.start.character + 1);
    if (range.start.character + 1 == 1) {
      failure_log += "st column\n";
    } else if (range.start.character + 1 == 2) {
      failure_log += "nd column\n";
    } else if (range.start.character + 1 == 3) {
      failure_log += "rd column\n";
    } else {
      failure_log += "th column\n";
    }
  }
  display_failure_log->setPlainText(
      QString::fromStdString(std::string(failure_log)));
}
