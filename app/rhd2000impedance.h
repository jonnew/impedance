#ifndef RHD2000IMPEDANCE_H
#define RHD2000IMPEDANCE_H

#include "globalconstants.h"
#include "signalprocessor.h"
#include "rhd2000evalboard.h"
#include "rhd2000registers.h"
#include "rhd2000datablock.h"
#include "platecontrol.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QDate>
#include <QJsonDocument>
#include <string>
#include <QDir>
#include <QFileInfo>

// Included class objects
class Rhd2000EvalBoard;
class SignalProcessor;
class Rhd2000Registers;
class SignalSources;
class SignalGroup;
class SignalChannel;
class PlateControl;
class Rhd2000DataBlock;

using namespace std;

class RHD2000Impedance
{

public:

    // Imdepdance measurement class
    RHD2000Impedance(Rhd2000EvalBoard::BoardPort port);

    // User-twidlable
    int selectChannel(int selectedChannel);
    void setImpedanceTestFrequency(double Fs);
    int measureImpedance(void);
    void printImpedance(void);
    double getImpedancePhase(void);
    double getImpedanceMagnitude(void);
    void setNumAverages(int n);
    void setNumPeriods(int n);
    void configurePlate(PlateControl *pc);
    int plate(PlateControl *pc);
    int clean(PlateControl *pc);

    bool saveLog(void);
    void clearLogFile(void);
    void setSaveLocation(QString f);

    // This structure exposes all channel information and impedance results
    SignalSources *signalSources;




private:

    // Private functions
    void setupEvalBoard(void);
    void setupAmplifier(void);
    void setupImpedanceTest(void);
    void generateAuxCmds(void);
    void generateAuxCmd3(void);



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


    void applyCurrent(PlateControl *pc);
    void write(QJsonObject &json, double mag, double phase) const;
    void writeParameters(QJsonObject &json) const;

    // Impedance/plate log array
    QTime timer;
    QJsonArray log;
    QFileInfo logFile;

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
    QQueue<Rhd2000DataBlock> dataQueue;
    QQueue<Rhd2000DataBlock> filteredDataQueue;
    unsigned int numUsbBlocksToRead;

    // Parameters and data used to derive impedance measurements
    int channel;
    bool impedanceConfigured;
    bool platingConfigured;
    bool channelSelected;
    int numAverages;
    int numPeriods;
    int numBlocks;
    int numSettleBlocks;
    int numSettlePeriods;
    double relativePeriod;
    QVector<QVector<QVector<double> > > measuredMagnitudeRaw;
    QVector<QVector<QVector<double> > > measuredPhaseRaw;
    QVector<QVector<QVector<double> > > measuredMagnitude;
    QVector<QVector<QVector<double> > > measuredPhase;

    // Data file stuff (this will be changed/removed)
    SaveFormat saveFormat;
    QDataStream *saveStream;


};

#endif // RHD2000IMPEDANCE_H
