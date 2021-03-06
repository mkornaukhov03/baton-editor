#include "directory_tree.h"

#include <stdlib.h>

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
  std::string user_name = getenv("USER");
  const std::string CONFIG_FILE_NAME = ".batonrc";
  std::string config_path = "/home/" + user_name + "/" + CONFIG_FILE_NAME;
  QFile file(QString::fromStdString(config_path));
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
    std::string root_path = "/home/" + user_name + "/";
    model.setRootPath(QString::fromStdString(root_path));
    tree.setModel(&model);
    QModelIndex rootIndex = model.index(QString::fromStdString(root_path));
    tree.setRootIndex(rootIndex);
  }
  tree.setAnimated(true);
  const int DEFAULT_INDENT = 20;
  tree.setIndentation(DEFAULT_INDENT);
  tree.setSortingEnabled(true);
  const int MAX_COLUMN = 3;
  for (int i = 1; i <= MAX_COLUMN; ++i) {
    tree.setColumnHidden(i, true);
  }
  // Make it flickable on touchscreens
  QScroller::grabGesture(&tree, QScroller::TouchGesture);
  ui->treeView->setModel(&model);
}

void Directory_tree::set_root_path() {
  model.setRootPath(dir_name);
  tree.setModel(&model);
  QModelIndex rootIndex = model.index(dir_name);
  tree.setRootIndex(rootIndex);
}
Directory_tree::~Directory_tree() { delete ui; }
