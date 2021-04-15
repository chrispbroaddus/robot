function  [width, height, frameNum, timestamp, x, y, z] = loadPointcloudImage(pointcloudFileName)

fid = fopen(pointcloudFileName);
width = fread(fid, 1, "int32");                 # Image width
height = fread(fid, 1, "int32");                # Image height
frameNum = fread(fid, 1, "int32");              # Frame number
timestamp = fread(fid, 1, "float32");           # Unity timestamp
im = fread(fid, width*height*3, "float32");     # Image data array

len = width*height;

x = zeros(1, len);                              # This will hold the X position world values
y = zeros(1, len);                              # This will hold the Y position world values
z = zeros(1, len);                              # This will hold the Z position world values

kk = 1;
for ii = 1:3:numel(im)
	x(kk) = im(ii);
	y(kk) = im(ii+1);
	z(kk) = im(ii+2);
	kk++;
end

# reshape to image dimensions
#x = reshape(x, width, height)';
#y = reshape(y, width, height)';
#z = reshape(z, width, height)';
