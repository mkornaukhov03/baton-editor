#include "autocompletedisplay.h"

#include <QString>

#include "ui_autocompletedisplay.h"

autocompleteDisplay::autocompleteDisplay(QWidget *parent)
    : QWidget(parent), ui(new Ui::autocompleteDisplay) {
  ui->setupUi(this);
  ui->textBrowser->append("This is autocomplete option");
}

void autocompleteDisplay::appendText(const std::string &text) {
  this->ui->textBrowser->append(QString::fromStdString(text));
  this->ui->textBrowser->append("\n");
}

void autocompleteDisplay::clear() { this->ui->textBrowser->clear(); }
autocompleteDisplay::~autocompleteDisplay() { delete ui; }
