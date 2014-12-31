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
    Rhd2000Impedance *impedance = new Rhd2000Impedance(Rhd2000EvalBoard::PortA);
    impedance->setupEvalBoard();
    impedance->setupAmplifier();
    impedance->selectChannel(1);
    impedance->configureImpedanceMeasurement();
    impedance->measureImpedance();
    impedance->printImpedance();

}
