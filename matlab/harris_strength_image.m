function S = harris_strength_image(I)

d = [-1 0 1];
I = double(I);

% Compute Ix,Iy
Ix = -conv2(I, d, 'same');
Iy = -conv2(I, d', 'same');
Ix = double(fix(Ix/2));
Iy = double(fix(Iy/2));
Ixx = Ix.*Ix;
Iyy = Iy.*Iy;
Ixy = Ix.*Iy;

h = [1 4 6 4 1];

% Sum Ix
Ixx = conv2(Ixx, h, 'same');
Gxx = conv2(Ixx, h', 'same');

% Sum Iy
Iyy = conv2(Iyy, h, 'same');
Gyy = conv2(Iyy, h', 'same');

% Sum Ixy
Ixy = conv2(Ixy, h, 'same');
Gxy = conv2(Ixy, h', 'same');

Gxx = single(Gxx);
Gyy = single(Gyy);
Gxy = single(Gxy);

% det(A)-k*trace(A)^2
k = single(1);
S = (Gxx.*Gyy-Gxy.^2) - k*(Gxx+Gyy).^2;