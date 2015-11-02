CALIBRATOR (1.1.30 version)
===========================

A software to perform calibrations or optimizations of empirical parameters.

AUTHORS
-------

* Javier Burguete Tolosa (jburguete@eead.csic.es)
* Borja Latorre Garcés (borja.latorre@csic.es)

TOOLS AND LIBRARIES REQUIRED TO BUILD THE EXECUTABLE
----------------------------------------------------

* [gcc](https://gcc.gnu.org) or [clang](http://clang.llvm.org) (to compile the
source code)
* [make](http://www.gnu.org/software/make) (to build the executable file)
* [autoconf](http://www.gnu.org/software/autoconf) (to generate the Makefile in
different operative systems)
* [automake](http://www.gnu.org/software/automake) (to check the operative
system)
* [pkg-config](http://www.freedesktop.org/wiki/Software/pkg-config) (to find the
libraries to compile)
* [gsl](http://www.gnu.org/software/gsl) (to generate random numbers)
* [libxml](http://xmlsoft.org) (to deal with XML files)
* [glib](https://developer.gnome.org/glib) (extended utilities of C to work with
data, lists, mapped files, regular expressions, using multicores in shared
memory machines, ...)
* [genetic](https://github.com/jburguete/genetic) (genetic algorithm)

OPTIONAL TOOLS AND LIBRARIES
----------------------------

* [gtk+](http://www.gtk.org) (to create the interactive GUI tool)
* [openmpi](http://www.open-mpi.org) or [mpich](http://www.mpich.org) (to run in
parallelized tasks on multiple computers)
* [doxygen](http://www.stack.nl/~dimitri/doxygen) (standard comments format to
generate documentation)
* [latex](https://www.latex-project.org/) (to build the PDF manuals)

FILES
-----

The source code has to have the following files:
* configure.ac: configure generator.
* Makefile.in: Makefile generator.
* config.h.in: config header generator.
* calibrator.c: main source code.
* calibrator.h: main header code.
* interface.h: interface header code.
* build: script to build all.
* logo.png: logo figure.
* logo2.png: alternative logo figure.
* Doxyfile: configuration file to generate doxygen documentation.
* TODO: tasks to do.
* README.md: this file.
* tests/testX/*: several tests to check the program working.
* locales/*/LC_MESSAGES/calibrator.po: translation files.
* manuals/*.png: manual figures.
* manuals/*.tex: documentation source files.
* applications/*/*: several practical application cases.
* check_errors/*.xml: several mistaken files to check error handling.

BUILDING INSTRUCTIONS
---------------------

This software has been built and tested in the following operative systems.
Probably, it can be built in other systems, distributions, or versions but it
has not been tested.

Debian 8 (Linux, kFreeBSD or Hurd)
__________________________________
DragonFly BSD 4.2
_________________
FreeBSD 10.2
____________
NetBSD 7.0
__________

1. Download the latest [genetic](https://github.com/jburguete/genetic) doing on
a terminal:
> $ git clone https://github.com/jburguete/genetic.git

2. Download this repository:
> $ git clone https://github.com/jburguete/calibrator.git

3. Link the latest genetic version to genetic:
> $ cd calibrator/1.1.30
>
> $ ln -s ../../genetic/0.6.1 genetic

4. Build doing on a terminal:
> $ ./build

OpenBSD 5.8
___________

1. Select adequate versions:
> $ export AUTOCONF_VERSION=2.69 AUTOMAKE_VERSION=1.15

2. Then, in a terminal, follow steps 1 to 4 of the previous Debian 8 section.

Microsoft Windows 7 (with MSYS2)
________________________________
Microsoft Windows 8.1 (with MSYS2)
__________________________________

1. Install [MSYS2](http://sourceforge.net/projects/msys2) and the required
libraries and utilities. You can follow detailed instructions in
[install-unix]
(https://github.com/jburguete/install-unix/blob/master/tutorial.pdf)

2. Then, in a MSYS2 terminal, follow steps 1 to 4 of the previous Debian 8
section.

3. Optional Windows binary package can be built doing in the terminal:
> $ make windist

MAKING REFERENCE MANUAL INSTRUCTIONS
------------------------------------

On UNIX type systems you need [texlive](https://www.tug.org/texlive) installed.
On Windows systems you need [MiKTeX](http://miktex.org).

USER INSTRUCTIONS
-----------------

* Command line in sequential mode:
> $ ./calibratorbin [-nthreads X] input_file.xml

* Command line in parallelized mode (where X is the number of threads to open in
every node):
> $ mpirun [MPI options] ./calibratorbin [-nthreads X] input_file.xml

* The syntax of the simulator has to be:
> $ ./simulator_name input_file_1 [input_file_2] [input_file_3] [input_file_4] output_file

* The syntax of the program to evaluate the objetive function has to be (where
the first data in the results file has to be the objective function value):
> $ ./evaluator_name simulated_file data_file results_file

* On UNIX type systems the GUI application can be open doing on a terminal:
> $ ./calibrator

INPUT FILE FORMAT
-----------------

    <?xml version="1.0"/>
    <calibrate simulator="simulator_name" evaluator="evaluator_name" algorithm="algorithm_type" nsimulations="simulations_number" niterations="iterations_number" tolerance="tolerance_value" nbest="best_number" npopulation="population_number" ngenerations="generations_number" mutation="mutation_ratio" reproduction="reproduction_ratio" adaptation="adaptation_ratio">
        <experiment name="data_file_1" template1="template_1_1" template2="template_1_2" ... weight="weight_1"/>
        ...
        <experiment name="data_file_N" template1="template_N_1" template2="template_N_2" ... weight="weight_N"/>
        <variable name="variable_1" minimum="min_value" maximum="max_value" precision="precision_digits" sweeps="sweeps_number" nbits="bits_number"/>
        ...
        <variable name="variable_M" minimum="min_value" maximum="max_value" precision="precision_digits" sweeps="sweeps_number" nbits="bits_number"/>
    </calibrate>

* *"precision"* defined for each variable. Number of precision digits to
evaluate the variable. 0 apply for integer numbers.

* *"weight"* defined for each experiment. Multiplies the objective value
obtained for each experiment in the final objective function value.

Implemented algorithms are:

* *"sweep"*: Sweep brutal force algorithm. Requires for each variable:
> sweeps: number of sweeps to generate for each variable in every experiment. 

  The total number of simulations to run is:
> (number of experiments) x (variable 1 number of sweeps) x ... x
> (variable n number of sweeps) x (number of iterations)

* *"Monte-Carlo"*: Monte-Carlo brutal force algorithm. Requires on calibrate:
> nsimulations: number of simulations to run in every experiment.

  The total number of simulations to run is:
> (number of experiments) x (number of simulations) x (number of iterations)

* Both brutal force algorithms can be iterated to improve convergence by using
the following parameters:
> nbest: number of best simulations to calculate convergence interval on next
> iteration (default 1).
>
> tolerance: tolerance parameter to increase convergence interval (default 0).
>
> niterations: number of iterations (default 1).

* *"genetic"*: Genetic algorithm. Requires the following parameters:
> npopulation: number of population.
>
> ngenerations: number of generations.
>
> mutation: mutation ratio.
>
> reproduction: reproduction ratio.
>
> adaptation: adaptation ratio.

  and for each variable:
> nbits: number of bits to encode each variable.

  The total number of simulations to run is:
> (number of experiments) x (npopulation) x [1 + (ngenerations - 1)
> x (mutation + reproduction + adaptation)]

SOME EXAMPLES OF INPUT FILES
----------------------------

Example 1
_________

* The simulator program name is: *pivot*
* The syntax is:
> $ ./pivot input_file output_file

* The program to evaluate the objective function is: *compare*
* The syntax is:
> $ ./compare simulated_file data_file result_file

* The calibration is performed with a *sweep brutal force algorithm*.

* The experimental data files are:
> 27-48.txt
>
> 42.txt
>
> 52.txt
>
> 100.txt

* Templates to get input files to simulator for each experiment are:
> template1.js
>
> template2.js
>
> template3.js
>
> template4.js

* The variables to calibrate, ranges, c-string format and sweeps number to perform are:
> alpha1, [179.70, 180.20], %.2lf, 5
>
> alpha2, [179.30, 179.60], %.2lf, 5
>
> random, [0.00, 0.20], %.2lf, 5
>
> boot-time, [0.0, 3.0], %.1lf, 5

* Then, the number of simulations to run is: 4x5x5x5x5=2500.

* The input file is:

_

    <?xml version="1.0"?>
    <calibrate simulator="pivot" evaluator="compare" algorithm="sweep">
        <experiment name="27-48.txt" template1="template1.js"/>
        <experiment name="42.txt" template1="template2.js"/>
        <experiment name="52.txt" template1="template3.js"/>
        <experiment name="100.txt" template1="template4.js"/>
        <variable name="alpha1" minimum="179.70" maximum="180.20" format="%.2lf" nsweeps="5"/>
        <variable name="alpha2" minimum="179.30" maximum="179.60" format="%.2lf" nsweeps="5"/>
        <variable name="random" minimum="0.00" maximum="0.20" format="%.2lf" nsweeps="5"/>
        <variable name="boot-time" minimum="0.0" maximum="3.0" format="%.1lf" nsweeps="5"/>
    </calibrate>


* A template file as *template1.js*:

_

    {
      "towers" :
      [
        {
          "length"      : 50.11,
          "velocity"    : 0.02738,
          "@variable1@" : @value1@,
          "@variable2@" : @value2@,
          "@variable3@" : @value3@,
          "@variable4@" : @value4@
        },
        {
          "length"    : 50.11,
          "velocity"  : 0.02824,
          "@variable1@" : @value1@,
          "@variable2@" : @value2@,
          "@variable3@" : @value3@,
          "@variable4@" : @value4@
        },
        {
          "length"    : 50.11,
          "velocity"  : 0.03008,
          "@variable1@" : @value1@,
          "@variable2@" : @value2@,
          "@variable3@" : @value3@,
          "@variable4@" : @value4@
        },
        {
          "length"    : 50.11,
          "velocity"  : 0.03753,
          "@variable1@" : @value1@,
          "@variable2@" : @value2@,
          "@variable3@" : @value3@,
          "@variable4@" : @value4@
        }
      ],
      "cycle-time"     : 71.0,
      "plot-time"     : 1.0,
      "comp-time-step": 0.1,
      "active-percent" : 27.48
    }

* Produce simulator input files to reproduce the experimental data file
*27-48.txt* as:

_

    {
      "towers" :
      [
        {
          "length"      : 50.11,
          "velocity"    : 0.02738,
          "alpha1" : 179.95,
          "alpha2" : 179.45,
          "random" : 0.10,
          "boot-time" : 1.5
        },
        {
          "length"    : 50.11,
          "velocity"  : 0.02824,
          "alpha1" : 179.95,
          "alpha2" : 179.45,
          "random" : 0.10,
          "boot-time" : 1.5
        },
        {
          "length"    : 50.11,
          "velocity"  : 0.03008,
          "alpha1" : 179.95,
          "alpha2" : 179.45,
          "random" : 0.10,
          "boot-time" : 1.5
        },
        {
          "length"    : 50.11,
          "velocity"  : 0.03753,
          "alpha1" : 179.95,
          "alpha2" : 179.45,
          "random" : 0.10,
          "boot-time" : 1.5
        }
      ],
      "cycle-time"     : 71.0,
      "plot-time"     : 1.0,
      "comp-time-step": 0.1,
      "active-percent" : 27.48
    }
