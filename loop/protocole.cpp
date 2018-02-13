#include "protocole.h"
#include <QDebug>

Protocole::Protocole()
{
    /// INIT AN EMPTY PROTOCOLE
    rewardStrength = 0;
    rewardLower = 0;
    rewardUpper = 0;
    timeOut = -1;
    brightness = 0;
    rewardInterval = -1;
    rewardThd = 0;
    input = "Spikes"; // Spikes or dataFile
    FrameList = {};
    GuiFrameList = {};
}

Protocole::Protocole(QString protocole, int mouse)
{
    /// INIT AN ACTUAL PROTOCOLE
    /// From ini file, get file name, reward parameter, and time outs
    QString IniFile = dataFolder + "Protocoles/"+protocole;
    QSettings settings(IniFile, QSettings::IniFormat);
    QString currentStimulusFileName = settings.value("Frames").toString();
    rewardStrength = settings.value("RewardStrength").toDouble();
    rewardLower = settings.value("rewardLower").toDouble();
    rewardUpper = settings.value("rewardUpper").toDouble();
    rewardInterval = settings.value("rewardInterval").toDouble();
    rewardThd = settings.value("rewardThreshold").toInt();
    timeOut = settings.value("timeOut").toDouble();
    brightness = settings.value("brightness").toFloat();

    /// open loop with pattern selection coming from datafile, or from spikes
    input = settings.value("input").toString();

    /// number of patterns and number of frame per pattern
    nPatterns = settings.value("nPatterns").toInt();
    nFrames = settings.value("nFrames").toLongLong();
    frameDuration = settings.value("frameDuration").toLongLong(); // duration during which a single frame is displayed
    frameIllumination = settings.value("frameIllumination").toLongLong(); // duration during which a single frame is illuminated

    /// Load the patterns
    ///==================
    QString imageFolder = dataFolder + "mouse" + QString::number(mouse) + "/Frames/" + currentStimulusFileName + "/";

    QImage image;

    // names are numbers from 0 to N for the pattern, an underline, then a second number for the frame number.
    // the parameter file contains the pattern count and frame count
    // names


    for (int i = 0; i<nPatterns; i++)
    {
        QList<QImage> FramesSinglePattern = {};

        for (int j = 0; j<nFrames; j++)
        {
            QString imageFile = imageFolder + QString::number(i) + "_" + QString::number(j) + ".tiff";
            QFileInfo check_file(imageFile);
            if (check_file.exists())
            {
                image.load(imageFile); 
            }
            else
            {
                qDebug() << "File dosen't exists";
            }
            FramesSinglePattern.append(image);
        }

        /// Compute average
        QImage AveragePattern = QImage(image.width(), image.height(), QImage::Format_Grayscale8);
        AveragePattern.fill(Qt::black);

        /// loop through pixels, compute average
        for (int w = 0; w<image.width(); w++)
        {
            for (int h = 0; h<image.height(); h++)
            {
                float ActivePixelCount = 0;
                /// loop through images
                for (int j = 0; j<nFrames; j++)
                {
                    ActivePixelCount += qGray(FramesSinglePattern[j].pixel(w,h));
                }
                uint8_t bfr = (uint8_t)(ActivePixelCount/nFrames);
                AveragePattern.setPixel(w,h,qRgb(bfr, bfr, bfr));
            }
        }
        /// Add the patterns in a frame list to be displayed over the projector
        FrameList.append(FramesSinglePattern);
        GuiFrameList.append(AveragePattern);
    }
}
