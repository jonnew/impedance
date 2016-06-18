function label = get_y_label(parent)
%GET_Y_LABEL Gets the label for the Y Axis of the plots
%
% label = get_y_label(parent) returns an array of strings that specify the
% YTickLabel values for the parent axes.
%
% Converts '10000' to '10 k', '10000000' to '10 M', etc.  So, for example,
% if there are ticks at [100,000 1,000,000 10,000,000] this will return
% { '100 k', '1 M', '10 M' }
%
% Example:
%     parent = gca;
%     set(parent, 'YTickLabel', get_y_label(parent))

ticks = get(parent, 'YTick');
label = cell(1, length(ticks));

units = { 'k', 'M', 'G' };

for i=1:length(ticks)
    num_zeros = log10(ticks(i));
    unit = floor(num_zeros / 3);
    tens = num_zeros - 3 * unit;
    if unit == 0
        label{i} = sprintf('%d', 10^tens);
    else
        label{i} = sprintf('%d %s', 10^tens, units{unit});
    end
end

end