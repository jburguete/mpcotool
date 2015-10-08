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

#define MAX_NINPUTS 8
#define PROGRAM_INTERFACE "Calibrator"
#define LOCALE_DIR "locales"

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
     * \var grid
     * \brief Main GtkGrid.
     * \var logo
     * \brief Logo GdkPixbuf.
     * \var window
     * \brief Main GtkWindow.
     */
    GtkButton *button_save, *button_help;
    GtkGrid *grid;
    GdkPixbuf *logo;
    GtkWindow *window;
} Window;

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

    // Opening the input file
    doc = xmlNewDoc((const xmlChar*)"1.0");

    // Root XML node
    node = xmlNewDocNode(doc, 0, XML_CALIBRATE, 0);
    xmlDocSetRootElement(doc, node);

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

    // If OK response then saving
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK)
        {
            buffer = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
            xmlSaveFormatFile(buffer, doc, 1);
            g_free(buffer);
        }

    // Closing and freeing memory
    gtk_widget_destroy(GTK_WIDGET(dlg));
    xmlFreeDoc(doc);
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
 * \fn int window_new(GtkApplication *application)
 * \brief Function to open the main window.
 * \param application
 * \brief Main GtkApplication.
 */
void window_new(GtkApplication *application)
{
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

    // Creating the grid and attaching the widgets to the grid
    window->grid = (GtkGrid*)gtk_grid_new();
    gtk_grid_attach(window->grid, GTK_WIDGET(window->button_save), 0, 0, 1, 1);
    gtk_grid_attach(window->grid, GTK_WIDGET(window->button_help), 1, 0, 1, 1);
    gtk_container_add(GTK_CONTAINER(window->window), GTK_WIDGET(window->grid));

    // Setting the window logo
    window->logo = gtk_image_get_pixbuf
    (GTK_IMAGE(gtk_image_new_from_file("logo.png")));
    gtk_window_set_icon(window->window, window->logo);

    // Showing the window
    gtk_widget_show_all(GTK_WIDGET(window->window));
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
    application = gtk_application_new("git.jburguete.calibrator",
                                      G_APPLICATION_FLAGS_NONE);
    g_signal_connect(application, "activate", (void(*))window_new, NULL);
    status = g_application_run(G_APPLICATION(application), argn, argc);
    g_object_unref(application);
    return status;
}
