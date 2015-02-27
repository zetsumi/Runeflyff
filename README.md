# Runeflyff
# Version 0.74.2.2

---- About ----
This Flyff v6 emulator was originally started in 2008/2009 by Duotune. It was too cool to let die.
You will need a v6 client with the Kiki Neuz to play with this.
While you can in theory run this on a production server, I highly advise against it.

---- Requirements ------
System requirements:
	* Windows XP or newer (I hope to get Linux support working in the cmake version eventually)
	* Pentium 4 / Athlon processor or better.
	* Minimum 512mb of ram (the world server eats around 120mb when idling I think)
You will need MySQL or MariaDB. Note that I have not tested MariaDB with this version but it should work
I recommend using the MySQL 5.x branch.

---- Build notes ------
The following are the current dependencies to build Runeflyff:
	* libmysqlclient (under lib folder - I'd like to move to the mariaDB 6.x client eventually)
	* pthreads-win32 (under lib folder - included available for free online)
	* I've been able to build under Visual Studio 2008 and 2010.
	* Note: The Cmake build doesn't copy the dlls yet, so you'll need to do this by hand.

---- Disclaimer ---
There is no warranty, implied warranty, or liability on my part if you use this software. It might
work perfectly, or it might sacrifice kittens to Satan while you sleep. As with Duotune's releases,
you are free to do whatever you wish with this. My only request is that you give credit where credit is
due. If I ever modify the codebase significantly, this will probably move to a GPL license, but for now 
it's a free for all.