/****************************************************************************************
/IMAGER_v2.0 (2016)
/imagegrab.cpp
/This file contains functions for performing acquisition of images using BASLER ace
/USB 3.0 CCD camera.
/Author: Aamir Abbasi
*****************************************************************************************/

#include <QDir>
#include <QString>
#include <QStringList>
#include <QMessageBox>
#include <QDebug>
#include <QCoreApplication>
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

/// Namespace for using pylon objects.
using namespace Pylon;
using namespace GenApi;

/// Namespace for using cout (C++ objects).
using namespace std;

/// Settings for using Basler USB cameras.
typedef Pylon::CBaslerUsbInstantCamera Camera_t;
using namespace Basler_UsbCameraParams;

/// The camera specific grab result smart pointer.
typedef Camera_t::GrabResultPtr_t GrabResultPtr_t;

/* This function grabs one reference image and store it as a .tiff file in the
directory specified by QString mPath.*/
void grabreference(QString mPath, QString exposure, QString xBin, QString yBin)
{
    /// Pylon runtime initialization.
    PylonInitialize();

    try
    {
        /// Create an instant camera object with the camera device found first.
        CInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());

        /// The parameter MaxNumBuffer can be used to control the count of buffers
        /// allocated for grabbing. The default value of this parameter is 10.
        camera.MaxNumBuffer = 10;

        /// Open the camera.
        camera.Open();

        /// This smart pointer will receive the grab result data.
        CGrabResultPtr ptrGrabResult;

        /// Change image bit depth to 12bits from default 8bits.
        GenApi::INodeMap& nodemap = camera.GetNodeMap();
        CEnumerationPtr pixelFormat ( nodemap.GetNode("PixelFormat"));
        pixelFormat->FromString("Mono12");

        /// Set camera exposure mode to timed with exposure time equal to camera trigger width
        CEnumerationPtr exposureMode ( nodemap.GetNode("ExposureMode"));
        exposureMode->FromString("Timed");
        CFloatPtr exposureTime (nodemap.GetNode("ExposureTime"));
        exposureTime->SetValue(exposure.toFloat() * 1000.0);

        /// Set horizontal and vertical bin size for binning the image
        CIntegerPtr horizontalBin (nodemap.GetNode("BinningHorizontal"));
        horizontalBin->SetValue(xBin.toInt(nullptr, 10));
        CIntegerPtr verticalBin (nodemap.GetNode("BinningVertical"));
        verticalBin->SetValue(yBin.toInt(nullptr, 10));

        /// Set camera gain and black level
        CFloatPtr gain (nodemap.GetNode("Gain"));
        gain->SetValue(0.00);
        CFloatPtr blackLevel (nodemap.GetNode("BlackLevel"));
        blackLevel->SetValue(0.00);

        /// Wait for an image and then retrieve it. A timeout of 5000 ms is used.
        camera.GrabOne(5000, ptrGrabResult, TimeoutHandling_ThrowException);

        /// Image grabbed successfully?
        if (ptrGrabResult->GrabSucceeded())
        {
            /// Save the image data.
            const char *imgsavename = "img_ref.tiff";
            QString fname = mPath.append(imgsavename);
            CImagePersistence::Save(ImageFileFormat_Tiff, fname.toStdString().c_str(), ptrGrabResult);
        }
        else
        {
            /// Print the error message in a QT Message Box
            QString msg = "Grab unsuccessful\n";
            QMessageBox errorMBox;
            errorMBox.setWindowTitle("Error");
            errorMBox.setIcon(QMessageBox::Critical);
            errorMBox.setText(msg.append(ptrGrabResult->GetErrorDescription()));
            errorMBox.exec();
        }
    }
    catch (const GenericException &e)
    {
        /// Error handling.
        /// Print the error message in a QT Message Box
        QString msg = "An exception has occurred.\n";
        QMessageBox errorMBox;
        errorMBox.setWindowTitle("Error");
        errorMBox.setIcon(QMessageBox::Critical);
        errorMBox.setText(msg.append(e.GetDescription()));
        errorMBox.exec();
    }

    /// Releases all pylon resources.
    PylonTerminate();
}


/*This function is used for performing intrinsic imaging by grabbing an image sequence based on
the hardware trigger. The function generates two analog pulses to trigger the camera and
the piezo. Grabbed images are saved as .tiff files in the directory specified by QString mPath*/
int grabsequence(QString mPath,  UINT16 channel_cam, UINT16 channel_stim,
                 cbSdkWaveformData acq_camera, int CameraTriggReps,
                 QString exposure, QString xBin, QString yBin,
                 int images_persecond,
                 QString camera_preStimDuration,
                 cbSdkWaveformData stimulator,
                 cbSdkWaveformData stimulator_sine,
                 bool sqWave_flag,
                 bool sineWave_flag)                   //cbSdkWaveformData preStim_camera
{
    /// Before using any pylon methods, the pylon runtime must be initialized.
    PylonInitialize();

    try
    {
        /// Create an instant camera object with the camera device found first.
        CInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());

        /// Create a hardware trigger configuration object
        CSoftwareTriggerConfiguration config;

        /// Number of images to grab.
        static const uint32_t c_countOfPreStimImagesToGrab = CameraTriggReps; // number of images to grab is based on the phases of camera trigger waveform

        /// Open the camera.
        camera.Open();

        /// The parameter MaxNumBuffer can be used to control the count of buffers
        /// allocated for grabbing. The default value of this parameter is 10.
        camera.MaxNumBuffer = CameraTriggReps;

        /// This smart pointer will receive the grab result data.
        CGrabResultPtr ptrGrabResult;

        /// Register the standard configuration event handler for setting up the camera for software
        /// triggering.
        /// The current configuration is replaced by the hardware trigger configuration by setting the
        /// registration mode to RegistrationMode_ReplaceAll
        /// SoftwareTriggerConfiguration.h file is modified to include hardware trigger functionality.
        config.ApplyConfiguration(camera.GetNodeMap());

        /// Set camera exposure time, exposure mode is defined as timed in the softwaretriggerconfiguration.h file.
        GenApi::INodeMap& nodemap = camera.GetNodeMap();
        CFloatPtr exposureTime (nodemap.GetNode("ExposureTime"));
        exposureTime->SetValue(exposure.toFloat()*1000.0);

        /// Set horizontal and vertical bin size for binning the image
        CIntegerPtr horizontalBin (nodemap.GetNode("BinningHorizontal"));
        horizontalBin->SetValue(xBin.toInt(nullptr, 10));
        CIntegerPtr verticalBin (nodemap.GetNode("BinningVertical"));
        verticalBin->SetValue(yBin.toInt(nullptr, 10));

        /// Set camera gain and black level
        CFloatPtr gain (nodemap.GetNode("Gain"));
        gain->SetValue(0.00);
        CFloatPtr blackLevel (nodemap.GetNode("BlackLevel"));
        blackLevel->SetValue(0.00);

        /// StartGrabbing() calls the camera's Open() automatically if the camera is not open yet.
        /// The Open method calls the configuration handler's OnOpened() method that
        /// sets the required parameters for enabling software triggering.

        /// Camera.StopGrabbing() is called automatically by the RetrieveResult() method
        /// when c_countOfImagesToGrab images have been retrieved.

        /// Start the grabbing of c_countOfImagesToGrab images.
        /// The camera device is parameterized with a hardware configuration which
        /// sets up camera in waiting for harware trigger mode
        camera.StartGrabbing(c_countOfPreStimImagesToGrab);

        int counter = 1; // Image counter
        const char *img;
        //uint16_t bufferDataAvg;

        int prestim_images = images_persecond*camera_preStimDuration.toInt(nullptr,10);

        /// Send analog waveform for prestimulus camera TTL
        cbSdkResult res = NSP_AnalogOut(acq_camera, channel_cam); // Waveform to trigger the camera
        /// Error handling.
        if (res != 0)
        {
            QString msg = "Prestimulus camera waveform is not generated ";
            QMessageBox eMsgBox;
            eMsgBox.setWindowTitle("Error");
            eMsgBox.setIcon(QMessageBox::Critical);
            eMsgBox.setText(msg);
            eMsgBox.exec();
            return -1;
        }

        /// Grabbing of prestimulation images
        while (camera.IsGrabbing())
        {

            /// Prevent GUI thread from freezing
            QCoreApplication::processEvents();

            /// Wait for an image and then retrieve it. A timeout of 10000 ms is used.
            camera.RetrieveResult(10000, ptrGrabResult, TimeoutHandling_ThrowException);

            /// Image grabbed successfully?
            if (ptrGrabResult->GrabSucceeded())
            {
                /*uint16_t *pImageBuffer = (uint16_t *)ptrGrabResult->GetBuffer();
                uint16_t bufferData = *pImageBuffer;
                bufferDataAvg += bufferData;*/
                // Save the image data.
                if (counter < 10){
                    img = "/img_00";
                }
                if (counter < 100 && counter > 10){
                    img = "/img_0";
                }
                if (counter >= 100){
                    img = "/img_";
                }
                QString imgsaveName = mPath;
                imgsaveName = imgsaveName.append(img);
                string number = to_string(counter);
                const char *imgNo = number.c_str();
                imgsaveName = imgsaveName.append(imgNo);
                imgsaveName = imgsaveName.append(".tiff");         
                CImagePersistence::Save(ImageFileFormat_Tiff, imgsaveName.toStdString().c_str(), ptrGrabResult);
            }
            else
            {
                /// Print the error message in a QT Message Box
                QString msg = "Grab unsuccessful\n";
                QMessageBox errorMBox;
                errorMBox.setWindowTitle("Error");
                errorMBox.setIcon(QMessageBox::Critical);
                errorMBox.setText( msg.append( ptrGrabResult->GetErrorDescription()) );
                errorMBox.exec();
                return -1;

                /// Print the error on QT Console
                cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
            }
            counter += 1;

            /// Generate a the analog waveform for piezo stimulation
            if (counter == prestim_images)
            {
                /// Send analog waveform 2 for stimulator TTL
                if (sqWave_flag == 1 && sineWave_flag == 0){
                    cbSdkResult res2 = NSP_AnalogOut(stimulator, channel_stim); // Waveform to trigger the whisker stimulator
                    /// Error handling.
                    if (res2 != 0)
                    {
                        QString msg = "Stimulator waveform not generated ";
                        QMessageBox eMsgBox;
                        eMsgBox.setWindowTitle("Error");
                        eMsgBox.setIcon(QMessageBox::Critical);
                        eMsgBox.setText(msg);
                        eMsgBox.exec();
                        return 1;
                    }
                }
                if (sqWave_flag == 0 && sineWave_flag == 1){
                    cbSdkResult res2 = NSP_AnalogOut(stimulator_sine,channel_stim);
                    /// Error handling.
                    if (res2 != 0)
                    {
                        QString msg = "Stimulator waveform not generated ";
                        QMessageBox eMsgBox;
                        eMsgBox.setWindowTitle("Error");
                        eMsgBox.setIcon(QMessageBox::Critical);
                        eMsgBox.setText(msg);
                        eMsgBox.exec();
                        return 1;
                    }
                }
            }
        }
        camera.StopGrabbing();
    }

    catch (const GenericException &e)
    {
        /// Error handling.
        /// Print the error message in a QT Message Box
        QString msg = "An exception has occurred.\n";
        QMessageBox errorMBox;
        errorMBox.setWindowTitle("Error");
        errorMBox.setIcon(QMessageBox::Critical);
        errorMBox.setText( msg.append(e.GetDescription()) );
        errorMBox.exec();
        return -1;
    }

    /// Releases all pylon resources.
    PylonTerminate();

    return 0;
}

