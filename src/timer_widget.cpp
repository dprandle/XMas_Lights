#include <timer_widget.h>
#include <ui_timer_widget.h>

Timer_Widget::Timer_Widget(QWidget * parent) : QWidget(parent), index(-1), ui(new Ui::Timer_Widget)
{
    ui->setupUi(this);
}

Timer_Widget::~Timer_Widget()
{
    delete ui;
}

void Timer_Widget::on_pushButton_delete_clicked()
{
    emit delete_me(index);
}

Timer_Info Timer_Widget::get_timer_info()
{
    Timer_Info tinf;
    tinf.duration = ui->spinBox_duration->value();
    tinf.hold = ui->spinBox_hold->value();
    tinf.period = ui->spinBox_period->value();
    tinf.ramp_down = ui->spinBox_ramp_down->value();
    tinf.ramp_up = ui->spinBox_ramp_up->value();
    tinf.start_delay = ui->spinBox_init_delay->value();
    return tinf;
}