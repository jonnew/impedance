#include <QCoreApplication>
#include "rhd2000evalboardimpedancetester.h"


int main(int argc, char *argv[])
{
    RHD2000Impedance *impedance = new RHD2000Impedance(Rhd2000EvalBoard::PortA);


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
