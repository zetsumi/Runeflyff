# Runeflyff

About
--- 
This Flyff v6 emulator was originally started in 2008/2009 by Duotune. It was too cool to let die.
You will need a v6 client with the Kiki Neuz to play with this.
While you can in theory run this on a production server, I highly advise against it.

System Requirements
---
* Windows 7 or later (64-bit preferred)
* At least a dual core processor (4 cores or more recommended)
* Minimum 512mb RAM available for the Runeflyff processes (the world server idles at around 120mb as of 0.74.3)
* MySQL 5.5 or later - or the MariaDB equivalent. If you use MySQL 8 or later, you MUST enable legacy authentication support.

Build Notes
---
You will need the following dependencies to build Runeflyff from source:
* [MySQL Connector/C](https://dev.mysql.com/downloads/connector/c/) - 6.1 or later
    * You will give this path to Cmake as MySQL_DIR
    * If your MySQL server is running on your build machine, you can use the install path to the server instead
* [Qt5](https://www.qt.io/download) - 5.13 or later (use the Windows installer under the open source edition)
    * You will give this path to Cmake as Qt5_DIR
    * After the install, you will have to [build the SQL drivers](https://doc.qt.io/qt-5/sql-driver.html) using your compiler of choice.
* pthreads-win32 (under lib and dll folder - included available for free online)
    * Note: Modern MinGW compilers include their own version and the Cmake script will import it for you. The bundled version is only used for Visual Studio builds.

Supported compilers:
* The MinGW compiler available as part of the Qt5 installation. This is the recommended way to build on Windows
* [Visual Studio](https://visualstudio.microsoft.com/downloads/) 2012 or later. I *strongly* recommened not using any version older than 2017.

Disclaimer
---
There is no warranty, implied warranty, or liability on my part if you use this software. It might
work perfectly, or it might sacrifice kittens to Satan while you sleep. As with Duotune's releases,
you are free to do whatever you wish with this. My only request is that you give credit where credit is
due. If I ever modify the codebase significantly, this will probably move to a GPL license, but for now 
it's a free for all.