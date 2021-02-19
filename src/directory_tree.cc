#include "directory_tree.h"

#include <QScreen>
#include <QScroller>
#include <QTreeView>
Directory_tree::Directory_tree(QWidget *parent)
    : QWidget(parent), ui(new Ui::Directory_tree) {
  ui->setupUi(this);
  model = new QFileSystemModel(this);
  model->setFilter(QDir::QDir::AllEntries);
  model->setRootPath(QDir::QDir::rootPath());
  QTreeView tree;
  tree.setModel(model);
  tree.setAnimated(false);
  tree.setIndentation(20);
  tree.setSortingEnabled(true);
  QScroller::grabGesture(&tree, QScroller::TouchGesture);
  ui->treeView->setModel(model);
}

Directory_tree::~Directory_tree() { delete ui; }
