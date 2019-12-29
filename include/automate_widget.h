#pragma once
#include <QWidget>

namespace Ui {
class Automate_Widget;
}

class Automate_Widget : public QWidget
{
   Q_OBJECT
   
  public:
   
   explicit Automate_Widget(QWidget *parent = 0);
   ~Automate_Widget();
   
  private:
   Ui::Automate_Widget * ui;
};