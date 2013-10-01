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

USER INSTRUCTIONS
-----------------

* Command line in sequential mode:
$ ./calibrator [-nthreads X] input_file.xml

* Command line in parallelized mode (where X is the number of threads to open in
every node):
$ mpirun [MPI options] ./calibrator [-nthreads X] input_file.xml

* The sintaxis of the simulator has to be (where in the results file the first
data has to be the objective function value):
$ ./simulator_name input_file_1 [input_file_2] [input_file_3] [input_file_4] output_file

* The sintaxis of the program to evaluate the objetive function has to be:
$ ./evaluator_name simulated_file data_file results file

INPUT FILE FORMAT
-----------------

    <?xml version="1.0"/>
    <calibrate simulator="simulator_name" evaluator="evaluator_name" algorithm="algorithm_type" simulations="simulations_number">
        <experiment name="data_file_1" template1="template_1_1" template2="template_1_2" template3="template_1_3" template4="template_1_4"/>
        ...
        <experiment name="data_file_N" template1="template_N_1" template2="template_N_2" template3="template_N_3" template4="template_N_4"/>
        <variable name="variable_1" minimum="min_value" maximum="max_value" format="c_string_format" sweeps="sweeps_number"/>
        ...
        <variable name="variable_M" minimum="min_value" maximum="max_value" format="c_string_format" sweeps="sweeps_number"/>
    </calibrate>

SOME EXAMPLES OF INPUT FILES
----------------------------

Example 1
_________

* The simulator program name is: pivot
* The program to
