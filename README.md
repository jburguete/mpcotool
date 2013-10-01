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
> $ ./calibrator [-nthreads X] input_file.xml

* Command line in parallelized mode (where X is the number of threads to open in
every node):
> $ mpirun [MPI options] ./calibrator [-nthreads X] input_file.xml

* The sintaxis of the simulator has to be (where in the results file the first
data has to be the objective function value):
> $ ./simulator_name input_file_1 [input_file_2] [input_file_3] [input_file_4] output_file

* The sintaxis of the program to evaluate the objetive function has to be:
> $ ./evaluator_name simulated_file data_file results file

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
* The sintaxis is:
> $./pivot input_file output_file
* The program to evaluate the objective function is: compare
* The sintaxis is:
> $./compare simulated_file data_file result_file
* The calibration is performed with a sweep brutal force algorithm.
* The experimental data files are:
> 27-48.txt
> 42.txt
> 52.txt
> 100.txt
* The variables to calibrate, ranges, c-string format and sweeps number to perform are:
> alpha1, [179.70, 180.20], %.2lf, 4
> alpha2, [179.30, 179.60], %.2lf, 4
> random, [0.00, 0.20], %.2lf, 4
> boot-time, [0.0, 3.0], %.1lf, 4
* Then, the input file is:

    <?xml version="1.0"?>
    <calibrate simulator="pivot" evaluator="compare" algorithm="sweep">
        <experiment name="Velocidad-Frecuencia-27,48.txt" template1="template1.js"/>
        <experiment name="Velocidad-Frecuencia-42.txt" template1="template2.js"/>
        <experiment name="Velocidad-Frecuencia-52.txt" template1="template3.js"/>
        <experiment name="Velocidad-Frecuencia-100.txt" template1="template4.js"/>
        <variable name="alpha1" minimum="179.70" maximum="180.20" format="%.2lf" sweeps="4"/>
        <variable name="alpha2" minimum="179.30" maximum="179.60" format="%.2lf" sweeps="4"/>
        <variable name="random" minimum="0.00" maximum="0.20" format="%.2lf" sweeps="4"/>
        <variable name="boot-time" minimum="0.0" maximum="3.0" format="%.1lf" sweeps="4"/>
    </calibrate>

