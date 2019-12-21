#include <main_window.h>
#include <ui_main_window.h>
#include <timer_widget.h>

#include <iostream>
#include <algorithm>

#include <fmod/fmod.hpp>
#include <fmod/fmod_errors.h>

//#include <DspFilters/Butterworth.h>
//#include <DspFilters/Filter.h>
#include <DspFilters/Dsp.h>

#include <QTcpSocket>
#include <QFileDialog>
#include <QSpinBox>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QGraphicsScene>
#include <QHostAddress>

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
      sample_count_(new QSpinBox),
      scene_(new QGraphicsScene),
      pause_elapsed_(0),
      ed_socket_(new QTcpSocket)
{
    this_ = this;
    ui->setupUi(this);
    sample_count_->setMaximum(INT_MAX);
    ui->toolBar->insertWidget(ui->actionRebuild_Light_Data, sample_count_);
    lwidgets.push_back(ui->widget_s1);
    lwidgets.push_back(ui->widget_s2);
    lwidgets.push_back(ui->widget_s3);
    lwidgets.push_back(ui->widget_s4);
    lwidgets.push_back(ui->widget_s5);
    lwidgets.push_back(ui->widget_s6);
    setup_light_graphics_();
    connect(&sim_sig_timer_, &QTimer::timeout, this, &Main_Window::run_sample);

    connect(ed_socket_, &QTcpSocket::readyRead, this, &Main_Window::socket_data_available);
    connect(ed_socket_, qOverload<QAbstractSocket::SocketError>(&QTcpSocket::error), this, &Main_Window::socket_error);
    connect(ed_socket_, &QTcpSocket::connected, this, &Main_Window::socket_connected);
}

void Main_Window::run_sample()
{
    uint32_t index = pause_elapsed_ + sim_timer_.elapsed();
    if (index > light_data_.sample_cnt)
    {
        on_actionStop_Simulation_triggered();
        return;
    }

    for (uint8_t li = 0; li < LIGHT_COUNT; ++li)
    {
        int light_val = light_data_.lights[li].ms_data[index];
        double opacity = light_val / 10.0;
        lights[li]->setOpacity(opacity);
    }
}

void Main_Window::socket_data_available()
{}

void Main_Window::socket_error(QAbstractSocket::SocketError err)
{
    statusBar()->showMessage("Socket error!: " + ed_socket_->errorString(),5000);
}

void Main_Window::socket_connected()
{
    qDebug() << "Connected!!";
    statusBar()->showMessage("Connected to " + ed_socket_->peerAddress().toString() + ":10000!!!",5000);
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
    statusBar()->showMessage("Disconnecting from " + ed_socket_->peerAddress().toString() + ":10000", 5000);
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
    if (!sim_sig_timer_.isActive())
    {
        Packet_ID id = {};
        id.hashed_id = hash_id(PACKET_ID_PLAY);
        ed_socket_->write((char*)id.data, PACKET_ID_SIZE);

        sim_timer_.restart();
        sim_sig_timer_.start();
    }
    ui->actionStart_Simulation->setChecked(true);
    ui->actionPause_Simulation->setChecked(false);
}

void Main_Window::on_actionPause_Simulation_triggered()
{
    if (sim_sig_timer_.isActive())
    {
        Packet_ID id = {};
        id.hashed_id = hash_id(PACKET_ID_PAUSE);
        ed_socket_->write((char*)id.data, PACKET_ID_SIZE);

        sim_sig_timer_.stop();
        pause_elapsed_ += sim_timer_.elapsed();
    }
    ui->actionPause_Simulation->setChecked(true);
    ui->actionStart_Simulation->setChecked(false);
}

void Main_Window::on_actionStop_Simulation_triggered()
{
    pause_elapsed_ = 0;
    sim_sig_timer_.stop();

    Packet_ID id = {};
    id.hashed_id = hash_id(PACKET_ID_STOP);
    ed_socket_->write((char*)id.data, PACKET_ID_SIZE);

    ui->actionPause_Simulation->setChecked(false);
    ui->actionStart_Simulation->setChecked(false);
    for (uint8_t li = 0; li < LIGHT_COUNT; ++li)
        lights[li]->setOpacity(0.0);
}

void Main_Window::on_actionRebuild_Light_Data_triggered()
{
    on_actionStop_Simulation_triggered();

    light_data_.sample_cnt = sample_count_->value();
    for (uint8_t i = 0; i < LIGHT_COUNT; ++i)
    {
        auto twidgets = lwidgets[i]->get_timer_widgets();
        light_data_.lights[i].ms_data.resize(light_data_.sample_cnt);
        for (uint32_t si = 0; si < light_data_.sample_cnt; ++si)
        {
            double frame_inf = 0.0;
            for (uint16_t ti = 0; ti < twidgets.size(); ++ti)
            {
                Timer_Info tinf = twidgets[ti]->get_timer_info();

                // If sample is before start delay or after duration disregard
                if (si < tinf.start_delay || si > tinf.duration)
                    continue;

                // cur_ms should be the elapsed milliseconds in this period
                double cur_ms = si % uint32_t(tinf.period);
                //std::cout << "Cur milliseconds: " << cur_ms << std::endl;

                double ramp_up_perc =
                    (cur_ms - (tinf.period - (tinf.ramp_up + 1))) / (tinf.ramp_up + 1);
                ramp_up_perc = std::clamp(ramp_up_perc, 0.0, 1.0);
                //std::cout << "Ramp up perc: " << ramp_up_perc << std::endl;

                double ramp_down_and_hold = (cur_ms - tinf.hold) / (tinf.ramp_down + 1);
                ramp_down_and_hold = std::clamp(ramp_down_and_hold, 0.0, 1.0);
                //std::cout << "Ramp down and hold perc: " << ramp_down_and_hold << std::endl;

                //std::cout << "Increase from ramp up: " << ramp_up_perc * LIGHT_LEVEL_MAX << std::endl;
                //std::cout << "Increase from ramp down and hold: " << (1.0 - ramp_down_and_hold) * LIGHT_LEVEL_MAX << std::endl;
                frame_inf += ramp_up_perc * LIGHT_LEVEL_MAX;
                frame_inf += (1.0 - ramp_down_and_hold) * LIGHT_LEVEL_MAX;
            }
            uint8_t rounded_int = std::lround(frame_inf);
            rounded_int = std::clamp(rounded_int, LIGHT_LEVEL_MIN, LIGHT_LEVEL_MAX);
            //std::cout << "Rounded int: " << int(rounded_int) << std::endl;
            std::cout << std::endl;
            light_data_.lights[i].ms_data[si] = rounded_int;
        }
    }

    Packet_ID id = {};
    id.hashed_id = hash_id(PACKET_ID_CONFIGURE);
    ed_socket_->write((char*)id.data, PACKET_ID_SIZE);
    ed_socket_->write((char*)light_data_.data, PACKET_ID_SIZE);
    for (uint32_t i = 0; i < LIGHT_COUNT; ++i)
    {
        ed_socket_->write((char*)light_data_.lights[i].ms_data.data(), light_data_.sample_cnt);
    }
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

void Main_Window::on_actionOpen_Audio_File_triggered()
{
    QString fn = QFileDialog::getOpenFileName(
        this, "Open Audio File", "../import", "Audio Files (*.wav *.mp3)");
    audio_system_->createSound(fn.toUtf8().constData(), FMOD_DEFAULT, nullptr, &sound_);
    //audio_system_->playSound(sound_, nullptr, false, &channel_);

    FMOD_SOUND_TYPE t;
    FMOD_SOUND_FORMAT f;
    int channel_count = 0;
    int bits_per_sample = 0;
    sound_->getFormat(&t, &f, &channel_count, &bits_per_sample);
    int bytes_per_sample = bits_per_sample / 8;

    unsigned int sample_count = 0;
    unsigned int ms_count = 0;
    sound_->getLength(&ms_count, FMOD_TIMEUNIT_MS);
    sound_->getLength(&sample_count, FMOD_TIMEUNIT_PCM);

    unsigned int sample_rate = (uint64_t(sample_count) * 1000) / ms_count;

    void * data = nullptr;
    unsigned int data_length = 0;
    void * data_wrap = nullptr;
    unsigned int data_wrap_length = 0;

    uint64_t byte_count = sample_count * channel_count * bytes_per_sample;
    sound_->lock(0, byte_count, &data, &data_wrap, &data_length, &data_wrap_length);
    int16_t * data_fmt = reinterpret_cast<int16_t *>(data);
    int16_t * channels[2];
    int16_t * filtered_interleaved_audio = new int16_t[sample_count * channel_count];

    channels[0] = new int16_t[sample_count];
    channels[1] = new int16_t[sample_count];

    for (unsigned int j = 0; j < sample_count; ++j)
    {
        channels[0][j] = data_fmt[j];
        channels[1][j] = data_fmt[j + 1];
    }

    Dsp::SimpleFilter<Dsp::Butterworth::BandPass<100>, 2> filter;
    filter.setup(10, sample_rate, 500, 300);
    filter.process(sample_count, channels);

    for (unsigned int j = 0; j < sample_count; ++j)
    {
        filtered_interleaved_audio[j] = channels[0][j];
        filtered_interleaved_audio[j + 1] = channels[1][j];
    }

    FMOD::Sound * snd = nullptr;
    FMOD_CREATESOUNDEXINFO info = {};
    info.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
    info.length = byte_count;
    info.numchannels = 2;
    info.defaultfrequency = 44100;
    info.format = FMOD_SOUND_FORMAT_PCM16;
    info.suggestedsoundtype = FMOD_SOUND_TYPE_WAV;
    FMOD_RESULT res =
        audio_system_->createSound(reinterpret_cast<const char *>(filtered_interleaved_audio),
                                   FMOD_OPENMEMORY | FMOD_CREATESAMPLE | FMOD_OPENRAW,
                                   &info,
                                   &snd);
    if (res != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", res, FMOD_ErrorString(res));
    }
    audio_system_->playSound(snd, nullptr, false, &channel_);

    using namespace std;
    cout << "Channel count: " << channel_count << endl;
    cout << "Bits per sample: " << bits_per_sample << endl;
    cout << "Bytes per sample: " << bytes_per_sample << endl;
    cout << "Sample rate: " << sample_rate << endl;
    cout << "Sample count: " << sample_count << endl;
    cout << "Length (s): " << ms_count / 60000.0 << endl;
    cout << "Byte count: " << byte_count << endl;
    cout << "Data length: " << data_length << endl;
    cout << "Data wrap length: " << data_wrap_length << endl;
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

    //scene_->addRect()
    ui->graphicsView->setScene(scene_);
}
