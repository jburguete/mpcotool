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
#include <locale.h>
#include <libxml/parser.h>
#include <libintl.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "interface.h"

/**
 * \var input
 * \brief Input struct to define the input file to calibrator.
 * \var window
 * \brief Window struct to define the main interface window.
 */
Input input[1];
Window window[1];

/**
 * \fn void show_error(char *msg)
 * \brief Function to show a dialog with an error message.
 * \param msg
 * \brief Error message.
 */
void show_error(char *msg)
{
    GtkMessageDialog *dlg;

    // Creating the dialog
    dlg = (GtkMessageDialog*)gtk_message_dialog_new
          (window->window, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
           "%s", msg);

    // Setting the dialog title
    gtk_window_set_title(GTK_WINDOW(dlg), gettext("ERROR!"));

    // Showing the dialog and waiting response
    gtk_dialog_run(GTK_DIALOG(dlg));

    // Closing and freeing memory
    gtk_widget_destroy(GTK_WIDGET(dlg));
}

/**
 * \fn void input_save()
 * \brief Function to save the input file.
 */
void input_save()
{
    char *buffer;
    xmlDoc *doc;
    xmlNode *node, *child;
    GtkFileChooserDialog *dlg;

    // Opening the saving dialog
    dlg = (GtkFileChooserDialog*)gtk_file_chooser_dialog_new(
              gettext("Save file"),
              window->window,
              GTK_FILE_CHOOSER_ACTION_SAVE,
              gettext("_Cancel"),
              GTK_RESPONSE_CANCEL,
              gettext("_OK"),
              GTK_RESPONSE_OK,
              NULL);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dlg), TRUE);

    // If OK response then saving
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK)
        {
            // Opening the input file
            doc = xmlNewDoc((const xmlChar*)"1.0");

            // Setting root XML node
            node = xmlNewDocNode(doc, 0, XML_CALIBRATE, 0);
            xmlDocSetRootElement(doc, node);

            // Adding properties to the root XML node
            xmlSetProp(node, XML_SIMULATOR,
                       (xmlChar*)gtk_file_chooser_get_filename
                       (GTK_FILE_CHOOSER(window->button_simulator)));
            buffer = gtk_file_chooser_get_filename
                     (GTK_FILE_CHOOSER(window->button_evaluator));
            if (xmlStrlen((xmlChar*)buffer))
                xmlSetProp(node, XML_EVALUATOR, (xmlChar*)buffer);

            // Setting the algorithm
            switch (window_get_algorithm())
                {
                case 0:
            		xmlSetProp(node, XML_NSIMULATIONS, (xmlChar*)gtk_entry_get_text(GTK_ENTRY(window->entry_simulations)));
            		xmlSetProp(node, XML_NITERATIONS, (xmlChar*)gtk_entry_get_text(GTK_ENTRY(window->entry_iterations)));
            		xmlSetProp(node, XML_TOLERANCE, (xmlChar*)gtk_entry_get_text(GTK_ENTRY(window->entry_tolerance)));
            		xmlSetProp(node, XML_NBEST, (xmlChar*)gtk_entry_get_text(GTK_ENTRY(window->entry_bests)));
                    break;
                case 1:
                    xmlSetProp(node, XML_ALGORITHM, XML_SWEEP);
            		xmlSetProp(node, XML_NITERATIONS, (xmlChar*)gtk_entry_get_text(GTK_ENTRY(window->entry_iterations)));
            		xmlSetProp(node, XML_TOLERANCE, (xmlChar*)gtk_entry_get_text(GTK_ENTRY(window->entry_tolerance)));
            		xmlSetProp(node, XML_NBEST, (xmlChar*)gtk_entry_get_text(GTK_ENTRY(window->entry_bests)));
                    break;
                default:
                    xmlSetProp(node, XML_ALGORITHM, XML_GENETIC);
            		xmlSetProp(node, XML_NPOPULATION, (xmlChar*)gtk_entry_get_text(GTK_ENTRY(window->entry_population)));
            		xmlSetProp(node, XML_NGENERATIONS, (xmlChar*)gtk_entry_get_text(GTK_ENTRY(window->entry_generations)));
            		xmlSetProp(node, XML_MUTATION, (xmlChar*)gtk_entry_get_text(GTK_ENTRY(window->entry_mutation)));
            		xmlSetProp(node, XML_REPRODUCTION, (xmlChar*)gtk_entry_get_text(GTK_ENTRY(window->entry_reproduction)));
            		xmlSetProp(node, XML_ADAPTATION, (xmlChar*)gtk_entry_get_text(GTK_ENTRY(window->entry_adaptation)));
                    break;
                }
    switch (window_get_algorithm())
        {
        case 0:
            break;
        case 1:
            gtk_widget_show(GTK_WIDGET(window->label_iterations));
            gtk_widget_show(GTK_WIDGET(window->entry_iterations));
            gtk_widget_show(GTK_WIDGET(window->label_tolerance));
            gtk_widget_show(GTK_WIDGET(window->entry_tolerance));
            gtk_widget_show(GTK_WIDGET(window->label_bests));
            gtk_widget_show(GTK_WIDGET(window->entry_bests));
            break;
        default:
            gtk_widget_show(GTK_WIDGET(window->label_population));
            gtk_widget_show(GTK_WIDGET(window->entry_population));
            gtk_widget_show(GTK_WIDGET(window->label_generations));
            gtk_widget_show(GTK_WIDGET(window->entry_generations));
            gtk_widget_show(GTK_WIDGET(window->label_mutation));
            gtk_widget_show(GTK_WIDGET(window->entry_mutation));
            gtk_widget_show(GTK_WIDGET(window->label_reproduction));
            gtk_widget_show(GTK_WIDGET(window->entry_reproduction));
            gtk_widget_show(GTK_WIDGET(window->label_adaptation));
            gtk_widget_show(GTK_WIDGET(window->entry_adaptation));
        }

            // Saving the XML file
            buffer = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
            xmlSaveFormatFile(buffer, doc, 1);

            // Freeing memory
            g_free(buffer);
            xmlFreeDoc(doc);
        }

    // Closing and freeing memory
    gtk_widget_destroy(GTK_WIDGET(dlg));
}

/**
 * \fn void window_help()
 * \brief Function to show a help dialog.
 */
void window_help()
{
    gchar *authors[] =
    {
        "Javier Burguete Tolosa (jburguete@eead.csic.es)",
        "Borja Latorre Garcés (borja.latorre@csic.es)",
        NULL
    };
    gtk_show_about_dialog(window->window,
                          "program_name",
                          "Calibrator",
                          "comments",
                          gettext("A software to make calibrations of empirical parameters"),
                          "authors",
                          authors,
                          "translator-credits",
                          gettext("Javier Burguete Tolosa (jburguete@eead.csic.es)"),
                          "version",
                          "0.8.0",
                          "copyright",
                          "Copyright 2012-2015 Javier Burguete Tolosa",
                          "logo",
                          window->logo,
                          "website-label",
                          gettext("Website"),
                          "website",
                          "https://github.com/jburguete/calibrator",
                          NULL);
}

/**
 * \fn int window_get_algorithm()
 * \brief Function to get the algorithm number.
 * \return Algorithm number.
 */
int window_get_algorithm()
{
    unsigned int i;
    for (i = 0; i < NALGORITHMS; ++i)
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(window->button_algorithm[i]))) break;
    return i;
}

/**
 * \fn void window_update()
 * \brief Function to update the main window view.
 */
void window_update()
{
    gtk_widget_hide(GTK_WIDGET(window->label_simulations));
    gtk_widget_hide(GTK_WIDGET(window->entry_simulations));
    gtk_widget_hide(GTK_WIDGET(window->label_iterations));
    gtk_widget_hide(GTK_WIDGET(window->entry_iterations));
    gtk_widget_hide(GTK_WIDGET(window->label_tolerance));
    gtk_widget_hide(GTK_WIDGET(window->entry_tolerance));
    gtk_widget_hide(GTK_WIDGET(window->label_bests));
    gtk_widget_hide(GTK_WIDGET(window->entry_bests));
    gtk_widget_hide(GTK_WIDGET(window->label_population));
    gtk_widget_hide(GTK_WIDGET(window->entry_population));
    gtk_widget_hide(GTK_WIDGET(window->label_generations));
    gtk_widget_hide(GTK_WIDGET(window->entry_generations));
    gtk_widget_hide(GTK_WIDGET(window->label_mutation));
    gtk_widget_hide(GTK_WIDGET(window->entry_mutation));
    gtk_widget_hide(GTK_WIDGET(window->label_reproduction));
    gtk_widget_hide(GTK_WIDGET(window->entry_reproduction));
    switch (window_get_algorithm())
        {
        case 0:
            gtk_widget_show(GTK_WIDGET(window->label_simulations));
            gtk_widget_show(GTK_WIDGET(window->entry_simulations));
            gtk_widget_show(GTK_WIDGET(window->label_iterations));
            gtk_widget_show(GTK_WIDGET(window->entry_iterations));
            gtk_widget_show(GTK_WIDGET(window->label_tolerance));
            gtk_widget_show(GTK_WIDGET(window->entry_tolerance));
            gtk_widget_show(GTK_WIDGET(window->label_bests));
            gtk_widget_show(GTK_WIDGET(window->entry_bests));
            break;
        case 1:
            gtk_widget_show(GTK_WIDGET(window->label_iterations));
            gtk_widget_show(GTK_WIDGET(window->entry_iterations));
            gtk_widget_show(GTK_WIDGET(window->label_tolerance));
            gtk_widget_show(GTK_WIDGET(window->entry_tolerance));
            gtk_widget_show(GTK_WIDGET(window->label_bests));
            gtk_widget_show(GTK_WIDGET(window->entry_bests));
            break;
        default:
            gtk_widget_show(GTK_WIDGET(window->label_population));
            gtk_widget_show(GTK_WIDGET(window->entry_population));
            gtk_widget_show(GTK_WIDGET(window->label_generations));
            gtk_widget_show(GTK_WIDGET(window->entry_generations));
            gtk_widget_show(GTK_WIDGET(window->label_mutation));
            gtk_widget_show(GTK_WIDGET(window->entry_mutation));
            gtk_widget_show(GTK_WIDGET(window->label_reproduction));
            gtk_widget_show(GTK_WIDGET(window->entry_reproduction));
            gtk_widget_show(GTK_WIDGET(window->label_adaptation));
            gtk_widget_show(GTK_WIDGET(window->entry_adaptation));
        }
}

/**
 * \fn int window_new(GtkApplication *application)
 * \brief Function to open the main window.
 * \param application
 * \brief Main GtkApplication.
 */
void window_new(GtkApplication *application)
{
    unsigned int i;
    char *label_algorithm[NALGORITHMS] =
    {
        "_Monte-Carlo",
        gettext("_Sweep"),
        gettext("_Genetic")
    };

    // Creating the window
    window->window = (GtkWindow*)gtk_application_window_new(application);

    // Setting the window title
    gtk_window_set_title(window->window, PROGRAM_INTERFACE);

    // Creating the save button
    window->button_save
        = (GtkButton*)gtk_button_new_with_mnemonic(gettext("_Save"));
    g_signal_connect(window->button_save, "clicked", input_save, NULL);

    // Creating the help button
    window->button_help
        = (GtkButton*)gtk_button_new_with_mnemonic(gettext("_Help"));
    g_signal_connect(window->button_help, "clicked", window_help, NULL);

    // Creating the exit button
    window->button_exit
        = (GtkButton*)gtk_button_new_with_mnemonic(gettext("E_xit"));
    g_signal_connect_swapped(window->button_exit, "clicked",
                             (void(*))gtk_widget_destroy, window->window);

    // Creating the simulator program label and entry
    window->label_simulator
        = (GtkLabel*)gtk_label_new(gettext("Simulator program"));
    window->button_simulator
        = (GtkFileChooserButton*)gtk_file_chooser_button_new
          (gettext("Simulator program"), GTK_FILE_CHOOSER_ACTION_OPEN);

    // Creating the evaluator program label and entry
    window->label_evaluator
        = (GtkLabel*)gtk_label_new(gettext("Evaluator program"));
    window->button_evaluator
        = (GtkFileChooserButton*)gtk_file_chooser_button_new
          (gettext("Evaluator program"), GTK_FILE_CHOOSER_ACTION_OPEN);

	// Creating the algorithm properties
    window->label_simulations = (GtkLabel*)gtk_label_new
                                (gettext("Simulations number"));
    window->entry_simulations
        = (GtkSpinButton*)gtk_spin_button_new_with_range(1., 1.e12, 1.);
    window->label_iterations
        = (GtkLabel*)gtk_label_new(gettext("Iterations number"));
    window->entry_iterations
        = (GtkSpinButton*)gtk_spin_button_new_with_range(1., 1.e6, 1.);
    window->label_tolerance = (GtkLabel*)gtk_label_new(gettext("Tolerance"));
    window->entry_tolerance
        = (GtkSpinButton*)gtk_spin_button_new_with_range(0., 1., 0.001);
    window->label_bests = (GtkLabel*)gtk_label_new(gettext("Bests number"));
    window->entry_bests
        = (GtkSpinButton*)gtk_spin_button_new_with_range(1., 1.e6, 1.);
    window->label_population = (GtkLabel*)gtk_label_new
                               (gettext("Population number"));
    window->entry_population
        = (GtkSpinButton*)gtk_spin_button_new_with_range(1., 1.e12, 1.);
    window->label_generations
        = (GtkLabel*)gtk_label_new(gettext("Generations number"));
    window->entry_generations
        = (GtkSpinButton*)gtk_spin_button_new_with_range(1., 1.e6, 1.);
    window->label_mutation
        = (GtkLabel*)gtk_label_new(gettext("Mutation ratio"));
    window->entry_mutation
        = (GtkSpinButton*)gtk_spin_button_new_with_range(0., 1., 0.001);
    window->label_reproduction
        = (GtkLabel*)gtk_label_new(gettext("Reproduction ratio"));
    window->entry_reproduction
        = (GtkSpinButton*)gtk_spin_button_new_with_range(0., 1., 0.001);
    window->label_adaptation
        = (GtkLabel*)gtk_label_new(gettext("Adaptation ratio"));
    window->entry_adaptation
        = (GtkSpinButton*)gtk_spin_button_new_with_range(0., 1., 0.001);

    // Creating the array of algorithms
    window->grid_algorithm = (GtkGrid*)gtk_grid_new();
    window->button_algorithm[0] = (GtkRadioButton*)
                                  gtk_radio_button_new_with_mnemonic(NULL, label_algorithm[0]);
    gtk_grid_attach(window->grid_algorithm, GTK_WIDGET(window->button_algorithm[0]), 0, 0, 1, 1);
    g_signal_connect(window->button_algorithm[0], "clicked", window_update, NULL);
    for (i = 0; ++i < NALGORITHMS;)
        {
            window->button_algorithm[i] = (GtkRadioButton*)
                                          gtk_radio_button_new_with_mnemonic(gtk_radio_button_get_group(window->button_algorithm[0]),
                                                  label_algorithm[i]);
            gtk_grid_attach(window->grid_algorithm, GTK_WIDGET(window->button_algorithm[i]), 0, i, 1, 1);
            g_signal_connect(window->button_algorithm[i], "clicked", window_update, NULL);
        }
    gtk_grid_attach(window->grid_algorithm, GTK_WIDGET(window->label_simulations), 0, NALGORITHMS, 1, 1);
    gtk_grid_attach(window->grid_algorithm, GTK_WIDGET(window->entry_simulations), 1, NALGORITHMS, 1, 1);
    gtk_grid_attach(window->grid_algorithm, GTK_WIDGET(window->label_iterations), 0, NALGORITHMS + 1, 1, 1);
    gtk_grid_attach(window->grid_algorithm, GTK_WIDGET(window->entry_iterations), 1, NALGORITHMS + 1, 1, 1);
    gtk_grid_attach(window->grid_algorithm, GTK_WIDGET(window->label_tolerance), 0, NALGORITHMS + 2, 1, 1);
    gtk_grid_attach(window->grid_algorithm, GTK_WIDGET(window->entry_tolerance), 1, NALGORITHMS + 2, 1, 1);
    gtk_grid_attach(window->grid_algorithm, GTK_WIDGET(window->label_bests), 0, NALGORITHMS + 3, 1, 1);
    gtk_grid_attach(window->grid_algorithm, GTK_WIDGET(window->entry_bests), 1, NALGORITHMS + 3, 1, 1);
    gtk_grid_attach(window->grid_algorithm, GTK_WIDGET(window->label_population), 0, NALGORITHMS + 4, 1, 1);
    gtk_grid_attach(window->grid_algorithm, GTK_WIDGET(window->entry_population), 1, NALGORITHMS + 4, 1, 1);
    gtk_grid_attach(window->grid_algorithm, GTK_WIDGET(window->label_generations), 0, NALGORITHMS + 5, 1, 1);
    gtk_grid_attach(window->grid_algorithm, GTK_WIDGET(window->entry_generations), 1, NALGORITHMS + 5, 1, 1);
    gtk_grid_attach(window->grid_algorithm, GTK_WIDGET(window->label_mutation), 0, NALGORITHMS + 6, 1, 1);
    gtk_grid_attach(window->grid_algorithm, GTK_WIDGET(window->entry_mutation), 1, NALGORITHMS + 6, 1, 1);
    gtk_grid_attach(window->grid_algorithm, GTK_WIDGET(window->label_reproduction), 0, NALGORITHMS + 7, 1, 1);
    gtk_grid_attach(window->grid_algorithm, GTK_WIDGET(window->entry_reproduction), 1, NALGORITHMS + 7, 1, 1);
    gtk_grid_attach(window->grid_algorithm, GTK_WIDGET(window->label_adaptation), 0, NALGORITHMS + 8, 1, 1);
    gtk_grid_attach(window->grid_algorithm, GTK_WIDGET(window->entry_adaptation), 1, NALGORITHMS + 8, 1, 1);
    window->frame_algorithm = (GtkFrame*)gtk_frame_new(gettext("Algorithm"));
    gtk_container_add(GTK_CONTAINER(window->frame_algorithm),
                      GTK_WIDGET(window->grid_algorithm));

    // Creating the grid and attaching the widgets to the grid
    window->grid = (GtkGrid*)gtk_grid_new();
    gtk_grid_attach(window->grid, GTK_WIDGET(window->button_save), 0, 0, 1, 1);
    gtk_grid_attach(window->grid, GTK_WIDGET(window->button_help), 1, 0, 1, 1);
    gtk_grid_attach(window->grid, GTK_WIDGET(window->button_exit), 2, 0, 1, 1);
    gtk_grid_attach(window->grid, GTK_WIDGET(window->label_simulator), 0, 1, 1, 1);
    gtk_grid_attach(window->grid, GTK_WIDGET(window->button_simulator), 1, 1, 1, 1);
    gtk_grid_attach(window->grid, GTK_WIDGET(window->label_evaluator), 2, 1, 1, 1);
    gtk_grid_attach(window->grid, GTK_WIDGET(window->button_evaluator), 3, 1, 1, 1);
    gtk_grid_attach(window->grid, GTK_WIDGET(window->frame_algorithm), 0, 2, 2, 1);
    gtk_container_add(GTK_CONTAINER(window->window), GTK_WIDGET(window->grid));

    // Setting the window logo
    window->logo = gtk_image_get_pixbuf
                   (GTK_IMAGE(gtk_image_new_from_file("logo.png")));
    gtk_window_set_icon(window->window, window->logo);

    // Showing the window
    gtk_widget_show_all(GTK_WIDGET(window->window));
	window_update();
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
    int status;
    char *buffer;
    GtkApplication *application;
    xmlKeepBlanksDefault(0);
    setlocale(LC_ALL, "");
    setlocale(LC_NUMERIC, "C");
    buffer = g_get_current_dir();
    bindtextdomain
    (PROGRAM_INTERFACE, g_build_filename(buffer, LOCALE_DIR, NULL));
    bind_textdomain_codeset(PROGRAM_INTERFACE, "UTF-8");
    textdomain(PROGRAM_INTERFACE);
	gtk_disable_setlocale();
    application = gtk_application_new("git.jburguete.calibrator",
                                      G_APPLICATION_FLAGS_NONE);
    g_signal_connect(application, "activate", (void(*))window_new, NULL);
    status = g_application_run(G_APPLICATION(application), argn, argc);
    g_object_unref(application);
    return status;
}
