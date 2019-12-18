#pragma once
#include <QWidget>

namespace Ui
{
class Timer_Widget;
}

struct Timer_Info
{
    double duration;
    double start_delay;
    double period;
    double hold;
    double ramp_up;
    double ramp_down;
};

class Timer_Widget : public QWidget
{
    Q_OBJECT

  public:
    explicit Timer_Widget(QWidget * parent = 0);
    ~Timer_Widget();

    Timer_Info get_timer_info();

    int index;

  public slots:
    void on_pushButton_delete_clicked();

  signals:

    void delete_me(int ind);

  private:
    Ui::Timer_Widget * ui;
};