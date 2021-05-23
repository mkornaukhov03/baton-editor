#ifndef AUTOCOMPLETEDISPLAY_H
#define AUTOCOMPLETEDISPLAY_H

#include <QWidget>
#include <string>

namespace Ui {
class autocompleteDisplay;
}

class autocompleteDisplay : public QWidget {
  Q_OBJECT

 public:
  explicit autocompleteDisplay(QWidget *parent = nullptr);
  void appendText(const std::string &text);
  void clear();
  ~autocompleteDisplay();

 private:
  Ui::autocompleteDisplay *ui;
};

#endif  // AUTOCOMPLETEDISPLAY_H
