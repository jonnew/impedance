classdef ElectroplatingBoardControl < handle
%ELECTROPLATINGBOARDCONTROL Calculations for the electroplating board
%
% The electroplating board has 8 digital control lines:
%    0   I_SINK_EN
%    1   I_SOURCE_EN
%    2   I_MODE_EN
%    3   RANGE_SEL_0
%    4   RANGE_SEL_1
%    5   ELEC_TEST1
%    6   ELEC_TEST2
%    7   REF_SEL
%
% See the datasheet for descriptions of what they do.
%
% This class encapsulates the details of the operation of the control
% lines.  Instead of setting them directly, you set Voltage or
% Current and PlatingChannel or ZCheckChannel.
%
% Example:
%     controller = ElectroplatingBoardControl();
%     controller.Voltage = 1.0;
%
%     % Start plating
%     controller.PlatingChannel = 125;
%     % more board control required
%     board.DigitalOutputs = controller.DigitalOutputs;
%
%     pause(10); % Plate for 10 seconds
%
%     controller.ZCheckChannel = 125; % Stop plating
%     board.DigitalOutputs = controller.DigitalOutputs;
%     % more board control required
%
% See also Voltage, Current, PlatingChannel, ZCheckChannel.

    properties (SetAccess = private)
        % These properties are the values of the digital control lines
        
        %CURRENTSOURCEENABLE Value of I_SOURCE_EN control line
        CurrentSourceEnable
        
        %CURRENTSINKENABLE Value of I_SINK_EN control line
        CurrentSinkEnable
        
        %CURRENTMODEENABLE Value of I_MODE_EN control line
        CurrentModeEnable
        
        %RESISTORSELECTION Which of the four resistors to use
        %
        % 0    100 MOhm
        % 1     10 MOhm
        % 2      1 MOhm
        % 3    100 kOhm
        %
        % The values RangeSel0 and RangeSel1 are derived from this.
        %
        % See also RangeSel0, RangeSel1.
        ResistorSelection

        %ELECTEST1 Value of ELEC_TEST1 control line
        ElecTest1

        %ELECTEST2 Value of ELEC_TEST2 control line
        ElecTest2

        %DACMANUALDESIRED The voltage you need to set DacManual to
        %
        % For example, if you want to electroplate with 1.0 V, this would
        % be 1.0.  If you want to electroplate with -1.0, the reference
        % would be +3.3 V, and this value would be 2.3 (2.3 - 3.3 = -1.0).
        DacManualDesired
        
        %DATASOURCE Which datasource to use when measuring impedance
        %
        % 0 (i.e., Port A MISO 1) for channels 0-63
        % 1 (i.e., Port A MISO 2) for channels 64-127
        DataSource
        
        %EFFECTIVECHANNEL Which channel to pass to impedance measurement calls.
        %
        % For example, channel 127 would be DataSource = 1,
        % EffectiveChannel = 63.
        EffectiveChannel
    end
    
    properties (GetAccess = private, Dependent = true)
        %VOLTAGE Set this to plate with constant voltage.
        %
        % Example:
        %     controller = ElectroplatingBoardControl();
        %     controller.Voltage = 1.0;
        %     controller.PlatingChannel = 125;
        %
        % Specified value is in volts.
        %
        % See also Current, PlatingChannel, ZCheckChannel.
        Voltage

        %CURRENT Set this to plate with constant current.
        %
        % Example:
        %     controller = ElectroplatingBoardControl();
        %     controller.Current = 1e-6; % 1uA
        %     controller.PlatingChannel = 125;
        %
        % Specified value is in amperes.
        %
        % See also Voltage, PlatingChannel, ZCheckChannel.
        Current
        
        %PLATINGCHANNEL Set this to plate a channel.
        %
        % Example:
        %     controller = ElectroplatingBoardControl();
        %     controller.Voltage = 1.0;
        %     controller.PlatingChannel = 125;
        %
        % Valid values 0-127.
        %
        % See also Voltage, Current, ZCheckChannel.
        PlatingChannel

        %ZCHECKCHANNEL Set this to measure impedance on a channel.
        %
        % Example:
        %     controller = ElectroplatingBoardControl();
        %     controller.Voltage = 1.0;
        %     controller.ZCheckChannel = 125;
        %
        % Valid values 0-127.
        %
        % See also Voltage, Current, PlatingChannel.
        ZCheckChannel
    end
    
    properties (SetAccess = private, Dependent = true)
        %DACMANUALACTUAL Best achievable DacManual value
        DacManualActual
        
        %VOLTAGEACTUAL Best achievable voltage, in constant voltage mode.
        VoltageActual

        %CURRENTACTUAL Best achievable current, in constant current mode.
        CurrentActual
        
        %RANGESEL0 Value of the RANGE_SEL_0 control line
        % See also ResistorSelection.
        RangeSel0

        %RANGESEL1 Value of the RANGE_SEL_1 control line
        % See also ResistorSelection.
        RangeSel1

        %CHANNEL Current ZCheck or Plating channel (0-127).
        Channel
        
        %DIGITALOUTPUTS Values of the digital outputs for these settings.
        %
        % Example:
        %     controller = ElectroplatingBoardControl();
        %     controller.Voltage = 1.0;
        %     controller.PlatingChannel = 125;
        %     board.DigitalOutputs = controller.DigitalOutputs;
        DigitalOutputs
        
        %REFERENCESELECTION Value of the REF_SEL control line
        ReferenceSelection
    end
    
    properties (Access = private, Hidden = true)
        Resistors
        PulseReferenceSelection
        ZCheckMode
    end
    
    methods
        function obj = ElectroplatingBoardControl()
        %ELECTROPLATINGBOARDCONTROL Constructor
            obj.CurrentSourceEnable = false;
            obj.CurrentSinkEnable = false;
            obj.CurrentModeEnable = false;
            obj.ResistorSelection = 0;
            obj.PulseReferenceSelection = false;
            obj.Resistors = [100e6 10e6 1e6 100e3];
            obj.ElecTest1 = false;
            obj.ElecTest2 = false;
            obj.DataSource = 0; % Port A, MISO 1
            obj.EffectiveChannel = 0;
            obj.ZCheckMode = true;
        end
        
        % Set to current mode with the given current
        function set.Current(obj, value)
            good = false;

            voltages = value * obj.Resistors;
            for i=1:4
                v = voltages(i);
                if (v >= -1.0 - 10*eps) && (v <= 1.0 + 10*eps)
                    obj.ResistorSelection = i - 1;
                    good = true;
                    
                    if v >= 0
                        obj.PulseReferenceSelection = false;
                        obj.DacManualDesired = 3.3 - v;
                    else
                        obj.PulseReferenceSelection = true;
                        obj.DacManualDesired = -v;
                    end

                    if obj.DacManualDesired > 3.3
                        obj.DacManualDesired = 3.3;
                    end
                    if obj.DacManualDesired < 0
                        obj.DacManualDesired = 0;
                    end
                    break;
                end
            end
            if ~good
                error('Couldn''t set value; valid range is -10uA..10uA');
            end
            obj.CurrentSourceEnable = value >= 0;
            obj.CurrentSinkEnable = value < 0;
            obj.CurrentModeEnable = true;
            obj.ZCheckMode = false;
        end
        
        % Set to voltage mode with the given voltage
        function set.Voltage(obj, value)
            obj.CurrentSourceEnable = false;
            obj.CurrentSinkEnable = false;
            obj.CurrentModeEnable = false;
            
            if value >= 0
                obj.PulseReferenceSelection = false;
                obj.DacManualDesired = value;
            else
                obj.PulseReferenceSelection = true;
                obj.DacManualDesired = 3.3 + value;
            end
            if obj.DacManualDesired > 3.3
                obj.DacManualDesired = 3.3;
            end
            if obj.DacManualDesired < 0
                obj.DacManualDesired = 0;
            end
            obj.ZCheckMode = false;
        end
        
        % Set to plating mode, with the given channel (value).
        function set.PlatingChannel(obj, value)
            obj.Channel = value;
            
            if value <= 63
                obj.ElecTest1 = true;
                obj.ElecTest2 = false;
            else
                obj.ElecTest1 = false;
                obj.ElecTest2 = true;
            end
            
            if obj.CurrentModeEnable
                obj.CurrentSourceEnable = ~obj.PulseReferenceSelection;
                obj.CurrentSinkEnable = obj.PulseReferenceSelection;
            end
            obj.ZCheckMode = false;
        end
        
        % Set to zcheck mode, with the given channel (value).
        function set.ZCheckChannel(obj, value)
            obj.Channel = value;
            
            obj.ElecTest1 = false;
            obj.ElecTest2 = false;
            obj.CurrentSourceEnable = false;
            obj.CurrentSinkEnable = false;
            obj.ZCheckMode = true;
        end
        
        function set.Channel(obj, value)
            if value < 0 || value > 127
                error('Value must be 0..127');
            end
            
            if value <= 63
                obj.DataSource = 0;
                obj.EffectiveChannel = value;
            else
                obj.DataSource = 1;
                obj.EffectiveChannel = value - 64;
            end
        end
        
        function value = get.Channel(obj)
            value = 64 * obj.DataSource + obj.EffectiveChannel;
        end
        
        % Best achievable DacManual, given hardware constraints
        function value = get.DacManualActual(obj)
            num_steps = 2^16;
            step_size = 3.3/num_steps;
            best_achievable_int = round(obj.DacManualDesired/step_size);
            value = best_achievable_int * step_size;
        end
        
        % Best achievable voltage, given hardware constraints
        function value = get.VoltageActual(obj)
            if obj.CurrentSourceEnable || obj.CurrentSinkEnable
                value = [];
            else
                if obj.PulseReferenceSelection
                    % Negative
                    value = obj.DacManualActual - 3.3;
                else
                    % Positive
                    value = obj.DacManualActual;
                end
            end
        end
        
        % Best achievable current, given hardware constraints
        function value = get.CurrentActual(obj)
            if obj.CurrentSourceEnable
                value = (3.3 - obj.DacManualActual) / obj.Resistors(obj.ResistorSelection + 1);
            elseif obj.CurrentSinkEnable
                value = -obj.DacManualActual / obj.Resistors(obj.ResistorSelection + 1);
            else
                value = 0;
            end
        end

        function value = get.RangeSel0(obj)
            value = bitget(obj.ResistorSelection, 1);
        end
        
        function value = get.RangeSel1(obj)
            value = bitget(obj.ResistorSelection, 2);
        end
        
        function value = get.ReferenceSelection(obj)
            if obj.ZCheckMode
                value = false;
            else
                value = obj.PulseReferenceSelection;
            end
        end
        
        function value = get.DigitalOutputs(obj)
            value = zeros(1, 16);
            value(1) = obj.CurrentSinkEnable;
            value(2) = obj.CurrentSourceEnable;
            value(3) = obj.CurrentModeEnable;
            value(4) = obj.RangeSel0;
            value(5) = obj.RangeSel1;
            value(6) = obj.ElecTest1;
            value(7) = obj.ElecTest2;
            value(8) = obj.ReferenceSelection;           
        end
    end
end

