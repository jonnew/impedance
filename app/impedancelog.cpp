#include "impedancelog.h"

ImpedanceLog::ImpedanceLog(QFileInfo fid)
{
    setSaveLocation(fid.absoluteFilePath(),true);

    // Start the clock
    timer.start();
}

ImpedanceLog::ImpedanceLog()
{
    // Start the clock
    timer.start();
}

void ImpedanceLog::clearLogFile(RHD2000Impedance *impedance) {

    // Clear the log and start over
    while (!log.isEmpty()) {
        log.removeFirst();
    }

    // Write down the parameters
    QJsonObject paramObject;
    impedance->writeParameters(paramObject);
    log.append(paramObject);

    cout << "Log cleared." << endl;
}

bool ImpedanceLog::saveLog() {

    QFile saveFile(logFile.absoluteFilePath());

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }

    QJsonObject json;
    json["log"] = log;

    QJsonDocument saveDoc(json);
    saveFile.write(saveDoc.toJson());

    cout << "Log saved." << endl;
    return true;
}

void ImpedanceLog::setSaveLocation(QString fname, bool overrideExistingFile) {

    if (!fname.endsWith(".json")){
        fname.append(".json");
    }

    QFileInfo f = QFileInfo(fname);

    if (!QDir(f.absoluteDir()).exists() ) {
        cout << "Selected log file save directory does not exist: " + f.path().toStdString() << endl;
        return;
    }

    if (!overrideExistingFile && f.exists() && f.isFile()) {
        string overwrite;
        cout << "Selected log file already exists. Overwrite (y/n)?" << endl;
        cin >> overwrite;
        if (!(overwrite == "Y" || overwrite == "y")) {
            return;
        }
    }
    else {
        cout << "Log save location set to " << f.absoluteFilePath().toStdString() << "." << endl;
        logFile = f;
    }
}

int ImpedanceLog::getElapsedTime()
{
    return timer.elapsed();
}

void ImpedanceLog::append(QJsonValue json) {

    log.append(json);
}

QJsonArray ImpedanceLog::getLog()
{
    return log;
}



