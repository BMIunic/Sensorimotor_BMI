/*************************************************************
/IMAGER (2016)
/imagegrab.h
/Decleration of function used in imagegrab.cpp
/Author: Aamir Abbasi
**************************************************************/

#ifndef IMAGEGRAB_H
#define IMAGEGRAB_H

#include <QString>
#include <pylon/PylonIncludes.h>

void grabreference(QString mPath);
int grabsequence(QString mPath, QString phaseCamera, QString durationHighCamera, QString amplitudeHighCamera, QString durationLowCamera, QString amplitudeLowCamera, QString phaseStimulator, QString durationHighStimulator, QString amplitudeHighStimulator, QString durationLowStimulator, QString amplitudeLowStimulator, QString amplitudeNegHighStimulator);

#endif // IMAGEGRAB_H
