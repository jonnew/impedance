function varargout = configuration(varargin)
% CONFIGURATION MATLAB code for configuration.fig
%
% [ok, is_voltage_mode_out, value_out, duration_out] = ...
%       configuration(is_voltage_mode_in, value_in, duration_in)
%
% Displays the configuration dialog box and returns its parameters.
%     is_voltage_mode_in, value_in, and duration_in are optional; without
%          them, default values will be used.
%
% ok                         - true if the user hit ok; false for cancel or
%                              closing with X
% is_voltage_mode (_in/_out) - true for constant voltage, false for
%                              constant current
% value (_in/_out)           - signed value (e.g., 3.3 or 3.3e-6)
%                              For constant voltage, this is a voltage, in
%                              Volts.  For constant current, this is a
%                              current, in amperes (note that the UI
%                              displays the value in nanoAmperes).
% duration (_in/_out)        - duration, in seconds
%
% Example:
%      [ok, voltage_mode, value, duration] = configuration(1, 2.2, 1.234);

% Edit the above text to modify the response to help configuration

% Last Modified by GUIDE v2.5 04-Feb-2015 13:23:19

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @configuration_OpeningFcn, ...
                   'gui_OutputFcn',  @configuration_OutputFcn, ...
                   'gui_LayoutFcn',  [] , ...
                   'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT


% --- Executes just before configuration is made visible.
function configuration_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to configuration (see VARARGIN)

% Choose default command line output for configuration
handles.output = hObject;
handles.okay = false;

nvarargin = length(varargin);

% Set current or voltage mode
if nvarargin > 0
    is_voltage_mode = varargin{1};
else
    is_voltage_mode = false;
end
set(handles.current_or_voltage_popup, 'Value', is_voltage_mode + 1);
current_or_voltage_popup_Callback(handles.current_or_voltage_popup, 0, handles);

% Set the value
if nvarargin > 1
    signed_value = varargin{2};
else
    signed_value = 0;
end
is_non_positive = (signed_value <= 0); % If 0, default to negative

set(handles.param_sign, 'Value', is_non_positive + 1);
param_sign_Callback(handles.param_sign, 0, handles);

if is_voltage_mode == false % Current mode
    % this dialog box uses nA, but the rest of the code uses Amps
    signed_value = signed_value * 1e9; 
end

set(handles.param_value, 'String', sprintf('%g', abs(signed_value)));
param_value_Callback(handles.param_value, 0, handles);

% Set duration
if nvarargin > 2
    duration = varargin{3};
else
    duration = 1; % 1 second
end
set(handles.duration, 'String', sprintf('%g', duration));

% Update handles structure
guidata(hObject, handles);

% Make the GUI modal
set(handles.figure1,'WindowStyle','modal')

% UIWAIT makes configuration wait for user response (see UIRESUME)
uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = configuration_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.okay;
varargout{2} = is_constant_voltage_mode(handles);
varargout{3} = get_signed_value(handles);
if varargout{2} == false % Current mode
    % this dialog box uses nA, but the rest of the code uses Amps
    varargout{3} = varargout{3} / 1e9; 
end

varargout{4} = get_duration(handles);

% The figure can be deleted now
delete(handles.figure1);


% --- Executes on selection change in current_or_voltage_popup.
function current_or_voltage_popup_Callback(hObject, eventdata, handles)
% hObject    handle to current_or_voltage_popup (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% When you switch modes, change the text labels appropriately
if is_constant_voltage_mode(handles)
    set(handles.value_panel, 'Title', 'Voltage Parameters');
    set(handles.param_unit, 'String', 'Volts (max 3.3 V)');
else
    % Current
    set(handles.value_panel, 'Title', 'Current Parameters');
    set(handles.param_unit, 'String', 'nA (max 10,000 nA)');
end
% And reset the value to 0
set(handles.param_value, 'String', '0');
% Set the best achievable.  This will be 0, but it also has units included.
set_best_achievable_string(handles);

% --- Executes during object creation, after setting all properties.
function current_or_voltage_popup_CreateFcn(hObject, eventdata, handles)
% hObject    handle to current_or_voltage_popup (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in ok_button.
function ok_button_Callback(hObject, eventdata, handles)
% hObject    handle to ok_button (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

handles.okay = true;

% Update handles structure
guidata(hObject, handles);

uiresume(handles.figure1)


% --- Executes when user attempts to close figure1.
function figure1_CloseRequestFcn(hObject, eventdata, handles)
% hObject    handle to figure1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

if isequal(get(hObject, 'waitstatus'), 'waiting')
    % The GUI is still in UIWAIT, us UIRESUME
    uiresume(hObject);
else
    % The GUI is no longer waiting, just close it
    delete(hObject);
end


% --- Executes on button press in cancel_button.
function cancel_button_Callback(hObject, eventdata, handles)
% hObject    handle to cancel_button (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

handles.okay = false;

% Update handles structure
guidata(hObject, handles);

uiresume(handles.figure1)


% --- Executes on selection change in param_sign.
function param_sign_Callback(hObject, eventdata, handles)
% hObject    handle to param_sign (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

set_best_achievable_string(handles);

% --- Executes during object creation, after setting all properties.
function param_sign_CreateFcn(hObject, eventdata, handles)
% hObject    handle to param_sign (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function param_value_Callback(hObject, eventdata, handles)
% hObject    handle to param_value (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get the new value
new_value = str2double(get(hObject,'String'));

% Limit it to the allowed values
if is_constant_voltage_mode(handles)
    % For voltage, 0..3.3V
    if new_value > 3.3
        new_value = 3.3;
    end
    if new_value < 0
        new_value = 0;
    end
else
    % For current, -10uA..+10uA, i.e., -10,000 nA ... + 10,000 nA
    if new_value > 10000
        new_value = 10000;
    end
    if new_value < -10000
        new_value = -10000;
    end
end
set(hObject,'String', sprintf('%g', new_value));
set_best_achievable_string(handles);



% --- Executes during object creation, after setting all properties.
function param_value_CreateFcn(hObject, eventdata, handles)
% hObject    handle to param_value (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end

% Sets the "Best Achievable Value" string.  This is the closest to the
% requested value we can get with the hardware
function set_best_achievable_string(handles)

new_value = get_signed_value(handles);

% All the logic for best achievable is in ElectroplatingBoardControl
ebc = ElectroplatingBoardControl();

if is_constant_voltage_mode(handles)
    ebc.Voltage = new_value;
    set(handles.param_best_achievable, 'String', sprintf('%3.2f Volts', ebc.VoltageActual));
else
    ebc.Current = new_value * 1e-9;
    set(handles.param_best_achievable, 'String', sprintf('%g nA', good_round(ebc.CurrentActual / 1e-9, 3, 'significant')));
end

% In this dialog box, we have + and - separated out.  Everywhere else, we
% just pass around a double that's either positive or negative.
function value = get_signed_value(handles)

value = str2double(get(handles.param_value,'String'));
if is_sign_negative(handles)
    value = -value;
end


% True if constant voltage
function value = is_constant_voltage_mode(handles)

% The get returns 1 or 2
value = get(handles.current_or_voltage_popup, 'Value') - 1;



% True if negative sign
function value = is_sign_negative(handles)

% The get returns 1 or 2
value = get(handles.param_sign, 'Value') - 1;


% Gets the value of the duration edit control
function value = get_duration(handles)

value = str2double(get(handles.duration,'String'));


% This gets called if duration changes.  We don't do anything here, though.
function duration_Callback(hObject, eventdata, handles)
% hObject    handle to duration (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --- Executes during object creation, after setting all properties.
function duration_CreateFcn(hObject, eventdata, handles)
% hObject    handle to duration (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end
