function plot_history(measurement_times, measurements, pulse_times, durations, threshold, selected)
%PLOT_HISTORY Plots the history of measurements and pulses.
%
% plot_history(measurement_times, measurements, pulse_times, durations, threshold, selected)
% plots the following:
%     * the history of 'measurements', measured at 'measurement_times'
%     * the pulses, occurring at 'pulse_times' with durations 'durations'
%     * the current automatic plating threshold, 'threshold'
% and labels the plot with the number of the currently selected electrode,
% 'selected'.

% Figure out appropriate x limits
xlim = [0 1];
if ~isempty(measurement_times) || ~isempty(pulse_times)
    xlim(1) = min([measurement_times pulse_times]);
end
tmp = [];
if ~isempty(pulse_times) && ~isempty(durations)
    tmp = (pulse_times + durations);
end
if ~isempty(measurement_times) || ~isempty(tmp)
    xlim(2) = max([measurement_times tmp]);
end
if xlim(1) >= xlim(2)
    xlim(2) = xlim(1) + 10;
end

% 'measurements' consists of complex impedances; we want to plot the
% magnitudes of those.
to_plot = abs(measurements);

% Upper and lower limits of the graph
upper = 1e7;
lower = 1e4;

% Calculate y limits - use upper and lower or actual values, whichever are
% greater
threshold_low_limit = threshold;
if floor(log10(threshold)) == log10(threshold)
    threshold_low_limit = 10^(log10(threshold) - 1);
end

threshold_high_limit = threshold;
if ceil(log10(threshold)) == log10(threshold)
    threshold_high_limit = 10^(log10(threshold) + 1);
end

minmax = [0 0];
minmax(1) = min([to_plot threshold_low_limit lower]);
minmax(2) = max([to_plot threshold_high_limit upper]);
ylim = [10^floor(log10(minmax(1))) 10^ceil(log10(minmax(2)))];

% Do this before hold on, so we get a new plot
% Add data points
semilogy(measurement_times, to_plot, 'b-');
hold on

line = semilogy(measurement_times, to_plot, 'bo');

% Fill in
set(line, 'MarkerFaceColor', get(line, 'Color'));

% Plot the threshold
if ~isempty(threshold)
    semilogy(xlim, [threshold threshold], 'g-');
end

% Add pulses to graph
for i=1:length(pulse_times)
    semilogy(pulse_times(i) + [0 durations(i)], [ylim(1) ylim(1)], 'r-', 'LineWidth', 2);
end

% Set X and Y axes
parent = gca;
set(parent, 'YLim', ylim);
set(parent, 'YScale', 'log');
set(parent, 'YTickLabel', get_y_label(parent));
set(parent, 'XLim', xlim);

hold off

% Label axes
yl = get(parent, 'YLabel');
set(yl, 'String', 'Impedance (ohms)');
xl = get(parent, 'XLabel');
set(xl, 'String', 'Time (seconds)');

% Title the graph itself
t = get(parent, 'Title');
set(t, 'String', sprintf('Impedance history (channel %d)', selected));

end