#pragma once
#include <QWidget>

namespace Ui {
class Filter_Widget;
}

class Filter_Widget : public QWidget
{
   Q_OBJECT
   
  public:
   
   explicit Filter_Widget(QWidget *parent = 0);
   ~Filter_Widget();
   
  private:
   Ui::Filter_Widget * ui;
};