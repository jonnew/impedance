#ifndef RHD2000IMPEDANCE_H
#define RHD2000IMPEDANCE_H

#include "globalconstants.h"
#include "signalprocessor.h"
#include "rhd2000evalboard.h"
#include "rhd2000registers.h"
#include "rhd2000datablock.h"
#include "platecontrol.h"

// Included class objects
class Rhd2000EvalBoard;
class SignalProcessor;
class Rhd2000Registers;
class SignalSources;
class SignalGroup;
class SignalChannel;
class PlateControl;

using namespace std;

class Rhd2000Impedance
{

public:

    // Imdepdance measurement class
    Rhd2000Impedance(Rhd2000EvalBoard::BoardPort port);

    // User-twidlable
    int selectChannel(int selectedChannel);
    void changeImpedanceFrequency(double Fs);
    int measureImpedance(void);
    int measureImpedance(double Fs);
    void printImpedance(void);
    double getImpedancePhase(void);
    double getImpedanceMagnitude(void);
    int plate(double currentuA, unsigned long durationMilliSec);

    // This structure exposes all channel information and impedance results
    SignalSources *signalSources;


private:

    // Private functions
    void setupEvalBoard(void);
    void setupAmplifier(void);
    void setupImpedanceTest(void);
    void generateAuxCmds(void);



    void changeSampleRate(Rhd2000EvalBoard::AmplifierSampleRate Fs);
    void performADCSelfCalibration(void);
    void updateImpedanceFrequency();
    int deviceId(Rhd2000DataBlock *dataBlock, int stream, int &register59Value);
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

    // Registers than can be used to configure the chip(s)
    Rhd2000Registers *chipRegisters;

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
    int impedanceTestSamples;
    int auxCmd1SequenceLength;
    int auxCmd3SequenceLength;

    // Data structs
    int ttlOut[16];
    QVector<int> chipId;
    queue<Rhd2000DataBlock> dataQueue;
    queue<Rhd2000DataBlock> filteredDataQueue;
    unsigned int numUsbBlocksToRead;

    // Parameters and data used to derive impedance measurements
    int channel;
    bool impedanceConfigured;
    bool channelSelected;
    int numPeriods;
    int numBlocks;
    int numSettleBlocks;
    int numSettlePeriods;
    double relativePeriod;
    QVector<QVector<QVector<double> > > measuredMagnitude;
    QVector<QVector<QVector<double> > > measuredPhase;

    // Plating
    PlateControl *plateControl;

    // Data file stuff (this will be changed/removed)
    SaveFormat saveFormat;
    QDataStream *saveStream;


};

#endif // RHD2000IMPEDANCE_H
