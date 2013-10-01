CALIBRATOR
==========

A software to perform calibrations of empirical parameters.

AUTHORS
-------

* Javier Burguete Tolosa (jburguete@eead.csic.es)
* Asier Lacasta Soto (asierlacasta@gmail.com)

REQUIRED
--------

* mpicc, gcc or clang (to compile the surce code)
* autoconf (to generate the Makefile in different systems)
* pkg-config (to find the libraries to compile)
* gthreads (to use multicores in shared memory machines)
* glib (extended utilities of C to work with data, lists, random numbers, ...)
* doxygen (standard comments format to generate documentation)
* latex (to build the PDF manuals)
* openmpi or mpich (optional: to run in parallelized tasks)

FILES
-----

* configure.ac: configure generator.
* Makefile.in: Makefile generator.
* config.h.in: config header generator.
* calibrator.c: source code.
* Doxyfile: configuration file to generate doxygen documentation.

BUILDING INSTRUCTIONS
---------------------

* aclocal
* autoconf
* ./configure
* make
* strip calibrator (optional: to make a final version)

MAKING REFERENCE MANUAL INSTRUCTIONS
------------------------------------

* doxygen
* cd doc/latex
* make
