#ifndef PLATECONTROL_H
#define PLATECONTROL_H

#include <QJsonObject>

class PlateControl
{
public:

    // Eval-board based plating control
    PlateControl(double currentGainuAPerVolt);

    // Possible modes
    enum modes {
        PLATE,
        CLEAN
    };

    // Plating control methods
    void turnCurrentSourceOn(void);
    void turnCurrentSourceOff(void);

    // Plating parameters
    void setMode(modes newMode);
    void setPlateDuration(int durationMilliSec);
    void setPlateCurrent(double currentuA);
    void setCleaningCurrent(double currentuA);
    void setCleaningDuration(int durationMilliSec);
    int setHeadstage(int headstageNumber);
    int getDacVoltage();
    int getDacNumber();
    int getPlatingDuration();
    int getCleaningDuration();

    // Get the nessesary information for writing to digital and analog
    // channels on eval board

    int* getTTLState(int ttlState[]);

    // State recording
    void writePlate(QJsonObject &json, int channel, int time) const;
    void writeClean(QJsonObject &json, int channel, int time) const;



private:

    // Current source
    double uAPerVolt;

    // Control bits
    int selectedHeadstage;
    int plateBit;
    int polarityBit;
    int headstageSelect0;
    unsigned short plateMask;
    unsigned short polarityMask;
    unsigned short headstageMask;
    unsigned short ttl;
    int mode;
    int polarity;
    double plateCurrentuA;
    double cleaningCurrentuA;
    int plateDurationMilliSec;
    int cleaningDurationMilliSec;
    int dacNumber;
    int dacVoltage;


    bool configured;

    // This is accomplished by a manual switch, but perhas an analog mux
    // would be better?
    //void enableESDDiodes();
    //void disableESDDiodes();

    // Pull the ground pin above or below the plating voltage
    // in order to plate/deplate
    void setPolarity(bool pol);

};

#endif // PLATECONTROL_H
