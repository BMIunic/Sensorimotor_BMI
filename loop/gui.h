#ifndef Gui_H
#define Gui_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLabel>
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <QMessageBox>
#include <QtCharts/QChartGlobal>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QGroupBox>
#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include <QDateTime>
#include <QVector>
#include <QDir>
#include <QThread>
#include <QApplication>
#include <QObject>
#include <cmath>
#include <QListWidget>
#include <QColor>
#include <QCloseEvent>
#include <QDebug>
#include "protocole.h"
#include "spkandbehav.h"
#include "chart.h"
#include "defines.h"
#include "cbsdk.h"
#include "time.h"

extern QAtomicInteger<unsigned int> LowerThd;
extern QAtomicInteger<unsigned int> UpperThd;
extern QAtomicInteger<unsigned int> WaitingPRD;
extern QAtomicInteger<unsigned int> SessionPRD;
extern QAtomicInteger<unsigned int> Mouse;
extern QAtomicInteger<unsigned int> Prot;
extern QAtomicInteger<unsigned int> Session;
extern QAtomicInteger<unsigned int> SpkCount;
extern QAtomicInteger<unsigned int> FrameIdx;
extern QAtomicInteger<unsigned int> LastLick;
extern QAtomicInteger<unsigned int> Active;
extern QAtomicInteger<unsigned int> MeanFR_active; // 0: mean OFF; 1: mean ON
extern QAtomicInteger<unsigned int> Water; // 0: controled by algorithm; 1: free reward; 2: flush
extern QAtomicInteger<unsigned int> RewardLowerThd;
extern QAtomicInteger<unsigned int> RewardUpperThd;
extern QAtomicInteger<int> RewardCounter;
extern QAtomicInteger<unsigned int> RoutineTime;
extern QAtomicInteger<int> RandProtocolID;
extern QVector<int> Channel;
extern QVector<int> Unit;
extern QMutex paramMut;
extern QString dataFolder;
extern Protocole param;
extern std::clock_t MeanFR_StartTime;
extern std::clock_t Session_StartTime;
extern float MeanFR;
extern float SdFR;
extern int cameraAOUT;
extern int waterAOUT;
extern int lickDIN;
extern int maxSpkCount;
extern int binsizeFR;

class Gui : public QWidget
{
    Q_OBJECT
private:
    /// Widgets
    QPushButton *lightONButton;
    QPushButton *lightOFFButton;
    QPushButton *sessionButton;
    QPushButton *protocolIdButton;
    QPushButton *rewardONButton;
    QPushButton *flushONButton;
    QPushButton *oneRewardONButton;
    QPushButton *meanrateCALCButton;
    QComboBox *m_mouseComboBox;
    QComboBox *m_protComboBox;
    QCheckBox *m_cameraCheckBox;
    QCheckBox *m_controlsCheckBox;
    QCheckBox *m_channelListCheckbox;

    QListWidget *m_channelListWidget;
    QListWidget *m_unitListWidget;
    QDoubleSpinBox *m_blwSpinBox;
    QDoubleSpinBox *m_abvSpinBox;
    QDoubleSpinBox *m_blwSDSpinBox;
    QDoubleSpinBox *m_abvSDSpinBox;
    QDoubleSpinBox *m_blwDispSpinBox;
    QDoubleSpinBox *m_abvDispSpinBox;
    QSpinBox *m_blwRewardSpinBox;
    QSpinBox *m_abvRewardSpinBox;

    QLabel *label_mean;
    QLabel *label_std;
    QLabel *label_reward;

    QFrame *line;
    QFrame *line2;

    QImage *baseline_image;
    QLabel *imageLabel;
    Chart *chart;
    QChartView *m_chartView;
    QGroupBox *groupBox;

    /// Layouts
    QFormLayout *ListWidgetLayout;
    QFormLayout *SpinboxLayout2;
    QFormLayout *SpinboxLayout3;
    QFormLayout *ComboLayout;
    QVBoxLayout *ControlLayout;
    QHBoxLayout *ControlLayout2;
    QHBoxLayout *ControlLayout3;
    QHBoxLayout *ControlLayout4;
    QHBoxLayout *WaterLayout;
    QHBoxLayout *CameraLayout;
    QHBoxLayout *SecondLevel;
    QVBoxLayout *FirstLevel;

    /// Data structures
    Protocole currentParam;
    QList<QImage> FrameList;
    QList<QImage> GuiFrameList;
    QList<int> mouseIDlist;
    QList<int> sessionIDlist;
    QStringList ProtocolesList;
    QString currentDataFolder;
    int currentSession;
    int currentMouse;
    bool currentIsRewarded;
    double currentTimeout;

    /// Time
    QTimer m_timer;
    uint StartTime_s;

    /// Spiking and display threads
    QThread *spkandbehav_thread;
    SpkAndBehav *spkandbehav;

    /// NSP output init
    cbSdkResult res;

    /// Waiting period active flag defination
    int meanFR_active_before;

    /// Session period active flag defination
    int session_active_before;

public:
    explicit Gui(QWidget *parent = 0);
    ~Gui();
    void startSession();
    void stopSession();
    void ListProtocoles();
    void ListMice();
    void GetLastSession();

public Q_SLOTS:
    void updateChannel(const QModelIndex &);
    void updateUnit(const QModelIndex &);
    void allChannels(bool allChannels_ON);
    void calc_meanFR(bool meanCALC_ON);
    void updateLowerThd(double lower);
    void updateUpperThd(double upper);
    void updateLowerSDThd(double lowerSD);
    void updateUpperSDThd(double upperSD);
    void updateLowerDisplay(double lower);
    void updateUpperDisplay(double upper);
    void updateRewardLowerThd(int);
    void updateRewardUpperThd(int);
    void setMouse();
    void LoadProtocole();
    void LoadNullProtocole();
    void sessionON_OFF(bool session_ON);
    void protocolIdButton_clicked();
    void rewardON_OFF(bool reward_ON);
    void flushON_OFF(bool flush_ON);
    void oneFreeRewardON();
    void update_displays();
    void updateChannelUnitComment(QVector<int> Unit, QVector<int> Channel);
    void initWaiting();
    void closeEvent(QCloseEvent);
};

#endif //Gui_H
