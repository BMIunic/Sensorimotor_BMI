/*************************************************************
/IMAGER (2016)
/fileio.h
/Decleration of function used in fileio.cpp
/Author: Aamir Abbasi
**************************************************************/

#ifndef FILEIO_H
#define FILEIO_H

#include <QString>
#include <QStringList>
QString genConfigFile(QString rootname);
void writeParameters(QString filename, QString camera_phaseHigh, QString camera_phaseLow, QString camera_AcqDuration, QString stimulator_phaseHigh, QString stimulator_phaseLow, QString stimulator_StimDuration, QString trialInterval, QString exposure, QString xBin, QString yBin, QString stim_rate, QString camera_framerate, QString numberOfTrials, QString camera_preStimDuration);
QStringList readFile(QString filename);
QString makedir (QString absPath);

#endif // FILEIO_H

