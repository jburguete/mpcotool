MPCOTool
========

The Multi-Purposes Calibration and Optimization Tool. A software to perform
calibrations or optimizations of empirical parameters.

VERSIONS
--------

* 3.0.0: Stable and recommended version.
* 3.1.0: Developing version to do new features.

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
* [json-glib](https://github.com/ebassi/json-glib) (to deal with JSON files)
* [genetic](https://github.com/jburguete/genetic) (genetic algorithm)

OPTIONAL TOOLS AND LIBRARIES
----------------------------

* [gettext](http://www.gnu.org/software/gettext) (to work with different
locales)
* [gtk+](http://www.gtk.org) (to create the interactive GUI tool)
* [openmpi](http://www.open-mpi.org) or [mpich](http://www.mpich.org) (to run in
parallelized tasks on multiple computers)
* [doxygen](http://www.stack.nl/~dimitri/doxygen) (standard comments format to
generate documentation)
* [latex](https://www.latex-project.org/) (to build the PDF manuals)

FILES
-----

The source code has to have the following files:
* 3.0.0/configure.ac: configure generator.
* 3.0.0/Makefile.in: Makefile generator.
* 3.0.0/config.h.in: config header generator.
* 3.0.0/mpcotool.c: main source code.
* 3.0.0/mpcotool.h: main header code.
* 3.0.0/interface.h: interface header code.
* 3.0.0/build: script to build all.
* 3.0.0/logo.png: logo figure.
* 3.0.0/Doxyfile: configuration file to generate doxygen documentation.
* TODO: tasks to do.
* README.md: this file.
* license.md: license file.
* tests/testX/\*: several tests to check the program working.
* locales/\*/LC_MESSAGES/mpcotool.po: translation files.
* manuals/\*.eps: manual figures in EPS format.
* manuals/\*.png: manual figures in PNG format.
* manuals/\*.tex: documentation source files.
* applications/\*/\*: several practical application cases.
* check_errors/\*.xml: several mistaken files to check error handling.

BUILDING INSTRUCTIONS
---------------------

This software has been built and tested in the following operative systems.
Probably, it can be built in other systems, distributions, or versions but it
has not been tested.

Debian 8 (Linux, kFreeBSD or Hurd)
__________________________________
DragonFly BSD 4.2
_________________
Dyson Illumos
_____________
Linux Mint DE 2
_______________
NetBSD 7.0
__________
OpenSUSE Linux 13
_________________
Ubuntu Linux 12, 14, and 15
___________________________

1. Download the latest [genetic](https://github.com/jburguete/genetic) doing on
a terminal:
> $ git clone https://github.com/jburguete/genetic.git

2. Download this repository:
> $ git clone https://github.com/jburguete/mpcotool.git

3. Link the latest genetic version to genetic:
> $ cd mpcotool/3.0.0
>
> $ ln -s ../../genetic/2.0.1 genetic

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

Fedora Linux 23
_______________

1. In order to use OpenMPI compilation do in a terminal (in 64 bits version):
> $ export PATH=$PATH:/usr/lib64/openmpi/bin

2. Then, follow steps 1 to 4 of the previous Debian 8 section.

FreeBSD 10.2
____________

1. In order to build in FreeBSD, due to a wrong error in default gcc version,
do in a terminal:
> $ export CC=gcc5 (or CC=clang)

2. Then, follow steps 1 to 4 of the previous Debian 8 section.

MAKING MANUALS INSTRUCTIONS
---------------------------

On UNIX type systems you need [texlive](https://www.tug.org/texlive) installed.
On Windows systems you need [MiKTeX](http://miktex.org). In order to compile the
manuals you can type on a terminal:
> $ make manuals

MAKING TESTS INSTRUCTIONS
-------------------------

In order to build the tests follow the next instructions:

1. Link some tests that needs genetic library doing in a terminal (assuming that
you are in the directory mpcotool/3.0.0):
> $ cd ../tests/test2
>
> $ ln -s ../../../genetic/2.0.1 genetic
>
> $ cd ../test3
>
> $ ln -s ../../../genetic/2.0.1 genetic
>
> $ cd ../test4
>
> $ ln -s ../../../genetic/2.0.1 genetic

2. Build all tests doing in the same terminal:
> $ cd ../../3.0.0
>
> $ make tests

USER INSTRUCTIONS
-----------------

Optional arguments are typed in square brackets.

* Command line in sequential mode (where X is the number of threads to execute
and S is a seed for the pseudo-random numbers generator):
> $ ./mpcotoolbin [-nthreads X] [-seed S] input_file.xml [result_file] [variables_file]

* Command line in parallelized mode (where X is the number of threads to
open for every node and S is a seed for the pseudo-random numbers generator):
> $ mpirun [MPI options] ./mpcotoolbin [-nthreads X] [-seed S] input_file.xml [result_file] [variables_file]

* The syntax of the simulator has to be:
> $ ./simulator_name input_file_1 [input_file_2] [input_file_3] [input_file_4] output_file

* The syntax of the program to evaluate the objetive function has to be (where
the first data in the results file has to be the objective function value):
> $ ./evaluator_name simulated_file data_file results_file

* On UNIX type systems the GUI application can be open doing on a terminal:
> $ ./mpcotool

INPUT FILE FORMAT
-----------------

The format of the main input file is as:

```xml
<?xml version="1.0"?>
<optimize simulator="simulator_name" evaluator="evaluator_name" algorithm="algorithm_type" nsimulations="simulations_number" niterations="iterations_number" tolerance="tolerance_value" nbest="best_number" npopulation="population_number" ngenerations="generations_number" mutation="mutation_ratio" reproduction="reproduction_ratio" adaptation="adaptation_ratio" direction="direction_search_type" nsteps="steps_number" relaxation="relaxation_parameter" nestimates="estimates_number" threshold="threshold_parameter" norm="norm_type" p="p_parameter" seed="random_seed" result_file="result_file" variables_file="variables_file">
    <experiment name="data_file_1" template1="template_1_1" template2="template_1_2" ... weight="weight_1"/>
    ...
    <experiment name="data_file_N" template1="template_N_1" template2="template_N_2" ... weight="weight_N"/>
    <variable name="variable_1" minimum="min_value" maximum="max_value" precision="precision_digits" sweeps="sweeps_number" nbits="bits_number" step="step_size"/>
    ...
    <variable name="variable_M" minimum="min_value" maximum="max_value" precision="precision_digits" sweeps="sweeps_number" nbits="bits_number" step="step_size"/>
</optimize>
```

with:

* **simulator**: simulator executable file name.
* **evaluator**: optional. When needed is the evaluator executable file name.
* **seed**: optional. Seed of the pseudo-random numbers generator (default value
is 7007).
* **result_file**: optional. It is the name of the optime result file (default name
is "result").
* **variables_file**: optional. It is the name of all simulated variables file
(default name is "variables").
* **precision**: optional, defined for each variable. Number of precision digits
to evaluate the variable. 0 apply for integer numbers (default value is 14).
* **weight**: optional, defined for each experiment. Multiplies the objective
value obtained for each experiment in the final objective function value
(default value is 1).
* **threshold**: optional, to stop the simulations if objective function value
less than the threshold is obtained (default value is 0).
* **algorithm**: optimization algorithm type.
* **norm**: error norm type.

Implemented algorithms are:

* **sweep**: Sweep brute force algorithm. It requires for each variable:
  * *sweeps*: number of sweeps to generate for each variable in every
  experiment. 

    The total number of simulations to run is:
> (number of experiments) x (variable 1 number of sweeps) x ... x
> (variable n number of sweeps) x (number of iterations)

* **Monte-Carlo**: Monte-Carlo brute force algorithm. It requires on calibrate:
  * *nsimulations*: number of simulations to run in every experiment.

    The total number of simulations to run is:
> (number of experiments) x (number of simulations) x (number of iterations)

* Both brute force algorithms can be iterated to improve convergence by using
the following parameters:
  * *nbest*: number of best simulations to calculate convergence interval on
  next iteration (default 1).
  * *tolerance*: tolerance parameter to increase convergence interval (default
  0).
  * *niterations*: number of iterations (default 1).

     It multiplies the total number of simulations:
> x (number of iterations)

* Moreover, both brute force algorithms can be coupled with a direction search
method by using:
  * *direction*: method to estimate the optimal direction. Two options are
  currently available:
    * coordinates: coordinates descent method.

      It increases the total number of simulations by:
> (number of experiments) x (number of iterations) x (number of steps) x 2
> x (number of variables)
    * random: random method. It requires:
    * nestimates: number of random checks to estimate the optimal direction.

      It increases the total number of simulations by:
> (number of experiments) x (number of iterations) x (number of steps)
> x (number of estimates)

  Both methods require also:
    * nsteps: number of steps to perform the direction search method,
    * relaxation: relaxation parameter,

  and for each variable:
    * step: initial step size for the direction search method.

* **genetic**: Genetic algorithm. It requires the following parameters:
  * *npopulation*: number of population.
  * *ngenerations*: number of generations.
  * *mutation*: mutation ratio.
  * *reproduction*: reproduction ratio.
  * *adaptation*: adaptation ratio.

  and for each variable:

  * *nbits*: number of bits to encode each variable.

  The total number of simulations to run is:
> (number of experiments) x (npopulation) x [1 + (ngenerations - 1)
> x (mutation + reproduction + adaptation)]

Implemented error noms are:

* **euclidian**: Euclidian norm.
* **maximum**: maximum norm.
* **p**: p-norm. It requires the parameter:
  * *p*: p exponent.
* **taxicab**: Taxicab norm.

Alternatively, the input file can be also written in JSON format as:

```json
{
	"simulator": "simulator_name",
	"evaluator": "evaluator_name",
	"algorithm": "algorithm_type",
	"nsimulations": "simulations_number",
	"niterations": "iterations_number",
	"tolerance": "tolerance_value",
	"nbest": "best_number",
	"npopulation": "population_number",
	"ngenerations": "generations_number",
	"mutation": "mutation_ratio",
	"reproduction": "reproduction_ratio",
	"adaptation": "adaptation_ratio",
	"direction": "direction_search_type",
	"nsteps": "steps_number",
	"relaxation": "relaxation_parameter",
	"nestimates": "estimates_number",
	"threshold": "threshold_parameter",
	"norm": "norm_type",
	"p": "p_parameter",
	"seed": "random_seed",
	"result_file": "result_file",
	"variables_file": "variables_file",
	"experiments":
	[
		{
			"name": "data_file_1",
			"template1": "template_1_1",
			"template2": "template_1_2",
			...
			"weight": "weight_1",
		},
	    ...
		{
			"name": "data_file_N",
			"template1": "template_N_1",
			"template2": "template_N_2",
			...
			"weight": "weight_N",
		}
	],
	"variables":
	[
		{

			"name": "variable_1",
			"minimum": "min_value",
			"maximum": "max_value",
			"precision": "precision_digits",
			"sweeps": "sweeps_number",
			"nbits": "bits_number",
			"step": "step_size",
		},
		...
		{
			"name": "variable_M",
			"minimum": "min_value",
			"maximum": "max_value",
			"precision": "precision_digits",
			"sweeps": "sweeps_number",
			"nbits": "bits_number",
			"step": "step_size",
		}
	]
}
```

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

* The calibration is performed with a *sweep brute force algorithm*.

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

* The variables to calibrate, ranges, precision and sweeps number to perform
are:
> alpha1, [179.70, 180.20], 2, 5
>
> alpha2, [179.30, 179.60], 2, 5
>
> random, [0.00, 0.20], 2, 5
>
> boot-time, [0.0, 3.0], 1, 5

* Then, the number of simulations to run is: 4x5x5x5x5=2500.

* The input file is:

```xml
<?xml version="1.0"?>
<optimize simulator="pivot" evaluator="compare" algorithm="sweep">
    <experiment name="27-48.txt" template1="template1.js"/>
    <experiment name="42.txt" template1="template2.js"/>
    <experiment name="52.txt" template1="template3.js"/>
    <experiment name="100.txt" template1="template4.js"/>
    <variable name="alpha1" minimum="179.70" maximum="180.20" precision="2" nsweeps="5"/>
    <variable name="alpha2" minimum="179.30" maximum="179.60" precision="2" nsweeps="5"/>
    <variable name="random" minimum="0.00" maximum="0.20" precision="2" nsweeps="5"/>
    <variable name="boot-time" minimum="0.0" maximum="3.0" precision="1" nsweeps="5"/>
</optimize>
```

* A template file as *template1.js*:

```
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
```

* produces simulator input files to reproduce the experimental data file
*27-48.txt* as:

```json
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
```
