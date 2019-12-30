#include <automate_widget.h>
#include <ui_automate_widget.h>
#include <QDebug>

struct Automate_Info
{};

Automate_Widget::Automate_Widget(QWidget * parent)
    : QWidget(parent), index(-1), recorded_data(), ui(new Ui::Automate_Widget)
{
    recorded_data.reserve(600000);
    ui->setupUi(this);
}

Automate_Widget::~Automate_Widget()
{
    delete ui;
}

void Automate_Widget::on_pushButton_delete_clicked()
{
    emit delete_me(index);
}

Automation_Info Automate_Widget::get_automation_info()
{
    Automation_Info ainf;
    ainf.enabled = ui->checkBox_enabled->isChecked();
    ainf.hold_time = ui->spinBox_hold->value();
    ainf.ramp_down = ui->spinBox_ramp_down->value();
    ainf.ramp_up = ui->spinBox_ramp_up->value();
    ainf.rec_mode = ui->comboBox_rec_mode->currentIndex();
    ainf.record_armed = ui->toolButton_rec->isChecked();
    if (ui->keySequenceEdit_key->keySequence().count() == 1)
        ainf.key_code = ui->keySequenceEdit_key->keySequence()[0];
    else
        ainf.key_code = 0;
    return ainf;
}

void Automate_Widget::on_toolButton_clear_clicked()
{
    for (int i = 0; i < recorded_data.size(); ++i)
        recorded_data[i] = 0.0;
}

void Automate_Widget::set_from_automation_info(const Automation_Info & info)
{
    ui->checkBox_enabled->setChecked(info.enabled);
    ui->spinBox_ramp_up->setValue(info.ramp_up);
    ui->spinBox_ramp_down->setValue(info.ramp_down);
    ui->spinBox_hold->setValue(info.hold_time);
    ui->toolButton_rec->setChecked(info.record_armed);
    ui->comboBox_rec_mode->setCurrentIndex(info.rec_mode);
    if (info.key_code != 0)
        ui->keySequenceEdit_key->setKeySequence(info.key_code);
}

void Automate_Widget::on_checkBox_enabled_stateChanged(int new_state)
{
    bool new_val(new_state == Qt::Checked);
    ui->spinBox_ramp_up->setEnabled(new_val);
    ui->spinBox_ramp_down->setEnabled(new_val);
    ui->spinBox_hold->setEnabled(new_val);
    ui->toolButton_rec->setEnabled(new_val);
    ui->comboBox_rec_mode->setEnabled(new_val);
    ui->keySequenceEdit_key->setEnabled(new_val);
}
