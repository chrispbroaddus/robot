
function [dx, Pp] = ekf_update_step(P, y, H, R) 

S = H*P*H'+R;
S
K = P*H'*inv(S);
K
dx = K*y;
Pp = (P-P*K*H);