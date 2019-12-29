#include <automate_widget.h>
#include <ui_automate_widget.h>

struct Automate_Info
{

};

Automate_Widget::Automate_Widget(QWidget *parent):
QWidget(parent),
ui(new Ui::Automate_Widget)
{
   ui->setupUi(this);
}

Automate_Widget::~Automate_Widget()
{
    delete ui;
}