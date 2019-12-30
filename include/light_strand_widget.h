#pragma once
#include <QWidget>
#include <archive_common.h>
#include <timer_widget.h>
#include <filter_widget.h>
#include <automate_widget.h>

namespace Ui {
class Light_Strand_Widget;
}

class Light_Strand_Widget : public QWidget
{
   Q_OBJECT
   pup_friend(Light_Strand_Widget);

  public:
   
   explicit Light_Strand_Widget(QWidget *parent = 0);
   ~Light_Strand_Widget();

   QVector<Timer_Widget*> get_timer_widgets();

   QVector<Filter_Widget*> get_filter_widgets();

   QVector<Automate_Widget*> get_automation_widgets();

   Timer_Widget * add_timer_wiget();

   void delete_timer(int index);

   Filter_Widget * add_filter_wiget();

   void delete_filter(int index);

   Automate_Widget * add_automation_wiget();

   void delete_automation(int index);

   uint32_t get_timer_max_duration();

   public slots:

   void on_pushButton_add_timer_clicked();

   void on_pushButton_add_filter_clicked();

   void on_pushButton_add_automation_clicked();

  private:
   Ui::Light_Strand_Widget * ui;
   QVector<Timer_Widget*> timer_widgets_;
   QVector<Filter_Widget*> filter_widgets_;
   QVector<Automate_Widget*> automate_widgets_;
};

pup_func(Light_Strand_Widget)
{
   QVector<Timer_Info> timers;
   for (int i = 0; i < val.timer_widgets_.size(); ++i)
      timers.push_back(val.timer_widgets_[i]->get_timer_info());
   pack_unpack(ar, timers, var_info(info.name + QString(SPLIT_CHAR) + "Timers", info.params));
   if (ar.io == PUP_IN)
   {
      while (!val.timer_widgets_.isEmpty())
         val.delete_timer(0);
      
      for (int i = 0; i < timers.size(); ++i)
      {
         Timer_Widget * tw = val.add_timer_wiget();
         tw->set_from_timer_info(timers[i]);
      }
   }

   QVector<Filter_Info> filters;
   for (int i = 0; i < val.filter_widgets_.size(); ++i)
      filters.push_back(val.filter_widgets_[i]->get_filter_info());
   pack_unpack(ar, filters, var_info(info.name + QString(SPLIT_CHAR) + "Filters", info.params));
   if (ar.io == PUP_IN)
   {
      while (!val.filter_widgets_.isEmpty())
         val.delete_filter(0);
      
      for (int i = 0; i < filters.size(); ++i)
      {
         Filter_Widget * fw = val.add_filter_wiget();
         fw->set_from_filter_info(filters[i]);
      }
   }

   QVector<Automation_Info> automation_data;
   for (int i = 0; i < val.automate_widgets_.size(); ++i)
      automation_data.push_back(val.automate_widgets_[i]->get_automation_info());
   pack_unpack(ar, automation_data, var_info(info.name + QString(SPLIT_CHAR) + "Automation_Info", info.params));
   if (ar.io == PUP_IN)
   {
      while (!val.automate_widgets_.isEmpty())
         val.delete_automation(0);
      
      for (int i = 0; i < automation_data.size(); ++i)
      {
         Automate_Widget * aw = val.add_automation_wiget();
         aw->set_from_automation_info(automation_data[i]);
      }
   }

}