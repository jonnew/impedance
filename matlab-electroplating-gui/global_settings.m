function varargout = global_settings(varargin)
% GLOBAL_SETTINGS MATLAB code for global_settings.fig
%
% [ok, max_pulses_out, delay_before_pulse_out, delay_after_pulse_out, channels_0_to_63_out, channels_64_to_127_out, use_target_impedance_out] = ...
%       global_settings(max_pulses_in, delay_before_pulse_in, delay_after_pulse_in, channels_0_to_63_in, channels_64_to_127_in, use_target_impedance_in)
%
% Displays the global settings dialog box and returns its parameters.
%     all input variables are optional; without them, default values 
%     will be used.
%
% ok                         - true if the user hit ok; false for cancel or
%                              closing with X
% max_pulses (_in/_out)      - maximum number of pulses to use for
%                              automated plating.  E.g., if this is 3, each
%                              channel will use at most 3 pulses before
%                              giving up and moving to the next channel.
%                              Of course, the channel might only need fewer
%                              pulses, if the target impedance is reached.
% delay_before_pulse (_in/_out) - delay_before_pulse, in seconds, after measurement and 
%                              before pulse. Used for both manual and automatic
%                              pulses.
% delay_after_pulse (_in/_out) - delay_before_pulse, in seconds, after pulse and before
%                              measurement. Used for both manual and automatic
%                              pulses.
% channels_0_to_63 (_in/_out)  - true if electrode channels 0-63 are
%                              present, false otherwise.  Can be used when
%                              plating a single 64-channel electrode, for
%                              instance.
% channels_64_to_127 (_in/_out)  - true if electrode channels 64-127 are
%                              present, false otherwise.  Can be used when
%                              plating a single 64-channel electrode, for
%                              instance.
% use_target_impedance (_in/_out) - true if plating should stop when the
%                              target impedance is reached; false if
%                              plating should ignore target impedance and
%                              only stop when the maximum number of pulses
%                              is reached (or when manually cancelled)
%
% Example:
%      [ok, max_pulses, delay_before, delay_after, channels_0_to_63, ...
%       channels_64_to_127, use_target_impedance] = ...
%        global_settings(10, 0.0, 1.0, true, true, true);


% Edit the above text to modify the response to help global_settings

% Last Modified by GUIDE v2.5 20-Feb-2015 12:57:28

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @global_settings_OpeningFcn, ...
                   'gui_OutputFcn',  @global_settings_OutputFcn, ...
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


% --- Executes just before global_settings is made visible.
function global_settings_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to global_settings (see VARARGIN)

% Choose default command line output for global_settings
handles.output = hObject;
handles.okay = false;

nvarargin = length(varargin);

% Set max_pulses
if nvarargin > 0
    max_pulses = varargin{1};
else
    max_pulses = 10;
end
set(handles.num_pulses, 'String', sprintf('%d', max_pulses));

% Set delay_before_pulse
if nvarargin > 1
    delay_before_pulse = varargin{2};
else
    delay_before_pulse = 0;
end
set(handles.delay_before_pulse, 'String', sprintf('%.1f', delay_before_pulse));

% Set delay_after_pulse
if nvarargin > 2
    delay_after_pulse = varargin{3};
else
    delay_after_pulse = 1.0;
end
set(handles.delay_after_pulse, 'String', sprintf('%.1f', delay_after_pulse));

% Set channels_0_to_63
if nvarargin > 3
    channels_0_to_63 = varargin{4};
else
    channels_0_to_63 = true;
end
set(handles.checkbox_0_to_63, 'Value', channels_0_to_63);

% Set channels_64_to_127
if nvarargin > 4
    channels_64_to_127 = varargin{5};
else
    channels_64_to_127 = true;
end
set(handles.checkbox_64_to_127, 'Value', channels_64_to_127);

% Set use_target_impedance
if nvarargin > 5
    use_target_impedance = varargin{6};
else
    use_target_impedance = true;
end
set(handles.plate_to_target, 'Value', use_target_impedance);
set(handles.plate_unlimited, 'Value', ~use_target_impedance);

% Update handles structure
guidata(hObject, handles);

% Make the GUI modal
set(handles.figure1,'WindowStyle','modal')

% UIWAIT makes global_settings wait for user response (see UIRESUME)
 uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = global_settings_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Set outputs
varargout{1} = handles.okay;
varargout{2} = str2double(get(handles.num_pulses, 'String'));
varargout{3} = str2double(get(handles.delay_before_pulse, 'String'));
varargout{4} = str2double(get(handles.delay_after_pulse, 'String'));
varargout{5} = get(handles.checkbox_0_to_63, 'Value');
varargout{6} = get(handles.checkbox_64_to_127, 'Value');
varargout{7} = get(handles.plate_to_target, 'Value');

% The figure can be deleted now
delete(handles.figure1);


% --- Executes on button press in ok_button.
function ok_button_Callback(hObject, eventdata, handles)
% hObject    handle to ok_button (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
handles.okay = true;

% Update handles structure
guidata(hObject, handles);

uiresume(handles.figure1)

% --- Executes on button press in cancel_button.
function cancel_button_Callback(hObject, eventdata, handles)
% hObject    handle to cancel_button (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
handles.okay = false;

% Update handles structure
guidata(hObject, handles);

uiresume(handles.figure1)


function delay_before_pulse_Callback(hObject, eventdata, handles)
% hObject    handle to delay_before_pulse (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of delay_before_pulse as text
%        str2double(get(hObject,'String')) returns contents of delay_before_pulse as a double


% --- Executes during object creation, after setting all properties.
function delay_before_pulse_CreateFcn(hObject, eventdata, handles)
% hObject    handle to delay_before_pulse (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function num_pulses_Callback(hObject, eventdata, handles)
% hObject    handle to num_pulses (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of num_pulses as text
%        str2double(get(hObject,'String')) returns contents of num_pulses as a double


% --- Executes during object creation, after setting all properties.
function num_pulses_CreateFcn(hObject, eventdata, handles)
% hObject    handle to num_pulses (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in checkbox_0_to_63.
function checkbox_0_to_63_Callback(hObject, eventdata, handles)
% hObject    handle to checkbox_0_to_63 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkbox_0_to_63


% --- Executes on button press in checkbox_64_to_127.
function checkbox_64_to_127_Callback(hObject, eventdata, handles)
% hObject    handle to checkbox_64_to_127 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkbox_64_to_127



function delay_after_pulse_Callback(hObject, eventdata, handles)
% hObject    handle to delay_after_pulse (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of delay_after_pulse as text
%        str2double(get(hObject,'String')) returns contents of delay_after_pulse as a double


% --- Executes during object creation, after setting all properties.
function delay_after_pulse_CreateFcn(hObject, eventdata, handles)
% hObject    handle to delay_after_pulse (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end
