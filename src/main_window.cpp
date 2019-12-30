#include <main_window.h>
#include <ui_main_window.h>
#include <timer_widget.h>
#include <json_archive.h>
#include <filter_widget.h>

#include <iostream>
#include <algorithm>

#include <fmod/fmod.hpp>
#include <fmod/fmod_errors.h>

//#include <DspFilters/Butterworth.h>
//#include <DspFilters/Filter.h>
#include <DspFilters/Dsp.h>

#include <QKeyEvent>
#include <QJsonDocument>
#include <QTcpSocket>
#include <QFileDialog>
#include <QSpinBox>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QGraphicsScene>
#include <QHostAddress>
#include <QSlider>

uint32_t hash_id(const std::string & strng)
{
    uint32_t hash = 5381;
    int32_t c;
    const char * str = strng.c_str();
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;

    return hash;
}

Main_Window * Main_Window::this_ = nullptr;

Main_Window::Main_Window(QWidget * parent)
    : QMainWindow(parent),
      ui(new Ui::Main_Window),
      audio_system_(nullptr),
      sound_(nullptr),
      channel_(nullptr),
      sample_count(0),
      slider_pressed_(0),
      scene_(new QGraphicsScene),
      play_slider_(new QSlider(Qt::Horizontal)),
      light_data_(),
      lwidgets(),
      lights(),
      sim_timer_(),
      sim_sig_timer_(),
      pause_elapsed_(0),
      ed_socket_(new QTcpSocket)
{
    this_ = this;
    ui->setupUi(this);
    lwidgets.push_back(ui->widget_s1);
    lwidgets.push_back(ui->widget_s2);
    lwidgets.push_back(ui->widget_s3);
    lwidgets.push_back(ui->widget_s4);
    lwidgets.push_back(ui->widget_s5);
    lwidgets.push_back(ui->widget_s6);
    setup_light_graphics_();

    connect(&sim_sig_timer_, &QTimer::timeout, this, &Main_Window::run_sample);
    connect(ed_socket_, &QTcpSocket::readyRead, this, &Main_Window::socket_data_available);
    connect(ed_socket_,
            qOverload<QAbstractSocket::SocketError>(&QTcpSocket::error),
            this,
            &Main_Window::socket_error);
    connect(ed_socket_, &QTcpSocket::connected, this, &Main_Window::socket_connected);
    connect(play_slider_, &QSlider::sliderPressed, this, &Main_Window::slider_pressed);
    connect(play_slider_, &QSlider::sliderReleased, this, &Main_Window::slider_released);
    connect(play_slider_, &QSlider::valueChanged, this, &Main_Window::slider_value_changed);


    ui->toolBar->addWidget(play_slider_);
    play_slider_->setMinimum(0);
    play_slider_->setMaximum(0);
    play_slider_->setTickInterval(1000);
}

void Main_Window::process_sample(int index)
{
    for (uint8_t li = 0; li < LIGHT_COUNT; ++li)
    {
        int light_val = light_data_.lights[li].ms_data[index];
        double opacity = double(light_val) / LIGHT_LEVEL_MAX;

        if (ui->actionArm_Record->isChecked())
        {
            auto awidgets = lwidgets[li]->get_automation_widgets();
            for (int j = 0; j < awidgets.size(); ++j)
            {
                Automation_Info ainf = awidgets[j]->get_automation_info();
                if (ainf.enabled && ainf.record_armed)
                {
                    if (is_key_pressed(ainf.key_code))
                    {
                        awidgets[j]->recorded_data[index] = 1.0;
                        opacity = 1.0;
                    }
                    else if (ainf.rec_mode == REC_OVERWRITE)
                    {
                        awidgets[j]->recorded_data[index] = 0.0;
                    }
                }
            }
        }

        lights[li]->setOpacity(opacity);
    }
}

void Main_Window::slider_pressed()
{
    if (channel_ == nullptr)
    {
        on_actionStart_Simulation_triggered();
        on_actionPause_Simulation_triggered();
    }
    
    if (sim_sig_timer_.isActive())
    {
        on_actionPause_Simulation_triggered();
        was_playing_before_moving_slider_ = true;
    }
    else
    {
        was_playing_before_moving_slider_ = false;
    }
    slider_pressed_ = true;
}

void Main_Window::slider_released()
{
    slider_pressed_ = false;
    pause_elapsed_ = play_slider_->value();
    if (was_playing_before_moving_slider_)
    {
        on_actionStart_Simulation_triggered();
    }
}

void Main_Window::slider_value_changed()
{
    if (slider_pressed_)
    {
        uint32_t index = play_slider_->value();
        process_sample(index);
        if (channel_ != nullptr)
        {
            channel_->setPosition(index, FMOD_TIMEUNIT_MS);
        }
    }
}

void Main_Window::run_sample()
{
    uint32_t index = pause_elapsed_ + sim_timer_.elapsed();

    if (index >= light_data_.sample_cnt)
    {
        sim_timer_.restart();
        return;
    }

    play_slider_->setValue(index);
    process_sample(index);
}

void Main_Window::socket_data_available()
{}

void Main_Window::socket_error(QAbstractSocket::SocketError err)
{
    statusBar()->showMessage("Socket error!: " + ed_socket_->errorString(), 5000);
}

void Main_Window::socket_connected()
{
    qDebug() << "Connected!!";
    statusBar()->showMessage("Connected to " + ed_socket_->peerAddress().toString() + ":10000!!!",
                             5000);
}

void Main_Window::connect_to_edison()
{
    QString host = "XMAS_Lights.local";
    ed_socket_->connectToHost(host, 10000);
    statusBar()->showMessage("Trying to connect to " + host + " on port 10000");
}

void Main_Window::disconnect_from_edison()
{
    ed_socket_->disconnectFromHost();
    statusBar()->showMessage(
        "Disconnecting from " + ed_socket_->peerAddress().toString() + ":10000", 5000);
}

void Main_Window::on_actionEstablish_Connection_triggered()
{
    if (ui->actionEstablish_Connection->isChecked())
    {
        connect_to_edison();
    }
    else
    {
        disconnect_from_edison();
    }
}

void Main_Window::on_actionStart_Simulation_triggered()
{
    if (get_sample_count() == 0)
    {
        ui->actionStart_Simulation->setChecked(false);
        statusBar()->showMessage("Can't play - no data loaded", 3000);
        return;
    }

    if (ed_socket_->state() != QAbstractSocket::ConnectedState)
    {
        statusBar()->showMessage("Not sending play message to edison - not connected", 3000);
    }
    else
    {
        Packet_ID id = {};
        id.hashed_id = hash_id(PACKET_ID_PLAY);
        ed_socket_->write((char *)id.data, PACKET_ID_SIZE);
        statusBar()->showMessage("Sent play message to edison", 3000);
        timespec sleep = {}, rm = {};
        sleep.tv_nsec = 200000000;
        nanosleep(&sleep, &rm);
    }

    if (!sim_sig_timer_.isActive())
    {
        sim_timer_.restart();
        sim_sig_timer_.start();
    }
    ui->actionStart_Simulation->setChecked(true);
    ui->actionPause_Simulation->setChecked(false);

    if (channel_ != nullptr)
    {
        bool is_playing = false;
        channel_->isPlaying(&is_playing);
        if (is_playing)
        {
            channel_->setPaused(false);
            return;
        }
    }
    if (sound_ != nullptr)
        audio_system_->playSound(sound_, nullptr, false, &channel_);
}

void Main_Window::on_actionPause_Simulation_triggered()
{
    if (get_sample_count() == 0)
    {
        ui->actionPause_Simulation->setChecked(false);
        statusBar()->showMessage("Can't pause - no data loaded", 3000);
        return;
    }

    if (sim_sig_timer_.isActive())
    {
        sim_sig_timer_.stop();
        pause_elapsed_ += sim_timer_.elapsed();
    }
    ui->actionPause_Simulation->setChecked(true);
    ui->actionStart_Simulation->setChecked(false);

    if (ed_socket_->state() != QAbstractSocket::ConnectedState)
    {
        statusBar()->showMessage("Not sending pause message to edison - not connected", 3000);
    }
    else
    {
        Packet_ID id = {};
        id.hashed_id = hash_id(PACKET_ID_PAUSE);
        ed_socket_->write((char *)id.data, PACKET_ID_SIZE);
        statusBar()->showMessage("Sent pause message to edison", 3000);
    }

    if (channel_ != nullptr)
        channel_->setPaused(true);
}

void Main_Window::on_actionStop_Simulation_triggered()
{
    pause_elapsed_ = 0;
    play_slider_->setValue(0);
    sim_sig_timer_.stop();

    ui->actionPause_Simulation->setChecked(false);
    ui->actionStart_Simulation->setChecked(false);
    for (uint8_t li = 0; li < LIGHT_COUNT; ++li)
        lights[li]->setOpacity(0.0);

    if (ed_socket_->state() != QAbstractSocket::ConnectedState)
    {
        statusBar()->showMessage("Not sending stop message to edison - not connected", 3000);
    }
    else
    {
        Packet_ID id = {};
        id.hashed_id = hash_id(PACKET_ID_STOP);
        ed_socket_->write((char *)id.data, PACKET_ID_SIZE);
        statusBar()->showMessage("Sending stop message to edison", 3000);
    }

    if (channel_ != nullptr)
        channel_->stop();
}

void Main_Window::normalize_samples(int16_t * channel,
                                    QVector<double> & dest,
                                    uint32_t offset,
                                    uint32_t subsize)
{
    double max_15_val = pow(2, 15);
    double max_16_val = pow(2, 16) - 1;

    if (subsize == 0)
        subsize = sample_count;

    assert(sample_count >= (offset + subsize));

    dest.resize(subsize);
    for (uint32_t si = 0; si < subsize; ++si)
        dest[si] = 2 * (double(channel[offset + si]) + max_15_val) / max_16_val - 1.0;
}

void Main_Window::keyPressEvent(QKeyEvent * event)
{
    auto fiter = keys_.find(event->key());
    if (fiter != keys_.end())
        fiter.value() = true;
}

bool Main_Window::is_key_pressed(int key)
{
    bool ret = false;
    auto fiter = keys_.find(key);
    if (fiter != keys_.end())
        ret = fiter.value();
    return ret;
}

void Main_Window::keyReleaseEvent(QKeyEvent * event)
{
    auto fiter = keys_.find(event->key());
    if (fiter != keys_.end())
        fiter.value() = false;
}

void Main_Window::on_actionRebuild_Light_Data_triggered()
{
    load_audio_data();
    light_data_.sample_cnt = get_sample_count();
    for (uint8_t i = 0; i < LIGHT_COUNT; ++i)
    {
        QVector<double> normalized_left;
        QVector<double> normalized_right;
        QVector<Filter_State_Info> filter_state;
        QVector<Automation_State_Info> automation_state;
        auto twidgets = lwidgets[i]->get_timer_widgets();
        auto fwidgets = lwidgets[i]->get_filter_widgets();
        auto awidgets = lwidgets[i]->get_automation_widgets();

        filter_state.resize(fwidgets.size());
        automation_state.resize(awidgets.size());

        light_data_.lights[i].ms_data.resize(light_data_.sample_cnt);

        for (uint32_t si = 0; si < light_data_.sample_cnt; ++si)
        {
            double timer_frame_contrib = 0.0;
            for (uint16_t ti = 0; ti < twidgets.size(); ++ti)
            {
                Timer_Info tinf = twidgets[ti]->get_timer_info();

                // If sample is before start delay or after duration disregard
                if (!tinf.enabled || (si < tinf.start_delay) ||
                    (si > (tinf.duration + tinf.start_delay)) || fcompare(tinf.period, 0.0))
                    continue;

                double cur_ms = (si - uint32_t(tinf.start_delay)) % uint32_t(tinf.period);

                double ramp_up_perc =
                    (cur_ms - (tinf.period - (tinf.ramp_up + 1))) / (tinf.ramp_up + 1);
                ramp_up_perc = std::clamp(ramp_up_perc, 0.0, 1.0);

                double ramp_down_and_hold = (cur_ms - (tinf.hold - 1)) / (tinf.ramp_down + 1);
                ramp_down_and_hold = std::clamp(ramp_down_and_hold, 0.0, 1.0);
                timer_frame_contrib += (ramp_up_perc + (1.0 - ramp_down_and_hold));
            }

            double filter_frame_contrib = 0.0;
            for (int fi = 0; fi < fwidgets.size(); ++fi)
            {
                Filter_Info finf = fwidgets[fi]->get_filter_info();

                if (!finf.enabled || (si < finf.start_delay) || (si >= finf.duration))
                    continue;

                if ((si % uint32_t(finf.window_size)) == 0)
                {
                    // If we are running out of ms, adjust the window size to what's left
                    if (si + finf.window_size > light_data_.sample_cnt)
                        finf.window_size = light_data_.sample_cnt - si;

                    normalize_samples(fwidgets[fi]->get_channels()[0],
                                      normalized_left,
                                      si * 44.1,
                                      finf.window_size * 44.1);
                    normalize_samples(fwidgets[fi]->get_channels()[1],
                                      normalized_right,
                                      si * 44.1,
                                      finf.window_size * 44.1);

                    double rms_value = 0.0;
                    double total_size = 0.0;

                    if (finf.use_left)
                    {
                        for (int li = 0; li < normalized_left.size(); ++li)
                            rms_value += normalized_left[li] * normalized_left[li];
                        total_size += normalized_left.size();
                    }
                    if (finf.use_right)
                    {
                        for (int ri = 0; ri < normalized_right.size(); ++ri)
                            rms_value += normalized_right[ri] * normalized_right[ri];
                        total_size += normalized_right.size();
                    }

                    rms_value /= total_size;
                    rms_value = sqrt(rms_value);
                    filter_state[fi].window_rms_vals.push_back(rms_value);

                    double db_val = 20 * log10(rms_value);

                    if (filter_state[fi].open_squelch)
                    {
                        if (db_val > (finf.min_threshold - finf.hysterysis))
                        {
                            // squelch to remain open
                            //                          std::cout << "Squelch remains opened" << std::endl;

                            // Update the intensity with this window's rms
                            double mult = (db_val - finf.min_threshold) /
                                          (finf.max_threshold - finf.min_threshold);
                            mult = std::clamp(mult, 0.0, 1.0);
                            filter_state[fi].frame_contrib_hold =
                                mult * (finf.max_intensity - finf.min_intensity) +
                                finf.min_intensity;
                        }
                        else
                        {
                            // close the squelch
                            //                            std::cout << "Squelch closed!" << std::endl;
                            filter_state[fi].open_squelch = false;

                            // squelch was open and now is closed - engage the hold time... if hold is zero then engage the release time
                            filter_state[fi].hold = finf.hold;
                            if (fcompare(finf.hold, 0.0))
                                filter_state[fi].release = finf.release;
                        }
                    }
                    else
                    {
                        if (db_val > finf.min_threshold)
                        {
                            // squelch opened
                            filter_state[fi].open_squelch = true;

                            // Reset the hold and release incase they were engaged
                            filter_state[fi].hold = 0;
                            filter_state[fi].release = 0;

                            // Set the intensity with this window's rms
                            double mult = (db_val - finf.min_threshold) /
                                          (finf.max_threshold - finf.min_threshold);
                            mult = std::clamp(mult, 0.0, 1.0);
                            filter_state[fi].frame_contrib_hold =
                                mult * (finf.max_intensity - finf.min_intensity) +
                                finf.min_intensity;
                        }
                        else
                        {
                            //std::cout << "Squelch remains closed" << std::endl;
                        }
                    }
                }

                if (filter_state[fi].open_squelch)
                {
                    filter_frame_contrib += filter_state[fi].frame_contrib_hold;
                }
                else if (filter_state[fi].hold > 0)
                {
                    filter_frame_contrib += filter_state[fi].frame_contrib_hold;
                    filter_state[fi].hold -= 1.0;
                    if (filter_state[fi].hold <= 0.0)
                        filter_state[fi].release = finf.release;
                }
                else if (filter_state[fi].release > 0)
                {
                    filter_frame_contrib += (double(filter_state[fi].release) / finf.release) *
                                            filter_state[fi].frame_contrib_hold;
                    // std::cout << "Release engaged: " << filter_state[fi].release << " at value " << filter_frame_contrib << std::endl;
                    filter_state[fi].release -= 1.0;
                }
                else
                {
                    filter_state[fi].frame_contrib_hold = 0;
                }

                //std::cout << "Filter frame contrib:" << filter_frame_contrib << std::endl;
            }

            double automation_contrib = 0.0;
            for (int ai = 0; ai < awidgets.size(); ++ai)
            {
                QVector<double> & rdata = awidgets[ai]->recorded_data;
                Automation_Info ainf = awidgets[ai]->get_automation_info();

                if (!ainf.enabled)
                    continue;
                // If the current value is 1.0 and the previous value is 0.0, for as long as previous values are 0.0
                // apply the ramp up time...
                //                int ramp_up = ainf.ramp_up;
                // int copy_si = si;
                // while (copy_si < rdata.size() && fcompare(rdata[copy_si], 0.0) && copy_si <= ramp_up)
                // {
                //     ++copy_si;
                // }

                // ramp up, down, or hold?
                if (fcompare(rdata[si], 0.0))
                {
                    if ((si > 0) && fcompare(rdata[si - 1], 1.0))
                        automation_state[ai].hold = ainf.hold_time;

                    if (automation_state[ai].hold > 0)
                    {
                        automation_contrib = 1.0;
                        automation_state[ai].hold -= 1.0;
                        if (automation_state[ai].hold <= 0.0)
                            automation_state[ai].ramp_down = ainf.ramp_down;
                    }
                    else if (automation_state[ai].ramp_down > 0)
                    {
                        automation_contrib = automation_state[ai].ramp_down / ainf.ramp_down;
                        automation_state[ai].ramp_down -= 1.0;
                    }
                }
                else
                {
                    automation_contrib = rdata[si];
                }
            }

            double total_contrib = timer_frame_contrib + filter_frame_contrib + automation_contrib;
            total_contrib = total_contrib * (LIGHT_LEVEL_MAX - LIGHT_LEVEL_MIN) + LIGHT_LEVEL_MIN;
            total_contrib = std::clamp(total_contrib, LIGHT_LEVEL_MIN, LIGHT_LEVEL_MAX);
            uint8_t rounded_int = std::lround(total_contrib);
            light_data_.lights[i].ms_data[si] = rounded_int;
        }

        for (int fi = 0; fi < fwidgets.size(); ++fi)
            fwidgets[fi]->calculate_statistics(filter_state[fi].window_rms_vals);
    }

    play_slider_->setMaximum(light_data_.sample_cnt);
    play_slider_->setTickInterval(5000);
    play_slider_->setTickPosition(QSlider::TicksBelow);
    play_slider_->setValue(0);

    if (ed_socket_->state() != QAbstractSocket::ConnectedState)
    {
        statusBar()->showMessage("Not sending build to edison - not connected", 3000);
        return;
    }

    Packet_ID id = {};
    id.hashed_id = hash_id(PACKET_ID_CONFIGURE);
    ed_socket_->write((char *)id.data, PACKET_ID_SIZE);
    ed_socket_->write((char *)light_data_.data, PACKET_ID_SIZE);
    for (uint32_t i = 0; i < LIGHT_COUNT; ++i)
    {
        ed_socket_->write((char *)light_data_.lights[i].ms_data.data(), light_data_.sample_cnt);
    }
    statusBar()->showMessage("Sent build to edison", 3000);
}

Main_Window::~Main_Window()
{
    delete ui;
    delete ed_socket_;
}

Main_Window & Main_Window::inst()
{
    return *this_;
}

Automation_Data * Main_Window::get_automation_data()
{
    return &light_data_;
}

void Main_Window::initialize()
{
    FMOD_RESULT result;
    result = FMOD::System_Create(&audio_system_);
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
    }

    result = audio_system_->init(512, FMOD_INIT_NORMAL, 0);
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
    }

    show();

    tabifyDockWidget(ui->dockWidget_s1, ui->dockWidget_s2);
    tabifyDockWidget(ui->dockWidget_s2, ui->dockWidget_s3);
    tabifyDockWidget(ui->dockWidget_s3, ui->dockWidget_s4);
    tabifyDockWidget(ui->dockWidget_s4, ui->dockWidget_s5);
    tabifyDockWidget(ui->dockWidget_s5, ui->dockWidget_s6);
    ui->dockWidget_s1->raise();
}

void Main_Window::terminate()
{}

void Main_Window::load_audio_data()
{
    if (song_fname.isEmpty())
        return;

    if (sound_ != nullptr)
    {
        sound_->release();
        sound_ = nullptr;
    }

    audio_system_->createSound(song_fname.toUtf8().constData(), FMOD_DEFAULT, nullptr, &sound_);

    FMOD_SOUND_TYPE t;
    FMOD_SOUND_FORMAT f;
    int bits_per_sample = 0;
    int32_t channel_count;
    sound_->getFormat(&t, &f, &channel_count, &bits_per_sample);
    int32_t bytes_per_sample = bits_per_sample / 8;

    uint32_t ms_count;
    sound_->getLength(&ms_count, FMOD_TIMEUNIT_MS);
    sound_->getLength(&sample_count, FMOD_TIMEUNIT_PCM);

    uint32_t sample_rate = (uint64_t(sample_count) * 1000) / ms_count;

    void * data = nullptr;
    unsigned int data_length = 0;
    void * data_wrap = nullptr;
    unsigned int data_wrap_length = 0;

    uint64_t byte_count = sample_count * channel_count * bytes_per_sample;
    sound_->lock(0, byte_count, &data, &data_wrap, &data_length, &data_wrap_length);
    int16_t * data_fmt = reinterpret_cast<int16_t *>(data);

    for (int li = 0; li < lwidgets.size(); ++li)
    {
        auto fwidgets = lwidgets[li]->get_filter_widgets();
        for (int fi = 0; fi < fwidgets.size(); ++fi)
        {
            int16_t ** channels = fwidgets[fi]->get_channels();
            for (int i = 0; i < 2; ++i)
            {
                delete[] channels[i];
                channels[i] = new int16_t[sample_count];
            }

            for (unsigned int j = 0; j < sample_count; ++j)
            {
                channels[0][j] = data_fmt[j];
                channels[1][j] = data_fmt[j + 1];
            }
            fwidgets[fi]->filter_audio(sample_count, sample_rate, channel_count, byte_count);
        }
    }
    sound_->unlock(data, data_wrap, data_length, data_wrap_length);
}

void Main_Window::on_actionOpen_Audio_File_triggered()
{
    song_fname = QFileDialog::getOpenFileName(this,
                                              "Open Audio File",
                                              "/home/daniel/Documents/Code/XMas_Lights/import",
                                              "Audio Files (*.wav *.mp3)");
}

void Main_Window::setup_light_graphics_()
{
    QPixmap pix_light, pix_light_lighting;
    pix_light.load(":graphics/lightbulb");
    pix_light_lighting.load(":graphics/lightbulb_lighting");

    QFont fnt;
    fnt.setBold(true);
    fnt.setPointSize(32);

    scene_->setBackgroundBrush(QColor(20, 20, 20));

    double scaled_width = 512.0 * LIGHT_SCALE_FACTOR;
    for (uint8_t i = 0; i < LIGHT_COUNT; ++i)
    {
        QGraphicsPixmapItem * l = scene_->addPixmap(pix_light);
        QGraphicsPixmapItem * ll = scene_->addPixmap(pix_light_lighting);
        l->setTransformationMode(Qt::SmoothTransformation);
        ll->setTransformationMode(Qt::SmoothTransformation);
        ll->setOpacity(0.0f);
        l->setScale(LIGHT_SCALE_FACTOR);
        l->setY(-150);
        ll->setY(-150);
        ll->setScale(LIGHT_SCALE_FACTOR);
        l->setX(i * scaled_width + LIGHT_PADDING);
        ll->setX(i * scaled_width + LIGHT_PADDING);
        lights.push_back(ll);

        QGraphicsTextItem * titem = scene_->addText("L" + QString::number(i + 1), fnt);
        titem->setDefaultTextColor(Qt::white);
        titem->setX(i * scaled_width + LIGHT_PADDING + scaled_width * 0.5 -
                    titem->boundingRect().width() / 2);
        titem->setY(50);
    }

    ui->graphicsView->setScene(scene_);
}

uint32_t Main_Window::get_sample_count()
{
    uint32_t max = 0;
    for (int i = 0; i < lwidgets.size(); ++i)
    {
        uint32_t tmax = lwidgets[i]->get_timer_max_duration();
        if (tmax > max)
            max = tmax;
    }

    if (sound_ != nullptr)
    {
        uint32_t ms_count = 0;
        sound_->getLength(&ms_count, FMOD_TIMEUNIT_MS);
        if (ms_count > max)
            max = ms_count;
    }

    return max;
}

void Main_Window::save_config_to_file(const QString & filename)
{
    if (filename.isEmpty())
        return;

    QFile json_file(filename);
    QFile json_file_dat(filename + ".dat");

    if (json_file.open(QIODevice::WriteOnly) && json_file_dat.open(QIODevice::WriteOnly))
    {
        QVariantMap vm;
        QVariantMap vm2;

        variant_map_archive ar;
        ar.io = PUP_OUT;
        
        for (int i = 0; i < lwidgets.size(); ++i)
        {
            ar.top_level = &vm;
            pack_unpack(ar, song_fname, var_info("song_fname", QVariantMap()));
            pack_unpack(ar,
                        *lwidgets[i],
                        var_info("Light_Widget[" + QString::number(i) + "]", QVariantMap()));

            ar.top_level = &vm2;
            auto awidgets = lwidgets[i]->get_automation_widgets();
            for (int j = 0; j < awidgets.size(); ++j)
            {
                pack_unpack(ar, awidgets[j]->recorded_data, var_info("Light_Widget[" + QString::number(i) + "].data(" + QString::number(j) + ")", QVariantMap()));
            }
        }
        QJsonDocument doc = QJsonDocument::fromVariant(vm);
        QJsonDocument doc2 = QJsonDocument::fromVariant(vm2);
        json_file.write(doc.toJson());
        json_file_dat.write(doc2.toBinaryData());
    }
}

void Main_Window::load_config_from_file(const QString & filename)
{
    if (filename.isEmpty())
        return;
    QFile json_file(filename);
    QFile json_file_dat(filename + ".dat");
    if (json_file.open(QIODevice::ReadOnly) && json_file_dat.open(QIODevice::ReadOnly))
    {
        QByteArray fdata(json_file.readAll());
        QByteArray fdata2(json_file_dat.readAll());

        QJsonDocument doc = QJsonDocument::fromJson(fdata);
        QJsonDocument doc2 = QJsonDocument::fromBinaryData(fdata2);

        QVariantMap vm = doc.toVariant().toMap();
        QVariantMap vm2 = doc2.toVariant().toMap();

        variant_map_archive ar;
        ar.io = PUP_IN;
        for (int i = 0; i < lwidgets.size(); ++i)
        {
            ar.top_level = &vm;
            pack_unpack(ar, song_fname, var_info("song_fname", QVariantMap()));
            pack_unpack(ar,
                        *lwidgets[i],
                        var_info("Light_Widget[" + QString::number(i) + "]", QVariantMap()));

            ar.top_level = &vm2;
            auto awidgets = lwidgets[i]->get_automation_widgets();
            for (int j = 0; j < awidgets.size(); ++j)
            {
                pack_unpack(ar, awidgets[j]->recorded_data, var_info("Light_Widget[" + QString::number(i) + "].data(" + QString::number(j) + ")", QVariantMap()));
            }
        }
    }
}

void Main_Window::on_actionSave_Configuration_triggered()
{
    QString fn = QFileDialog::getSaveFileName(this,
                                              "Save Config File",
                                              "/home/daniel/Documents/Code/XMas_Lights/config",
                                              "Config Files (*.json)");
    save_config_to_file(fn);
}

void Main_Window::on_actionLoad_Configuration_triggered()
{
    QString fn = QFileDialog::getOpenFileName(this,
                                              "Open Config File",
                                              "/home/daniel/Documents/Code/XMas_Lights/config",
                                              "Config Files (*.json)");
    load_config_from_file(fn);
    on_actionRebuild_Light_Data_triggered();
}

FMOD::System * Main_Window::get_audio_system()
{
    return audio_system_;
}

void Main_Window::on_actionArm_Record_triggered()
{
    keys_.clear();
    for (int i = 0; i < lwidgets.size(); ++i)
    {
        auto awidgets = lwidgets[i]->get_automation_widgets();
        for (int j = 0; j < awidgets.size(); ++j)
        {
            Automation_Info ainf = awidgets[j]->get_automation_info();
            if (ainf.key_code != 0)
                keys_[ainf.key_code] = false;
            awidgets[j]->recorded_data.resize(get_sample_count());
        }
    }
}
