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
// Permission is granted to anyone to use this software for any applications that
// use Intan Technologies integrated circuits, and to alter it and redistribute it
// freely.
//
// See http://www.intantech.com for documentation and product information.
//----------------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <time.h>
#include <QCommandLineParser>

#include "rhd2000impedance.h"

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("Impedance");
    QCoreApplication::setApplicationVersion("1.0");
    QCommandLineParser parser;

    // A file bname (-f, --file)
    QCommandLineOption fileOption(QStringList() << "f" << "save-location",
                                  QCoreApplication::translate("main", "Impedance log save <fname>."),
                                  QCoreApplication::translate("main", "fname"));
    parser.addOption(fileOption);

    // The voltageToCurrent gain (-g, --gain)
    QCommandLineOption gainOption(QStringList() << "g" << "current-gain",
                                  QCoreApplication::translate("main", "Microamps per volt for plating <currentGain>."),
                                  QCoreApplication::translate("main", "currentGain"));
    parser.addOption(gainOption);

    // Process the actual command line arguments given by the user
    parser.process(app);

    //const QStringList args = parser.positionalArguments();
    // source is args.at(0), destination is args.at(1)

    QString fname = parser.value(fileOption);
    double currentGain = parser.value(gainOption).toDouble();

    // TODO: The headstage selection and port selection is currently hardcoded
    // and screwed up.
    // e.g
    // Handle 2X32 chan headstages
    // handle multiple ports
    // handle 64 chan headstages.
    RHD2000Impedance *impedance = new RHD2000Impedance(Rhd2000EvalBoard::PortA);
    PlateControl *plateControl = new PlateControl(currentGain);
    plateControl->setHeadstage(0);

    int ch, mode;
    double plateCurrent, plateTime;
    impedance->setImpedanceTestFrequency(1000);
    impedance->setRecordingState(true);
    impedance->setSaveLocation(fname);

    while(true) {
        cout << "Select action:" << endl;
        cout << "   [1]: Manual impedance and plating." << endl;
        cout << "   [2]: Single channel impedance check. " << endl;
        cout << "   [3]: All channel impedance check. "  << endl;
        cout << "   [4]: All channel with plate."  << endl;
        cout << "   [5]: Change parameters." << endl;
        cout << "   [8]: Clear log." << endl;
        cout << "   [9]: Save and exit." << endl;
        cin >> mode;

        switch(mode) {

        case 1:


            cout << "Select a channel." << endl;
            cin >> ch;
            impedance->selectChannel(ch);
            impedance->measureImpedance();
            impedance->printImpedance();

            cout << "Select a plate current (uA)" << endl;
            cin >> plateCurrent;

            cout << "Select a plate time (s)" << endl;
            cin >> plateTime;

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
            cout << "   [6]: Change clean duration." << endl;
            cout << "   [7]: Change clean current." << endl;

            cout << "   [9]: Change log file name." << endl;
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

            case 9:
            {
                string fn;
                cout << "Enter a new file name." << endl;
                cin >> fn;
                QString fid = QString::fromStdString(fn);
                impedance->setSaveLocation(fid);

                break;
            }
            default:
                cout << "Invalid parameter selection. Try again." << endl;
                break;
            }

            break;


        case 9:
            if(impedance->saveLog())
                return 0;
            else
                cout << "Something went wrong while saving the log." << endl;

            break;

        default:
            cout << "Invalid mode selection. Try again." << endl;
            break;
        }
    }
}


// TODO User settable protocol parameters (channels to test/plate)
//    for (int ch=0; ch < 32; ch++) {
//
//        impedance->selectChannel(ch);
//
//        // Clean electrode and apply initial plating
//        // TODO: User settable protocol parameters (plating times, plating currents)
//        //impedance->plate(0.1,500);
//        //impedance->plate(-0.05,500);
//
//        impedance->measureImpedance();
//        impedance->printImpedance();
//
//
//        //        // While the impedance of the electrode is not within the desired range
//        //        // TODO: User settable protocol parameters (target, threshold, plating times, plating currents)
//        //        while (abs(impedance->getImpedanceMagnitude() - 470e3) > 50e3) {
//
//        //            if (impedance->getImpedanceMagnitude() - 470e3 > 0) {
//
//        //                // If the impedance is too high
//        //                impedance->plate(-0.05, 3000);
//        //            }
//        //            else {
//
//        //                // If the impedance is too low
//        //                impedance->plate(0.05, 1000);
//        //            }
//
//        //            impedance->measureImpedance();
//        //            impedance->printImpedance();
//        //        }
//    }
