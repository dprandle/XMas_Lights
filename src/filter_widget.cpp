#include <fmod/fmod.hpp>
#include <fmod/fmod_errors.h>

#include <main_window.h>
#include <filter_widget.h>
#include <ui_filter_widget.h>
#include <DspFilters/Dsp.h>
#include <DspFilters/ChebyshevII.h>
#include <QComboBox>

#include <QStatusBar>

Filter_Widget::Filter_Widget(QWidget * parent)
    : QWidget(parent),
      index(-1),
      ui(new Ui::Filter_Widget),
      sound_(nullptr),
      channel_(nullptr),
      filtered_interleaved_audio_(nullptr),
      channels()
{
    ui->setupUi(this);
    connect(ui->comboBox_filter_type, qOverload<int>(&QComboBox::currentIndexChanged), this, &Filter_Widget::adjust_ui);
}

void Filter_Widget::adjust_ui(int ci)
{
    ui->label_sb_ripple->setVisible(ci != ButterWorth_BP);
    ui->spinBox_sb_ripple->setVisible(ci != ButterWorth_BP);
    if (ci < ChebyshevI_LP)
    {
        ui->label_sb_ripple->setVisible(false);
        ui->spinBox_sb_ripple->setVisible(false);
    }
    else if (ci < ChebyshevII_LP)
    {
        ui->label_sb_ripple->setText("Ripple (dB)");
    }
    else
    {
        ui->label_sb_ripple->setText("Stob Band (dB)");
    }
    
    if (((ci+1) % 3) == 0)
    {
        ui->label_center_freq->setText("Center (Hz)");
        ui->label_bw->setVisible(true);
        ui->spinBox_bandwidth->setVisible(true);
    }
    else
    {
        ui->label_center_freq->setText("Cutoff (Hz)");
        ui->label_bw->setVisible(false);
        ui->spinBox_bandwidth->setVisible(false);
    }
}

Filter_Widget::~Filter_Widget()
{
    delete ui;
}

Filter_Info Filter_Widget::get_filter_info()
{
    Filter_Info finf = {};

    finf.ftype = ui->comboBox_filter_type->currentIndex();

    int index = ui->comboBox_source->currentIndex();
    if (index == 0)
    {
        finf.use_left = true;
        finf.use_right = false;
    }
    else if (index == 1)
    {
        finf.use_left = false;
        finf.use_right = true;
    }
    else
    {
        finf.use_left = true;
        finf.use_right = true;
    }

    finf.center_freq = ui->spinBox_center_freq->value();
    finf.sb_ripple = ui->spinBox_sb_ripple->value();
    finf.bandwidth = ui->spinBox_bandwidth->value();
    finf.start_delay = ui->spinBox_init_delay->value();
    finf.order = ui->spinBox_order->value();
    finf.hold = ui->spinBox_hold->value();
    finf.release = ui->spinBox_release->value();
    finf.min_intensity = ui->doubleSpinBox_min_int->value();
    finf.max_intensity = ui->doubleSpinBox_max_int->value();
    finf.min_threshold = ui->spinBox_min_threshold->value();
    finf.max_threshold = ui->spinBox_max_threshold->value();
    finf.hysterysis = ui->spinBox_hysterysis->value();
    finf.window_size = ui->spinBox_win_size->value();
    finf.duration = ui->spinBox_duration->value();

    return finf;
}

int16_t ** Filter_Widget::get_channels()
{
    return channels;
}

void Filter_Widget::filter_audio(uint32_t sample_count,
                                 uint32_t sample_rate,
                                 int32_t channel_count,
                                 uint64_t byte_count)
{
    delete[] filtered_interleaved_audio_;
    filtered_interleaved_audio_ = new int16_t[sample_count * channel_count];

    Filter_Info finf = get_filter_info();

    switch (finf.ftype)
    {
    case (ButterWorth_LP):
    {
        Dsp::SimpleFilter<Dsp::Butterworth::LowPass<10>, 2> filter;
        filter.setup(finf.order, sample_rate, finf.center_freq);
        filter.process(sample_count, channels);
        break;
    }
    case (ButterWorth_HP):
    {
        Dsp::SimpleFilter<Dsp::Butterworth::HighPass<10>, 2> filter;
        filter.setup(finf.order, sample_rate, finf.center_freq);
        filter.process(sample_count, channels);
        break;
    }
    case (ButterWorth_BP):
    {
        Dsp::SimpleFilter<Dsp::Butterworth::BandPass<10>, 2> filter;
        filter.setup(finf.order, sample_rate, finf.center_freq, finf.bandwidth);
        filter.process(sample_count, channels);
        break;
    }
    case (ChebyshevI_LP):
    {
        Dsp::SimpleFilter<Dsp::ChebyshevI::LowPass<10>, 2> filter;
        filter.setup(finf.order, sample_rate, finf.center_freq, finf.sb_ripple);
        filter.process(sample_count, channels);
        break;
    }
    case (ChebyshevI_HP):
    {
        Dsp::SimpleFilter<Dsp::ChebyshevI::HighPass<10>, 2> filter;
        filter.setup(finf.order, sample_rate, finf.center_freq, finf.sb_ripple);
        filter.process(sample_count, channels);
        break;
    }
    case (ChebyshevI_BP):
    {
        Dsp::SimpleFilter<Dsp::ChebyshevI::BandPass<10>, 2> filter;
        filter.setup(finf.order, sample_rate, finf.center_freq, finf.bandwidth, finf.sb_ripple);
        filter.process(sample_count, channels);
        break;
    }
    case (ChebyshevII_LP):
    {
        Dsp::SimpleFilter<Dsp::ChebyshevII::LowPass<10>, 2> filter;
        filter.setup(finf.order, sample_rate, finf.center_freq, finf.sb_ripple);
        filter.process(sample_count, channels);
        break;
    }
    case (ChebyshevII_HP):
    {
        Dsp::SimpleFilter<Dsp::ChebyshevII::HighPass<10>, 2> filter;
        filter.setup(finf.order, sample_rate, finf.center_freq, finf.sb_ripple);
        filter.process(sample_count, channels);
        break;
    }
    case (ChebyshevII_BP):
    {
        Dsp::SimpleFilter<Dsp::ChebyshevII::BandPass<10>, 2> filter;
        filter.setup(finf.order, sample_rate, finf.center_freq, finf.bandwidth, finf.sb_ripple);
        filter.process(sample_count, channels);
        break;
    }
    }

    //filter.setup(10, sample_rate, finf.center_freq, finf.bandwidth);

    for (unsigned int j = 0; j < sample_count; ++j)
    {
        filtered_interleaved_audio_[j] = channels[0][j];
        filtered_interleaved_audio_[j + 1] = channels[1][j];
    }

    if (sound_ != nullptr)
    {
        sound_->release();
        sound_ = nullptr;
    }

    FMOD_CREATESOUNDEXINFO info = {};
    info.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
    info.length = byte_count;
    info.numchannels = 2;
    info.defaultfrequency = 44100;
    info.format = FMOD_SOUND_FORMAT_PCM16;
    info.suggestedsoundtype = FMOD_SOUND_TYPE_WAV;
    FMOD_RESULT res = mwin.get_audio_system()->createSound(
        reinterpret_cast<const char *>(filtered_interleaved_audio_),
        FMOD_OPENMEMORY | FMOD_CREATESAMPLE | FMOD_OPENRAW,
        &info,
        &sound_);
    if (res != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", res, FMOD_ErrorString(res));
    }
}

void Filter_Widget::on_pushButton_preview_clicked()
{
    if (sound_ != nullptr)
    {
        if (channel_ != nullptr)
        {
            channel_->stop();
            channel_ = nullptr;
            ui->pushButton_preview->setText("Preview");
        }
        else
        {
            mwin.get_audio_system()->playSound(sound_, nullptr, false, &channel_);
            ui->pushButton_preview->setText("Stop");
        }
    }
    else
    {
        mwin.statusBar()->showMessage("Can't play - no audio data detected - try building first");
    }
}

void Filter_Widget::set_from_filter_info(const Filter_Info & finf)
{
    ui->comboBox_filter_type->setCurrentIndex(finf.ftype);
    if (finf.use_left && finf.use_right)
        ui->comboBox_source->setCurrentIndex(2);
    else if (finf.use_right)
        ui->comboBox_source->setCurrentIndex(1);
    else
        ui->comboBox_source->setCurrentIndex(0);
    ui->spinBox_center_freq->setValue(finf.center_freq);
    ui->spinBox_bandwidth->setValue(finf.bandwidth);
    ui->spinBox_order->setValue(finf.order);
    ui->spinBox_init_delay->setValue(finf.start_delay);
    ui->spinBox_hold->setValue(finf.hold);
    ui->spinBox_release->setValue(finf.release);
    ui->doubleSpinBox_min_int->setValue(finf.min_intensity);
    ui->doubleSpinBox_max_int->setValue(finf.max_intensity);
    ui->spinBox_min_threshold->setValue(finf.min_threshold);
    ui->spinBox_max_threshold->setValue(finf.max_threshold);
    ui->spinBox_hysterysis->setValue(finf.hysterysis);
    ui->spinBox_win_size->setValue(finf.window_size);
    ui->spinBox_duration->setValue(finf.duration);
    ui->spinBox_sb_ripple->setValue(finf.sb_ripple);
}

void Filter_Widget::calculate_statistics(const QVector<double> & rms_values)
{
    if (rms_values.isEmpty())
        return;
    double min = 1.0;
    double max = 0.0;
    double avg = 0.0;
    double sd = 0.0;
    double var = 0.0;
    for (int i = 0; i < rms_values.size(); ++i)
    {
        if (rms_values[i] < min)
            min = rms_values[i];
        if (rms_values[i] > max)
            max = rms_values[i];
        avg += rms_values[i];
    }
    avg /= double(rms_values.size());

    for (int i = 0; i < rms_values.size(); ++i)
        var += (rms_values[i] - avg) * (rms_values[i] - avg);

    if (rms_values.size() > 2)
        var /= (rms_values.size() - 1);

    sd = sqrt(var);
    min = 20 * log10(min);
    max = 20 * log10(max);
    avg = 20 * log10(avg);
    sd = 20 * log10(sd);
    ui->doubleSpinBox_min->setValue(min);
    ui->doubleSpinBox_max->setValue(max);
    ui->doubleSpinBox_avg->setValue(avg);
    ui->doubleSpinBox_sd->setValue(sd);
}

void Filter_Widget::on_pushButton_delete_clicked()
{
    emit delete_me(index);
}
