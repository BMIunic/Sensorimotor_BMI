/****************************************************************************************
/IMAGER_v2.0 (2016)
/imagesequence.cpp
/This file contains function implementing acquistion of image sequences using BASLER ace
/USB 3.0 CCD camera on a sepereate thread. The thread can be start and stopped using Start
/trial and Stop trial push buttons on the GUI.
/Author: Aamir Abbasi
*****************************************************************************************/

#include "imagesequence.h"

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

    emit workRequested();
}

void LiveThread::abort()
{
    if (_working) {
        _abort = true;
        qDebug()<<"Request worker aborting in Thread "<<thread()->currentThreadId();
    }
}

/*This function is used for performing intrinsic imaging by grabbing an image sequence based on
the hardware trigger. The function generates two analog pulses to trigger the camera and
the piezo. Grabbed images are saved as .tiff files in the directory specified by QString mPath*/
void LiveThread::doWork(QString mPath, QString phaseCamera, QString durationHighCamera,
                        QString amplitudeHighCamera, QString durationLowCamera, QString amplitudeLowCamera,
                        QString phaseStimulator, QString durationHighStimulator, QString amplitudeHighStimulator,
                        QString durationLowStimulator,QString amplitudeLowStimulator, QString amplitudeNegHighStimulator)
{
    QMutex mutex;
    mutex.lock();
    // Generate an analog pulse to trigger the camera to acquire images.
    if (NSP_On_Open() != 0){
        QString msg = "Check NSP connection\n";
        QMessageBox eMsgBox;
        eMsgBox.setWindowTitle("Error");
        eMsgBox.setIcon(QMessageBox::Critical);
        eMsgBox.setText(msg);
        eMsgBox.exec();
        return -1;
    }
    Sleep(1000);

    /// Initialisation of camera analog waveform
    /// In Samples 3000 = 100ms. Sampling Frequency 30000 Hz. Total 10s pulse
    int nPhase1 = phaseCamera.toInt();

    UINT16* w1_nDuration;
    w1_nDuration = new UINT16 [nPhase1];

    INT16* w1_nAmplitude;
    w1_nAmplitude = new INT16 [nPhase1];

    for (int i = 0; i < nPhase1; i++)
    {
        if (i % 2 == 0)
        {
            w1_nDuration[i] = durationHighCamera.toUInt(nullptr, 10);
            w1_nAmplitude[i] = amplitudeHighCamera.toInt(nullptr, 16);
        }
        if (i % 2 == 1)
        {
            w1_nDuration[i] = durationLowCamera.toUInt(nullptr, 10);
            w1_nAmplitude[i] = amplitudeLowCamera.toInt(nullptr, 16);
        }

    }

    /// Initialisation of piezo analog waveform
    /// In Samples 3000 = 100ms. 60000 = 2s. Total pulse 6s
    int nPhase2 = phaseStimulator.toInt() + 1; // +1 for baseling phase of 2s duration and 0V amplitude before the start of piezo stimulation.

    UINT16* w2_nDuration;
    w2_nDuration = new UINT16 [nPhase2];

    INT16* w2_nAmplitude;
    w2_nAmplitude = new INT16 [nPhase2];

    int counter = 0;
    for (int j = 0; j < nPhase2; j++)
    {

        if (j == 0)
        {
            w2_nDuration[j] = 60000;
            w2_nAmplitude[j] = 0x0000;
        }
        if (j % 2 == 1)
        {
            counter += 1;
            w2_nDuration[j] = durationHighStimulator.toUInt(nullptr, 10);

            if (counter % 2 == 1)
            {
                w2_nAmplitude[j] = amplitudeHighStimulator.toInt(nullptr, 16);
            }
            if (counter % 2 == 0)
            {
                w2_nAmplitude[j] = amplitudeNegHighStimulator.toInt(nullptr, 16);
            }

        }
        if (j % 2 == 0 && j != 0)
        {
            w2_nDuration[j] = durationLowStimulator.toUInt(nullptr, 10);
            w2_nAmplitude[j] = amplitudeLowStimulator.toInt(nullptr, 16);
        }
    }

    // Analog channel number defination
    UINT16 channel_cam = 147; //Analog output channel number on NSP Ch1-145 Ch2-146 Ch3-147 Ch4-148
    UINT16 channel_stim = 148; //Analog output channel number on NSP Ch1-145 Ch2-146 Ch3-147 Ch4-148

    // Waveform defination
    cbSdkWaveformData w1 = waveform(1, w1_nDuration, w1_nAmplitude, nPhase1);
    cbSdkWaveformData w2 = waveform(1, w2_nDuration, w2_nAmplitude, nPhase2);

    // Before using any pylon methods, the pylon runtime must be initialized.
    PylonInitialize();

    try
    {
        // Create an instant camera object with the camera device found first.
        CInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());

        // Create a software configuration object
        CSoftwareTriggerConfiguration config;

        // Number of images to grab.
        static const uint32_t c_countOfImagesToGrab = qRound(float(nPhase1)/2); // number of images to grab is based on the phases of camera trigger waveform

        // Open the camera.
        camera.Open();

        // The parameter MaxNumBuffer can be used to control the count of buffers
        // allocated for grabbing. The default value of this parameter is 10.
        camera.MaxNumBuffer = 50;

        // This smart pointer will receive the grab result data.
        CGrabResultPtr ptrGrabResult;

        // Register the standard configuration event handler for setting up the camera for software
        // triggering.
        // The current configuration is replaced by the hardware trigger configuration by setting the
        // registration mode to RegistrationMode_ReplaceAll
        // SoftwareTriggerConfiguration.h file is modified to include hardware trigger functionality.
        config.ApplyConfiguration(camera.GetNodeMap());

        // StartGrabbing() calls the camera's Open() automatically if the camera is not open yet.
        // The Open method calls the configuration handler's OnOpened() method that
        // sets the required parameters for enabling software triggering.

        // Camera.StopGrabbing() is called automatically by the RetrieveResult() method
        // when c_countOfImagesToGrab images have been retrieved.

        // Start the grabbing of c_countOfImagesToGrab images.
        // The camera device is parameterized with a hardware configuration which
        // sets up camera in waiting for harware trigger mode
        camera.StartGrabbing(c_countOfImagesToGrab);

        // Send analog waveform 1 for camera TTL
        cbSdkResult res1 = NSP_AnalogOut(w1, channel_cam); // Waveform to trigger the camera
        // Error handling.
        if (res1 != 0)
        {
            QString msg = "Analog waveform not generated ";
            QMessageBox eMsgBox;
            eMsgBox.setWindowTitle("Error");
            eMsgBox.setIcon(QMessageBox::Critical);
            eMsgBox.setText(msg);
            eMsgBox.exec();
            return -1;
        }

        // Send analog waveform 2 for stimulator TTL
        cbSdkResult res2 = NSP_AnalogOut(w2, channel_stim); // Waveform to trigger the whisker stimulator
        // Error handling.
        if (res2 != 0)
        {
            QString msg = "Analog waveform not generated ";
            QMessageBox eMsgBox;
            eMsgBox.setWindowTitle("Error");
            eMsgBox.setIcon(QMessageBox::Critical);
            eMsgBox.setText(msg);
            eMsgBox.exec();
            return -1;
        }


        int counter = 1; // Image counter
        const char *img;

        // Send digital trigger
        cbSdkResult dhigh = NSP_DigitalOut(156, 1);	     //dout1- 153 dout2- 154 dout3- 155 dout4- 156
        Sleep(100);
        cbSdkResult dlow =  NSP_DigitalOut(156, 0);		  //dout1- 153 dout2- 154 dout3- 155 dout4- 156
        // Error handling.
        if (dhigh != 0 || dlow != 0)
        {
            QString msg = "Digital ouput not set properly ";
            QMessageBox eMsgBox;
            eMsgBox.setWindowTitle("Error");
            eMsgBox.setIcon(QMessageBox::Critical);
            eMsgBox.setText(msg);
            eMsgBox.exec();
            return -1;
        }

        while (camera.IsGrabbing())
        {

            // Wait for an image and then retrieve it. A timeout of 5000 ms is used.
            camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);

            // Image grabbed successfully?
            if (ptrGrabResult->GrabSucceeded())
            {
                // Save the image data.
                if (counter < 10){
                    img = "/img_00";
                }
                else{
                    img = "/img_0";
                }
                QString imgsaveName = mPath;
                imgsaveName = imgsaveName.append(img);
                string number = to_string(counter);
                const char *imgNo = number.c_str();
                imgsaveName = imgsaveName.append(imgNo);
                imgsaveName = imgsaveName.append(".tiff");
                qDebug() << imgsaveName;
                CImagePersistence::Save(ImageFileFormat_Tiff, imgsaveName.toStdString().c_str(), ptrGrabResult);
            }
            else
            {
                // Print the error message in a QT Message Box
                QString msg = "Grab unsuccessful\n";
                QMessageBox errorMBox;
                errorMBox.setWindowTitle("Error");
                errorMBox.setIcon(QMessageBox::Critical);
                errorMBox.setText( msg.append( ptrGrabResult->GetErrorDescription()) );
                errorMBox.exec();
                return -1;

                // Print the error on QT Console
                cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
            }
            counter += 1;
        }
    }

    catch (const GenericException &e)
    {
        // Error handling.
        // Print the error message in a QT Message Box
        QString msg = "An exception has occurred.\n";
        QMessageBox errorMBox;
        errorMBox.setWindowTitle("Error");
        errorMBox.setIcon(QMessageBox::Critical);
        errorMBox.setText( msg.append(e.GetDescription()) );
        errorMBox.exec();
        return -1;
    }

    // Releases all pylon resources.
    PylonTerminate();

    // Close the interface with the NSP
    //Sleep(30000); // 30s wait before closing the nsp
    NSP_On_Close();
    return 0;
    mutex.unlock();
    emit finished();
}
