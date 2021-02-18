#include "../include/interface/mainwindow.h"
#include "ui_mainwindow.h"
#include <QLayout>
#include "../include/interface/directory_tree.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    Directory_tree *directory_tree = new Directory_tree(this);
    layout()->addWidget(directory_tree);
}

MainWindow::~MainWindow()
{
    delete ui;
}

