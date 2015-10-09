/*
Calibrator: a software to make calibrations of empirical parameters.

AUTHORS: Javier Burguete and Borja Latorre.

Copyright 2012-2015, AUTHORS.

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
 * \file interface.h
 * \brief Header file of the interface.
 * \authors Javier Burguete.
 * \copyright Copyright 2012-2015, all rights reserved.
 */
#ifndef INTERFACE__H
#define INTERFACE__H 1

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
     * \var button_help
     * \brief Help GtkButton.
     * \var button_exit
     * \brief Exit GtkButton.
	 * \var button_add_variable
	 * \brief GtkButton to add a variable.
	 * \var button_remove_variable
	 * \brief GtkButton to remove a variable.
	 * \var button_add_experiment
	 * \brief GtkButton to add a experiment.
	 * \var button_remove_experiment
	 * \brief GtkButton to remove a experiment.
     * \var button_algorithm
     * \brief Array of GtkButtons to set the algorithm.
     * \var label_simulator
     * \brief Simulator program GtkLabel.
     * \var label_evaluator
     * \brief Evaluator program GtkLabel.
     * \var label_simulations
     * \brief GtkLabel to set the simulations number.
     * \var label_iterations
     * \brief GtkLabel to set the iterations number.
     * \var label_tolerance
     * \brief GtkLabel to set the tolerance.
     * \var label_bests
     * \brief GtkLabel to set the best number.
     * \var label_population
     * \brief GtkLabel to set the population number.
     * \var label_generations
     * \brief GtkLabel to set the generations number.
     * \var label_mutation
     * \brief GtkLabel to set the mutation ratio.
     * \var label_reproduction
     * \brief GtkLabel to set the reproduction ratio.
     * \var label_adaptation
     * \brief GtkLabel to set the adaptation ratio.
	 * \var label_variable
	 * \brief Variable GtkLabel.
	 * \var label_min
	 * \brief Minimum GtkLabel.
	 * \var label_max
	 * \brief Maximum GtkLabel.
	 * \var label_absmin
	 * \brief Absolute minimum GtkLabel.
	 * \var label_absmax
	 * \brief Absolute maximum GtkLabel.
	 * \var label_sweeps
	 * \brief Sweeps number GtkLabel.
	 * \var label_experiment
	 * \brief Experiment GtkLabel.
	 * \var entry_min
	 * \brief Minimum GtkEntry.
	 * \var entry_max
	 * \brief Maximum GtkEntry.
	 * \var entry_absmin
	 * \brief Absolute minimum GtkEntry.
	 * \var entry_absmax
	 * \brief Absolute maximum GtkEntry.
	 * \var entry_sweeps
	 * \brief Sweeps number GtkEntry.
	 * \var combo_variable
	 * \brief Variable GtkComboBoxEntry.
	 * \var combo_experiment
	 * \brief Experiment GtkComboBoxEntry.
     * \var button_simulator
     * \brief Simulator program GtkFileChooserButton.
     * \var button_evaluator
     * \brief Evaluator program GtkFileChooserButton.
     * \var entry_simulations
     * \brief GtkSpinButton to set the simulations number.
     * \var entry_iterations
     * \brief GtkSpinButton to set the iterations number.
     * \var entry_tolerance
     * \brief GtkSpinButton to set the tolerance.
     * \var entry_bests
     * \brief GtkSpinButton to set the best number.
     * \var entry_population
     * \brief GtkSpinButton to set the population number.
     * \var entry_generations
     * \brief GtkSpinButton to set the generations number.
     * \var entry_mutation
     * \brief GtkSpinButton to set the mutation ratio.
     * \var entry_reproduction
     * \brief GtkSpinButton to set the reproduction ratio.
     * \var entry_adaptation
     * \brief GtkSpinButton to set the adaptation ratio.
     * \var grid
     * \brief Main GtkGrid.
     * \var grid_algorithm
     * \brief GtkGrid to set the algorithm
	 * \var grid_variable
	 * \brief Variable GtkGrid.
	 * \var grid_experiment
	 * \brief Experiment GtkGrid.
     * \var frame_algorithm
     * \brief GtkFrame to set the algorithm
	 * \var frame_variable
	 * \brief Variable GtkFrame.
	 * \var frame_experiment
	 * \brief Experiment GtkFrame.
     * \var logo
     * \brief Logo GdkPixbuf.
     * \var window
     * \brief Main GtkWindow.
     */
    GtkButton *button_save, *button_help, *button_exit, *button_add_variable,
			  *button_remove_variable, *button_add_experiment, *button_remove_experiment;
    GtkRadioButton *button_algorithm[NALGORITHMS];
    GtkLabel *label_simulator, *label_evaluator, *label_simulations,
    *label_iterations, *label_tolerance, *label_bests,
    *label_population, *label_generations, *label_mutation,
    *label_reproduction, *label_adaptation, *label_variable, *label_min,
	*label_max, *label_absmin, *label_absmax, *label_sweeps, *label_experiment;
	GtkEntry *entry_min, *entry_max, *entry_absmin, *entry_absmax,
			 *entry_sweeps;
	GtkComboBoxText *combo_variable, *combo_experiment;
    GtkFileChooserButton *button_simulator, *button_evaluator;
    GtkSpinButton *entry_simulations, *entry_iterations, *entry_tolerance,
    *entry_bests, *entry_population, *entry_generations,
    *entry_mutation, *entry_reproduction, *entry_adaptation;
    GtkGrid *grid, *grid_algorithm, *grid_variable, *grid_experiment;
    GtkFrame *frame_algorithm, *frame_variable, *frame_experiment;
    GdkPixbuf *logo;
    GtkWindow *window;
} Window;

// Public functions
void show_error(char *msg);
void input_save();
void window_help();
int window_get_algorithm();
void window_update();
void window_new(GtkApplication *application);

#endif
