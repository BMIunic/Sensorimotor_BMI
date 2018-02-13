/****************************************************************************************
/IMAGER_v2.0 (2016)
/livethread.h
/Deceralation of thread class livethread inherited from class QThread. This class contains
/decleration of thread fucntion run and variable for stopping the thread bool Stop.
/Author: Aamir Abbasi
*****************************************************************************************/

#ifndef LIVETHREAD_H
#define LIVETHREAD_H
#include <QObject>
#include <QMutex>
#include <QThread>
#include <QtCore>
#include <QDebug>
#include <QString>
#include <QMessageBox>
#include <QCoreApplication>
#include <pylon/PylonIncludes.h>
#include <pylon/usb/BaslerUsbInstantCamera.h>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif

// Namespace for using pylon objects.
using namespace Pylon;
using namespace GenApi;

// Settings for using Basler USB cameras.
typedef Pylon::CBaslerUsbInstantCamera Camera_t;
using namespace Basler_UsbCameraParams;

// The camera specific grab result smart pointer.
typedef Camera_t::GrabResultPtr_t GrabResultPtr_t;

// These global variables are defined in the main.cpp file.
extern QString exposure;
extern QString xBin;
extern QString yBin;

class LiveThread : public QObject
{
    Q_OBJECT
public:
    explicit LiveThread(QObject *parent = 0);
    void requestWork();
    void abort();

private:
    bool _working;
    bool _abort;
    QMutex mutex;

signals:
    /**
     * @brief This signal is emitted when the Worker request to Work
     * @sa requestWork()
     */
    void workRequested();
    /**
     * @brief This signal is emitted when counted value is changed (every sec)
     */
    void valueChanged(const QString &value);
    /**
     * @brief This signal is emitted when process is finished (either by counting 60 sec or being aborted)
     */
    void finished();

public slots:
    void doWork();

};

#endif // LIVETHREAD_H
