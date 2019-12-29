#pragma once
#include <QWidget>
#include <archive_common.h>

namespace Ui
{
class Filter_Widget;
}

enum Filter_Type
{
    ButterWorth_LP,
    ButterWorth_HP,
    ButterWorth_BP,
    ChebyshevI_LP,
    ChebyshevI_HP,
    ChebyshevI_BP,
    ChebyshevII_LP,
    ChebyshevII_HP,
    ChebyshevII_BP
};

struct Filter_Info
{
    int ftype;
    int order;
    double sb_ripple;
    double center_freq;
    double bandwidth;
    double start_delay;
    double duration;
    double hold;
    double release;
    double min_threshold;
    double max_threshold;
    double hysterysis;
    double min_intensity;
    double max_intensity;
    bool use_right;
    bool use_left;
    uint32_t window_size;
};

namespace FMOD
{
class Sound;
class System;
class Channel;
} // namespace FMOD

class Filter_Widget : public QWidget
{
    Q_OBJECT

  public:
    explicit Filter_Widget(QWidget * parent = 0);
    ~Filter_Widget();

    Filter_Info get_filter_info();

    void set_from_filter_info(const Filter_Info & finf);

    void filter_audio(uint32_t sample_count,
                      uint32_t sample_rate,
                      int32_t channel_count,
                      uint64_t byte_count);

    int16_t ** get_channels();

    void calculate_statistics(const QVector<double> & rms_values);

    int index;

  public slots:

    void on_pushButton_preview_clicked();

    void on_pushButton_delete_clicked();

  signals:

    void delete_me(int ind);

  private:
    void adjust_ui(int ci);
    Ui::Filter_Widget * ui;
    FMOD::Sound * sound_;
    FMOD::Channel * channel_;
    int16_t * filtered_interleaved_audio_;
    int16_t * channels[2];
};

pup_func(Filter_Info)
{
    pup_member(ftype);
    pup_member(order);
    pup_member(sb_ripple);
    pup_member(center_freq);
    pup_member(bandwidth);
    pup_member(duration);
    pup_member(start_delay);
    pup_member(duration);
    pup_member(hold);
    pup_member(release);
    pup_member(min_threshold);
    pup_member(max_threshold);
    pup_member(hysterysis);
    pup_member(min_intensity);
    pup_member(max_intensity);
    pup_member(use_right);
    pup_member(use_left);
    pup_member(window_size);
}

pup_func(Filter_Widget)
{
    Filter_Info finf = val.get_filter_info();
    pack_unpack(ar, finf, info);
    val.set_from_filter_info(finf);
}
