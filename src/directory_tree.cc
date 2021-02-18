#include "../include/interface/directory_tree.h"
#include <QTreeView>
#include <QScroller>
#include <QScreen>
Directory_tree::Directory_tree(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Directory_tree)
{
    ui->setupUi(this);
    model = new QFileSystemModel(this);
    model->setFilter(QDir::QDir::AllEntries);
    model->setRootPath(QDir::QDir::rootPath());
    QTreeView tree;
    tree.setModel(model);
    tree.setAnimated(false);
    tree.setIndentation(20);
    tree.setSortingEnabled(true);
    const QSize availableSize = tree.screen()->availableGeometry().size();
    tree.resize(availableSize);
    QScroller::grabGesture(&tree, QScroller::TouchGesture);
    ui->treeView->setModel(model);
}

Directory_tree::~Directory_tree()
{
    delete ui;
}
