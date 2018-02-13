#include <QAtomicInt>
#include <QVector>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QDebug>
#include "gui.h"

/// Decleration of Global Variables
QAtomicInteger<unsigned int> Mouse; // mouse number
QAtomicInteger<unsigned int> Session; // session number
QAtomicInteger<unsigned int> Prot; // protocole number.
QAtomicInteger<unsigned int> Active; // gui and session active. Only GUI: 1. Session Also: 2.
QAtomicInteger<unsigned int> MeanFR_active; // 0: mean OFF; 1: mean ON
QAtomicInteger<unsigned int> LastLick; // A lick has to be processed/ was not yet processed
QAtomicInteger<unsigned int> LastReward; // A lick has to be processed/ was not yet processed
QAtomicInteger<unsigned int> SpkCount; // spiking count in time window
QAtomicInteger<unsigned int> FrameIdx; // Index of the current frame, to keep both displays up to date
QAtomicInteger<unsigned int> LowerThd; // Lower firing rate boundary for feedback
QAtomicInteger<unsigned int> UpperThd; // Upper firing rate boundary for feedback
QAtomicInteger<unsigned int> RewardLowerThd; // Lower reward boundary
QAtomicInteger<unsigned int> RewardUpperThd; // Upper reward boundary
QAtomicInteger<unsigned int> WaitingPRD; // waiting period in minutes for calculating the baseline activity.
QAtomicInteger<unsigned int> SessionPRD; // total duration of one session in minutes
QAtomicInteger<unsigned int> RoutineTime; // varaible holding the routine call value
QAtomicInteger<unsigned int> Water; // 0: controled by algo; 1: free reward; 2: flush
QAtomicInteger<unsigned int> Flip;

QAtomicInteger<int> RandProtocolID; // A random varaible telling the user which protocol to choose
QAtomicInteger<int> RewardCounter; // To count the number of rewards and display on the GUI

QVector <int> Channel; // selected channel number
QVector <int> Unit; // selected unit number

QMutex paramMut; // protects all parameter updates

std::clock_t MeanFR_StartTime;
std::clock_t Session_StartTime;

float MeanFR = 0; // Mean firing rate of recorded cells.
float SdFR = 0; // standard deviation of the firing rate of recorded cells.

Protocole param = Protocole(); // the current protocole

QString input;
QString dataFolder;

int cameraAOUT;
int waterAOUT;
int lickDIN;
int maxSpkCount;
int binsizeFR;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Gui G;
    G.resize(720, 480);
    G.show();

    return a.exec();
}
