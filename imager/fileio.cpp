/*****************************************************
/IMAGER_v2.0 (2016)
/fileio.cpp
/Functions to read a text file and make directories
/Author: Aamir Abbasi
*****************************************************/

#include <QFile>
#include <QTextStream>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QMessageBox>
#include <QDebug>
#include "fileio.h"

QString genConfigFile(QString rootname)
{
    QString rootfilename = rootname + "configFile.txt";
    QFile rFile(rootfilename);

    if(!rFile.open(QFile::WriteOnly | QFile::Text)){
        QMessageBox mBox;
        mBox.setWindowTitle("Error");
        mBox.setIcon(QMessageBox::Critical);
        mBox.setText("Could not open the configuration file for writing");
        mBox.exec();
    }

    QTextStream out(&rFile);
    out << rootname << endl;
    out << rootname + "Alpha/" << endl;
    out << rootname + "A1/" << endl;
    out << rootname + "A2/" << endl;
    out << rootname + "A3/" << endl;
    out << rootname + "A4/" << endl;
    out << rootname + "Beta/" << endl;
    out << rootname + "B1/" << endl;
    out << rootname + "B2/" << endl;
    out << rootname + "B3/" << endl;
    out << rootname + "B4/" << endl;
    out << rootname + "Gamma/" << endl;
    out << rootname + "C1/" << endl;
    out << rootname + "C2/" << endl;
    out << rootname + "C3/" << endl;
    out << rootname + "C4/" << endl;
    out << rootname + "Delta/" << endl;
    out << rootname + "D1/" << endl;
    out << rootname + "D2/" << endl;
    out << rootname + "D3/" << endl;
    out << rootname + "D4/" << endl;
    out << rootname + "E1/" << endl;
    out << rootname + "E2/" << endl;
    out << rootname + "E3/" << endl;
    out << rootname + "E4/" << endl;


    rFile.flush();
    rFile.close();

    return rootfilename;
}

void writeParameters(QString filename, QString camera_phaseHigh, QString camera_phaseLow, QString camera_AcqDuration,
           QString stimulator_phaseHigh, QString stimulator_phaseLow,
           QString stimulator_StimDuration, QString trialInterval, QString exposure, QString xBin, QString yBin,
           QString stim_rate, QString camera_framerate, QString numberOfTrials,  QString camera_preStimDuration )
{

    filename = filename.append("/Trial_Parameters.txt");
    QFile mFile(filename);

    if (!mFile.open(QFile::WriteOnly | QFile::Text)){
        QMessageBox mBox;
        mBox.setWindowTitle("Error");
        mBox.setIcon(QMessageBox::Critical);
        mBox.setText("Could not open the parameter file for writing");
        mBox.exec();
    }

    QTextStream out(&mFile);
    out << "Number of trials = " << numberOfTrials << endl;
    out << "Inter trial interval(s) = " << trialInterval << endl;
    out << "                                   "<< endl;

    out << "Camera Tigger Parameters " << endl;
    out << "  High phase duration(ms) = " << camera_phaseHigh << endl;
    out << "  Low phase duration(ms) = " << camera_phaseLow << endl;
    out << "  Total camera acquisition(s) = " << camera_AcqDuration << endl;
    out << "  Prestimulus camera acquisition(s) = " << camera_preStimDuration << endl;
    out << "  Camera frame rate(Hz) = " << camera_framerate << endl;
    out << "                                   "<< endl;

    out << "Piezo Stimulator Trigger Parameters " << endl;
    out << "  Plateau phase duration(ms) = " << stimulator_phaseHigh << endl;
    out << "  Baseline phase duration(ms) = " << stimulator_phaseLow << endl;
    out << "  Stimulus piezo stimulation(ms) = " << stimulator_StimDuration << endl;
    out << "  Stimulation rate(Hz) = " << stim_rate <<endl;
    out << "                                   "<< endl;

    out << "Image Acquisition Parameters " << endl;
    out << "  Exposure(ms) = "<< exposure << endl;
    out << "  Horizontal Bin Size = " << xBin << endl;
    out << "  Vertical Bin Size = "<< yBin << endl;

    mFile.flush();
    mFile.close();
}

QStringList readFile(QString filename)
{

    QFile mFile(filename);
    QString line;

    if (!mFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        QMessageBox mBox;
        mBox.setWindowTitle("Error");
        mBox.setIcon(QMessageBox::Critical);
        mBox.setText("Could not open the file for reading");
        mBox.exec();
    }

    line = mFile.readAll();

    QStringList sList = line.split(QRegExp("(\\r\\n)|(\\n\\r)|\\r|\\n"), QString::SkipEmptyParts);

    return sList;
}

QString makedir (QString absPath)
{
    QDir mDir;
    QString mPath = absPath;

    if(!mDir.exists (mPath))
    {
        mDir.mkpath(mPath);
    }
    return mPath;
}
