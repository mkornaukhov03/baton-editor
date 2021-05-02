#ifndef SUGGEST_LABEL_H
#define SUGGEST_LABEL_H

#include <QLabel>
#include <QWidget>
#include <QtCore>

#include "interface.h"

//namespace Ui {
//class Suggest_lable;
//}
// 1) Нужно знать имя файла (V)
// 2) Нужно настроить слоты и сигналы
// 3) Нужно передавать\знать строку и столбец
class Suggest_label : public QLabel{
  Q_OBJECT

 public:
  explicit Suggest_label(QWidget *parent = nullptr): QLabel(parent) {
    this->setText("I am a suggested label!");
  }
  virtual ~Suggest_label() = default;

 private:
//  Ui::Suggest_lable *ui;
  // lsp::LSPHandler handler_;
};


#endif // SUGGEST_LABEL_H
