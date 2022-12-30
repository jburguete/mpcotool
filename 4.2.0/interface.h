/*
MPCOTool:
The Multi-Purposes Calibration and Optimization Tool. A software to perform
calibrations or optimizations of empirical parameters.

AUTHORS: Javier Burguete and Borja Latorre.

Copyright 2012-2022, AUTHORS.

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
 * \brief Header file to define the graphical interface functions.
 * \authors Javier Burguete.
 * \copyright Copyright 2012-2022, all rights reserved.
 */
#ifndef INTERFACE__H
#define INTERFACE__H 1

#define MAX_LENGTH (DEFAULT_PRECISION + 8)
  ///< Max length of texts allowed in GtkSpinButtons.

/**
 * \struct Options
 * \brief Struct to define the options dialog.
 */
typedef struct
{
  GtkDialog *dialog;            ///< Main GtkDialog.
  GtkGrid *grid;                ///< Main GtkGrid.
  GtkLabel *label_seed;
  ///< Pseudo-random numbers generator seed GtkLabel.
  GtkSpinButton *spin_seed;
  ///< Pseudo-random numbers generator seed GtkSpinButton.
  GtkLabel *label_threads;      ///< Threads number GtkLabel.
  GtkSpinButton *spin_threads;  ///< Threads number GtkSpinButton.
  GtkLabel *label_climbing;     ///< Climbing threads number GtkLabel.
  GtkSpinButton *spin_climbing; ///< Climbing threads number GtkSpinButton.
} Options;

/**
 * \struct Running
 * \brief Struct to define the running dialog.
 */
typedef struct
{
  GtkDialog *dialog;            ///< Main GtkDialog.
  GtkLabel *label;              ///< Label GtkLabel.
  GtkSpinner *spinner;          ///< Animation GtkSpinner.
  GtkGrid *grid;                ///< Grid GtkGrid.
} Running;

/**
 * \struct Window
 * \brief Struct to define the main window.
 */
typedef struct
{
  GtkWindow *window;            ///< Main GtkWindow.
  GtkGrid *grid;                ///< Main GtkGrid.
  GtkBox *box_buttons;          ///< GtkBox to store the main buttons.
  GtkButton *button_open;       ///< Open GtkButton.
  GtkButton *button_save;       ///< Save GtkButton.
  GtkButton *button_run;        ///< Run GtkButton.
  GtkButton *button_options;    ///< Options GtkButton.
  GtkButton *button_help;       ///< Help GtkButton.
  GtkButton *button_about;      ///< Help GtkButton.
  GtkButton *button_exit;       ///< Exit GtkButton.
  GtkGrid *grid_files;          ///< Files GtkGrid.
  GtkLabel *label_simulator;    ///< Simulator program GtkLabel.
  GtkButton *button_simulator;  ///< Simulator program GtkButton.
  GtkCheckButton *check_evaluator;      ///< Evaluator program GtkCheckButton.
  GtkButton *button_evaluator;   ///< Evaluator program GtkButton.
  GtkLabel *label_result;       ///< Result file GtkLabel.
  GtkEntry *entry_result;       ///< Result file GtkEntry.
  GtkLabel *label_variables;    ///< Variables file GtkLabel.
  GtkEntry *entry_variables;    ///< Variables file GtkEntry.
  GtkFrame *frame_norm;         ///< GtkFrame to set the error norm.
  GtkGrid *grid_norm;           ///< GtkGrid to set the error norm.
#if !GTK4
  GtkRadioButton *button_norm[NNORMS];
  ///< Array of GtkRadioButtons to set the error norm.
#else
  GtkCheckButton *button_norm[NNORMS];
  ///< Array of GtkCheckButtons to set the error norm.
#endif
  GtkLabel *label_p;            ///< GtkLabel to set the p parameter.
  GtkSpinButton *spin_p;        ///< GtkSpinButton to set the p parameter.
  GtkScrolledWindow *scrolled_p;
  ///< GtkScrolledWindow to set the p parameter.
  GtkFrame *frame_algorithm;    ///< GtkFrame to set the algorithm.
  GtkGrid *grid_algorithm;      ///< GtkGrid to set the algorithm.
#if !GTK4
  GtkRadioButton *button_algorithm[NALGORITHMS];
  ///< Array of GtkRadioButtons to set the algorithm.
#else
  GtkCheckButton *button_algorithm[NALGORITHMS];
  ///< Array of GtkCheckButtons to set the algorithm.
#endif
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
  GtkCheckButton *check_climbing;
  ///< GtkCheckButton to check running the hill climbing method.
  GtkGrid *grid_climbing;
  ///< GtkGrid to pack the hill climbing method widgets.
#if !GTK4
  GtkRadioButton *button_climbing[NCLIMBINGS];
  ///< Array of GtkRadioButtons array to set the hill climbing method.
#else
  GtkCheckButton *button_climbing[NCLIMBINGS];
  ///< Array of GtkCheckButtons array to set the hill climbing method.
#endif
  GtkLabel *label_steps;        ///< GtkLabel to set the steps number.
  GtkSpinButton *spin_steps;    ///< GtkSpinButton to set the steps number.
  GtkLabel *label_estimates;    ///< GtkLabel to set the estimates number.
  GtkSpinButton *spin_estimates;
  ///< GtkSpinButton to set the estimates number.
  GtkLabel *label_relaxation;
  ///< GtkLabel to set the relaxation parameter.
  GtkSpinButton *spin_relaxation;
  ///< GtkSpinButton to set the relaxation parameter.
  GtkLabel *label_threshold;    ///< GtkLabel to set the threshold.
  GtkSpinButton *spin_threshold;        ///< GtkSpinButton to set the threshold.
  GtkScrolledWindow *scrolled_threshold;
  ///< GtkScrolledWindow to set the threshold.
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
  GtkLabel *label_step;         ///< GtkLabel to set the step.
  GtkSpinButton *spin_step;     ///< GtkSpinButton to set the step.
  GtkScrolledWindow *scrolled_step;     ///< step GtkScrolledWindow.
  GtkFrame *frame_experiment;   ///< Experiment GtkFrame.
  GtkGrid *grid_experiment;     ///< Experiment GtkGrid.
  GtkComboBoxText *combo_experiment;    ///< Experiment GtkComboBoxEntry.
  GtkButton *button_add_experiment;     ///< GtkButton to add a experiment.
  GtkButton *button_remove_experiment;  ///< GtkButton to remove a experiment.
  GtkLabel *label_experiment;   ///< Experiment GtkLabel.
  GtkButton *button_experiment;
  ///< GtkButton to set the experimental data file.
  GtkLabel *label_weight;       ///< Weight GtkLabel.
  GtkSpinButton *spin_weight;   ///< Weight GtkSpinButton.
  GtkCheckButton *check_template[MAX_NINPUTS];
  ///< Array of GtkCheckButtons to set the input templates.
  GtkButton *button_template[MAX_NINPUTS];
  ///< Array of GtkButtons to set the input templates.
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

// Global variables
extern Window window[1];

// Public functions
void window_new (GtkApplication * application);

#endif
