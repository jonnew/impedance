#ifndef RHD2000IMPEDANCE_H
#define RHD2000IMPEDANCE_H

#include "globalconstants.h"
#include "signalprocessor.h"
#include "rhd2000evalboard.h"
#include "rhd2000registers.h"
#include "rhd2000datablock.h"
#include "okFrontPanelDLL.h"

// Included class objects
class Rhd2000EvalBoard;
class SignalProcessor;
class Rhd2000Registers;

using namespace std;

class Rhd2000impedance
{

public:

    // Imdepdance measurement class
    Rhd2000impedance(int port);

    // User-twidlable
    void measureImpedance();
    void changeImpedanceFrequency(double Fs);


private:

    // Private functions
    void setupEvalBoard();
    void setupAmplifier();
    void changeSampleRate(Rhd2000EvalBoard::AmplifierSampleRate Fs);
    void updateImpedanceFrequency();
    int deviceID();
    void factorOutParallelCapacitance(double &impedanceMagnitude,
                                      double &impedancePhase,
                                      double frequency,
                                      double parasiticCapacitance);
    void empiricalResistanceCorrection(double &impedanceMagnitude,
                                       double &impedancePhase,
                                       double boardSampleRate);


    //void saveImpedance();

    // Class-wide objects
    Rhd2000EvalBoard *evalBoard;
    SignalProcessor *signalProcessor;

    // Background desired/corrected parameter tracking
    Rhd2000EvalBoard::BoardPort usedPort;
    double desiredDspCutoffFreq;
    double actualDspCutoffFreq;
    double desiredUpperBandwidth;
    double actualUpperBandwidth;
    double desiredLowerBandwidth;
    double actualLowerBandwidth;
    bool dspEnabled;
    double notchFilterFrequency;
    double notchFilterBandwidth;
    bool notchFilterEnabled;
    double highpassFilterFrequency;
    bool highpassFilterEnabled;
    bool fastSettleEnabled;
    double desiredImpedanceFreq;
    double actualImpedanceFreq;
    bool impedanceFreqValid;
    double cableLengthMeters;
    double boardSampleRate;

};

#endif // RHD2000IMPEDANCE_H
