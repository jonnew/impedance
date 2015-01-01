#ifndef PLATECONTROL_H
#define PLATECONTROL_H

class PlateControl
{
public:

    // Eval-board based plating control
    PlateControl(int plateControlDAC,
                 double currentGainuAPerVolt,
                 int plateTriggerBit,
                 int platePolarityBit,
                 int headStageSelectBit0);

    // Plating control methods
    void turnPlatingOn(void);
    void turnPlatingOff(void);
    void applyPlatingDelay(void);

    // Plating parameters
    int setPlateParameters(double currentuA, unsigned long durationMilliSec);
    int selectHeadstage(int headstageNumber);

    // Get the nessesary information for writing to digital and analog
    // channels on eval board
    int dacNumber;
    int dacVoltage;
    unsigned long plateDurationMilliSec;
    int* getTTLState(int ttlState[]);


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
