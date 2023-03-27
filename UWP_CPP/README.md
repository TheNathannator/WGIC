# UWP_CPP

A C++/WinRT demonstration of how to use Windows.Gaming.Input.Custom in a UWP environment.

## Dependencies

This project uses some packages from vcpkg. Follow the [vcpkg quick start](https://github.com/Microsoft/vcpkg#quick-start), then run `./vcpkg install spdlog:x64-uwp fmt:x64-uwp` to install them.

## Potential Startup Issues

If you get missing DLL errors, make sure that you are launching from either the debugger or from the Start menu, and not from the .exe directly. Also ensure that `fmtd.dll` is present inside the `AppX` folder of the build output directory. If it is not, copy it there manually.

If you're debugging and it's still giving you trouble, try going into the project properties and setting the Debugging > Launch App setting to No. After doing this, you will have to go into the Start menu and launch the deployed app yourself, after which the debugger will attach to the process and should continue as normal.
