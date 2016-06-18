classdef StoredData < handle
    %STOREDDATA Data we store for the electroplating GUI
    %
    % See properties for more information
    
    properties
        %THRESHOLD The automatic plating threshold, in ohms.
        Threshold
        
        %SELECTED Selected channel (0-127)        
        Selected
        
        %ELECTRODES Array of electrode data.
        % See also OneElectrode.
        Electrodes
        
        %DISPLAYMAGNITUDES True for magnitudes, false for phases.
        DisplayMagnitudes
        
        
        
        
        % Automatic plating parameters
        
        %AUTOMATICISVOLTAGEMODE True = constant voltage, false = constant current.
        AutomaticIsVoltageMode
        
        %AUTOMATICVALUE Value of the voltage or current
        AutomaticValue
        
        %AUTOMATICDURATION Duration of pulses
        AutomaticDuration

        
        
        
        % Manual Pulse parameters

        %MANUALISVOLTAGEMODE True = constant voltage, false = constant current.
        ManualIsVoltageMode

        %MANUALVALUE Value of the voltage or current
        ManualValue

        %MANUALDURATION Duration of manual pulse
        ManualDuration
        
        
        % Settings from the Configure Settings menu
        
        %MAXPULSES Maximum number of pulses to run automatically, before
        % giving up on the electrode and moving on
        MaxPulses
        
        %DELAYBEFOREPULSE Delay between reading impedance and pulse.
        DelayBeforePulse

        %DELAYAFTERPULSE Delay between pulse and reading impedance.
        DelayAfterPulse
        
        %CHANNELS0TO60 True if electrodes 0-63 are present
        Channels0to63

        %CHANNELS64TO127 True if electrodes 64-127 are present
        Channels64to127
        
        %USETARGETIMPEDANCE If true, automatic plating should stop once the
        %target impedance is reached.
        UseTargetImpedance
    end
    
    properties (Access = private, Hidden = true)
        StoreFields
    end
        
    methods
        function obj = StoredData()
        % Constructor
            obj.Threshold = 100000;
            obj.Selected = 0;
            
            obj.Electrodes = cell(128,1);
            for i=1:128
                obj.Electrodes{i} = OneElectrode();
            end
            obj.DisplayMagnitudes = true;

            obj.AutomaticIsVoltageMode = false;
            obj.AutomaticValue = 0;
            obj.AutomaticDuration = 1;

            obj.ManualIsVoltageMode = false;
            obj.ManualValue = 0;
            obj.ManualDuration = 1;
            
            obj.MaxPulses = 10;
            obj.DelayBeforePulse = 0.0;
            obj.DelayAfterPulse = 1.0;
            obj.Channels0to63 = true;
            obj.Channels64to127 = true;
            obj.UseTargetImpedance = true;
        
            obj.StoreFields = {'AutomaticIsVoltageMode', ...
                               'AutomaticValue', ...
                               'AutomaticDuration', ...
                               'ManualIsVoltageMode', ...
                               'ManualValue', ...
                               'ManualDuration', ...
                               'Threshold', ...
                               'MaxPulses', ...
                               'DelayBeforePulse', ...
                               'DelayAfterPulse', ...
                               'Channels0to63', ...
                               'Channels64to127', ...
                               'UseTargetImpedance'};
        end
        
        function [indices, impedances] = get_impedances(obj)
        %GET_IMPEDANCES Gets the most recently measured impedances
        %
        % [indices, impedances] = stored.get_impedances() returns:
        %    * indices of electrodes whose impedances have been measured
        %    * impedances as complex numbers
            impedances_tmp = zeros(128,1);
            valid = zeros(128,1);
            
            for i=1:128
                if ~isempty(obj.Electrodes{i}.ImpedanceHistory)
                    valid(i) = true;
                    impedances_tmp(i) = obj.Electrodes{i}.CurrentImpedance;
                end
            end
            indices_tmp = 0:127;
            
            impedances = impedances_tmp(valid == true);
            indices = indices_tmp(valid == true);
        end
        
        function save_settings(obj, filename)
        %SAVE_SETTINGS Save the current settings to 'filename'.
            fid = fopen(filename, 'w+t');
            
            for i=1:length(obj.StoreFields)
                field = obj.StoreFields{i};
                fprintf(fid, '%s\t%g\n', field, obj.(field));
            end
            
            fclose(fid);
        end
        
        function obj = load_settings(obj, filename)
        %LOAD_SETTINGS Load settings from 'filename'.
            fid = fopen(filename, 'r+t');
            
            data = textscan(fid, '%s', 'delimiter', '\t');
            data = data{:};
            data = reshape(data, 2, length(data)/2);
            sizes = size(data);
            
            for i=1:sizes(2);
                field = data{1, i};
                value = data{2, i};
                obj.(field) = sscanf(value, '%g');
            end
            
            fclose(fid);
        end
        
        function save_impedances(obj, filename)
        %SAVE_IMPEDANCES Save the current impedances to 'filename'.
            fid = fopen(filename, 'w+t');

            fprintf(fid, 'Channel Number,Channel Name,Port,Enabled,Impedance Magnitude at 1000 Hz (ohms),Impedance Phase at 1000 Hz (degrees),Series RC equivalent R (Ohms),Series RC equivalent C (Farads)\n');
            
            for i=1:128
                if ~isempty(obj.Electrodes{i}.ImpedanceHistory)
                    z = obj.Electrodes{i}.CurrentImpedance;
                    fprintf(fid, 'A-%03d,A-%03d,Port A,1,', i-1, i-1);
                    
                    % Magnitude
                    fprintf(fid, '%.2e,', abs(z));
                    % Phase
                    fprintf(fid, '%.0f,', angle(z)*180/pi);
                    % Equivalent R
                    fprintf(fid, '%.2e,', real(z));
                    % Equivalent C
                    frequency = 1000;
                    fprintf(fid, '%.2e\n', 1/(-2*pi*frequency*imag(z)));
                    
                end
            end
            
            fclose(fid);
        end
    end    
end

