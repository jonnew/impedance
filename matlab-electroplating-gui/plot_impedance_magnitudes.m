function plot_impedance_magnitudes(indices, impedances, threshold, selected)
%PLOT_IMPEDANCE_MAGNITUDES Plots the impedance magnitude graph.
%
% plot_impedance_magnitudes(indices, impedances, threshold, selected)
% plots the impedance magnitude graph with the following:
%      * 'impedances' corresponding to electrodes number 'indices' (for
%        example, if you only have a single 64-channel chip connected, then
%        'indices' would be 0:63 and 'impedances' would be 64 complex
%        numbers.
%      * the plating 'threshold', in ohms
%      * the 'selected' channel is highlighted

% Upper and lower limits of the graph
upper = 1e7;
lower = 1e4;

% 'impedances' are complex numbers; we only want the magnitudes
to_plot = abs(impedances);

% Each point is either:
%    * low, below the lower limit, red down arrow
%    * in range, between the limits, blue dot
%    * high, above the upper limit, red up arrow
in_range = (to_plot >= lower) & (to_plot <= upper);
low = to_plot < lower;
high = to_plot > upper;

% Plot whichever of those have more than 0 elements.  See add_to_plot for
% details.
if sum(in_range) > 0
    add_to_plot(indices(in_range), to_plot(in_range), 'b.');
end
if sum(low) > 0
    add_to_plot(indices(low), lower*ones(sum(low),1), 'rv');
end
if sum(high) > 0
    add_to_plot(indices(high), upper*ones(sum(high),1), 'r^');
end

if ~isempty(threshold)
    add_to_plot([0 127], [threshold threshold], 'g-');
end
hold off

% Set Y axis limits
parent = gca;
set(parent, 'YLim', [lower upper]);
set(parent, 'YScale', 'log');
set(parent, 'YTickLabel', get_y_label(parent))

% Set X limits
set(parent, 'XLim', [0 127]);

% Label the axes
yl = get(parent, 'YLabel');
set(yl, 'String', 'Impedance (ohms)');
xl = get(parent, 'XLabel');
set(xl, 'String', 'Channel');

% Title the graph
t = get(parent, 'Title');
if ~isempty(threshold)
    num = length(to_plot);
    num_below = sum(to_plot < threshold);
    title = sprintf('Present impedance magnitudes - (%d of %d below threshold)', num_below, num);
else
    title = 'Present impedance magnitudes';
end
set(t, 'String', title);

% Now let's handle the selected index
hold on
index = find(indices == selected);
if ~isempty(index)
    value = to_plot(index);
    
    % Figure out which shape to plot (and whether to plot the actual value
    % or the limit, if out of range)
    if value < lower
        plot_pattern = 'rv';
        value = lower;
    else
        if value > upper
            plot_pattern = 'r^';
            value = upper;
        else
            plot_pattern = 'bo';
        end
    end
    
    % Plot
    line = semilogy(selected, value, plot_pattern);

    % Fill in
    set(line, 'MarkerFaceColor', get(line, 'Color'));
end
hold off

end

function add_to_plot(x, y, s)
% Used above.  The key is that we do a 'hold on' whenever any one of the
% three groups (low, high, in_range) above plots.
semilogy(x, y, s);
hold on
end

