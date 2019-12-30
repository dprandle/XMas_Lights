#pragma once

#include <QMainWindow>
#include <QElapsedTimer>
#include <QTimer>
#include <QAbstractSocket>
#include <cmath>

const uint8_t LIGHT_COUNT = 6;
const double LIGHT_LEVEL_MIN = 0.0;
const double LIGHT_LEVEL_MAX = 100.0;

const uint8_t LIGHT_PADDING = 20;
const double LIGHT_SCALE_FACTOR = 0.37;
const std::string PACKET_ID_PLAY = "Play";
const std::string PACKET_ID_PAUSE = "Pause";
const std::string PACKET_ID_STOP = "Stop";
const std::string PACKET_ID_CONFIGURE = "Configure";
const uint8_t PACKET_ID_SIZE = 4;
const double EPS = 0.000001;

template <class T>
bool fcompare(T left, T right)
{
    return fabs(left - right) <= EPS;
}

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

struct Filter_State_Info
{
    double release;
    double hold;
    double frame_contrib_hold;
    bool open_squelch;
    QVector<double> window_rms_vals;
};

struct Automation_State_Info
{
    double ramp_down;
    double ramp_up;
    double hold;
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
class QSlider;

class Main_Window : public QMainWindow
{
    Q_OBJECT

  public:
    explicit Main_Window(QWidget * parent = 0);
    ~Main_Window();

    void initialize();

    void terminate();

    void normalize_samples(int16_t * channel, QVector<double> & dest, uint32_t offset=0, uint32_t subsize=0);

    static Main_Window & inst();

    Automation_Data * get_automation_data();

    void run_sample();

    void connect_to_edison();

    void disconnect_from_edison();

    void save_config_to_file(const QString & filename);

    void load_config_from_file(const QString & filename);

    uint32_t get_sample_count();

    void load_audio_data();

    FMOD::System * get_audio_system();

    bool is_key_pressed(int key);

    void process_sample(int index);

    void slider_pressed();

    void slider_released();

    void slider_value_changed();

  public slots:
    void on_actionOpen_Audio_File_triggered();

    void on_actionRebuild_Light_Data_triggered();

    void on_actionStart_Simulation_triggered();

    void on_actionPause_Simulation_triggered();

    void on_actionStop_Simulation_triggered();

    void on_actionEstablish_Connection_triggered();

    void on_actionSave_Configuration_triggered();

    void on_actionLoad_Configuration_triggered();

    void on_actionArm_Record_triggered();

    protected:

    void keyPressEvent(QKeyEvent *event);

    void keyReleaseEvent(QKeyEvent *event);

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
    uint32_t sample_count;
    bool slider_pressed_;
    bool was_playing_before_moving_slider_;

    QMap<int, bool> keys_;

    QGraphicsScene * scene_;
    QSlider * play_slider_;
    Automation_Data light_data_;
    QVector<Light_Strand_Widget*> lwidgets;
    QVector<QGraphicsPixmapItem*> lights;
    QElapsedTimer sim_timer_;
    QTimer sim_sig_timer_;
    uint32_t pause_elapsed_;
    QTcpSocket * ed_socket_;
    QString song_fname;
};

#define mwin Main_Window::inst()