#ifndef AUTOIMPEDANCE_H
#define AUTOIMPEDANCE_H

#include "rhd2000impedance.h"
#include "platecontrol.h"
#include "impedancelog.h"

class AutoImpedance
{
public:

    AutoImpedance(double targetImpedanceOhms, double maxErrorPercent, int maxPlatingTries);
    bool autoPlate(RHD2000Impedance *impedance, PlateControl *plate, ImpedanceLog *log);
    void setTargetImpedance(double targetOhms);
    void setMaxTries(int maxPlateTries);
    void setMaxError(double maxErrorPercent);
    void write(QJsonObject &json, int timeMSec) const;

private:

    ImpedanceLog *sessionLog;
    double targetImpedance;
    double allowableErrorPercent;
    int maxTries;
    bool plateSuccess;

};

#endif // AUTOIMPEDANCE_H
