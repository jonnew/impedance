//----------------------------------------------------------------------------------
// main.cpp
//
// Intan RHD2000 automated impedance measurement
//
// Copyright (c) 2014 Jon Newman
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the
// use of this software.
//
//----------------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <time.h>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QDir>
#include "autoimpedance.h"
#include "rhd2000impedance.h"
#include "impedancelog.h"

using namespace std;

struct CLOptions
{
    QFileInfo fid;
    double currentGain;
    bool force;
};

enum CommandLineParseResult
{
    CommandLineOk,
    CommandLineError,
    CommandLineVersionRequested,
    CommandLineHelpRequested
};

CommandLineParseResult parseCommandLine(QCommandLineParser &parser, CLOptions *options, QString *errorMessage)
{

    parser.addPositionalArgument("log", QCoreApplication::translate("main", "File where the log will be saved."));

    // A file bname (-f, --force)
    const QCommandLineOption forceOption(QStringList() << "f" << "force",
                                         QCoreApplication::translate("main", "Force log file overwrite if applicable."));

    parser.addOption(forceOption);

    // The voltageToCurrent gain (-g, --gain)
    const QCommandLineOption gainOption(QStringList() << "g" << "gain",
                                        QCoreApplication::translate("main", "Microamps per volt setting for electrode activation circuit."),
                                        QCoreApplication::translate("main", "currentGain"));
    parser.addOption(gainOption);

    // Help and version options
    const QCommandLineOption helpOption = parser.addHelpOption();
    const QCommandLineOption versionOption = parser.addVersionOption();

    if (!parser.parse(QCoreApplication::arguments())) {
        *errorMessage = parser.errorText();
        return CommandLineError;
    }

    if (parser.isSet(versionOption))
        return CommandLineVersionRequested;

    if (parser.isSet(helpOption))
        return CommandLineHelpRequested;

    if (parser.isSet(forceOption)) {
        options->force = true;
    }
    else
    {
        options->force = false;
    }


    const QStringList positionalArguments = parser.positionalArguments();
    if (positionalArguments.isEmpty()) {
        *errorMessage = "A save location for the log file was not provided.";
        return CommandLineError;
    }
    if (positionalArguments.size() > 1) {
        *errorMessage = "Several 'log' arguments specified.";
        return CommandLineError;
    }

    QString fname = positionalArguments.first();
    if (!fname.endsWith(".json")){
        fname.append(".json");
    }

    options->fid = QFileInfo(fname);

    if (!QDir(options->fid.absoluteDir()).exists() ) {
        *errorMessage = "Selected log file save directory does not exist: " + options->fid.path();
        return CommandLineError;
    }
    if (options->fid.exists() && options->fid.isFile() && !options->force) {
        *errorMessage = "Selected log file already exists, use the -f option if you wish to overwrite: " + options->fid.absoluteFilePath();
        return CommandLineError;
    }

    if (parser.isSet(gainOption)) {
        const double gain = parser.value(gainOption).toDouble();
        if (gain < 0) {
            *errorMessage = "The voltage to current gain cannot be negative,";
            return CommandLineError;
        }
        options->currentGain = gain;
    }
    else
    {
        cout << "Warning: no current gain was not provided. Assuming 10uA/3.3V." << endl;
        options->currentGain = 10.0/3.3;
    }

    return CommandLineOk;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("Impedance");
    QCoreApplication::setApplicationVersion("1.0");
    QCommandLineParser parser;
    parser.setApplicationDescription("Automated electrode impedance testing and activation using RHD2000 chips.");

    CLOptions options;
    QString errorMessage;
    switch (parseCommandLine(parser, &options, &errorMessage)) {
    case CommandLineOk:
        break;
    case CommandLineError:
        fputs(qPrintable(errorMessage), stderr);
        fputs("\n\n", stderr);
        fputs(qPrintable(parser.helpText()), stderr);
        return 1;
    case CommandLineVersionRequested:
        printf("%s %s\n", qPrintable(QCoreApplication::applicationName()),
               qPrintable(QCoreApplication::applicationVersion()));
        return 0;
    case CommandLineHelpRequested:
        parser.showHelp();
        Q_UNREACHABLE();
    }

    // TODO: The headstage selection and port selection is currently hardcoded
    // and screwed up.
    // e.g
    // Handle 2X32 chan headstages
    // handle multiple ports
    // handle 64 chan headstages.
    PlateControl *plateControl = new PlateControl(options.currentGain);
    AutoImpedance *autoImpedance = new AutoImpedance(350.0e3, 100.0 * 50.0e3/350.0e3, 5);
    ImpedanceLog *logger = new ImpedanceLog(options.fid);
    RHD2000Impedance *impedance = new RHD2000Impedance(Rhd2000EvalBoard::PortA, logger);

    impedance->configurePlate(plateControl);
    plateControl->setHeadstage(0);

    int ch, mode;
    impedance->setImpedanceTestFrequency(1000);


    while(true) {

        cout << endl;
        cout << "Select an action:" << endl;
        cout << "   [1]: Single channel impedance test." << endl;
        cout << "   [2]: Single channel plate. " << endl;
        cout << "   [3]: Single channel electrode clean. " << endl;
        cout << "   [4]: All channel impedance check. "  << endl;
        cout << "   [5]: All channel plate."  << endl;
        cout << "   [6]: All channel electrode clean."  << endl;
        cout << "   [7]: Automated impedance test and electrode activation." << endl;
        cout << "   [8]: Change impedance test and plating parameters." << endl;
        cout << "   [9]: Change log save location." << endl;
        cout << "   [10]: Clear log." << endl;
        cout << "   [11]: Exit without saving." << endl;
        cout << "   [12]: Save log and exit program." << endl;
        cin >> mode;

        switch(mode) {

        case 1:

            cout << "Select a channel." << endl;
            cin >> ch;
            impedance->setChannel(ch);
            impedance->measureImpedance(logger);
            impedance->printImpedance();
            break;

        case 2:

            cout << "Select a channel." << endl;
            cin >> ch;
            impedance->setChannel(ch);
            impedance->plate(plateControl, logger);
            break;

        case 3:

            cout << "Select a channel." << endl;
            cin >> ch;
            impedance->setChannel(ch);
            impedance->clean(plateControl, logger);
            break;

        case 4:

            // TODO: Fix hardcoded number of channels, ports,etc
            for (int ch=0; ch < 32; ch++) {

                impedance->setChannel(ch);
                impedance->measureImpedance(logger);
                impedance->printImpedance();
            }
            break;

        case 5:

            // TODO: Fix hardcoded number of channels, ports,etc
            for (int ch=0; ch < 32; ch++) {

                impedance->setChannel(ch);
                impedance->plate(plateControl, logger);
            }
            break;

        case 6:

            // TODO: Fix hardcoded number of channels, ports,etc
            for (int ch=0; ch < 32; ch++) {

                impedance->setChannel(ch);
                impedance->clean(plateControl, logger);
            }
            break;

        case 7:
        {
            cout << "Select a channel." << endl;
            cin >> ch;
            impedance->setChannel(ch);
            autoImpedance->autoPlate(impedance, plateControl, logger);

            break;
        }
        case 8:
        {
            bool exitParamsDialog = false;
            do {

                int parameter;
                cout << endl;
                cout << "Select parameter to modify:" << endl;
                cout << "   [1]: Test frequency." << endl;
                cout << "   [2]: Number of averages. " << endl;
                cout << "   [3]: Number of test periods. "  << endl;
                cout << "   [4]: Change plate duration." << endl;
                cout << "   [5]: Change plate current." << endl;
                cout << "   [6]: Change cleaning duration." << endl;
                cout << "   [7]: Change cleaning current." << endl;
                cout << "   [10]: Exit to main menu." << endl;
                cin >> parameter;

                switch(parameter) {

                case 1:
                {
                    double freq;
                    cout << "Enter an impedance test frequency (Hz)." << endl;
                    cin >> freq;
                    impedance->setImpedanceTestFrequency(freq);

                    break;
                }

                case 2:
                {
                    int na;
                    cout << "Enter a number of aveages for each impedance measurement." << endl;
                    cin >> na;
                    impedance->setNumAverages(na);

                    break;
                }

                case 3:
                {
                    int np;
                    cout << "Enter a number of test waveform periods for each measurement." << endl;
                    cin >> np;
                    impedance->setNumPeriods(np);

                    break;
                }

                case 4:
                {
                    int pd;
                    cout << "Enter a new plating duration (msec)." << endl;
                    cin >> pd;
                    plateControl->setPlateDuration(pd);

                    break;
                }

                case 5:
                {
                    int pc;
                    cout << "Enter a new plating current (uA)." << endl;
                    cin >> pc;
                    plateControl->setPlateCurrent(pc);

                    break;
                }

                case 6:
                {
                    int ct;
                    cout << "Enter a new cleaning duration (msec)." << endl;
                    cin >> ct;
                    plateControl->setCleaningDuration(ct);

                    break;
                }

                case 7:
                {
                    int cc;
                    cout << "Enter a new cleaning current (uA)" << endl;
                    cin >> cc;
                    plateControl->setCleaningCurrent(cc);
                }

                case 10:
                {
                    exitParamsDialog = true;
                    break;
                }


                default:


                    cout << "Invalid parameter selection. Try again." << endl;
                    break;
                }
            } while(!exitParamsDialog);

            break;
        }
        case 9:
        {
            string fn;
            cout << "Enter a new file name." << endl;
            cin >> fn;
            QString fid = QString::fromStdString(fn);
            logger->setSaveLocation(fid, false);

            break;
        }
        case 10:
        {
            logger->clearLogFile(impedance);
            break;
        }

        case 11:

            return 0;

            break;

        case 12:
            if(logger->saveLog())
                return 0;
            else
                cout << "Something went wrong while trying to save the log." << endl;
            cout << "You can specify a new save location in the main menu." << endl;

            break;

        default:
            cout << "Invalid mode selection. Try again." << endl;
            break;
        }
    }
}
