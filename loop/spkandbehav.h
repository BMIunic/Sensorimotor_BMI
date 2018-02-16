#ifndef SpkAndBehav_H
#define SpkAndBehav_H

#include "defines.h"
#include "protocole.h"
#include <iostream>
#include <stdio.h>
#include <ctime>
#include <stdint.h>
#include <string>
#include <random>
#include <QDebug>
#include <QObject>
#include <QCoreApplication>
#include <QMutex>
#include <QTimer>
#include <QEventLoop>
#include <QQueue>
#include <QtMath>
#include <QtGlobal>
#include <QThread>
#include <QtGui/QImage>
#include <QString>
#include <QDir>
#include <QAtomicInteger>
#include <QVector>
#include <QDateTime>
#include <QElapsedTimer>
#include <QTextStream>

extern QAtomicInteger<unsigned int> LowerThd;
extern QAtomicInteger<unsigned int> UpperThd;
extern QAtomicInteger<unsigned int> WaitingPRD;
extern QAtomicInteger<unsigned int> SessionPRD;
extern QAtomicInteger<unsigned int> Mouse;
extern QAtomicInteger<unsigned int> Prot;
extern QAtomicInteger<unsigned int> Session;
extern QAtomicInteger<unsigned int> SpkCount;
extern QAtomicInteger<unsigned int> FrameIdx;
extern QAtomicInteger<unsigned int> DisplayedBfr;
extern QAtomicInteger<unsigned int> LastLick;
extern QAtomicInteger<unsigned int> LastReward;
extern QAtomicInteger<unsigned int> Active; // 0: program OFF; 1: program ON, session OFF; 2: session ON, camera OFF; 3: camera ON
extern QAtomicInteger<unsigned int> MeanFR_active; // 0: mean OFF; 1: mean ON
extern QAtomicInteger<unsigned int> Water; // 0: controled by algorithm; 1: free reward; 2: flush
extern QAtomicInteger<unsigned int> RewardLowerThd;
extern QAtomicInteger<unsigned int> RewardUpperThd;
extern QAtomicInteger<int> RewardCounter;
extern QAtomicInteger<unsigned int> RoutineTime;
extern QAtomicInteger<int> RandProtocolID;
extern QAtomicInteger<unsigned int> Flip;
extern QVector<int> Channel;
extern QVector<int> Unit;
extern QString dataFolder;
extern QMutex paramMut;
extern Protocole param;
extern std::clock_t MeanFR_StartTime;
extern std::clock_t Session_StartTime;
extern int cameraAOUT;
extern int waterAOUT;
extern int lickDIN;
extern int maxSpkCount;
extern int binsizeFR;
extern float MeanFR;
extern float SdFR;

#if DEMO == 0
    #include <TCHAR.h>
    #include <windows.h>
    #include <conio.h>
    #include "cbsdk.h"
    #include "alp.h"
#endif
#if DEMO == 1
    #include <QDateTime>
#endif

class SpkAndBehav : public QObject
{
    Q_OBJECT

public:
    explicit SpkAndBehav(QObject *parent = 0); // read all protocoles to know the corresponding parameters
#if DEMO == 0
    static void LogCallback(UINT32, const cbSdkPktType type, const void* pEventData, void*);
#endif
    static QList<unsigned int> SpkList;
    static QList<unsigned int> LickList;
    static int currentLickDIN;
    int pace_counter=0;

private:
    unsigned int Lick;
    unsigned int rewardStrength;
    unsigned int timeOut;
    unsigned int LickHist;
    unsigned int ActiveBefore;
    unsigned int WaterBefore;
    unsigned int frameDuration;
    unsigned int frameIllumination;

    int rewardThd;
    int rewardCounter;
    int meanFR_active_flag;
    int currentCameraAOUT;
    int currentWaterAOUT;
    int counter;
    int frame_count;

    double rewardInterval;
    long brightness;

    uint young_spk_count;
    uint CurrentTime;
    uint start_wait_time;
    uint StartTime_s;
    uint int_time = 0;

    QString input;
    QString currentSessionFileName;
    QString currentDataFolder;

    QElapsedTimer timer;
    QTimer* m_timer;
    #if DEMO == 1
        QTimer* s_timer;
    #endif

    QVector <int> spkcount_vector;
    QVector<uint32_t>BufferList;

    QList<QList<QImage>> FrameList;
    QFile* inputFile;
    QTextStream* inputStream;

    Protocole currentParam;

    uint32_t FrameIdx_bfr_before = -1;

    void initSession(); // when active = 1: update reward thresholds, an log the properties of the trials. For that
    void initControlSession();
    void stopSession(); // stop the session when active = 0
    void closeProgram();
    void one_waterReward();
    void continuous_waterReward();
    void onefree_waterReward();
    void start_flush();
    void stop_water();
    void start_cameraTrig();
    void stop_cameraTrig();
    void masking_light();

#if DEMO == 0
        cbSdkResult res;
        ALP_ID nDevId, AlpLedId; // Projector
        QVector<ALP_ID> nSeqId_list;
#endif

public Q_SLOTS:
    void routine();
#if DEMO == 1
    void add_fake_spike();
#endif

};

#endif // SpkAndBehav_H
