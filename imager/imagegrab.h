/*************************************************************
/IMAGER_v2.0 (2016)
/imagegrab.h
/Decleration of function used in imagegrab.cpp
/Author: Aamir Abbasi
**************************************************************/

#ifndef IMAGEGRAB_H
#define IMAGEGRAB_H

#include <QString>
#include <pylon/PylonIncludes.h>
#include <nsp.h>

void grabreference(QString mPath, QString exposure, QString xBin, QString yBin);
int grabsequence(QString mPath,  UINT16 channel_cam, UINT16 channel_stim,
                 cbSdkWaveformData acq_camera
                 , int CameraTriggReps
                 , QString exposure, QString xBin, QString yBin, int images_persecond, QString camera_preStimDuration, cbSdkWaveformData stimulator, cbSdkWaveformData stimulator_sine, bool sqWave_flag, bool sineWave_flag);

#endif // IMAGEGRAB_H

