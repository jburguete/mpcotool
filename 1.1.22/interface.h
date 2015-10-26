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
 * \def MAX_LENGTH
 * \brief Max length of texts allowed in GtkSpinButtons.
 */
#define MAX_LENGTH (DEFAULT_PRECISION + 8)

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
	 * \var rangemin
	 * \brief Minimum value.
	 * \var rangemax
	 * \brief Maximum value.
	 * \var rangeminabs
	 * \brief Minimum allowed value.
	 * \var rangemaxabs
	 * \brief Maximum allowed value.
	 * \var precision
	 * \brief Precision digits.
	 * \var nsweeps
	 * \brief Sweeps number of the sweep algorithm.
	 * \var nbits
	 * \brief Bits number of the genetic algorithm.
	 */
  char *label;
  double rangemin, rangemax, rangeminabs, rangemaxabs;
  unsigned int precision, nsweeps, nbits;
} Variable;

/**
 * \struct Options
 * \brief Struct to define the options dialog.
 */
typedef struct
{
    /**
	 * \var label_processors
	 * \brief Processors number GtkLabel.
	 * \var entry_processors
	 * \brief Processors number GtkSpinButton.
	 * \var grid
	 * \brief main GtkGrid.
	 * \var dialog
	 * \brief main GtkDialog.
	 */
  GtkLabel *label_processors;
  GtkSpinButton *entry_processors;
  GtkGrid *grid;
  GtkDialog *dialog;
} Options;

/**
 * \struct Running
 * \brief Struct to define the running dialog.
 */
typedef struct
{
    /**
	 * \var label
	 * \brief GtkLabel.
	 * \var dialog
	 * \brief main GtkDialog.
	 */
  GtkLabel *label;
  GtkDialog *dialog;
} Running;

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
	 * \var button_run
	 * \brief Run GtkButton.
	 * \var button_options
	 * \brief Options GtkButton.
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
     * \var check_evaluator
     * \brief Evaluator program GtkCheckButton.
     * \var check_minabs
     * \brief Absolute minimum GtkCheckButton.
     * \var check_maxabs
     * \brief Absolute maximum GtkCheckButton.
     * \var check_template
     * \brief Array of GtkCheckButtons to set the input templates.
     * \var label_simulator
     * \brief Simulator program GtkLabel.
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
     * \var label_precision
     * \brief Precision GtkLabel.
     * \var label_sweeps
     * \brief Sweeps number GtkLabel.
     * \var label_bits
     * \brief Bits number GtkLabel.
     * \var label_experiment
     * \brief Experiment GtkLabel.
     * \var label_weight
     * \brief Weight GtkLabel.
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
     * \var entry_min
     * \brief Minimum GtkSpinButton.
     * \var entry_max
     * \brief Maximum GtkSpinButton.
     * \var entry_minabs
     * \brief Absolute minimum GtkSpinButton.
     * \var entry_maxabs
     * \brief Absolute maximum GtkSpinButton.
	 * \var entry_precision
	 * \brief Precision digits GtkSpinButton.
     * \var entry_sweeps
     * \brief Sweeps number GtkSpinButton.
     * \var entry_bits
     * \brief Bits number GtkSpinButton.
     * \var entry_weight
     * \brief Weight GtkSpinButton.
     * \var bar_buttons
     * \brief GtkToolbar to store the main buttons.
     * \var grid
     * \brief Main GtkGrid.
     * \var grid_algorithm
     * \brief GtkGrid to set the algorithm.
     * \var grid_variable
     * \brief Variable GtkGrid.
     * \var grid_experiment
     * \brief Experiment GtkGrid.
     * \var frame_algorithm
     * \brief GtkFrame to set the algorithm.
     * \var frame_variable
     * \brief Variable GtkFrame.
     * \var frame_experiment
     * \brief Experiment GtkFrame.
     * \var logo
     * \brief Logo GdkPixbuf.
     * \var scrolled_min
     * \brief Minimum GtkScrolledWindow.
     * \var scrolled_max
     * \brief Maximum GtkScrolledWindow.
     * \var scrolled_minabs
     * \brief Absolute minimum GtkScrolledWindow.
     * \var scrolled_maxabs
     * \brief Absolute maximum GtkScrolledWindow.
     * \var window
     * \brief Main GtkWindow.
	 * \var application
	 * \brief Main GtkApplication.
	 * \var experiment
	 * \brief Array of experiments data.
	 * \var variable
	 * \brief Array of variables data.
	 * \var id_experiment
	 * \brief Identifier (gulong) of the combo_experiment signal.
	 * \var id_experiment_name
	 * \brief Identifier (gulong) of the button_experiment signal.
	 * \var id_variable
	 * \brief Identifier (gulong) of the combo_variable signal.
	 * \var id_variable_label
	 * \brief Identifier (gulong) of the entry_variable signal.
	 * \var id_template
	 * \brief Array of identifiers (gulong) of the check_template signal.
	 * \var id_input
	 * \brief Array of identifiers (gulong) of the button_template signal.
	 * \var nexperiments
	 * \brief Number of experiments.
	 * \var nvariables
	 * \brief Number of variables.
     */
  GtkToolButton *button_open, *button_save, *button_run, *button_options,
    *button_help, *button_exit;
  GtkButton *button_add_variable, *button_remove_variable,
    *button_add_experiment, *button_remove_experiment;
  GtkRadioButton *button_algorithm[NALGORITHMS];
  GtkCheckButton *check_evaluator, *check_minabs, *check_maxabs,
    *check_template[MAX_NINPUTS];
  GtkLabel *label_simulator, *label_simulations, *label_iterations,
    *label_tolerance, *label_bests, *label_population, *label_generations,
    *label_mutation, *label_reproduction, *label_adaptation, *label_variable,
    *label_min, *label_max, *label_precision, *label_sweeps, *label_bits,
    *label_experiment, *label_weight;
  GtkEntry *entry_variable;
  GtkComboBoxText *combo_variable, *combo_experiment;
  GtkFileChooserButton *button_simulator, *button_evaluator, *button_experiment,
    *button_template[MAX_NINPUTS];
  GtkSpinButton *entry_min, *entry_max, *entry_minabs, *entry_maxabs,
    *entry_simulations, *entry_iterations, *entry_tolerance, *entry_bests,
    *entry_population, *entry_generations, *entry_mutation, *entry_reproduction,
    *entry_adaptation, *entry_precision, *entry_sweeps, *entry_bits,
    *entry_weight;
  GtkToolbar *bar_buttons;
  GtkGrid *grid, *grid_algorithm, *grid_variable, *grid_experiment;
  GtkFrame *frame_algorithm, *frame_variable, *frame_experiment;
  GdkPixbuf *logo;
  GtkScrolledWindow *scrolled_min, *scrolled_max, *scrolled_minabs,
    *scrolled_maxabs;
  GtkWindow *window;
  GtkApplication *application;
  Experiment *experiment;
  Variable *variable;
  gulong id_experiment, id_experiment_name, id_variable, id_variable_label,
    id_template[MAX_NINPUTS], id_input[MAX_NINPUTS];
  unsigned int nexperiments, nvariables;
} Window;

// Public functions
void input_save (char *filename);
void options_new ();
void running_new ();
void window_save ();
void window_run ();
void window_help ();
int window_get_algorithm ();
void window_update ();
void window_set_algorithm ();
void window_set_experiment ();
void window_remove_experiment ();
void window_add_experiment ();
void window_name_experiment ();
void window_weight_experiment ();
void window_inputs_experiment ();
void window_template_experiment (void *data);
void window_set_variable ();
void window_remove_variable ();
void window_add_variable ();
void window_label_variable ();
void window_precision_variable ();
void window_rangemin_variable ();
void window_rangemax_variable ();
void window_rangeminabs_variable ();
void window_rangemaxabs_variable ();
void window_update_variable ();
int window_read (char *filename);
void window_open ();
void window_new ();
int cores_number ();

#endif
