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

    // Set some defaults
    plateCurrentuA = -1.0;
    plateDurationMilliSec = 1000;
    cleaningCurrentuA = 1.0;
    cleaningDurationMilliSec = 1000;
    mode = PLATE;

}

int PlateControl::getDacNumber() {

    return dacNumber;
}

int PlateControl::getPlatingDuration()
{
    return plateDurationMilliSec;
}

int PlateControl::getCleaningDuration()
{
    return cleaningDurationMilliSec;
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

void PlateControl::setPolarity(bool pol) {

    if (pol) {
        polarity = 1;
    }
    else {
        polarity = -1;
    }

    ttl = ((ttl & ~polarityMask) | pol << polarityBit);
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

void PlateControl::setCleaningDuration(int durationMilliSec) {

    if (durationMilliSec > 0) {

        cleaningDurationMilliSec = durationMilliSec;
        cout << "Electrode cleaning duration set to " << cleaningDurationMilliSec << " ms." << endl;
    }
    else {
        cleaningDurationMilliSec = 1000;
        qWarning("Cleaning duration cannot be negative.");
        cout << "Cleaning duration set to 1000 ms." << endl;
    }
}

void PlateControl::turnCurrentSourceOn() {
    ttl = ((ttl & ~plateMask) | 0x0001 << plateBit);
}

void PlateControl::turnCurrentSourceOff() {
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

void PlateControl::setPlateCurrent(double currentuA) {

    // Set the current
    plateCurrentuA = currentuA;
}

void PlateControl::setCleaningCurrent(double currentuA) {
    // Set the current
    cleaningCurrentuA = currentuA;
}

int PlateControl::getDacVoltage(){

    switch(mode) {

    case PLATE:

        // Set the polarity
        if (plateCurrentuA < 0) {

            setPolarity(false);
        }
        else {
            setPolarity(true);
        }

        // Convert to a DAC voltage
        dacVoltage = 32768 + qFloor((32768/3.3) *  fabs(plateCurrentuA)/uAPerVolt);

        if (dacVoltage > 65535) {
            dacVoltage = 65535;
            plateCurrentuA = (double)polarity * 3.3 * uAPerVolt;
            qWarning("The requested plating current is too high."
                     "It has been set to %f.", plateCurrentuA);
        }
        else {
            cout << "Plating current set to " << plateCurrentuA << " uA" << endl;
        }

        break;

    case CLEAN:

        // Set the polarity
        if (cleaningCurrentuA < 0) {

            setPolarity(false);
        }
        else {
            setPolarity(true);
        }

        // Convert to a DAC voltage
        dacVoltage = 32768 + qFloor((32768/3.3) *  cleaningCurrentuA/uAPerVolt);

        if (dacVoltage > 65535) {
            dacVoltage = 65535;
            cleaningCurrentuA = (double)polarity * 3.3 * uAPerVolt;
            qWarning("The requested cleaning current is too high."
                     "It has been set to %f.", cleaningCurrentuA);
        }
        else {
            cout << "Cleaning current set to " << cleaningCurrentuA << " uA" << endl;
        }

        break;
    }

    return dacVoltage;
}

void PlateControl::setMode(modes newMode) {

    mode = newMode;
}

void PlateControl::writePlate(QJsonObject &json, int channel, int time) const {

    json["name"] = "plating_entry";
    json["channel"] = channel;
    json["durationMilliSec"] = (int)plateDurationMilliSec;
    json["currentMicroA"] =  plateCurrentuA;
    json["time"] = time;

}

void PlateControl::writeClean(QJsonObject &json, int channel, int time) const {

    json["name"] = "cleaning_entry";
    json["channel"] = channel;
    json["durationMilliSec"] = (int)cleaningDurationMilliSec;
    json["currentMicroA"] =  cleaningCurrentuA;
    json["time"] = time;

}


