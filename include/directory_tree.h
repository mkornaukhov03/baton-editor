#ifndef DIRECTORY_TREE_H
#define DIRECTORY_TREE_H
#include <QFileSystemModel>
#include <QString>
#include <QWidget>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QWidget>
#include <string>
QT_BEGIN_NAMESPACE

class Ui_Directory_tree {
 public:
  QGridLayout *gridLayout_2;
  QGridLayout *gridLayout;
  QTreeView *treeView;

  void setupUi(QWidget *Directory_tree) {
    if (Directory_tree->objectName().isEmpty())
      Directory_tree->setObjectName(QString::fromUtf8("Directory_tree"));
    Directory_tree->resize(290, 677);
    gridLayout_2 = new QGridLayout(Directory_tree);
    gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
    gridLayout = new QGridLayout();
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));

    gridLayout_2->addLayout(gridLayout, 0, 0, 1, 1);

    treeView = new QTreeView(Directory_tree);
    treeView->setObjectName(QString::fromUtf8("treeView"));

    gridLayout_2->addWidget(treeView, 0, 1, 1, 1);

    retranslateUi(Directory_tree);

    QMetaObject::connectSlotsByName(Directory_tree);
  }  // setupUi

  void retranslateUi(QWidget *Directory_tree) {
    Directory_tree->setWindowTitle(
        QCoreApplication::translate("Directory_tree", "Form", nullptr));
  }  // retranslateUi
};
namespace Ui {
class Directory_tree : public Ui_Directory_tree {};
}  // namespace Ui
class Directory_tree : public QWidget {
  Q_OBJECT

 public:
  QTreeView tree;
  explicit Directory_tree(QWidget *parent = nullptr);
  virtual ~Directory_tree();
  QFileSystemModel model;
  Ui::Directory_tree *ui;

 private:
  QString root;
  QModelIndex rootIndex;
};

#endif  // DIRECTORY_TREE_H
