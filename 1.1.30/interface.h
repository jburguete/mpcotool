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

#define MAX_LENGTH (DEFAULT_PRECISION + 8)
  ///< Max length of texts allowed in GtkSpinButtons.

/**
 * \struct Experiment
 * \brief Struct to define experiment data.
 */
typedef struct
{
  char *template[MAX_NINPUTS];  ///< Array of input template names.
  char *name;                   ///< File name.
  double weight;
  ///< Weight to calculate the objective function value.
} Experiment;

/**
 * \struct Variable
 * \brief Struct to define variable data.
 */
typedef struct
{
  char *label;                  ///< Variable label.
  double rangemin;              ///< Minimum value.
  double rangemax;              ///< Maximum value.
  double rangeminabs;           ///< Minimum allowed value.
  double rangemaxabs;           ///< Maximum allowed value.
  unsigned int precision;       ///< Precision digits.
  unsigned int nsweeps;         ///< Sweeps number of the sweep algorithm.
  unsigned int nbits;           ///< Bits number of the genetic algorithm.
} Variable;

/**
 * \struct Options
 * \brief Struct to define the options dialog.
 */
typedef struct
{
  GtkDialog *dialog;            ///< Main GtkDialog.
  GtkGrid *grid;                ///< Main GtkGrid.
  GtkLabel *label_processors;   ///< Processors number GtkLabel.
  GtkSpinButton *spin_processors;       ///< Processors number GtkSpinButton.
  GtkLabel *label_seed;
  ///< Pseudo-random numbers generator seed GtkLabel.
  GtkSpinButton *spin_seed;
  ///< Pseudo-random numbers generator seed GtkSpinButton.
} Options;

/**
 * \struct Running
 * \brief Struct to define the running dialog.
 */
typedef struct
{
  GtkDialog *dialog;            ///< Main GtkDialog.
  GtkLabel *label;              ///< Label GtkLabel.
} Running;

/**
 * \struct Window
 * \brief Struct to define the main window.
 */
typedef struct
{
  GtkWindow *window;            ///< Main GtkWindow.
  GtkGrid *grid;                ///< Main GtkGrid.
  GtkToolbar *bar_buttons;      ///< GtkToolbar to store the main buttons.
  GtkToolButton *button_open;   ///< Open GtkToolButton.
  GtkToolButton *button_save;   ///< Save GtkToolButton.
  GtkToolButton *button_run;    ///< Run GtkToolButton.
  GtkToolButton *button_options;        ///< Options GtkToolButton.
  GtkToolButton *button_help;   ///< Help GtkToolButton.
  GtkToolButton *button_about;  ///< Help GtkToolButton.
  GtkToolButton *button_exit;   ///< Exit GtkToolButton.
  GtkLabel *label_simulator;    ///< Simulator program GtkLabel.
  GtkFileChooserButton *button_simulator;
  ///< Simulator program GtkFileChooserButton.
  GtkCheckButton *check_evaluator;      ///< Evaluator program GtkCheckButton.
  GtkFileChooserButton *button_evaluator;
  ///< Evaluator program GtkFileChooserButton.
  GtkFrame *frame_algorithm;    ///< GtkFrame to set the algorithm.
  GtkGrid *grid_algorithm;      ///< GtkGrid to set the algorithm.
  GtkRadioButton *button_algorithm[NALGORITHMS];
  ///< Array of GtkButtons to set the algorithm.
  GtkLabel *label_simulations;  ///< GtkLabel to set the simulations number.
  GtkSpinButton *spin_simulations;
  ///< GtkSpinButton to set the simulations number.
  GtkLabel *label_iterations;   ///< GtkLabel to set the iterations number.
  GtkSpinButton *spin_iterations;
  ///< GtkSpinButton to set the iterations number.
  GtkLabel *label_tolerance;    ///< GtkLabel to set the tolerance.
  GtkSpinButton *spin_tolerance;        ///< GtkSpinButton to set the tolerance.
  GtkLabel *label_bests;        ///< GtkLabel to set the best number.
  GtkSpinButton *spin_bests;    ///< GtkSpinButton to set the best number.
  GtkLabel *label_population;   ///< GtkLabel to set the population number.
  GtkSpinButton *spin_population;
  ///< GtkSpinButton to set the population number.
  GtkLabel *label_generations;  ///< GtkLabel to set the generations number.
  GtkSpinButton *spin_generations;
  ///< GtkSpinButton to set the generations number.
  GtkLabel *label_mutation;     ///< GtkLabel to set the mutation ratio.
  GtkSpinButton *spin_mutation; ///< GtkSpinButton to set the mutation ratio.
  GtkLabel *label_reproduction; ///< GtkLabel to set the reproduction ratio.
  GtkSpinButton *spin_reproduction;
  ///< GtkSpinButton to set the reproduction ratio.
  GtkLabel *label_adaptation;   ///< GtkLabel to set the adaptation ratio.
  GtkSpinButton *spin_adaptation;
  ///< GtkSpinButton to set the adaptation ratio.
  GtkFrame *frame_variable;     ///< Variable GtkFrame.
  GtkGrid *grid_variable;       ///< Variable GtkGrid.
  GtkComboBoxText *combo_variable;
  ///< GtkComboBoxEntry to select a variable.
  GtkButton *button_add_variable;       ///< GtkButton to add a variable.
  GtkButton *button_remove_variable;    ///< GtkButton to remove a variable.
  GtkLabel *label_variable;     ///< Variable GtkLabel.
  GtkEntry *entry_variable;     ///< GtkEntry to set the variable name.
  GtkLabel *label_min;          ///< Minimum GtkLabel.
  GtkSpinButton *spin_min;      ///< Minimum GtkSpinButton.
  GtkScrolledWindow *scrolled_min;      ///< Minimum GtkScrolledWindow.
  GtkLabel *label_max;          ///< Maximum GtkLabel.
  GtkSpinButton *spin_max;      ///< Maximum GtkSpinButton.
  GtkScrolledWindow *scrolled_max;      ///< Maximum GtkScrolledWindow.
  GtkCheckButton *check_minabs; ///< Absolute minimum GtkCheckButton.
  GtkSpinButton *spin_minabs;   ///< Absolute minimum GtkSpinButton.
  GtkScrolledWindow *scrolled_minabs;   ///< Absolute minimum GtkScrolledWindow.
  GtkCheckButton *check_maxabs; ///< Absolute maximum GtkCheckButton.
  GtkSpinButton *spin_maxabs;   ///< Absolute maximum GtkSpinButton.
  GtkScrolledWindow *scrolled_maxabs;   ///< Absolute maximum GtkScrolledWindow.
  GtkLabel *label_precision;    ///< Precision GtkLabel.
  GtkSpinButton *spin_precision;        ///< Precision digits GtkSpinButton.
  GtkLabel *label_sweeps;       ///< Sweeps number GtkLabel.
  GtkSpinButton *spin_sweeps;   ///< Sweeps number GtkSpinButton.
  GtkLabel *label_bits;         ///< Bits number GtkLabel.
  GtkSpinButton *spin_bits;     ///< Bits number GtkSpinButton.
  GtkFrame *frame_experiment;   ///< Experiment GtkFrame.
  GtkGrid *grid_experiment;     ///< Experiment GtkGrid.
  GtkComboBoxText *combo_experiment;    ///< Experiment GtkComboBoxEntry.
  GtkButton *button_add_experiment;     ///< GtkButton to add a experiment.
  GtkButton *button_remove_experiment;  ///< GtkButton to remove a experiment.
  GtkLabel *label_experiment;   ///< Experiment GtkLabel.
  GtkFileChooserButton *button_experiment;
  ///< GtkFileChooserButton to set the experimental data file.
  GtkLabel *label_weight;       ///< Weight GtkLabel.
  GtkSpinButton *spin_weight;   ///< Weight GtkSpinButton.
  GtkCheckButton *check_template[MAX_NINPUTS];
  ///< Array of GtkCheckButtons to set the input templates.
  GtkFileChooserButton *button_template[MAX_NINPUTS];
  ///< Array of GtkFileChooserButtons to set the input templates.
  GdkPixbuf *logo;              ///< Logo GdkPixbuf.
  Experiment *experiment;       ///< Array of experiments data.
  Variable *variable;           ///< Array of variables data.
  char *application_directory;  ///< Application directory.
  gulong id_experiment;         ///< Identifier of the combo_experiment signal.
  gulong id_experiment_name;    ///< Identifier of the button_experiment signal.
  gulong id_variable;           ///< Identifier of the combo_variable signal.
  gulong id_variable_label;     ///< Identifier of the entry_variable signal.
  gulong id_template[MAX_NINPUTS];
  ///< Array of identifiers of the check_template signal.
  gulong id_input[MAX_NINPUTS];
  ///< Array of identifiers of the button_template signal.
  unsigned int nexperiments;    ///< Number of experiments.
  unsigned int nvariables;      ///< Number of variables.
} Window;

// Public functions
void input_save (char *filename);
void options_new ();
void running_new ();
int window_save ();
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
