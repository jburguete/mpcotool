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
 * \struct Experiment
 * \brief Struct to define experiment data.
 */
typedef struct
{
    /**
	 * \var name
	 * \brief File name.
	 * \var template
	 * \brief Array of input template names.
	 * \var weight
	 * \brief Weight to calculate the objective function value.
	 */
  char *name, *template[MAX_NINPUTS];
  double weight;
} Experiment;

/**
 * \struct Variable
 * \brief Struct to define variable data.
 */
typedef struct
{
    /**
	 * \var label
	 * \brief Variable label.
	 * \var format
	 * \brief C-string format.
	 * \var rangemin
	 * \brief Minimum value.
	 * \var rangemax
	 * \brief Maximum value.
	 * \var rangeminabs
	 * \brief Minimum allowed value.
	 * \var rangemaxabs
	 * \brief Maximum allowed value.
	 * \var nsweeps
	 * \brief Sweeps number of the sweep algorithm.
	 * \var nbits
	 * \brief Bits number of the genetic algorithm.
	 */
  char *label, *format;
  double rangemin, rangemax, rangeminabs, rangemaxabs;
  unsigned int nsweeps, nbits;
} Variable;

/**
 * \struct Window
 * \brief Struct to define the main window.
 */
typedef struct
{
    /**
     * \var button_open
     * \brief Open GtkButton.
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
     * \var label_bits
     * \brief Bits number GtkLabel.
     * \var label_experiment
     * \brief Experiment GtkLabel.
     * \var label_weight
     * \brief Weight GtkLabel.
     * \var entry_min
     * \brief Minimum GtkEntry.
     * \var entry_max
     * \brief Maximum GtkEntry.
     * \var entry_absmin
     * \brief Absolute minimum GtkEntry.
     * \var entry_absmax
     * \brief Absolute maximum GtkEntry.
     * \var entry_variable
     * \brief GtkEntry to set the variable name.
     * \var combo_variable
     * \brief Variable GtkComboBoxEntry.
     * \var combo_experiment
     * \brief Experiment GtkComboBoxEntry.
     * \var button_simulator
     * \brief Simulator program GtkFileChooserButton.
     * \var button_evaluator
     * \brief Evaluator program GtkFileChooserButton.
     * \var button_experiment
     * \brief GtkFileChooserButton to set the experimental data file.
     * \var button_template
     * \brief Array of GtkFileChooserButtons to set the input templates.
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
     * \var entry_sweeps
     * \brief Sweeps number GtkSpinButton.
     * \var entry_bits
     * \brief Bits number GtkSpinButton.
     * \var entry_weight
     * \brief Weight GtkSpinButton.
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
	 * \var experiment
	 * \brief Array of experiments data.
	 * \var variable
	 * \brief Array of variables data.
	 * \var nexperiments
	 * \brief Number of experiments.
	 * \var nvariables
	 * \brief Number of variables.
     */
  GtkButton *button_open, *button_save, *button_help, *button_exit,
    *button_add_variable, *button_remove_variable, *button_add_experiment,
    *button_remove_experiment;
  GtkRadioButton *button_algorithm[NALGORITHMS];
  GtkLabel *label_simulator, *label_evaluator, *label_simulations,
    *label_iterations, *label_tolerance, *label_bests, *label_population,
    *label_generations, *label_mutation, *label_reproduction, *label_adaptation,
    *label_variable, *label_min, *label_max, *label_absmin, *label_absmax,
    *label_sweeps, *label_bits, *label_experiment, *label_weight;
  GtkEntry *entry_min, *entry_max, *entry_absmin, *entry_absmax,
    *entry_variable;
  GtkComboBoxText *combo_variable, *combo_experiment;
  GtkFileChooserButton *button_simulator, *button_evaluator, *button_experiment,
    *button_template[MAX_NINPUTS];
  GtkSpinButton *entry_simulations, *entry_iterations, *entry_tolerance,
    *entry_bests, *entry_population, *entry_generations, *entry_mutation,
    *entry_reproduction, *entry_adaptation, *entry_sweeps, *entry_bits,
    *entry_weight;
  GtkGrid *grid, *grid_algorithm, *grid_variable, *grid_experiment;
  GtkFrame *frame_algorithm, *frame_variable, *frame_experiment;
  GdkPixbuf *logo;
  GtkWindow *window;
  Experiment *experiment;
  Variable *variable;
  unsigned int nexperiments, nvariables;
} Window;

// Public functions
void input_save (char *filename);
void input_save ();
void window_help ();
int window_get_algorithm ();
void window_update ();
void window_open ();
void window_new (GtkApplication * application);

#endif
