#pragma once
#include <QWidget>
#include <archive_common.h>

namespace Ui
{
class Automate_Widget;
}

enum Record_Mode
{
    REC_ADD,
    REC_OVERWRITE
};

struct Automation_Info
{
    bool enabled;
    int key_code;
    int rec_mode;
    bool record_armed;
    double hold_time;
    double ramp_up;
    double ramp_down;
};

class Automate_Widget : public QWidget
{
    Q_OBJECT
  public:
    explicit Automate_Widget(QWidget * parent = 0);
    ~Automate_Widget();

    Automation_Info get_automation_info();

    void set_from_automation_info(const Automation_Info & info);

    int index;

    QVector<double> recorded_data;

  public slots:

    void on_pushButton_delete_clicked();

    void on_checkBox_enabled_stateChanged(int new_state);

    void on_toolButton_clear_clicked();

  signals:

    void delete_me(int index);

  private:
    Ui::Automate_Widget * ui;
};

pup_func(Automation_Info)
{
    pup_member(enabled);
    pup_member(key_code);
    pup_member(rec_mode);
    pup_member(record_armed);
    pup_member(hold_time);
    pup_member(ramp_up);
    pup_member(ramp_down);
}

pup_func(Automate_Widget)
{
    pup_member(recorded_data);
    Automation_Info ainf = val.get_automation_info();
    pack_unpack(ar, ainf, info);
    val.set_from_automation_info(ainf);
}
