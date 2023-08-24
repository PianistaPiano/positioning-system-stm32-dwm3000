function [Nsoll, Nsoll_2] = GetPosition(distances, pos_anchors)

P = pos_anchors; % Reference points matrix
% P = pos_anchors(:,1:3);
S = distances; % Distance vector
% S = distances(1:3); % Distance vector
n = length(distances);
% fprintf('\n Trilateration \n');
[N1 N2] = Trilateration(P,S', diag(ones(1,n)));

%Solutions
Nsol1 = N1(2:4,1);
Nsol2 = N2(2:4,1);
Nsoll = Nsol1;
Nsoll_2 = Nsol2;
end