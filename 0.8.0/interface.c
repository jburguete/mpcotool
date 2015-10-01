/*
Calibrator: a software to make calibrations of empirical parameters.

AUTHORS: Javier Burguete and Borja Latorre.

Copyright 2012-2014, AUTHORS.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice,
		this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY AUTHORS ``AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
SHALL AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

/**
 * \file interface.c
 * \brief Source file of the interface.
 * \authors Javier Burguete.
 * \copyright Copyright 2012-2015, all rights reserved.
 */
#define _GNU_SOURCE
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <libxml/parser.h>

/**
 * \struct Input
 * \brief Struct to define the calibration input file.
 */
typedef struct
{
    /**
     * \var simulator
     * \brief Name of the simulator program.
     * \var evaluator
     * \brief Name of the program to evaluate the objective function.
     * \var template
     * \brief Matrix of template names of input files.
     * \var experiment
     * \brief Array of experimental data file names.
     * \var label
     * \brief Array of variable names.
     * \var format
     * \brief Array of variable formats.
     * \var nvariables
     * \brief Variables number.
     * \var nexperiments
     * \brief Experiments number.
     * \var ninputs
     * \brief Number of input files to the simulator.
     * \var nsimulations
     * \brief Simulations number per experiment.
     * \var algorithm
     * \brief Algorithm type.
     * \var nsweeps
     * \brief Array of sweeps of the sweep algorithm.
     * \var niterations
     * \brief Number of algorithm iterations
     * \var nbest
     * \brief Number of best simulations.
     * \var rangemin
     * \brief Array of minimum variable values.
     * \var rangemax
     * \brief Array of maximum variable values.
     * \var rangeminabs
     * \brief Array of absolute minimum variable values.
     * \var rangemaxabs
     * \brief Array of absolute maximum variable values.
     * \var weight
     * \brief Array of the experiment weights.
     * \var tolerance
     * \brief Algorithm tolerance.
     * \var mutation_ratio
     * \brief Mutation probability.
     * \var reproduction_ratio
     * \brief Reproduction probability.
     * \var adaptation_ratio
     * \brief Adaptation probability.
     */
    char *simulator, *evaluator, **experiment, **template[MAX_NINPUTS], **label,
    **format;
    unsigned int nvariables, nexperiments, ninputs, nsimulations, algorithm,
    *nsweeps, niterations, nbest;
    double *rangemin, *rangemax, *rangeminabs, *rangemaxabs, *weight, tolerance,
		  mutation_ratio, reproduction_ratio, adaptation_ratio;
} Input;

/**
 * \struct Window
 * \brief Struct to define the main window.
 */
typedef struct
{
	/**
	 * \var button_save
	 * \brief Save GtkButton.
	 * \var window
	 * \brief Main GtkWindow.
	 */
	GtkButton *button_save;
	GtkWindow *window;
} Window;

Input input[1];
Window window[1];

/**
 * \fn void input_save()
 * \brief Function to save the input file.
 */
void input_save()
{
	char *buffer;
	xmlDoc *doc;
	xmlNode *node, *child;
	doc = xmlNewDoc((const xmlChar*)"1.0");
	node = xmlNewDocNode(doc, 0, XML_CALIBRATE, 0);
	xmlDocSetRootElement(doc, node);
	xmlSaveFormatFile(buffer, doc, 1);
	xmlFreeDoc(doc);
}

/**
 * \fn int window_new()
 * \brief Function to open the main window.
 * \return 1 on success, 0 on error.
 */
int window_new()
{
	window->window = (GtkWindow*)gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(window->window, PROGRAM_INTERFACE);
	return 1;
}

/**
 * \fn int main(int argn, char **argc)
 * \brief Main function.
 * \param argn
 * \brief Arguments number.
 * \param argc
 * \brief Arguments pointer.
 * \return 0 on success, >0 on error.
 */
int main(int argn, char **argc)
{
	xmlKeepBlanksDefault(0);
	setlocale(LC_ALL, "");
	setlocale(LC_NUMERIC, "C");
	buffer = g_get_current_dir();
	bindtextdomain
		(PROGRAM_INTERFACE, g_build_filename(buffer, LOCALE_DIR, NULL));
	bind_textdomain_codeset(PROGRAM_INTERFACE, "UTF-8");
	textdomain(PROGRAM_INTERFACE);
}
