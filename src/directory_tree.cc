#include "directory_tree.h"

#include <QCommandLineParser>
#include <QScreen>
#include <QScroller>
#include <QTextStream>
#include <QTreeView>
#include <fstream>
#include <iostream>
#include <string>

#include "mainwindow.h"
Directory_tree::Directory_tree(QWidget *parent)
    : QWidget(parent), ui(new Ui::Directory_tree) {
  ui->setupUi(this);
  QFile file(
      "/home/mdmalofeev/programm/final_project/baton-editor/src/config.txt");
  if (file.exists() &&
      file.open(QIODevice::ReadOnly | QIODevice::WriteOnly | QIODevice::Text)) {
    QTextStream in(&file);
    root = in.readLine();
    file.close();
  }
  if (root.size() > 0) {
    model.setRootPath(root);
    tree.setModel(&model);
    root = root + "/";
    QModelIndex rootIndex = model.index(root);
    tree.setRootIndex(rootIndex);
  } else {
    model.setRootPath("");
    tree.setModel(&model);
    QModelIndex rootIndex = model.index("/");
    tree.setRootIndex(rootIndex);
  }
  //  model.setRootPath("/home/mdmalofeev/programm/labs-mmalofeev");
  //  tree.setModel(&model);
  //  QModelIndex rootIndex =
  //      model.index("/home/mdmalofeev/programm/labs-mmalofeev/");
  //  tree.setRootIndex(rootIndex);
  tree.setAnimated(false);
  tree.setIndentation(20);
  tree.setSortingEnabled(true);
  const QSize availableSize = tree.screen()->availableGeometry().size();
  tree.resize(availableSize / 2);
  tree.setColumnWidth(0, tree.width() / 3);
  // Make it flickable on touchscreens
  QScroller::grabGesture(&tree, QScroller::TouchGesture);

  //  connect(ui->treeView, SIGNAL(doubleClicked(QModelIndex)), this,
  //          SLOT(double_clicked(QModelIndex)));
  //  tree.show();
  ui->treeView->setModel(&model);
}

Directory_tree::~Directory_tree() { delete ui; }
