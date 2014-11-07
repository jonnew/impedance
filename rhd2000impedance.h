#ifndef RHD2000IMPEDANCE_H
#define RHD2000IMPEDANCE_H

#include "globalconstants.h"
#include "signalprocessor.h"
#include "rhd2000evalboard.h"
#include "rhd2000registers.h"
#include "rhd2000datablock.h"

// Included class objects
class Rhd2000EvalBoard;
class SignalProcessor;
class Rhd2000Registers;
class SignalSources;
class SignalGroup;
class SignalChannel;

using namespace std;

class Rhd2000Impedance
{

public:

    // Imdepdance measurement class
    Rhd2000Impedance(Rhd2000EvalBoard::BoardPort port);

    // User-twidlable
    void setupEvalBoard();
    void setupAmplifier();
    void configureImpedanceMeasurement();
    void changeImpedanceFrequency(double Fs);
    int measureImpedance();
    int measureImpedance(int channel);
    int measureImpedance(int channel, double Fs);
    void printImpedance(int channel);

    // This structure exposes all channel information and impedance results
    SignalSources *signalSources;

private:

    // Private functions
    void changeSampleRate(Rhd2000EvalBoard::AmplifierSampleRate Fs);
    //void createCommands();
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

    // Data structs
    int ttlOut[16];
    QVector<int> chipId;
    queue<Rhd2000DataBlock> dataQueue;
    queue<Rhd2000DataBlock> filteredDataQueue;
    unsigned int numUsbBlocksToRead;

    // Parameters and data used to derive impedance measurements
    bool impedanceConfigured;
    int numPeriods;
    int numBlocks;
    double relativePeriod;
    QVector<QVector<QVector<double> > > measuredMagnitude;
    QVector<QVector<QVector<double> > > measuredPhase;

    // Data file stuff (this will be changed/removed)
    SaveFormat saveFormat;
    QDataStream *saveStream;


};

#endif // RHD2000IMPEDANCE_H
