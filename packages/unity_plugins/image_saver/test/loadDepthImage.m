function  [w, h, frameNum, timestamp, depthIm] = loadDepthImage(depthFileName)

fid = fopen(depthFileName);
w = fread(fid, 1, "int32");				# Image width
h = fread(fid, 1, "int32");				# Image height
frameNum = fread(fid, 1, "int32");		# Frame number
timestamp = fread(fid, 1, "float32");	# Unity Timestamp
depthIm = fread(fid, h*w, "float32");	# Depth image as array of floats
depthIm = reshape(depthIm, w, h)';		# Reshape into image
