/****************************************************************************************
/IMAGER_v2.0 (2016)
/livethread.cpp
/This file contains function implementing live imaging using BASLER ace
/USB 3.0 CCD camera on a sepereate thread. The thread can be start and stopped using Live
/and Stop Live push buttons on the GUI.
/Author: Aamir Abbasi
*****************************************************************************************/

#include "livethread.h"

LiveThread::LiveThread(QObject *parent) :
    QObject(parent)
{
    _working = false;
    _abort = false;
}

void LiveThread::requestWork()
{
    mutex.lock();
    _working = true;
    _abort = false;
    mutex.unlock();
    doWork();

    emit workRequested();
}

void LiveThread::abort()
{
    if (_working) {
        _abort = true;
        qDebug()<<"Request worker aborting in Thread "<<thread()->currentThreadId();
    }
}

/* This function performes continous grabbing of images in real time. The grabbed
   images are displyaed in a Pylon GUI */
void LiveThread::doWork()
{
    qDebug()<< "Do work";

    QMutex mutex;
    mutex.lock();
    // Before using any pylon methods, the pylon runtime must be initialized.
    PylonInitialize();

    try
    {
        // Create an instant camera object with the camera device found first.
        CInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());

        // The parameter MaxNumBuffer can be used to control the count of buffers
        // allocated for grabbing. The default value of this parameter is 10.
        camera.MaxNumBuffer = 10;

        // Open the camera.
        camera.Open();

        // Change the exposure time of the camera
        // Change image bit depth to 12bits from default 8bits.
        GenApi::INodeMap& nodemap = camera.GetNodeMap();
        CEnumerationPtr pixelFormat ( nodemap.GetNode("PixelFormat"));
        pixelFormat->FromString("Mono12");

        // Set camera exposure mode back to timed
        CEnumerationPtr exposureMode ( nodemap.GetNode("ExposureMode"));
        exposureMode->FromString("Timed");
        CFloatPtr exposureTime (nodemap.GetNode("ExposureTime"));
        exposureTime->SetValue(exposure.toFloat()*1000.0);

        // Set horizontal and vertical bin size for binning the image
        CIntegerPtr horizontalBin (nodemap.GetNode("BinningHorizontal"));
        horizontalBin->SetValue(xBin.toInt(nullptr, 10));
        CIntegerPtr verticalBin (nodemap.GetNode("BinningVertical"));
        verticalBin->SetValue(yBin.toInt(nullptr, 10));

        // Deactivate trigger.
        CEnumerationPtr triggerMode( nodemap.GetNode("TriggerMode"));
        triggerMode->FromString( "Off");

        // Set camera gain and black level
        CFloatPtr gain (nodemap.GetNode("Gain"));
        gain->SetValue(0.00);
        CFloatPtr blackLevel (nodemap.GetNode("BlackLevel"));
        blackLevel->SetValue(0.00);

        // Start the grabbing of c_countOfImagesToGrab images.
        // The camera device is parameterized with a default configuration which
        // sets up free-running continuous acquisition.
        camera.StartGrabbing();

        // This smart pointer will receive the grab result data.
        CGrabResultPtr ptrGrabResult;

        // Camera.StopGrabbing() is called automatically by the RetrieveResult() method
        // when c_countOfImagesToGrab images have been retrieved.

        while (camera.IsGrabbing())
        {
            QCoreApplication::processEvents();
            if(_abort == true){
                camera.StopGrabbing();
                /*QMessageBox info;
                info.setWindowTitle("Information");
                info.setIcon(QMessageBox::Information);
                info.setText("Live display stopped");
                info.exec();
                break;*/
            }
            // Wait for an image and then retrieve it. A timeout of 5000 ms is used.
            camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);

            // Image grabbed successfully?i
            if (ptrGrabResult->GrabSucceeded())
            {
#ifdef PYLON_WIN_BUILD
                // Display the grabbed image.
                Pylon::DisplayImage(1, ptrGrabResult);
#endif
            }

            else
            {
                // Print the error message in a QT Message Box
                /*QString msg = "Grab unsuccessful\n";
                QMessageBox errorMBox;
                errorMBox.setWindowTitle("Error");
                errorMBox.setIcon(QMessageBox::Critical);
                errorMBox.setText( msg.append( ptrGrabResult->GetErrorDescription()) );
                errorMBox.exec();*/
            }
        }
    }
    catch (const GenericException &e)
    {
        qDebug()<< "Catch" << e.GetDescription();
        // Error handling.
        // Print the error message in a QT Message Box
        /*QString msg = "An exception has occurred.\n Check if the camera is connected properly";
        QMessageBox errorMBox;
        errorMBox.setWindowTitle("Error");
        errorMBox.setIcon(QMessageBox::Critical);
        errorMBox.setText( msg );
        errorMBox.exec();*/
    }

    // Releases all pylon resources.
    PylonTerminate();
    mutex.unlock();

    emit finished();
}
