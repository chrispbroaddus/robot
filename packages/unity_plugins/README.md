# Unity Plugins
- These plugins are for use in the Unity Zippy Simulator.
- The built *.so files should be copied to the appropriate location in the Unity project.
- The must be configured to run in the Editor and Standalone on Linux, x64 only.

## Image Saver
Native Unity plugin to save downloaded images from the GPU to disk

### Requirements
- None

### Building
`bazel build packages/unity_plugins:image_saver`

## TextureDownloader
Unity C++ Plugin for downloading textures from the GPU

### Requirements
- OpenGL
- GLEW

### Building
`bazel build packages/unity_plugins:texture_downloader`
