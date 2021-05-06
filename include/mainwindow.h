#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGridLayout>
#include <QTimer>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>
#include <string>
#include <vector>

#include "directory_tree.h"
#include "editor.h"
#include "file_view.h"
#include "interface.h"
#include "suggest_label.h"
#include "terminal.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QPlainTextEdit;
class QSessionManager;
class QComboBox;

class Ui_MainWindow {
 public:
  QWidget *centralwidget;
  QMenuBar *menubar;
  QStatusBar *statusbar;

  void setupUi(QMainWindow *MainWindow) {
    retranslateUi(MainWindow);

    QMetaObject::connectSlotsByName(MainWindow);
  }  // setupUi

  void retranslateUi(QMainWindow *MainWindow) {
    MainWindow->setWindowTitle(
        QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
  }  // retranslateUi
};

namespace Ui {
class MainWindow : public Ui_MainWindow {};
}  // namespace Ui

QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  //  MainWindow();
  explicit MainWindow(QWidget *parent = nullptr);

  void loadFile(const QString &fileName);

  ~MainWindow();

 protected:
  void closeEvent(QCloseEvent *event) override;

 private slots:
  void newFile();
  void open();
  bool save();
  bool saveAs();
  void split();
  void documentWasModified();
  void textSize(const QString &p);
  void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
  void currentCharFormatChanged(const QTextCharFormat &format);
  void showCursorPosition();
  void showCursorPositionOnSplitted();
  void tree_clicked(const QModelIndex &index);

 private:
  Ui::MainWindow *ui;

  void createActions();
  void createStatusBar();
  void readSettings();
  void writeSettings();
  bool maybeSave();
  bool saveFile(const QString &fileName);
  void setCurrentFile(const QString &fileName);
  void fontChanged(const QFont &f);
  QString strippedName(const QString &fullFileName);

  Editor *textEdit;
  Editor *splittedTextEdit;
  QString curFile;
  QComboBox *comboSize;
  QToolBar *tb;
  QWidget *central_widget;
  QGridLayout *grid_layout;
  QWidget *terminal;
  Directory_tree directory_tree;
  bool splitted;
  // setting up autocomplete below
  lsp::LSPHandler *lsp_handler;
  QTimer *timer;
  Suggest_label *lbl;
  FileView *fv = nullptr;
 private slots:
  void update_autocomplete();
  void set_autocomplete_to_label(const std::vector<std::string> &);
  void display_diagnostics(const std::vector<lsp::DiagnosticsResponse> &);
};
#endif  // MAINWINDOW_H
