#pragma once
#include <QWidget>

namespace Ui {
class Light_Strand_Widget;
}

class Timer_Widget;
class Filter_Widget;

class Light_Strand_Widget : public QWidget
{
   Q_OBJECT
   
  public:
   
   explicit Light_Strand_Widget(QWidget *parent = 0);
   ~Light_Strand_Widget();

   QVector<Timer_Widget*> get_timer_widgets();

   public slots:
   void on_pushButton_add_timer_clicked();

   void timer_widget_deleted(int index);
   
  private:
   Ui::Light_Strand_Widget * ui;
   QVector<Timer_Widget*> timer_widgets_;
};