# Zippy Simulator

## ZippySimUnity
* Contains the Unity Zippy Simulator project
* Assets/Zippy Simulator/Demo contains a demo project for testing the simulator

### Requirements
* Linux
* Unity 5.6 running in OpenGL mode (This should be default on Linux, add command line argument -force-glcore if not)
* nVidia GTX 1080 or faster (will run on lower cards, but slower, and may run out of vRAM)

### Building

Can be built from the editor or from command line.

#### Editor
* Set platform as "PC, Mac & Linux Standalone"
* Set Target Platform as "Linux"
* Set Architecture as "x86_64"
* Build

#### Command Line
`</path/to/Unity> -batchmode -nographics -buildLinux64Player <full/path/to/build> -projectPath <full/path/to/project/dir> -quit`

e.g.

`/opt/Unity/Editor/Unity -batchmode -nographics -buildLinux64Player $PWD/zippysim -projectPath $PWD/packages/unity_simulator/ZippySimUnity/ -quit`

### Plugins
ZippySim relies on a native code plugin to be built before opening the Unity editor or building from the command line. Build with
`bazel build packages/unity_plugins/...`

### Unit-testing
The C# codes are unit tested with NUnit. While there is no continuous integration testing support at the time of writing this document (June 14, 2017), the manual unit tests can be done as follows:
* Environment : Ubuntu 16.04 LTS
  * Install dependencies : `sudo apt-get install monodevelop monodevelop-nunit`
  * Inside Unity, set /usr/bin/monodevelop as a default editor (Edit > Preferences > External Tools > External Script Editor)
  * Make sure the project options are targeting at the existing framework. If not sure, right-click on Assembly-CSharp, click "Options", go to "Build/General", and set "Target framework" to "Mono / .NET 4.5".
  * Make sure the supported version of nunit.framework is under Assembly-CSharp / References. If not sure, right-click on Assembly-CSharp / References, "Edit References", make sure "nunit.framework" toggle box is marked.
  * If some subfolders are missing on the Assembly-CSharp folder, please add
  * Run the tests (Run > Run Unit Tests) 
