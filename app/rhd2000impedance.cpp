#include "rhd2000impedance.h"
#include "rhd2000evalboard.h"
#include "rhd2000registers.h"
#include "rhd2000datablock.h"
#include "globalconstants.h"
#include "signalprocessor.h"
#include "signalchannel.h"
#include "signalsources.h"
#include "platecontrol.h"
#include "qtinclude.h"

#include "okFrontPanelDLL.h"

//#include <cmath>

using namespace std;

Rhd2000Impedance::Rhd2000Impedance(Rhd2000EvalBoard::BoardPort port)
{
    // Initialized default parameters
    cout << "Intializing internal default parameters...";

    // define the port to which the RHD2000 is attached
    usedPort = port; //0 = A, 1 = B, etc
    if (usedPort < 0 || usedPort > 3)
    {
        cout << "An invalid port number, " << usedPort << ", was selected. Please select a port between 0 and 3." << endl;
        exit(EXIT_FAILURE);
    }

    // Default amplifier bandwidth settings
    desiredLowerBandwidth = 0.1;
    desiredUpperBandwidth = 7500.0;
    desiredDspCutoffFreq = 1.0;
    dspEnabled = true;

    // Default electrode impedance measurement frequency
    desiredImpedanceFreq = 1000.0;
    actualImpedanceFreq = 0.0;
    impedanceFreqValid = false;
    impedanceConfigured = false;

    // Get a signal processor, sources, etc
    signalProcessor = new SignalProcessor();
    signalSources = new SignalSources();
    chipRegisters = new Rhd2000Registers(boardSampleRate);

    // Create plating control object
    plateControl = new PlateControl(0, 0.1/3.3, 0, 1, 2);

    // Set default filter freqs
    notchFilterFrequency = 60.0;
    notchFilterBandwidth = 10.0;
    notchFilterEnabled = false;
    signalProcessor->setNotchFilterEnabled(notchFilterEnabled);
    highpassFilterFrequency = 250.0;
    highpassFilterEnabled = false;
    signalProcessor->setHighpassFilterEnabled(highpassFilterEnabled);

    // Chip metadata
    chipId.resize(MAX_NUM_DATA_STREAMS);
    chipId.fill(-1);

    //Create a shared array for controlling digital lines. them.
    for (int i = 0; i < 16; ++i) {
        ttlOut[i] = 0;
    }

    cout << "done." << endl;
}

//void Rhd2000Impedance::GenerateAuxCommands() {

//}

// Create auxiliary commands required to perform impedance testing
void Rhd2000Impedance::configureImpedanceMeasurement()
{

    // Update the chipregister object with the board sample rate and ensure
    // the DSP is on
    chipRegisters->defineSampleRate(boardSampleRate);
    chipRegisters->enableDsp(dspEnabled);

    // Temp variables to hold aux commands
    vector<int> commandList;
    int command1SequenceLength, command3SequenceLength;

    // Create a command list for the AuxCmd1 slot defining either DC or a sine wave
    // at the selected impedance test waveform frequency

    // AuxCmd1, Bank 0 : DC output to reduce noise
    // NOTE: Because the frequency of this waveform is 0 Hz, it will
    // occupy all 1024 samples allotted for a given command sequence.
    // Don't use this length to determine the evalboard->run() time!
    chipRegisters->createCommandListZcheckDac(commandList, 0, 0);
    evalBoard->uploadCommandList(commandList, Rhd2000EvalBoard::AuxCmd1, 0);

    // AuxCmd1, Bank 1 : sine wave at impedance test frequency
    command1SequenceLength =
            chipRegisters->createCommandListZcheckDac(commandList, actualImpedanceFreq, 128.0);
    evalBoard->uploadCommandList(commandList, Rhd2000EvalBoard::AuxCmd1, 1);

    // Default selected bank to the DC output
    evalBoard->selectAuxCommandLength(Rhd2000EvalBoard::AuxCmd1,
                                      0, command1SequenceLength - 1);
    evalBoard->selectAuxCommandBank(usedPort,Rhd2000EvalBoard::AuxCmd1, 0);

    // For the AuxCmd3 slot, we will create two command sequences. Both sequences
    // will configure and read back the RHD2000 chip registers, but one sequence will
    // also run ADC calibration.
    // Before generating register configuration command sequences, set amplifier
    // bandwidth paramters.

    // Update filter settings etc
    actualDspCutoffFreq = chipRegisters->setDspCutoffFreq(desiredDspCutoffFreq);
    actualLowerBandwidth = chipRegisters->setLowerBandwidth(desiredLowerBandwidth);
    actualUpperBandwidth = chipRegisters->setUpperBandwidth(desiredUpperBandwidth);

    // AuxCmd3, Bank 0: ADC calibration
    command3SequenceLength = chipRegisters->createCommandListRegisterConfig(commandList, true);
    evalBoard->uploadCommandList(commandList, Rhd2000EvalBoard::AuxCmd3, 0);

    // AuxCmd3, Bank 1: No calibration
    command3SequenceLength = chipRegisters->createCommandListRegisterConfig(commandList, true);
    evalBoard->uploadCommandList(commandList, Rhd2000EvalBoard::AuxCmd3, 1);

    // AuxCmd3, Bank 2: No calibration, 100fF series cap for zcheck, zcheck enabled
    chipRegisters->enableZcheck(true);
    chipRegisters->setZcheckScale(Rhd2000Registers::ZcheckCs100fF);
    command3SequenceLength = chipRegisters->createCommandListRegisterConfig(commandList, true);
    evalBoard->uploadCommandList(commandList, Rhd2000EvalBoard::AuxCmd3, 2);

    // AuxCmd3, Bank 3: No calibration, 1pF series cap for zcheck, zcheck enabled
    chipRegisters->enableZcheck(true);
    chipRegisters->setZcheckScale(Rhd2000Registers::ZcheckCs1pF);
    command3SequenceLength = chipRegisters->createCommandListRegisterConfig(commandList, true);
    evalBoard->uploadCommandList(commandList, Rhd2000EvalBoard::AuxCmd3, 3);

    // AuxCmd3, Bank 4: No calibration, 10pF series cap for zcheck, zcheck enabled
    chipRegisters->enableZcheck(true);
    chipRegisters->setZcheckScale(Rhd2000Registers::ZcheckCs10pF);
    command3SequenceLength = chipRegisters->createCommandListRegisterConfig(commandList, true);
    evalBoard->uploadCommandList(commandList, Rhd2000EvalBoard::AuxCmd3, 4);
    evalBoard->selectAuxCommandLength(Rhd2000EvalBoard::AuxCmd3, 0, command3SequenceLength - 1);

    // Select RAM Bank 1 for AuxCmd3 initially, so the ADC is calibrated.
    evalBoard->selectAuxCommandBank(usedPort, Rhd2000EvalBoard::AuxCmd3, 0);

    // Determine the longest command sequence to ensure that all evalBoard->run()
    // calls will go for that many cycles
    commandSequenceLength = max(command1SequenceLength,command3SequenceLength);
    evalBoard->setMaxTimeStep(commandSequenceLength);
    evalBoard->setContinuousRunMode(false);

    // Calibrate the ADC
    cout << "Calibrating the ADC...";

    // Set the number of run cycles to the larger of the two command sequences
    evalBoard->run(); // Wait for the 60-sample run to complete.
    while (evalBoard->isRunning()) { }

    cout << "done." << endl;

    // Read the resulting single data block from the USB interface.
    Rhd2000DataBlock *dataBlock = new Rhd2000DataBlock(evalBoard->getNumEnabledDataStreams());
    evalBoard->readDataBlock(dataBlock);

    // Display register contents from data stream 0.
    dataBlock->print(0);

    // Now that calibration is complete, we must set the number of cycles to match
    // amount of time it takes to produce the impedance test waveform
    // TODO: I actually do not understand how this works!!!!
    numPeriods = qRound(0.020 * actualImpedanceFreq); // Test each channel for at least 20 msec...
    if (numPeriods < 5) numPeriods = 5; // ...but always measure across no fewer than 5 complete periods
    relativePeriod = boardSampleRate / actualImpedanceFreq;
    numBlocks = qCeil((numPeriods + 2.0) * relativePeriod / (double)SAMPLES_PER_DATA_BLOCK);  // + 2 periods to give time to settle initially
    if (numBlocks < 2) numBlocks = 2;   // need first block for command to switch channels to take effect.

    // All subsequence calls to evalBoard->run() will last for this many cycles
    evalBoard->setMaxTimeStep(SAMPLES_PER_DATA_BLOCK * numBlocks);

    // Create matrices of doubles of size (numStreams x 32 x 3) to store complex amplitudes
    // of all amplifier channels (32 on each data stream) at three different Cseries values.
    measuredMagnitude.resize(evalBoard->getNumEnabledDataStreams());
    measuredPhase.resize(evalBoard->getNumEnabledDataStreams());
    for (int i = 0; i < evalBoard->getNumEnabledDataStreams(); ++i)
    {
        measuredMagnitude[i].resize(32);
        measuredPhase[i].resize(32);
        for (int j = 0; j < 32; ++j)
        {
            measuredMagnitude[i][j].resize(3);
            measuredPhase[i][j].resize(3);
        }
    }

    // Now we are configured
    impedanceConfigured = true;

}

// Select the channel, relative the the currenly selected port, that should
// be tied to the elec_test/impedance measurment AC coupling path on the RHD
int Rhd2000Impedance::selectChannel(int selectedChannel) {

    // Check to see if the 64 channel chip is present
    int stream;
    bool rhd2164ChipPresent = false;
    for (stream = 0; stream < MAX_NUM_DATA_STREAMS; ++stream)
    {
        if (chipId[stream] == CHIP_ID_RHD2164_B)
        {
            rhd2164ChipPresent = true;
        }
    }

    if (selectedChannel >= 32 && !rhd2164ChipPresent)
    {
        cout << "Selected channel = " << channel << " but a 64-channel "
                "chip was not detected." << endl;
        return -1 ;
    }
    else if (selectedChannel >= 64) {
        cout << "Attempted to select a channel greater than 63. Please select a "
                "channel within the range of your amplifer" << endl;
        return -1 ;
    }

    channel = selectedChannel;
    chipRegisters->setZcheckChannel(channel);
    channelSelected = true;

    return 0;

}

// Measure the impedance on selected channel
int Rhd2000Impedance::measureImpedance()
{
    // Stuff we will need to do the measurements
    int stream, capRange;
    double cSeries;
    int triggerIndex;                       // dummy reference variable; not used
    queue<Rhd2000DataBlock> bufferQueue;    // dummy reference variable; not used


    if (!impedanceConfigured)
    {
        cout << "You must configure the impedance measurement "
                "before measuring impedance." << endl;
        return -1;
    }

    if (!channelSelected)
    {
        cout << "You must select a channel to test before measuring "
                "impedance." << endl;
        return -1;
    }

    // We execute three complete electrode impedance measurements: one each with
    // Cseries set to 0.1 pF, 1 pF, and 10 pF.  Then we select the best measurement
    // for each channel so that we achieve a wide impedance measurement range.
    for (capRange = 0; capRange < 3; ++ capRange)
    {
        switch (capRange) {
        case 0:
            evalBoard->selectAuxCommandBank(usedPort, Rhd2000EvalBoard::AuxCmd3, 2);
            cSeries = 0.1e-12;
            break;
        case 1:
            evalBoard->selectAuxCommandBank(usedPort, Rhd2000EvalBoard::AuxCmd3, 3);
            cSeries = 1.0e-12;
            break;
        case 2:
            evalBoard->selectAuxCommandBank(usedPort, Rhd2000EvalBoard::AuxCmd3, 4);
            cSeries = 10.0e-12;
            break;
        }

        // Select AuxCmd1 with the sine wave impedance test waveform
        evalBoard->selectAuxCommandBank(usedPort,Rhd2000EvalBoard::AuxCmd1, 1);

        // Run the test signal, collect the data
        // TODO: Is this thing actually running for the correct number of periods??
        evalBoard->run();
        while (evalBoard->isRunning() ) { }

        evalBoard->readDataBlocks(numBlocks, dataQueue);
        signalProcessor->loadAmplifierData(dataQueue, numBlocks, false,
                                           0, 0, triggerIndex, bufferQueue,
                                           false, *saveStream, saveFormat,
                                           false, false, 0);

        for (stream = 0; stream < evalBoard->getNumEnabledDataStreams(); ++stream)
        {
            signalProcessor->measureComplexAmplitude(measuredMagnitude,
                                                     measuredPhase,
                                                     capRange,
                                                     stream,
                                                     channel,
                                                     numBlocks,
                                                     boardSampleRate,
                                                     actualImpedanceFreq,
                                                     numPeriods);

        }

        //            // If an RHD2164 chip is plugged in, we have to set the Zcheck select register to channels 32-63
        //            // and repeat the previous steps.
        //            if (rhd2164ChipPresent) {
        //                chipRegisters.setZcheckChannel(channel + 32); // address channels 32-63
        //                commandSequenceLength =
        //                        chipRegisters.createCommandListRegisterConfig(commandList, false);
        //                // Upload version with no ADC calibration to AuxCmd3 RAM Bank 1.
        //                evalBoard->uploadCommandList(commandList, Rhd2000EvalBoard::AuxCmd3, 3);

        //                evalBoard->run();
        //                while (evalBoard->isRunning() ) { }
        //                evalBoard->readDataBlocks(numBlocks, dataQueue);
        //                signalProcessor->loadAmplifierData(dataQueue, numBlocks, false, 0, 0, triggerIndex, bufferQueue,
        //                                                   false, *saveStream, saveFormat, false, false, 0);
        //                for (stream = 0; stream < evalBoard->getNumEnabledDataStreams(); ++stream) {
        //                    if (chipId[stream] == CHIP_ID_RHD2164_B) {
        //                        signalProcessor->measureComplexAmplitude(measuredMagnitude, measuredPhase,
        //                                                                 capRange, stream, channel,  numBlocks, boardSampleRate,
        //                                                                 actualImpedanceFreq, numPeriods);
        //                    }
        //                }
        //            }


        //}
    }

    // Now that we have the raw response data for each series cap value
    // we can process that data to derive the reistance and phase values for
    // the channel under test
    SignalChannel *signalChannel;
    double distance, minDistance, current, Cseries;
    double impedanceMagnitude, impedancePhase;

    const double bestAmplitude = 250.0;  // we favor voltage readings that are closest to 250 uV: not too large,
    // and not too small.
    const double dacVoltageAmplitude = 128 * (1.225 / 256);  // this assumes the DAC amplitude was set to 128
    const double parasiticCapacitance = 14.0e-12;  // 14 pF: an estimate of on-chip parasitic capacitance,
    // including 10 pF of amplifier input capacitance.
    double relativeFreq = actualImpedanceFreq / boardSampleRate;

    int bestAmplitudeIndex;
    for (stream = 0; stream < evalBoard->getNumEnabledDataStreams(); ++stream)
    {
        signalChannel = signalSources->findAmplifierChannel(stream, channel);

        if (signalChannel)
        {
            minDistance = 9.9e99;  // ridiculously large number
            for (capRange = 0; capRange < 3; ++capRange)
            {
                // Find the measured amplitude that is closest to bestAmplitude on a logarithmic scale
                distance = qAbs(qLn(measuredMagnitude[stream][channel][capRange] / bestAmplitude));
                if (distance < minDistance)
                {
                    bestAmplitudeIndex = capRange;
                    minDistance = distance;
                }
            }
            switch (bestAmplitudeIndex)
            {
            case 0:
                Cseries = 0.1e-12;
                break;
            case 1:
                Cseries = 1.0e-12;
                break;
            case 2:
                Cseries = 10.0e-12;
                break;
            }

            // Calculate current amplitude produced by on-chip voltage DAC
            current = TWO_PI * actualImpedanceFreq * dacVoltageAmplitude * Cseries;

            // Calculate impedance magnitude from calculated current and measured voltage.
            impedanceMagnitude = 1.0e-6 * (measuredMagnitude[stream][channel][bestAmplitudeIndex] / current) *
                    (18.0 * relativeFreq * relativeFreq + 1.0);

            // Calculate impedance phase, with small correction factor accounting for the
            // 3-command SPI pipeline delay.
            impedancePhase = measuredPhase[stream][channel][bestAmplitudeIndex] + (360.0 * (3.0 / relativePeriod));

            // Factor out on-chip parasitic capacitance from impedance measurement.
            factorOutParallelCapacitance(impedanceMagnitude, impedancePhase, actualImpedanceFreq,
                                         parasiticCapacitance);

            // Perform empirical resistance correction to improve accuarcy at sample rates below
            // 15 kS/s.
            empiricalResistanceCorrection(impedanceMagnitude, impedancePhase,
                                          boardSampleRate);

            signalChannel->electrodeImpedanceMagnitude = impedanceMagnitude;
            signalChannel->electrodeImpedancePhase = impedancePhase;
        }
    }

    evalBoard->setContinuousRunMode(false);
    evalBoard->setMaxTimeStep(0);
    evalBoard->flush();

    // Switch back to flatline
    // TODO: is the first command overwritten?
    evalBoard->selectAuxCommandBank(usedPort, Rhd2000EvalBoard::AuxCmd1, 0);
    evalBoard->selectAuxCommandLength(Rhd2000EvalBoard::AuxCmd1, 0, 59);
    evalBoard->selectAuxCommandBank(usedPort, Rhd2000EvalBoard::AuxCmd3, 1);

    // Success
    return 0;
}

// Measure the impedance on selected channel at selected frequency
int Rhd2000Impedance::measureImpedance(double Fs)
{
    // Switch to desired impedance test frequency, generate
    // the test signal, and test impedance
    changeImpedanceFrequency(Fs);
    return measureImpedance();
}

// Public method for changing impedance test signal frequency
void Rhd2000Impedance::changeImpedanceFrequency(double Fs)
{
    // Examine desired frequency to make sure its OK
    desiredImpedanceFreq = Fs;
    updateImpedanceFrequency();

    // Update impedance measurment configurations with new test freq.
    impedanceConfigured = false;
    configureImpedanceMeasurement();

}

void Rhd2000Impedance::printImpedance()
{

    if (!channelSelected) {
        cout << "Channel has not been selected, so a impedance results cannot "
                "be printed." << endl;

        return;
    }

    // Construct channel name
    SignalChannel *signalChannel;

    switch (usedPort)
    {
    case Rhd2000EvalBoard::PortA :
        signalChannel = &signalSources->signalPort[0].channel[channel];
        break;
    case Rhd2000EvalBoard::PortB :
        signalChannel = &signalSources->signalPort[1].channel[channel];
        break;
    case Rhd2000EvalBoard::PortC :
        signalChannel = &signalSources->signalPort[2].channel[channel];
        break;
    case Rhd2000EvalBoard::PortD :
        signalChannel = &signalSources->signalPort[3].channel[channel];
        break;
    }

    cout << "   Channel: " << channel << endl;
    cout << "   Test freq. (Hz): " <<  actualImpedanceFreq << endl;
    cout << "   Magnitude (ohms): " << signalChannel->electrodeImpedanceMagnitude << endl;
    cout << "   Phase (deg.): " << signalChannel->electrodeImpedancePhase << endl;
}

double Rhd2000Impedance::getImpedanceMagnitude() {

    if (!channelSelected) {
        cout << "Channel has not been selected, so a impedance magnitude measurement "
                "cannot be returned." << endl;

        return std::numeric_limits<double>::quiet_NaN();;
    }

    // Construct channel name
    SignalChannel *signalChannel;

    switch (usedPort)
    {
    case Rhd2000EvalBoard::PortA :
        signalChannel = &signalSources->signalPort[0].channel[channel];
        break;
    case Rhd2000EvalBoard::PortB :
        signalChannel = &signalSources->signalPort[1].channel[channel];
        break;
    case Rhd2000EvalBoard::PortC :
        signalChannel = &signalSources->signalPort[2].channel[channel];
        break;
    case Rhd2000EvalBoard::PortD :
        signalChannel = &signalSources->signalPort[3].channel[channel];
        break;
    }

    return signalChannel->electrodeImpedanceMagnitude;

}

double Rhd2000Impedance::getImpedancePhase() {

    if (!channelSelected) {
        cout << "Channel has not been selected, so a impedance phase measurement "
                "cannot be returned." << endl;

        return std::numeric_limits<double>::quiet_NaN();;
    }

    // Construct channel name
    SignalChannel *signalChannel;

    switch (usedPort)
    {
    case Rhd2000EvalBoard::PortA :
        signalChannel = &signalSources->signalPort[0].channel[channel];
        break;
    case Rhd2000EvalBoard::PortB :
        signalChannel = &signalSources->signalPort[1].channel[channel];
        break;
    case Rhd2000EvalBoard::PortC :
        signalChannel = &signalSources->signalPort[2].channel[channel];
        break;
    case Rhd2000EvalBoard::PortD :
        signalChannel = &signalSources->signalPort[3].channel[channel];
        break;
    }

    return signalChannel->electrodeImpedancePhase;

}

// This function loads the Intan firmware to the evaluation board and scans port
// A for an Intan chip
void Rhd2000Impedance::setupEvalBoard()
{
    cout << "Intializing evaluation board..." << endl;
    evalBoard = new Rhd2000EvalBoard;

    // Open Opal Kelly XEM6010 board.
    int errorCode = evalBoard->open();
    if (errorCode == -1)
    {
        cout << "Opal Kelly USB drivers not installed. Please install the correct "
                "Opal Kelly drivers and restart the application." << endl;

        exit(EXIT_FAILURE);
    }

    // Load Rhythm FPGA configuration bitfile (provided by Intan Technologies).
    // Place main.bit in the executable directory, or add a complete path to file.
    string bitfilename = "M:/public/impedance/build/debug/main.bit";
    if (!evalBoard->uploadFpgaBitfile(bitfilename))
    {
        cout << "FPGA configuration file upload error. "
                "Make sure file main.bit is in the same "
                "directory as the executable file." << endl;

        exit(EXIT_FAILURE);
    }

    // Initialize board.
    evalBoard->initialize();

    // Set the ttlMode to manual and clear the digital lines
    evalBoard->setTtlMode(0);
    evalBoard->clearTtlOut();

    // Set sample rate. 20000 Hz is a good choice for impedance testing
    changeSampleRate(Rhd2000EvalBoard::SampleRate20000Hz);

    // Disable external fast settling, since this interferes with DAC commands in AuxCmd1.
    evalBoard->enableExternalFastSettle(false);

    // Disable auxiliary digital output control during impedance measurements.
    evalBoard->enableExternalDigOut(usedPort, false);

    cout << "Evaluation board initialization successful." << endl;
}

// Perform amplifier calibration, find the cable delay, prepare configuration and
// data measurement commands, etc.
void Rhd2000Impedance::setupAmplifier()
{

    int delay, stream, id, i, auxName, vddName;
    int register59Value;
    int numChannelsOnPort = 0;
    QVector<int> portIndex, portIndexOld, chipIdOld;

    portIndex.resize(MAX_NUM_DATA_STREAMS);
    portIndexOld.resize(MAX_NUM_DATA_STREAMS);
    chipIdOld.resize(MAX_NUM_DATA_STREAMS);

    chipId.fill(-1);
    chipIdOld.fill(-1);
    portIndexOld.fill(-1);
    portIndex.fill(-1);

    // This program only allows one port to be use, but will allow
    // multiple chips to be attached to that port and for 64 channel
    // DDR chips to be used.
    Rhd2000EvalBoard::BoardDataSource initStreamPorts[2];
    Rhd2000EvalBoard::BoardDataSource initStreamDdrPorts[2];

    switch (usedPort)
    {
    case Rhd2000EvalBoard::PortA :
        initStreamPorts[0]  = Rhd2000EvalBoard::PortA1;
        initStreamPorts[1]  = Rhd2000EvalBoard::PortA2;
        initStreamDdrPorts[0] = Rhd2000EvalBoard::PortA1Ddr;
        initStreamDdrPorts[1] = Rhd2000EvalBoard::PortA2Ddr;
        break;
    case Rhd2000EvalBoard::PortB :
        initStreamPorts[0]  = Rhd2000EvalBoard::PortB1;
        initStreamPorts[1]  = Rhd2000EvalBoard::PortB2;
        initStreamDdrPorts[0] = Rhd2000EvalBoard::PortB1Ddr;
        initStreamDdrPorts[1] = Rhd2000EvalBoard::PortB2Ddr;
        break;
    case Rhd2000EvalBoard::PortC :
        initStreamPorts[0]  = Rhd2000EvalBoard::PortC1;
        initStreamPorts[1]  = Rhd2000EvalBoard::PortC2;
        initStreamDdrPorts[0] = Rhd2000EvalBoard::PortC1Ddr;
        initStreamDdrPorts[1] = Rhd2000EvalBoard::PortC2Ddr;
        break;
    case Rhd2000EvalBoard::PortD :
        initStreamPorts[0]  = Rhd2000EvalBoard::PortD1;
        initStreamPorts[1]  = Rhd2000EvalBoard::PortD2;
        initStreamDdrPorts[0] = Rhd2000EvalBoard::PortD1Ddr;
        initStreamDdrPorts[1] = Rhd2000EvalBoard::PortD2Ddr;
        break;
    }

    // Bring the sample rate up the highest possible amount to maximize the need
    // for precise temporal resolution in order to find the correct cable delay
    changeSampleRate(evalBoard->SampleRate30000Hz);

    // Set the data source to cover up to two chips/port
    evalBoard->setDataSource(0, initStreamPorts[0]);
    evalBoard->setDataSource(1, initStreamPorts[1]);
    evalBoard->enableDataStream(0, true);

    portIndexOld[0] = 0;
    portIndexOld[1] = 0;
    portIndexOld[2] = 1;
    portIndexOld[3] = 1;
    portIndexOld[4] = 2;
    portIndexOld[5] = 2;
    portIndexOld[6] = 3;
    portIndexOld[7] = 3;

    evalBoard->enableDataStream(0, true);
    evalBoard->enableDataStream(1, true);

    // Select the command sequence with amplifier configuration
    evalBoard->selectAuxCommandBank(usedPort, Rhd2000EvalBoard::AuxCmd3, 0);

    // Since our longest command sequence is 60 commands, we run the SPI
    // interface for 60 samples.
    evalBoard->setMaxTimeStep(60);
    evalBoard->setContinuousRunMode(false);

    // Temporary data from the delay finding sequence
    Rhd2000DataBlock *dataBlock =
            new Rhd2000DataBlock(evalBoard->getNumEnabledDataStreams());
    QVector<int> sumGoodDelays(MAX_NUM_DATA_STREAMS, 0);
    QVector<int> indexFirstGoodDelay(MAX_NUM_DATA_STREAMS, -1);
    QVector<int> indexSecondGoodDelay(MAX_NUM_DATA_STREAMS, -1);

    cout << "Finding MISO read delays to account for cable length..." << endl;

    // Run SPI command sequence at all 16 possible FPGA MISO delay settings
    // to find optimum delay for each SPI interface cable.
    for (delay = 0; delay < 16; ++delay)
    {
        // Set current cable delay
        evalBoard->setCableDelay(usedPort, delay);
        double delayns = 1.0e9 * (double)evalBoard->getCableDelay(usedPort)*(1.0/2800.0) * (1.0/evalBoard->getSampleRate());
        cout << "   testing " << delayns << " ns delay." << endl;

        // Start SPI interface.
        evalBoard->run();

        // Wait for the 60-sample run to complete.
        while (evalBoard->isRunning()) {}

        // Read the resulting single data block from the USB interface.
        evalBoard->readDataBlock(dataBlock);

        // Read the Intan chip ID number from each RHD2000 chip found.
        // Record delay settings that yield good communication with the chip.
        for (stream = 0; stream < evalBoard->getNumEnabledDataStreams(); ++stream)
        {
            // Figure out if the dataBlock contains valid data
            id = deviceId(dataBlock, stream, register59Value);

            if (id == CHIP_ID_RHD2132 || id == CHIP_ID_RHD2216 ||
                    (id == CHIP_ID_RHD2164 && register59Value == REGISTER_59_MISO_A))
            {

                sumGoodDelays[stream] = sumGoodDelays[stream] + 1;
                if (indexFirstGoodDelay[stream] == -1)
                {
                    indexFirstGoodDelay[stream] = delay;
                    chipIdOld[stream] = id;
                }
                else if (indexSecondGoodDelay[stream] == -1)
                {
                    indexSecondGoodDelay[stream] = delay;
                    chipIdOld[stream] = id;
                }
            }
        }
    }

    // Set cable delay settings that yield good communication with each
    // RHD2000 chip.
    QVector<int> optimumDelay(MAX_NUM_DATA_STREAMS, 0);
    for (stream = 0; stream < MAX_NUM_DATA_STREAMS; ++stream)
    {
        if (sumGoodDelays[stream] == 1 || sumGoodDelays[stream] == 2)
        {
            optimumDelay[stream] = indexFirstGoodDelay[stream];
        } else if (sumGoodDelays[stream] > 2)
        {
            optimumDelay[stream] = indexSecondGoodDelay[stream];
        }
    }

    int optDelay = qMax(optimumDelay[0], optimumDelay[1]);
    double delayns = 1.0e9 * (double)optDelay*(1.0/2800.0) * (1.0/evalBoard->getSampleRate());
    cout << "Optimal delay: " << delayns << " ns." << endl;
    evalBoard->setCableDelay(usedPort,optDelay);
    cableLengthMeters =
            evalBoard->estimateCableLengthMeters(optDelay);
    cout << "Estimated cable length: " << cableLengthMeters << " meters." << endl;

    // Now that we found the cable length, discard the datablock
    delete dataBlock;

    // Now that we know which RHD2000 amplifier chips are plugged into each SPI port,
    // add up the total number of amplifier channels on each port and calcualate the number
    // of data streams necessary to convey this data over the USB interface.
    int numStreamsRequired = 0;
    bool rhd2216ChipPresent = false;
    for (stream = 0; stream < MAX_NUM_DATA_STREAMS; ++stream) {
        if (chipIdOld[stream] == CHIP_ID_RHD2216) {
            numStreamsRequired++;
            if (numStreamsRequired <= MAX_NUM_DATA_STREAMS) {
                numChannelsOnPort += 16;
            }
            rhd2216ChipPresent = true;
        }
        if (chipIdOld[stream] == CHIP_ID_RHD2132) {
            numStreamsRequired++;
            if (numStreamsRequired <= MAX_NUM_DATA_STREAMS) {
                numChannelsOnPort += 32;
            }
        }
        if (chipIdOld[stream] == CHIP_ID_RHD2164) {
            numStreamsRequired += 2;
            if (numStreamsRequired <= MAX_NUM_DATA_STREAMS) {
                numChannelsOnPort += 64;
            }
        }
    }

    // Reconfigure USB data streams in consecutive order to accommodate all connected chips.
    stream = 0;
    for (int oldStream = 0; oldStream < MAX_NUM_DATA_STREAMS; ++oldStream) {
        if ((chipIdOld[oldStream] == CHIP_ID_RHD2216) && (stream < MAX_NUM_DATA_STREAMS)) {
            chipId[stream] = CHIP_ID_RHD2216;
            portIndex[stream] = portIndexOld[oldStream];
            evalBoard->enableDataStream(stream, true);
            evalBoard->setDataSource(stream, initStreamPorts[oldStream]);
            stream++;
        } else if ((chipIdOld[oldStream] == CHIP_ID_RHD2132) && (stream < MAX_NUM_DATA_STREAMS)) {
            chipId[stream] = CHIP_ID_RHD2132;
            portIndex[stream] = portIndexOld[oldStream];
            evalBoard->enableDataStream(stream, true);
            evalBoard->setDataSource(stream, initStreamPorts[oldStream]);
            stream++ ;
        } else if ((chipIdOld[oldStream] == CHIP_ID_RHD2164) && (stream < MAX_NUM_DATA_STREAMS - 1)) {
            chipId[stream] = CHIP_ID_RHD2164;
            chipId[stream + 1] =  CHIP_ID_RHD2164_B;
            portIndex[stream] = portIndexOld[oldStream];
            portIndex[stream + 1] = portIndexOld[oldStream];
            evalBoard->enableDataStream(stream, true);
            evalBoard->enableDataStream(stream + 1, true);
            evalBoard->setDataSource(stream, initStreamPorts[oldStream]);
            evalBoard->setDataSource(stream + 1, initStreamDdrPorts[oldStream]);
            stream += 2;
        }
    }

    // Disable unused data streams.
    for (; stream < MAX_NUM_DATA_STREAMS; ++stream) {
        evalBoard->enableDataStream(stream, false);
    }

    // Add channel descriptions to the SignalSources object to create a list of all waveforms.
    if (numChannelsOnPort == 0)
    {
        signalSources->signalPort[usedPort].channel.clear();
        signalSources->signalPort[usedPort].enabled = false;
    }
    else if (signalSources->signalPort[usedPort].numAmplifierChannels() !=
               numChannelsOnPort)
    {  // if number of channels on port has changed...

        // ...clear existing channels...
        signalSources->signalPort[usedPort].channel.clear();

        // ...and create new ones.
        int channelIdx = 0;

        // Create amplifier channels for each chip.
        for (stream = 0; stream < MAX_NUM_DATA_STREAMS; ++stream) {
            if (portIndex[stream] == usedPort) {
                if (chipId[stream] == CHIP_ID_RHD2216) {
                    for (i = 0; i < 16; ++i) {
                        signalSources->signalPort[usedPort].addAmplifierChannel(channelIdx++, i, stream);
                    }
                } else if (chipId[stream] == CHIP_ID_RHD2132) {
                    for (i = 0; i < 32; ++i) {
                        signalSources->signalPort[usedPort].addAmplifierChannel(channelIdx++, i, stream);
                    }
                } else if (chipId[stream] == CHIP_ID_RHD2164) {
                    for (i = 0; i < 32; ++i) {  // 32 channels on MISO A; another 32 on MISO B
                        signalSources->signalPort[usedPort].addAmplifierChannel(channelIdx++, i, stream);
                    }
                } else if (chipId[stream] == CHIP_ID_RHD2164_B) {
                    for (i = 0; i < 32; ++i) {  // 32 channels on MISO A; another 32 on MISO B
                        signalSources->signalPort[usedPort].addAmplifierChannel(channelIdx++, i, stream);
                    }
                }
            }
        }

        // Now create auxiliary input channels and supply voltage channels for each chip.
        auxName = 1;
        vddName = 1;
        for (stream = 0; stream < MAX_NUM_DATA_STREAMS; ++stream) {
            if (portIndex[stream] == usedPort) {
                if (chipId[stream] == CHIP_ID_RHD2216 ||
                        chipId[stream] == CHIP_ID_RHD2132 ||
                        chipId[stream] == CHIP_ID_RHD2164) {
                    signalSources->signalPort[usedPort].addAuxInputChannel(channel++, 0, auxName++, stream);
                    signalSources->signalPort[usedPort].addAuxInputChannel(channel++, 1, auxName++, stream);
                    signalSources->signalPort[usedPort].addAuxInputChannel(channel++, 2, auxName++, stream);
                    signalSources->signalPort[usedPort].addSupplyVoltageChannel(channel++, 0, vddName++, stream);
                }
            }
        }
    }
    else
    {    // If number of channels on port has not changed, don't create new channels (since this
        // would clear all user-defined channel names.  But we must update the data stream indices
        // on the port.
        int channelIdx = 0;
        // Update stream indices for amplifier channels.
        for (stream = 0; stream < MAX_NUM_DATA_STREAMS; ++stream) {
            if (portIndex[stream] == usedPort) {
                if (chipId[stream] == CHIP_ID_RHD2216) {
                    for (i = channel; i < channel + 16; ++i) {
                        signalSources->signalPort[usedPort].channel[i].boardStream = stream;
                    }
                    channel += 16;
                } else if (chipId[stream] == CHIP_ID_RHD2132) {
                    for (i = channelIdx; i < channelIdx + 32; ++i) {
                        signalSources->signalPort[usedPort].channel[i].boardStream = stream;
                    }
                    channelIdx += 32;
                } else if (chipId[stream] == CHIP_ID_RHD2164) {
                    for (i = channelIdx; i < channelIdx + 32; ++i) {  // 32 channels on MISO A; another 32 on MISO B
                        signalSources->signalPort[usedPort].channel[i].boardStream = stream;
                    }
                    channelIdx += 32;
                } else if (chipId[stream] == CHIP_ID_RHD2164_B) {
                    for (i = channelIdx; i < channelIdx + 32; ++i) {  // 32 channels on MISO A; another 32 on MISO B
                        signalSources->signalPort[usedPort].channel[i].boardStream = stream;
                    }
                    channelIdx += 32;
                }
            }
        }
        // Update stream indices for auxiliary channels and supply voltage channels.
        for (stream = 0; stream < MAX_NUM_DATA_STREAMS; ++stream) {
            if (portIndex[stream] == usedPort) {
                if (chipId[stream] == CHIP_ID_RHD2216 ||
                        chipId[stream] == CHIP_ID_RHD2132 ||
                        chipId[stream] == CHIP_ID_RHD2164) {
                    signalSources->signalPort[usedPort].channel[channelIdx++].boardStream = stream;
                    signalSources->signalPort[usedPort].channel[channelIdx++].boardStream = stream;
                    signalSources->signalPort[usedPort].channel[channelIdx++].boardStream = stream;
                    signalSources->signalPort[usedPort].channel[channelIdx++].boardStream = stream;
                }
            }
        }
    }



    // Allocate space for data being streamed from the eval board in the
    // signal processor object
    signalProcessor->allocateMemory(evalBoard->getNumEnabledDataStreams());

    // Return sample rate to 20 kHz which is good for impedance testing.
    changeSampleRate(Rhd2000EvalBoard::SampleRate20000Hz);
}

// Change the sample rate of the evalboard, update all parameters dependent on
// the new sample rate
void Rhd2000Impedance::changeSampleRate(Rhd2000EvalBoard::AmplifierSampleRate Fs)
{
    Rhd2000EvalBoard::AmplifierSampleRate sampleRate;

    // Allow three sample rates to be choosen
    switch (Fs)
    {
    case Rhd2000EvalBoard::SampleRate10000Hz:
        sampleRate = Rhd2000EvalBoard::SampleRate10000Hz;
        boardSampleRate = 10000.0;
        numUsbBlocksToRead = 6;
        break;
    case Rhd2000EvalBoard::SampleRate20000Hz:
        sampleRate = Rhd2000EvalBoard::SampleRate20000Hz;
        boardSampleRate = 20000.0;
        numUsbBlocksToRead = 12;
        break;
    case Rhd2000EvalBoard::SampleRate30000Hz:
        sampleRate = Rhd2000EvalBoard::SampleRate30000Hz;
        boardSampleRate = 30000.0;
        numUsbBlocksToRead = 16;
        break;
    default:
        cout << "Use either SampleRate10000Hz, 20000Hz, or 30000Hz. "
                "Set by defualt set to 20000Hz." << endl;
        sampleRate = Rhd2000EvalBoard::SampleRate20000Hz;
        boardSampleRate = 20000.0;
        numUsbBlocksToRead = 12;
        break;
    }

//    TODO: THIS SECTION OF CODE IS HIGHLY REDUNDANT WITH THE IMPEDANCE CONFIGURATION
//            CODE. IDEALLY, THERE SHOULD BE A CHIP CALIBRATION ROUTINE ANYTIME A 'HARDWARE'
//            PARAMETER IS CHANGED (E.G. BOARD SAMPLE RATE) AND AN IMPEDANCE CONFIGURATION
//            FOR WHEN AN IMPEDANCE PARAMETER IS CHANGED (E.G. TEST FREQUENCY)

    // Set up an RHD2000 register object using this sample rate to
    // optimize MUX-related register settings.
    chipRegisters->defineSampleRate(boardSampleRate);
    //Rhd2000Registers chipRegisters(boardSampleRate);

    // Now, we will optimize the chip for the selected sample rate
    int commandSequenceLength;
    vector<int> commandList;

    evalBoard->setSampleRate(sampleRate);
    evalBoard->setCableLengthMeters(Rhd2000EvalBoard::PortA, cableLengthMeters);

    // Create a command list for the AuxCmd1 slot.  This command sequence will continuously
    // update Register 3, which controls the auxiliary digital output pin on each RHD2000 chip.
    // In concert with the v1.4 Rhythm FPGA code, this permits real-time control of the digital
    // output pin on chips on each SPI port.
    chipRegisters->setDigOutLow();   // Take auxiliary output out of HiZ mode.
    commandSequenceLength = chipRegisters->createCommandListUpdateDigOut(commandList);
    evalBoard->uploadCommandList(commandList, Rhd2000EvalBoard::AuxCmd1, 0);
    evalBoard->selectAuxCommandLength(Rhd2000EvalBoard::AuxCmd1, 0, commandSequenceLength - 1);
    evalBoard->selectAuxCommandBank(usedPort, Rhd2000EvalBoard::AuxCmd1, 0);

    // Next, we'll create a command list for the AuxCmd2 slot.  This command sequence
    // will sample the temperature sensor and other auxiliary ADC inputs.
    commandSequenceLength = chipRegisters->createCommandListTempSensor(commandList);
    evalBoard->uploadCommandList(commandList, Rhd2000EvalBoard::AuxCmd2, 0);
    evalBoard->selectAuxCommandLength(Rhd2000EvalBoard::AuxCmd2, 0, commandSequenceLength - 1);
    evalBoard->selectAuxCommandBank(usedPort, Rhd2000EvalBoard::AuxCmd2, 0);

    // Before generating register configuration command sequences, set amplifier
    // bandwidth paramters.
    actualDspCutoffFreq = chipRegisters->setDspCutoffFreq(desiredDspCutoffFreq);
    actualLowerBandwidth = chipRegisters->setLowerBandwidth(desiredLowerBandwidth);
    actualUpperBandwidth = chipRegisters->setUpperBandwidth(desiredUpperBandwidth);
    chipRegisters->enableDsp(dspEnabled);

    // Generate configuration commands
    chipRegisters->createCommandListRegisterConfig(commandList, true);

    // Upload version with ADC calibration to AuxCmd3 RAM Bank 0.
    evalBoard->uploadCommandList(commandList, Rhd2000EvalBoard::AuxCmd3, 0);
    evalBoard->selectAuxCommandLength(Rhd2000EvalBoard::AuxCmd3, 0,
                                      commandSequenceLength - 1);

    // Upload version with no ADC calibration to AuxCmd3 RAM Bank 1.
    commandSequenceLength = chipRegisters->createCommandListRegisterConfig(commandList, false);
    evalBoard->uploadCommandList(commandList, Rhd2000EvalBoard::AuxCmd3, 1);
    evalBoard->selectAuxCommandLength(Rhd2000EvalBoard::AuxCmd3, 0,
                                      commandSequenceLength - 1);

    // Select the AuxCmd3 RAM w/o calibration as the default
    evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortA, Rhd2000EvalBoard::AuxCmd3, 1);

    // Update signal processor settings since they are dependent on the sample rate
    signalProcessor->setNotchFilter(notchFilterFrequency, notchFilterBandwidth, boardSampleRate);
    signalProcessor->setHighpassFilter(highpassFilterFrequency, boardSampleRate);

    // Update the impedance test frequency and measurement configuration
    // since they dependent on the sample rate
    impedanceFreqValid = false;
    updateImpedanceFrequency();
    impedanceConfigured = false;
    configureImpedanceMeasurement();
}

// Update electrode impedance measurement frequency, after checking that
// requested test frequency lies within acceptable ranges based on the
// amplifier bandwidth and the sampling rate.
void Rhd2000Impedance::updateImpedanceFrequency()
{
    cout << "Updating impedance test frequency..." << endl;

    int impedancePeriod;
    double lowerBandwidthLimit, upperBandwidthLimit;

    upperBandwidthLimit = actualUpperBandwidth / 1.5;
    lowerBandwidthLimit = actualLowerBandwidth * 1.5;

    // Ensure that the choosen impedance test frequency is sane
    if (dspEnabled)
    {
        if (actualDspCutoffFreq > actualLowerBandwidth)
        {
            lowerBandwidthLimit = actualDspCutoffFreq * 1.5;
        }
    }

    if (desiredImpedanceFreq > 0.0)
    {
        impedancePeriod = qRound(boardSampleRate / desiredImpedanceFreq);
        if (impedancePeriod >= 4 && impedancePeriod <= 1024 &&
                desiredImpedanceFreq >= lowerBandwidthLimit &&
                desiredImpedanceFreq <= upperBandwidthLimit)
        {
            actualImpedanceFreq = boardSampleRate / impedancePeriod;
            impedanceFreqValid = true;
        }
        else
        {
            actualImpedanceFreq = 0.0;
            impedanceFreqValid = false;
        }
    }
    else
    {
        actualImpedanceFreq = 0.0;
        impedanceFreqValid = false;
    }
    if (impedanceFreqValid)
    {
        cout << "done." << endl;
    }
    else
    {
        cout << "   Invalid impedance test frequency."      << endl;
        cout << "   Impedance waveform must have :"         << endl;
        cout << "       freq >= " << lowerBandwidthLimit    << endl;
        cout << "       freq <= " << upperBandwidthLimit    << endl;
        cout << "       period > 4 samples"                 << endl;
        cout << "       period < 1024 samples"              << endl;
        cout << "   Test frequency temporarly set to 0."    << endl;
    }
}

// Return the Intan chip ID stored in ROM register 63.  If the data is invalid
// (due to a SPI communication channel with the wrong delay or a chip not present)
// then return -1.  The value of ROM register 59 is also returned.  This register
// has a value of 0 on RHD2132 and RHD2216 chips, but in RHD2164 chips it is used
// to align the DDR MISO A/B data from the SPI bus.  (Register 59 has a value of 53 on MISO A and a value of 58 on MISO B.)
int Rhd2000Impedance::deviceId(Rhd2000DataBlock *dataBlock, int stream, int &register59Value)
{
    bool intanChipPresent;

    // First, check ROM registers 32-36 to verify that they hold 'INTAN', and
    // the initial chip name ROM registers 24-26 that hold 'RHD'.
    // This is just used to verify that we are getting good data over the SPI
    // communication channel.
    intanChipPresent = ((char) dataBlock->auxiliaryData[stream][2][32] == 'I' &&
            (char) dataBlock->auxiliaryData[stream][2][33] == 'N' &&
            (char) dataBlock->auxiliaryData[stream][2][34] == 'T' &&
            (char) dataBlock->auxiliaryData[stream][2][35] == 'A' &&
            (char) dataBlock->auxiliaryData[stream][2][36] == 'N' &&
            (char) dataBlock->auxiliaryData[stream][2][24] == 'R' &&
            (char) dataBlock->auxiliaryData[stream][2][25] == 'H' &&
            (char) dataBlock->auxiliaryData[stream][2][26] == 'D');

    // If the SPI communication is bad, return -1.  Otherwise, return the Intan
    // chip ID number stored in ROM regstier 63.
    if (!intanChipPresent)
    {
        register59Value = -1;
        return -1;
    } else
    {
        register59Value = dataBlock->auxiliaryData[stream][2][23]; // Register 59
        return dataBlock->auxiliaryData[stream][2][19]; // chip ID (Register 63)
    }
}

// Given a measured complex impedance that is the result of an electrode impedance in parallel
// with a parasitic capacitance (i.e., due to the amplifier input capacitance and other
// capacitances associated with the chip bondpads), this function factors out the effect of the
// parasitic capacitance to return the acutal electrode impedance.

void Rhd2000Impedance::factorOutParallelCapacitance(double &impedanceMagnitude,
                                                    double &impedancePhase,
                                                    double frequency,
                                                    double parasiticCapacitance)
{
    // First, convert from polar coordinates to rectangular coordinates.
    double measuredR = impedanceMagnitude * qCos(DEGREES_TO_RADIANS * impedancePhase);
    double measuredX = impedanceMagnitude * qSin(DEGREES_TO_RADIANS * impedancePhase);

    double capTerm = TWO_PI * frequency * parasiticCapacitance;
    double xTerm = capTerm * (measuredR * measuredR + measuredX * measuredX);
    double denominator = capTerm * xTerm + 2 * capTerm * measuredX + 1;
    double trueR = measuredR / denominator;
    double trueX = (measuredX + xTerm) / denominator;

    // Now, convert from rectangular coordinates back to polar coordinates.
    impedanceMagnitude = qSqrt(trueR * trueR + trueX * trueX);
    impedancePhase = RADIANS_TO_DEGREES * qAtan2(trueX, trueR);
}

// This is a purely empirical function to correct observed errors in the real component
// of measured electrode impedances at sampling rates below 15 kS/s.  At low sampling rates,
// it is difficult to approximate a smooth sine wave with the on-chip voltage DAC and 10 kHz
// 2-pole lowpass filter.  This function attempts to somewhat correct for this, but a better
// solution is to always run impedance measurements at 20 kS/s, where they seem to be most
// accurate.
void Rhd2000Impedance::empiricalResistanceCorrection(double &impedanceMagnitude,
                                                     double &impedancePhase,
                                                     double boardSampleRate)
{
    // First, convert from polar coordinates to rectangular coordinates.
    double impedanceR = impedanceMagnitude * qCos(DEGREES_TO_RADIANS * impedancePhase);
    double impedanceX = impedanceMagnitude * qSin(DEGREES_TO_RADIANS * impedancePhase);

    // Emprically derived correction factor (i.e., no physical basis for this equation).
    impedanceR /= 10.0 * qExp(-boardSampleRate / 2500.0) * qCos(TWO_PI * boardSampleRate / 15000.0) + 1.0;

    // Now, convert from rectangular coordinates back to polar coordinates.
    impedanceMagnitude = qSqrt(impedanceR * impedanceR + impedanceX * impedanceX);
    impedancePhase = RADIANS_TO_DEGREES * qAtan2(impedanceX, impedanceR);
}


int Rhd2000Impedance::plate(double currentuA, unsigned long durationMilliSec) {

    // Make sure that a channel has been selected for plating
    if (!channelSelected)
    {
        cout << "You must select a channel before plating." << endl;
        return -1;
    }

    // Tell the plating object what headstage to aim at
    plateControl->selectHeadstage(usedPort); //TODO: this is wrong I think because ports can have two headstages.

    // Update plating parameters in the platecontrol object
    plateControl->setPlateParameters(currentuA, durationMilliSec);

    // Power up the DAC used for plating control. The gate from the plating source
    // to the electrode will not be opened until the ttl lines are set
    evalBoard->clearTtlOut();
    evalBoard->enableDac(plateControl->dacNumber, true);
    evalBoard->selectDacDataStream(plateControl->dacNumber, 8);
    evalBoard->setDacManual(plateControl->dacVoltage);

    // Configure the plate start bit
    plateControl->turnPlatingOn();

    // Write the ttl configutation to the evalboard
    plateControl->getTTLState(ttlOut);
    evalBoard->setTtlOut(ttlOut);

    // Apply the requested plating delay
    //plateControl->applyPlatingDelay();

    evalBoard->setMaxTimeStep(evalBoard->getSampleRate()*plateControl->plateDurationMilliSec/1000);
    evalBoard->run();
    while (evalBoard->isRunning()) {}

    evalBoard->setMaxTimeStep(1);
    evalBoard->setDacManual(0);
    evalBoard->run();
    while (evalBoard->isRunning()) {}

    evalBoard->enableDac(plateControl->dacNumber, false);

    // Configure the plate stop bit
    plateControl->turnPlatingOff();

    // End the plating session
    plateControl->getTTLState(ttlOut);
    evalBoard->setTtlOut(ttlOut);

    return 0;

}
































