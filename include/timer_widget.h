#pragma once
#include <QWidget>
#include <archive_common.h>

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

    void set_from_timer_info(const Timer_Info & inf);

    int index;

  public slots:
    void on_pushButton_delete_clicked();

  signals:

    void delete_me(int ind);

  private:
    Ui::Timer_Widget * ui;
};

pup_func(Timer_Info)
{
    pup_member(duration);
    pup_member(start_delay);
    pup_member(period);
    pup_member(hold);
    pup_member(ramp_up);
    pup_member(ramp_down);
}

pup_func(Timer_Widget)
{
    Timer_Info tinf = val.get_timer_info();
    pack_unpack(ar, tinf, info);
    val.set_from_timer_info(tinf);
}
