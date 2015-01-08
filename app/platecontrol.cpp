#include "platecontrol.h"
#include "qtinclude.h"

using namespace std;

PlateControl::PlateControl(double currentGainuAPerVolt)
{
    // Select the DAC used to control the current or voltage source and provide
    // the uA/V selection
    dacNumber = 0;
    uAPerVolt = currentGainuAPerVolt;

    // Select the digital control lines
    plateBit = 0;
    polarityBit = 1;
    headstageSelect0 = 2;

    // Bitmasks
    plateMask =  1 << plateBit;
    polarityMask =  1 << polarityBit;
    headstageMask =  3 << headstageSelect0;

    // TTL array
    ttl = 0;

}

int PlateControl::setHeadstage(int rhd2000AmpNumber) {

    // TODO: This now is hard coded to handle up to 4 elec_test lines. I suppose
    // at full capacity, the eval board can handle 8.
    if (rhd2000AmpNumber < 3 && rhd2000AmpNumber >= 0) {
        selectedHeadstage = rhd2000AmpNumber;
        ttl = ((ttl & ~headstageMask) | selectedHeadstage << headstageSelect0);

        return 0;
    }
    else {
        cout << "Invalid headstage selection. Please choose a headstage between 0 and 3." << endl;;
        return -1;
    }

}

void PlateControl::setPolarity(bool polarity) {

    ttl = ((ttl & ~polarityMask) | polarity << polarityBit);

    if (polarity) {
        platePolarity = 1;
        cout << "Plating polarity set to +." << endl;
    }
    else {
        platePolarity = -1;
        cout << "Plating polarity set to -." << endl;
    }


}

void PlateControl::setPlateDuration(int durationMilliSec) {

    if (durationMilliSec > 0) {

        plateDurationMilliSec = durationMilliSec;
        cout << "Plating duration set to " << plateDurationMilliSec << " ms." << endl;
    }
    else {
        plateDurationMilliSec = 3000;
        qWarning("Plating duration cannot be negative.");
        cout << "Plating duration set to 3000 ms." << endl;
    }
}

void PlateControl::turnPlatingOn() {
    ttl = ((ttl & ~plateMask) | 0x0001 << plateBit);
}

void PlateControl::turnPlatingOff() {
    ttl = ((ttl & ~plateMask) | 0x0000 << plateBit);
}

// Get the current TTL state in the format required by the Rhythm driver
int* PlateControl::getTTLState(int ttlState[]) {


    // Turn the TTL int into an array
    for (int i=0; i < 16; i++) {
        ttlState[i] = (1 & (ttl >> i));
    }

    return ttlState;

}

void PlateControl::write(QJsonObject &json, int channel) const {


    json["channel"] = channel;
    json["polarity"] = platePolarity;
    json["uAPerVolt"] = uAPerVolt;
    json["durationMilliSec"] = (int)plateDurationMilliSec;
    json["currentMicroA"] =  plateCurrentuA;

}

void PlateControl::setPlateCurrent(double currentuA) {
    // Set the current
    plateCurrentuA = currentuA;

    // Set the polarity
    if (plateCurrentuA < 0) {
        setPolarity(false);
    }
    else {
        setPolarity(true);
    }

    // Set the DAC voltage
    plateCurrentuA = fabs( plateCurrentuA);
    dacVoltage = 32768 + qFloor((32768/3.3) *  plateCurrentuA/uAPerVolt);

    if (dacVoltage > 65535) {
        dacVoltage = 65535;
        plateCurrentuA = 3.3 * uAPerVolt;
        qWarning("The requested plating current is too high."
                 "It has been set to the maximum value of %f.", plateCurrentuA);
    }
    else {
        cout << "Plating current set to " << currentuA << " uA" << endl;
    }
}
