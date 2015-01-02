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

#include "rhd2000impedance.h"

using namespace std;

int main(int argc, char *argv[])
{
    RHD2000EvalBoardImpedanceTester *impedance = new RHD2000EvalBoardImpedanceTester(Rhd2000EvalBoard::PortA);


    //impedance->selectChannel(5);
    //impedance->measureImpedance();
    //impedance->printImpedance();

    impedance->changeImpedanceFrequency(2000);

    // TODO User settable protocol parameters (channels to test/plate)
    for (int ch=0; ch < 32; ch++) {

        impedance->selectChannel(ch);

        // Clean electrode and apply initial plating
        // TODO: User settable protocol parameters (plating times, plating currents)
        impedance->plate(0.1,500);
        impedance->plate(-0.05,500);

        impedance->measureImpedance();
        impedance->printImpedance();


        //        // While the impedance of the electrode is not within the desired range
        //        // TODO: User settable protocol parameters (target, threshold, plating times, plating currents)
        //        while (abs(impedance->getImpedanceMagnitude() - 470e3) > 50e3) {

        //            if (impedance->getImpedanceMagnitude() - 470e3 > 0) {

        //                // If the impedance is too high
        //                impedance->plate(-0.05, 3000);
        //            }
        //            else {

        //                // If the impedance is too low
        //                impedance->plate(0.05, 1000);
        //            }

        //            impedance->measureImpedance();
        //            impedance->printImpedance();
        //        }
    }
}
