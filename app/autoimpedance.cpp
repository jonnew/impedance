#include "autoimpedance.h"

using namespace std;


// Automated impedance testing and plating
AutoImpedance::AutoImpedance(double targetImpedanceOhms, double maxErrorPercent, int maxPlatingTries)
{
    // Defaults
    setTargetImpedance(targetImpedanceOhms);
    setMaxTries(maxPlatingTries);
    setMaxError(maxErrorPercent);
}

// TODO: This is an extremely primative strategy. Of course maybe a PI
// scheme would work a lot better, but lets just try this to start
bool AutoImpedance::autoPlate(RHD2000Impedance *imp, PlateControl *plateControl, ImpedanceLog *log)
{
    imp->measureImpedance(log);
    double currentImp = imp->getImpedanceMagnitude();
    double errorPerc = 100.0 * (targetImpedance - currentImp)/targetImpedance;

    // Clear the autoplate session log
    sessionLog = new ImpedanceLog();

    int n = 0;
    while(fabs(errorPerc) > allowableErrorPercent && n < maxTries) {

        if (errorPerc < 0) {
            imp->plate(plateControl, sessionLog);

//            QJsonObject jObject;
//            plateControl->writePlate(jObject, imp->getChannel(),imp->getElapsedTime());
//            autoPlateLog.append(jObject);

        }
        else if (errorPerc > 0) {
            imp->clean(plateControl,sessionLog);

//            QJsonObject jObject;
//            plateControl->writeClean(jObject, imp->getChannel(),imp->getElapsedTime());
//            autoPlateLog.append(jObject);
        }

        imp->measureImpedance(sessionLog);
        imp->printImpedance();
        double currentImp = imp->getImpedanceMagnitude();
        double errorPerc = 100.0 * (targetImpedance - currentImp)/targetImpedance;
        n++;

    }

    plateSuccess = fabs(errorPerc) < allowableErrorPercent;

    QJsonObject jObject;
    write(jObject, log->getElapsedTime());
    log->append(jObject);

    if (fabs(errorPerc) > allowableErrorPercent) {
        return false;
    }
    else {
        return true;
    }
}

void AutoImpedance::setTargetImpedance(double targetOhms) {

    if (targetOhms > 0) {
        targetImpedance = targetOhms;
    }
    else {

        targetImpedance = 350.0e3;
        cout << "Warning: target impedance must be positive." << endl;
        cout << "Target impedance set to default value of " << targetImpedance << " Ohms." << endl;
    }
}

void AutoImpedance::setMaxTries(int maxPlateTries) {

    if (maxPlateTries > 0) {
        maxTries = maxPlateTries;
    }
    else {

        maxTries = 5;
        cout << "Warning: Number of plating tries must be positive." << endl;
        cout << "Number of plating tries set to default value of " << maxTries << "." << endl;
    }
}

void AutoImpedance::setMaxError(double maxErrorPercent) {

    allowableErrorPercent = fabs(maxErrorPercent);

    cout << "Autoimpedance allowable error set to " << allowableErrorPercent << " \%." << endl;

}

void AutoImpedance::write(QJsonObject &json, int timeMSec) const
{
    json["name"] =  (QString)"auto_impedance_session";
    json["time"] =  timeMSec;
    json["max_tries"] = maxTries;
    json["target_impedance_ohms"] = targetImpedance;
    json["allowable_error_percent"] = allowableErrorPercent;
    json["success"] = plateSuccess;
    json["session"] = sessionLog->getLog();
}
