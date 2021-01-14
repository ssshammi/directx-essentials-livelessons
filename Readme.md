###Overview

This is the source code repository for the video series *DirectX Essentials LiveLessons* by Paul Varcholik.

[![DirectX Essentials LiveLessons](http://www.varcholik.org/DirectXEssentialsLiveLessons/Cover.jpg)](http://my.safaribooksonline.com/video/illustration-and-graphics/9780134030036)

You'll find a forum for discussions and questions about the videos at [http://www.varcholik.org/](http://www.varcholik.org/).

###Prerequisites
* Windows 10
* Visual Studio 2019 or newer

###Quick Start (Win32 latest)

Retrieve this repository:
```
> git clone https://bitbucket.org/pvarcholik/directx-essentials-livelessons.git
```

Install vcpkg ([additional detail](https://github.com/microsoft/vcpkg)):
```
> git clone https://github.com/Microsoft/vcpkg.git
> cd vcpkg
> .\bootstrap-vcpkg.bat
> vcpkg integrate install
```

Install dependencies (assuming default triplet of x86-windows):
```
> vcpkg install ms-gsl directxtk assimp imgui
> vcpkg install ms-gsl:x64-windows directxtk:x64-windows assimp:x64-windows imgui:x64-windows
```

Open the DirectX.sln file (within the build directory) in Visual Studio and enjoy!

###Using Older versions

Retrieve this repository:
```
> git clone https://bitbucket.org/pvarcholik/directx-essentials-livelessons.git
> cd directx-essentials-livelessons
> git checkout <tagname> (see list of tags below. e.g. git checkout v1.0)
```

###Tags
* v1.0 - 7/16/2014 - Original code published with the lessons (Visual Studio 2013 projects)
* v2.0 - 7/13/2016 - Significant refactoring (Visual Studio 2015 projects)
	* Visual Studio 2015 projects
	* Pre-compiled headers
	* Project references to the Library project
	* The Library project has been reorganized to support multiple platforms (e.g. UWP)* 
	* NuGet projects for DirectXTK and assimp
	* Increased warning level (and now treating warnings as errors)
	* More C++11 usage
	* Replaced most raw pointers with smart pointers
	* Additional refactoring for [C++ Core Guidelines](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md) 
	* Bug fixes
* v3.0
    * Visual Studio 2019 projects
    * [vcpkg](https://github.com/microsoft/vcpkg) for dependencies
    * Refactoring for reduced code duplication and reduced platform dependencies

###Library Dependencies

* [GSL](https://github.com/Microsoft/GSL) - Guidlines Support Library (Microsoft)
* [DirectXTK](https://github.com/microsoft/DirectXTK) - DirectX Tool Kit
* [Assimp](http://www.assimp.org/) - Open Asset Import Library
* [ImGui](https://github.com/ocornut/imgui) - Dear ImGui