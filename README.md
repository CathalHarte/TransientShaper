![PAMPLEJUCE](assets/images/primitiveBanner.png)

<!---
## [![](https://github.com/sudara/pamplejuce/actions/workflows/build_and_test.yml/badge.svg)](https://github.com/sudara/pamplejuce/actions)
--->

Living the Pamplejuce lifestyle: Transient Shaper, a first plugin offering from Primitive, I guess?

Intended for multi platform development. On Windows, VS2022 _can_ be used, just ensure that C++ desktop development package, with the cmake module added, is installed.
Simply open the TransientShaper folder in VS2022, and any edit to the CMakeLists file will auto trigger the cmake build. I found it necessary to install [Intel IPP](https://www.intel.com/content/www/us/en/developer/tools/oneapi/ipp.html#gs.cykfky). I downloaded it as part of a toolkit, which was probably overkill.

Preferred Build and Debug Environment
-------------------------------------
VS Code offers a transparent build and debug environment, and is available for Windows, Mac and Linux, so it is our preferred tool. 
Configuring it to run on each particular machine is simply a matter of installing some extensions, and writing the launch.json and settings.json files in the .vscode folder. These are machine specific, so are not included as part of the repo, but a specific config can be found below as a guide

launch.json
```json
{
  "version": "0.2.0",
  "configurations": [
      {
          "name": "Debug VST in AudioPluginHost",
          "type": "cppvsdbg",
          "request": "launch",
          "program": "${workspaceFolder}/JUCE/extras/AudioPluginHost/Builds/VisualStudio2022/x64/Release/App/AudioPluginHost.exe", 
          "args": ["--plugin", "${workspaceFolder}/build/TransientShaper_artefacts/Debug/VST3/Transient Shaper.vst3"],  
          "stopAtEntry": false,
          "cwd": "${workspaceFolder}",
          "environment": [],
          "externalConsole": false
      }
  ]
}
```

settings.json
```json
{
    "cmake.cmakePath": "C:/Program Files/Cmake/bin/cmake.exe",  // Adjust this to your system's path
    "cmake.generator": "Ninja",
    "dotnet.defaultSolution": "disable", // Or "Unix Makefiles" or "Visual Studio 16 2019", depending on your environment
}
```
The extension **C/C++ Extension Pack** can be installed, as it contains all the individual extensions required to build and debug cmake projects: C/C++, CMake, and CMake tools.



For any additional information not covered in this README, the pamplejuce template project can be found [here](https://github.com/sudara/pamplejuce).

