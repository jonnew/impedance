classdef OneElectrode < handle
    %ONEELECTRODE Data from one electrode
    %
    % This is used for storing data from the given electrode only, no
    % control functions.
    %
    % See properties for more information
    
    properties
        %IMPEDANCEHISTORY Array of measured complex impedance values.
        % See also MeasurementTimes.
        ImpedanceHistory

        %MEASUREMENTTIMES Array of times (in seconds) when impedance was measured.
        % See also ImpedanceHistory, InitialTime.
        MeasurementTimes

        
        %PULSETIMES Times (in seconds) when pulses were applied
        % See also PulseDurations, InitialTime.
        PulseTimes

        %PULSEDURATIONS Durations of applied pulses.
        % See also PulseTimes.
        PulseDurations
        
        %INITIALTIME Absolute time that corresponds to 0.
        %
        % Reset this with reset_time().
        %
        % All MeasurementTimes and PulseTimes are seconds after this.
        %
        % See also MeasurementTimes, PulseTimes, reset_time.
        InitialTime;        
    end
    
    properties (Dependent = true, SetAccess = private)
        %CURRENTIMPEDANCE Returns the most recently measured impedance.
        CurrentImpedance
        
        %ELAPSEDTIME Returns the elapsed time (in seconds) since InitialTime.
        % See also InitialTime.
        ElapsedTime
    end
    
    methods
        function obj = OneElectrode()
        %ONEELECTRODE Constructor
            obj.reset_time();
        end
        
        function obj = reset_time(obj)
        %RESET_TIME Clears history and sets InitialTime.
        
            obj.InitialTime = now;
            obj.ImpedanceHistory = [];
            obj.MeasurementTimes = [];
            obj.PulseTimes = [];
            obj.PulseDurations = [];
        end
        
        function obj = add_measurement(obj, value)
        %ADD_MEASUREMENT Adds 'value' to the list of impedance measurements
            obj.ImpedanceHistory = [obj.ImpedanceHistory value];
            if isempty(obj.MeasurementTimes)
                obj.InitialTime = now;
                time = 0;
            else
                time = obj.ElapsedTime();
            end
            obj.MeasurementTimes = [obj.MeasurementTimes time];
        end
        
        function obj = add_pulse(obj, duration)
        %ADD_PULSE Adds a pulse of duration 'duration' to the list of pulses
            time = obj.ElapsedTime();
            obj.PulseTimes = [obj.PulseTimes time];
            obj.PulseDurations = [obj.PulseDurations duration];
        end
        
        function value = get.CurrentImpedance(obj)
            value = obj.ImpedanceHistory(end);
        end
        
        function value = get.ElapsedTime(obj)
            value = (now - obj.InitialTime) * 24*60*60;
        end        
    end
    
end

