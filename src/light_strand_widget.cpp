#include <light_strand_widget.h>
#include <ui_light_strand_widget.h>
#include <timer_widget.h>
#include <filter_widget.h>

#include <QPushButton>
#include <QVBoxLayout>

Light_Strand_Widget::Light_Strand_Widget(QWidget * parent)
    : QWidget(parent), ui(new Ui::Light_Strand_Widget)
{
    ui->setupUi(this);
}

Light_Strand_Widget::~Light_Strand_Widget()
{
    delete ui;
}

QVector<Timer_Widget *> Light_Strand_Widget::get_timer_widgets()
{
    return timer_widgets_;
}

void Light_Strand_Widget::timer_widget_deleted(int index)
{
    Timer_Widget * cur_tw = timer_widgets_[index];
    Timer_Widget * tw = timer_widgets_.last();
    tw->index = index;
    timer_widgets_[index] = tw;
    timer_widgets_.pop_back();
    QVBoxLayout * layout = static_cast<QVBoxLayout *>(ui->scrollArea->widget()->layout());
    layout->removeWidget(cur_tw);
    delete cur_tw;
}

void Light_Strand_Widget::on_pushButton_add_timer_clicked()
{
    Timer_Widget * tw = new Timer_Widget(this);

    connect(tw, &Timer_Widget::delete_me, this, &Light_Strand_Widget::timer_widget_deleted);
    QVBoxLayout * layout = static_cast<QVBoxLayout *>(ui->scrollArea->widget()->layout());
    layout->insertWidget(0, tw);
    tw->index = timer_widgets_.size();
    timer_widgets_.push_back(tw);
}