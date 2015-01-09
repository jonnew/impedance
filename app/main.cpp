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

#include "rhd2000impedance.h"

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

    // A file bname (-f, --force)
    const QCommandLineOption forceOption(QStringList() << "f" << "force",
                                        QCoreApplication::translate("main", "Force log file overwrite if applicable."));

    parser.addOption(forceOption);

    // A file bname (-l, --log)
    const QCommandLineOption fileOption(QStringList() << "l" << "log",
                                        QCoreApplication::translate("main", "Log file save location."),
                                        QCoreApplication::translate("main", "Save location"));
    parser.addOption(fileOption);

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

    if (parser.isSet(fileOption)) {
        QString fname = parser.value(fileOption);
        if (!fname.endsWith(".json")){
            fname.append(".json");
        }

        options->fid = QFileInfo(fname);

        if (!QDir(options->fid.absoluteDir()).exists() ) {
            *errorMessage = "Selected log file save directory does not exist: " + options->fid.path();
            return CommandLineError;
        }
        if (options->fid.exists() && options->fid.isFile() && !options->force) {
            *errorMessage = "Selected log file already exists, use the -f option to overwrite:" + options->fid.absoluteFilePath();
            return CommandLineError;
        }
    }

    if (parser.isSet(gainOption)) {
        const double gain = parser.value(gainOption).toDouble();
        if (gain < 0) {
            *errorMessage = "The voltage to current gain cannot be negative,";
            return CommandLineError;
        }
        options->currentGain = gain;
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
    RHD2000Impedance *impedance = new RHD2000Impedance(Rhd2000EvalBoard::PortA);
    PlateControl *plateControl = new PlateControl(options.currentGain);
    impedance->configurePlate(plateControl);
    plateControl->setHeadstage(0);

    int ch, mode;
    impedance->setImpedanceTestFrequency(1000);
    impedance->setSaveLocation(options.fid.absoluteFilePath());

    while(true) {

        cout << endl;
        cout << "Select an action:" << endl;
        cout << "   [1]: Manual impedance testing and plating." << endl;
        cout << "   [2]: Single channel impedance check. " << endl;
        cout << "   [3]: All channel impedance check. "  << endl;
        cout << "   [4]: All channel impedance check and plating."  << endl;
        cout << "   [5]: Change test and plating parameters." << endl;

        cout << "   [7]: Change log save location." << endl;
        cout << "   [8]: Clear log." << endl;
        cout << "   [9]: Save log and exit program." << endl;
        cin >> mode;

        switch(mode) {

        case 1:


            cout << "Select a channel." << endl;
            cin >> ch;
            impedance->selectChannel(ch);
            impedance->measureImpedance();
            impedance->printImpedance();
            impedance->plate(plateControl);
            impedance->measureImpedance();
            impedance->printImpedance();

            break;
        case 2:


            cout << "Select a channel." << endl;
            cin >> ch;
            impedance->selectChannel(ch);
            impedance->measureImpedance();
            impedance->printImpedance();
            break;
        case 3:
            for (int ch=0; ch < 32; ch++) {

                impedance->selectChannel(ch);
                impedance->measureImpedance();
                impedance->printImpedance();
            }
            break;
        case 4:
            for (int ch=0; ch < 32; ch++) {

                impedance->selectChannel(ch);
                impedance->measureImpedance();
                impedance->printImpedance();
                impedance->plate(plateControl);
            }
            break;

        case 5:

            int parameter;
            cout << "Select parameter to modify:" << endl;
            cout << "   [1]: Test frequency." << endl;
            cout << "   [2]: Number of averages. " << endl;
            cout << "   [3]: Number of test periods. "  << endl;
            cout << "   [4]: Change plate duration." << endl;
            cout << "   [5]: Change plate current." << endl;
            cout << "   [6]: Change cleaning duration." << endl;
            cout << "   [7]: Change cleaning current." << endl;
            cin >> parameter;

            switch(parameter) {

            case 1:
            {
                double freq;
                cout << "Enter a test frequency." << endl;
                cin >> freq;
                impedance->setImpedanceTestFrequency(freq);

                break;
            }

            case 2:
            {
                int na;
                cout << "Enter a number of aveages for each measurement." << endl;
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
                cout << "Enter a new plating duraction in milliseconds." << endl;
                cin >> pd;
                plateControl->setPlateDuration(pd);

                break;
            }

            case 5:
            {
                int pc;
                cout << "Enter a new plating current in uA." << endl;
                cin >> pc;
                plateControl->setPlateCurrent(pc);

                break;
            }

            case 6:
            {
                int ct;
                cout << "Enter a new cleaning duraction in milliseconds." << endl;
                cin >> ct;
                plateControl->setCleaningDuration(ct);

                break;
            }

            case 7:
            {
                int cc;
                cout << "Enter a new cleaning current in uA." << endl;
                cin >> cc;
                plateControl->setCleaningCurrent(cc);

                break;
            }

            default:
                cout << "Invalid parameter selection. Try again." << endl;
                break;
            }

            break;

        case 7:
        {
            string fn;
            cout << "Enter a new file name." << endl;
            cin >> fn;
            QString fid = QString::fromStdString(fn);
            impedance->setSaveLocation(fid);

            break;
        }

        case 9:
            if(impedance->saveLog())
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
