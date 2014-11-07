#ifndef RHD2000EVALBOARDPLATECONTROL_H
#define RHD2000EVALBOARDPLATECONTROL_H

class Rhd2000EvalBoardPlateControl
{
public:

    // Eval-board based plating control
    Rhd2000EvalBoardPlateControl();

    //
    int plate();

    // User definable parameters
    double plateDurationSec;

private:


    void enableESDDiodes();
    void disableESDDiodes();

    // Pull the ground pin above or below the plating voltage
    // in order to plate/deplate
    void setRefHigh();
    void setRefLow();


};

#endif // RHD2000EVALBOARDPLATECONTROL_H
