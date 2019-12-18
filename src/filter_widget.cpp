#include <filter_widget.h>
#include <ui_filter_widget.h>

Filter_Widget::Filter_Widget(QWidget *parent):
QWidget(parent),
ui(new Ui::Filter_Widget)
{
   ui->setupUi(this);
}

Filter_Widget::~Filter_Widget()
{
    delete ui;
}