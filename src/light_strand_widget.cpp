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

QVector<Filter_Widget *> Light_Strand_Widget::get_filter_widgets()
{
    return filter_widgets_;
}

void Light_Strand_Widget::delete_timer(int index)
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

void Light_Strand_Widget::delete_filter(int index)
{
    Filter_Widget * cur_fw = filter_widgets_[index];
    Filter_Widget * fw = filter_widgets_.last();
    fw->index = index;
    filter_widgets_[index] = fw;
    filter_widgets_.pop_back();
    QVBoxLayout * layout = static_cast<QVBoxLayout *>(ui->scrollArea->widget()->layout());
    layout->removeWidget(cur_fw);
    delete cur_fw;
}

uint32_t Light_Strand_Widget::get_timer_max_duration()
{
    uint32_t max = 0;
    for (int i = 0; i < timer_widgets_.size(); ++i)
    {
        Timer_Info inf = timer_widgets_[i]->get_timer_info();
        if (inf.duration > max)
            max = inf.duration;
    }
    return max;
}

Timer_Widget * Light_Strand_Widget::add_timer_wiget()
{
    Timer_Widget * tw = new Timer_Widget(this);

    connect(tw, &Timer_Widget::delete_me, this, &Light_Strand_Widget::delete_timer);
    QVBoxLayout * layout = static_cast<QVBoxLayout *>(ui->scrollArea->widget()->layout());
    layout->insertWidget(0, tw);
    tw->index = timer_widgets_.size();
    timer_widgets_.push_back(tw);
    return tw;
}

Filter_Widget * Light_Strand_Widget::add_filter_wiget()
{
    Filter_Widget * fw = new Filter_Widget(this);
    connect(fw, &Filter_Widget::delete_me, this, &Light_Strand_Widget::delete_filter);
    QVBoxLayout * layout = static_cast<QVBoxLayout *>(ui->scrollArea->widget()->layout());
    layout->insertWidget(0, fw);
    fw->index = filter_widgets_.size();
    filter_widgets_.push_back(fw);
    return fw;
}

void Light_Strand_Widget::on_pushButton_add_automation_clicked()
{
    
}

void Light_Strand_Widget::on_pushButton_add_timer_clicked()
{
    add_timer_wiget();
}

void Light_Strand_Widget::on_pushButton_add_filter_clicked()
{
    add_filter_wiget();
}
