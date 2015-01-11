#ifndef IMPEDANCELOG_H
#define IMPEDANCELOG_H

#include <QDir>
#include <QFile>
#include <QTime>
#include <QDate>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include "rhd2000impedance.h"

class RHD2000Impedance;

class ImpedanceLog
{

public:

    ImpedanceLog();
    ImpedanceLog(QFileInfo fid);

    bool saveLog(void);
    void clearLogFile(RHD2000Impedance *impedance);
    void setSaveLocation(QString f, bool overrideExistingFile);
    int getElapsedTime();
    void append(QJsonValue json);
    QJsonArray getLog();

private:
    // Impedance/plate log array
    QTime timer;
    QFileInfo logFile;
    QJsonArray log;

};

#endif // IMPEDANCELOG_H
