#ifndef PROTOCOLE_H
#define PROTOCOLE_H

#include <QString>
#include <QDir>
#include <QList>
#include <QImage>
#include <QSettings>
#include <stdint.h>

extern QString dataFolder;
extern QString input;

class Protocole
{
public:
    Protocole();
    Protocole(QString protocole, int mouse);

    QList<QList<QImage>> FrameList;
    QList<QImage> GuiFrameList;

    double rewardStrength;
    double rewardLower;
    double rewardUpper;
    double timeOut;
    double brightness;
    double rewardInterval;

    int rewardThd;
    int nPatterns;
    int nFrames;
    int frameDuration; // duration during which a single frame is displayed
    int frameIllumination; //duration during which a single frame is illuminated
};

#endif // PROTOCOLE_H
