# Runeflyff

This Flyff v6 emulator was originally started in 2008/2009 by Duotune. It was too cool to let die.
You will need a v6 client with the Kiki Neuz to play with this.
While you can in theory run this on a production server, I highly advise against it.

There is no warranty, implied warranty, or liability on my part if you use this software. It might
work perfectly, or it might sacrifice kittens to Satan while you sleep. As with Duotune's releases,
you are free to do whatever you wish with this. My only request is that you give credit where credit is
due. If I ever modify the codebase significantly, this will probably move to a GPL license, but for now 
it's a free for all.

System Requirements
===
|  |Minimum|Recommended|
|:---:|:---:|:---:|
|Processor|Pentium 4 or equivalent AMD|64 bit compatible<sup>(1)</sup>|
|Memory|1GB|2GB|
|Operating System<sup>(2)</sup>|Windows XP|Windows 7|
|MariaDB / MySQL|5.5|5.7<sup>(3)</sup>|

Notes
---
1. At least a dual core processor is recommended.
2. Using a 64 bit operating system is recommended for performance. If you do so, double the memory requirements above.
3. MySQL / MariaDB versions greater than 5.7 are untested.

Build Requirements
===
Compilers
---
* MinGW-W64 - GCC 4.8 or later 
    * Using the [online installer](https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/installer/mingw-w64-install.exe) is recommended.
    * CMake is included with this compiler
* [Visual Studio](https://visualstudio.microsoft.com/downloads/) - 2012 or later
    * Visual Studio 2017 or later is recommended.

Software
---
You will need the following dependencies to build Runeflyff from source:
* [Cmake](https://cmake.org/download/) - 3.1 or later 
* [MySQL Connector/C](https://dev.mysql.com/downloads/connector/c/) - 6.1 or later
    * You will give this path to Cmake as **MySQL_DIR** 
* pthreads-win32 (under lib and dll folder)
    * MinGW-W64 compilers include their own version and the Cmake script will import it for you. The bundled version is only used for Visual Studio builds.

Changelog
===

0.74.4
---
* CMake autodetects MySQL Connector/C if installed to the default ProgramFiles (x86) folder. 
    * For custom paths, set the Cmake variable **MySQL_DIR**
* Break Cmake logic into modules
* Restrict compilers to 32 bit
* Update winpthreads-32
* Support compiling under MSVC 16 (Visual Studio 2019)  
* Remove bundled MySQL  

0.74.3
---
Cmake version of 0.74.2. This version will build under MinGW 4.8.1 as well as Visual Studio. I was able to build it
successfully using --std=c++11 as well.

0.74.2
---
This is the latest version I could find online of the Runeflyff source. Very minor changes
from the 0.74 version.

0.74
---
This was the last official version floating around. There is also an unofficial Runeflyff+
based on this branch, and a 0.74.2 branch. The latter will be added after this.