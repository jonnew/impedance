#ifndef PLATECONTROL_H
#define PLATECONTROL_H

#include <QJsonObject>

class PlateControl
{
public:

    // Eval-board based plating control
    PlateControl(double currentGainuAPerVolt);

    // Plating control methods
    void turnPlatingOn(void);
    void turnPlatingOff(void);

    // Plating parameters
    void setPlateDuration(int durationMilliSec);
    void setPlateCurrent(double currentuA);
    int setHeadstage(int headstageNumber);

    // Get the nessesary information for writing to digital and analog
    // channels on eval board
    int dacNumber;
    int dacVoltage;
    int plateDurationMilliSec;
    int cleanDurationMilliSec;
    int* getTTLState(int ttlState[]);

    // State recording
    void write(QJsonObject &json, int channel) const;


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
    int platePolarity;
    double plateCurrentuA;
    double cleanCurrentuA;
    //

    // This is accomplished by a manual switch, but perhas an analog mux
    // would be better?
    //void enableESDDiodes();
    //void disableESDDiodes();

    // Pull the ground pin above or below the plating voltage
    // in order to plate/deplate
    void setPolarity(bool polarity);

};

#endif // PLATECONTROL_H
