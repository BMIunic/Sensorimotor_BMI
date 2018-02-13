/****************************************************************************************
/IMAGER_v2.0 (2016)
/imagesequence.h
/Deceralation of worker thread inherited from class QThread for acquisition of image
/sequence.
/Author: Aamir Abbasi
*****************************************************************************************/

#ifndef IMAGESEQUENCE_H
#define IMAGESEQUENCE_H
#include <QDir>
#include <QString>
#include <QStringList>
#include <QMessageBox>
#include <QDebug>
#include <qmath.h>
#include <iostream>
#include "nsp.h"
#include "imagegrab.h"
#include "fileio.h"
#include "mainwindow.h"
#include <pylon/PylonIncludes.h>
#include <pylon/usb/BaslerUsbInstantCamera.h>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif

// Namespace for using pylon objects.
using namespace Pylon;
using namespace GenApi;

// Namespace for using cout (C++ objects).
using namespace std;

// Settings for using Basler USB cameras.
typedef Pylon::CBaslerUsbInstantCamera Camera_t;
using namespace Basler_UsbCameraParams;

// The camera specific grab result smart pointer.
typedef Camera_t::GrabResultPtr_t GrabResultPtr_t;

class ImageSequence : public QObject
{
    Q_OBJECT
public:
    explicit ImageSequence(QObject *parent = 0);
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

#endif // IMAGESEQUENCE_H
