function [ value ] = good_round( A, B, C )
%GOOD_ROUND On R2014b, calls round.  Fakes it on other versions

v = version('-release');
if strcmp(v, '2014b')
    value = round(A, B, C);
else
    value = A;
end

end

