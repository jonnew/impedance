function plot_impedance_phases(indices, impedances, selected)
%PLOT_IMPEDANCE_PHASES Plots the impedance phase graph.
%
% plot_impedance_phases(indices, impedances, selected)
% plots the impedance phase graph with the following:
%      * 'impedances' corresponding to electrodes number 'indices' (for
%        example, if you only have a single 64-channel chip connected, then
%        'indices' would be 0:63 and 'impedances' would be 64 complex
%        numbers.
%      * the 'selected' channel is highlighted

% 'impedances' are complex numbers; we only want the phases, in degrees
to_plot = angle(impedances)*180/pi;

% Plot them
p = plot(indices, to_plot, '.');

% Set the axes and the title
parent = gca;
set(parent, 'YLim', [-180 180]);
set(parent, 'XLim', [0 127]);
yl = get(parent, 'YLabel');
set(yl, 'String', 'Phase (degrees)');
xl = get(parent, 'XLabel');
set(xl, 'String', 'Channel');
t = get(parent, 'Title');
set(t, 'String', 'Current impedance phases');

% Now let's handle the selected index
hold on
index = find(indices == selected);
if ~isempty(index)
    value = to_plot(index);
    line = plot(selected, value, 'bo');
    set(line, 'Color', get(p, 'Color'));
    % And fill it in
    set(line, 'MarkerFaceColor', get(line, 'Color'));
end

% Add black lines at 0 (pure resistive) and -90 degrees (pure capacitive)
plot([0 127], [0 0], 'k-', [0 127], [-90 -90], 'k-');
hold off

end
