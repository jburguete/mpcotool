MPCOTool
========

:gb:[english](README.md) :es:[español](README.es.md)
:fr:[français](README.fr.md)

The Multi-Purposes Calibration and Optimization Tool. Un programa para realizar
calibraciones u optimizaciones de parámetros empíricos.

VERSIONES
---------

* 4.12.1: versión estable y recomendada.

AUTORES
-------

* Javier Burguete Tolosa (jburguete@eead.csic.es)
* Borja Latorre Garcés (borja.latorre@csic.es)

FICHEROS EJECUTABLES PARA WINDOWS
---------------------------------

Este repositorio contiene los ficheros de código fuente y de ejemplos con la
última versión de MPCOTool. Versiones estables con ficheros ejecutables y
manuales para sistemas Microsoft Windows pueden descargarse de: 
[digital.csic](http://digital.csic.es/handle/10261/148471)

UTILIDADES Y BIBLIOTECAS REQUERIDAS PARA CONSTRUIR EL EJECUTABLE
----------------------------------------------------------------

* [gcc](https://gcc.gnu.org) o [clang](http://clang.llvm.org) (para compilar el
  código fuente)
* [make](http://www.gnu.org/software/make) (para construir el fichero
  ejecutable)
* [autoconf](http://www.gnu.org/software/autoconf) (para generar el Makefile en
  diferentes sistemas operativos)
* [automake](http://www.gnu.org/software/automake) (para detectar el sistema
  operativo)
* [pkg-config](http://www.freedesktop.org/wiki/Software/pkg-config) (para
  encontrar las librerías para compilar)
* [gsl](http://www.gnu.org/software/gsl) (para generar números aleatorios)
* [libxml](http://xmlsoft.org) (para trabajar con ficheros XML)
* [glib](https://developer.gnome.org/glib) (utilidades extendidas de C para
  trabajar con datos, listas, ficheros mapeados, expresiones regulares, usar
  múltiples núcles en máquinas de memoria compartida, ...)
* [json-glib](https://github.com/ebassi/json-glib) (para trabajar con ficheros
  JSON)
* [genetic](https://github.com/jburguete/genetic) (algoritmos genéticos)

UTILIDADES Y BIBLIOTECAS OPCIONALES
-----------------------------------

* [gettext](http://www.gnu.org/software/gettext) (para trabajar con diferentes
  locales)
* [gtk](http://www.gtk.org) (para crear la utilidad GUI interactiva)
* [openmpi](http://www.open-mpi.org) or [mpich](http://www.mpich.org) (para
  correr en tareas paralelizadas en múltiples computadoras)
* [doxygen](http://www.stack.nl/~dimitri/doxygen) (formato de comentarios
  estándar para generar documentación)
* [latex](https://www.latex-project.org/) (para construir los ficheros PDF)

FICHEROS
--------

El código fuente consta de los siguientes ficheros:
* 4.12.1/configure.ac: generador del script configure.
* 4.12.1/Makefile.in: generador del fichero Makefile.
* 4.12.1/config.h.in: generador del fichero cabecera de configuración.
* 4.12.1/\*.c: ficheros fuente de C.
* 4.12.1/\*.h: ficheros cabecera de C.
* 4.12.1/mpcotool.ico: fichero de icono.
* 4.12.1/build.sh: script de shell para construir todo.
* 4.12.1/logo.png: figura de logo.
* 4.12.1/Doxyfile: fichero de configuración para generar documentación doxygen.
* TODO: tareas a realizar.
* README\*.md: ficheros LÉAME.
* license.md: fichero de licencia.
* tests/testX/\*: algunos ficheros para probar cómo funciona el programa.
* locales/\*/LC\_MESSAGES/mpcotool.po: ficheros de traducción.
* manuals/\*.eps: figuras del manual en formato EPS.
* manuals/\*.png: figuras del manual en formato PNG.
* manuals/\*.tex: ficheros fuente de la documentación.
* applications/\*/\*: algunos casos prácticos de aplicación.
* check\_errors/\*.xml: algunos ficheros erróneos para probar el manejo de
  errores.

BUILDING INSTRUCTIONS
---------------------

En sistemas Microsoft Windows se debe instalar 
[MSYS2](http://sourceforge.net/projects/msys2), y las librerías y utilidades
requeridas. Puede seguir instrucciones detalladas en
[install-unix](https://github.com/jburguete/install-unix/blob/master/tutorial.pdf).
Un paquete opcional de binarios Windows se puede construir ejecutando en la
terminal:
> $ make windist

En NetBSD 10.1, para compilar con la última versión de GCC, debe hacer primero
en la terminal:
> $ export PATH=/usr/pkg/gcc8/bin:$PATH"

En OpenBSD 7.7 se debe hacer primero en la terminal de compilación para
seleccionar las versiones correctas y para desactivar OpenMPI compilando con
Clang:
> $ export AUTOCONF\_VERSION=2.69 AUTOMAKE\_VERSION=1.16 CC=clang

En OpenIndiana Hipster, para activar la compilación con OpenMPI, ejecute en una
terminal:
> $ export PATH=$PATH:/usr/lib/openmpi/gcc/bin

On OpenSUSE Leap, in order to enable OpenMPI compilation, in 64 bits version do
in a terminal (OpenMPI configure script does not work in last OpenSUSE versions
then does not apply this step):
> $ export PATH=$PATH:/usr/lib64/mpi/gcc/openmpi/bin

This software has been built and tested in the following operative systems:
* Arch Linux
* Debian Linux 12
* Devuan Linux 5
* DragonFly BSD 6.4
* Fedora Linux 42
* FreeBSD 14.2
* Gentoo Linux
* Linux Mint DE 6
* MacOS Monterey + Homebrew
* Manjaro Linux
* Microsoft Windows 11 + MSYS2
* NetBSD 10.1
* OpenBSD 7.7
* OpenInidiana Hipster
* OpenSUSE Linux Leap 15.5
* Ubuntu Linux 25.04

Probably, it can be built in other systems, distributions, or versions but it
has not been tested.

1. Download the latest [genetic](https://github.com/jburguete/genetic) doing on
a terminal:
> $ git clone https://github.com/jburguete/genetic.git

2. Build the genetic library:
> $ cd genetic/3.0.1
>
> $ sh build.sh

3. Download this repository:
> $ cd ../..
>
> $ git clone https://github.com/jburguete/mpcotool.git

4. Link the latest genetic version to genetic:
> $ cd mpcotool/4.12.1
>
> $ ln -s ../../genetic/3.0.1 genetic
> 
> $ ln -s genetic/libgenetic.so (or .dll in Windows systems)

5. Build doing on a terminal:
> $ sh build.sh

Building no-GUI version on servers
__________________________________

On servers or clusters, where no-GUI with MPI parallelization is desirable,
replace the 5th step of the previous section by:
> $ sh build\_without\_gui.sh
 
Linking as an external library
______________________________

MPCOTool can also be used as an external library:

1. First copy the dynamic libraries (libmpcotool.so and libgenetic.so on Unix 
  systems or libmpcotool.dll and libgenetic.dll on Windows systems) to your 
  program directory.

2. Include the function header in your source code:
> extern int mpcotool (int argn, char \*\*argc);

3. Build the executable file with the linker and compiler flags:
> $ gcc -L. -Wl,-rpath=. -lmpcotool -lgenetic ...
> \`pkg-config --cflags --libs gsl glib-2.0 json-glib-1.0 ...\'

4. Calling to this function is equivalent to command line order (see next 
  chapter USER INSTRUCTIONS):
  * argn: number of arguments
  * argc[0]: "mpcotool"
  * argc[1]: first command line argument.

  ...

  * argc[argn-1]: last argument.

FINAL VERSIONS
______________

Optionally, final compact versions without debug information can be built doing
on the terminal:
> $ make strip

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
you are in the directory mpcotool/4.12.1):
> $ cd ../tests/test2
>
> $ ln -s ../../../genetic/3.0.1 genetic
>
> $ ln -s genetic/libgenetic.so (or .dll on Windows systems)
>
> $ cd ../test3
>
> $ ln -s ../../../genetic/3.0.1 genetic
>
> $ ln -s genetic/libgenetic.so (or .dll on Windows systems)
>
> $ cd ../test4
>
> $ ln -s ../../../genetic/3.0.1 genetic
>
> $ ln -s genetic/libgenetic.so (or .dll on Windows systems)

2. Build all tests doing in the same terminal:
> $ cd ../../4.12.1
>
> $ make tests

USER INSTRUCTIONS
-----------------

Optional arguments are typed in square brackets.

* Command line in sequential mode (where X is the number of threads to execute
  and S is a seed for the pseudo-random numbers generator):
> $ ./mpcotoolbin [-nthreads X] [-seed S] input\_file.xml [result\_file]
> [variables\_file]

* Command line in parallelized mode (where X is the number of threads to
  open for every node and S is a seed for the pseudo-random numbers generator):
> $ mpirun [MPI options] ./mpcotoolbin [-nthreads X] [-seed S] input\_file.xml
> [result\_file] [variables\_file]

* The syntax of the simulator has to be:
> $ ./simulator\_name input\_file\_1 [input\_file\_2] [input\_file\_3] [input\_file\_4]
> output\_file

* The syntax of the program to evaluate the objetive function has to be (where
  the first data in the results file has to be the objective function value):
> $ ./evaluator\_name simulated\_file data\_file results\_file

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
    <variable name="variable_1" minimum="min_value" maximum="max_value" precision="precision_digits" nsweeps="sweeps_number" nbits="bits_number" step="step_size"/>
    ...
    <variable name="variable_M" minimum="min_value" maximum="max_value" precision="precision_digits" nsweeps="sweeps_number" nbits="bits_number" step="step_size"/>
</optimize>
```

with:

* **simulator**: simulator executable file name.
* **evaluator**: optional. When needed is the evaluator executable file name.
* **seed**: optional. Seed of the pseudo-random numbers generator (default value
  is 7007).
* **result\_file**: optional. It is the name of the optime result file (default 
  name is "result").
* **variables\_file**: optional. It is the name of all simulated variables file
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
  * *nsweeps*: number of sweeps to generate for each variable in every
    experiment. 

    The total number of simulations to run is:
> (number of experiments) x (variable 1 number of sweeps) x ... x
> (variable n number of sweeps) x (number of iterations)

* **Monte-Carlo**: Monte-Carlo brute force algorithm. It requires on calibrate:
  * *nsimulations*: number of simulations to run in every experiment.

    The total number of simulations to run is:
> (number of experiments) x (number of simulations) x (number of iterations)

* **orthogonal**: Orthogonal sampling brute force algorithm. It requires for
  each variable:
  * *nsweeps*: number of sweeps to generate for each variable in every
    experiment. 

    The total number of simulations to run is:
> (number of experiments) x (variable 1 number of sweeps) x ... x
> (variable n number of sweeps) x (number of iterations)

* Three former brute force algorithms can be iterated to improve convergence by
  using the following parameters:
  * *nbest*: number of best simulations to calculate convergence interval on
    next iteration (default 1).
  * *tolerance*: tolerance parameter to increase convergence interval (default
    0).
  * *niterations*: number of iterations (default 1).

     It multiplies the total number of simulations:
> x (number of iterations)

* Moreover, brute force algorithms can be coupled with a direction search
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

  Former methods require also:
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
			"nsweeps": "sweeps_number",
			"nbits": "bits_number",
			"step": "step_size",
		},
		...
		{
			"name": "variable_M",
			"minimum": "min_value",
			"maximum": "max_value",
			"precision": "precision_digits",
			"nsweeps": "sweeps_number",
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

```json
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
