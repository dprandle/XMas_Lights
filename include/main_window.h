#pragma once

#include <QMainWindow>
#include <QElapsedTimer>
#include <QTimer>
#include <QAbstractSocket>

const uint8_t LIGHT_COUNT = 6;
const uint8_t LIGHT_LEVEL_MIN = 0;
const uint8_t LIGHT_LEVEL_MAX = 10;
const uint8_t LIGHT_PADDING = 20;
const double LIGHT_SCALE_FACTOR = 0.37;
const std::string PACKET_ID_PLAY = "Play";
const std::string PACKET_ID_PAUSE = "Pause";
const std::string PACKET_ID_STOP = "Stop";
const std::string PACKET_ID_CONFIGURE = "Configure";
const uint8_t PACKET_ID_SIZE = 4;

struct Packet_ID
{
    union
    {
        uint32_t hashed_id;
        uint8_t data[PACKET_ID_SIZE];
    };
};

namespace Ui
{
class Main_Window;
}

namespace FMOD
{
class System;
class Sound;
class Channel;
} // namespace FMOD

struct Light_Info
{
    std::vector<uint8_t> ms_data;
};

struct Automation_Data
{
    union
    {
        uint32_t sample_cnt;
        uint8_t data[4];
    };
    Light_Info lights[LIGHT_COUNT];
};

uint32_t hash_id(const std::string & strng);

class QSpinBox;
class Light_Strand_Widget;
class QGraphicsScene;
class QGraphicsPixmapItem;
class QTcpSocket;

class Main_Window : public QMainWindow
{
    Q_OBJECT

  public:
    explicit Main_Window(QWidget * parent = 0);
    ~Main_Window();

    void initialize();

    void terminate();

    static Main_Window & inst();

    Automation_Data * get_automation_data();

    void run_sample();

    void connect_to_edison();

    void disconnect_from_edison();

  public slots:
    void on_actionOpen_Audio_File_triggered();

    void on_actionRebuild_Light_Data_triggered();

    void on_actionStart_Simulation_triggered();

    void on_actionPause_Simulation_triggered();

    void on_actionStop_Simulation_triggered();

    void on_actionEstablish_Connection_triggered();

  private:
    void socket_data_available();
    void socket_error(QAbstractSocket::SocketError);
    void socket_connected();

    void setup_light_graphics_();

    static Main_Window * this_;
    Ui::Main_Window * ui;
    FMOD::System * audio_system_;
    FMOD::Sound * sound_;
    FMOD::Channel * channel_;
    QSpinBox * sample_count_;
    QGraphicsScene * scene_;
    Automation_Data light_data_;
    QVector<Light_Strand_Widget*> lwidgets;
    QVector<QGraphicsPixmapItem*> lights;
    QElapsedTimer sim_timer_;
    QTimer sim_sig_timer_;
    uint32_t pause_elapsed_;
    QTcpSocket * ed_socket_;
};

#define mwin Main_Window::inst()