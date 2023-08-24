function Nsoll = GetPosition_2(distances, pos_anchors)

n = length(distances);

P = pos_anchors; % Reference points matrix
S = distances'; % Distance vector

% ---Trilateration: Recursive least square---
W = diag(ones(1,length(S)));
Nmat0 = RecTrilateration(P,S,W);
Nmat = Nmat0(:,1:n);
[n_mat m_mat]=size(Nmat);
for i1=1:m_mat
    Nn = Nmat(:,i1);
    Nn = Nn(2:4);
    [Sn(i1,:) , F(i1)] = distanzen(Nn,P,S);
    Diff(i1)= Nmat(1,i1) - (norm(Nn))^2;
end
[Fmin Imin] = min(F);
Nrec = Nmat(:,Imin);
% Nrec = Nmat(:,2);
Nsoll = Nmat(2:4)
end