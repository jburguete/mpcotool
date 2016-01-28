/*
MPCOTool:
The Multi-Purposes Calibration and Optimization Tool. A software to perform
calibrations or optimizations of empirical parameters.

AUTHORS: Javier Burguete and Borja Latorre.

Copyright 2012-2016, AUTHORS.

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
 * \file utils.c
 * \brief Source file to define some useful functions.
 * \authors Javier Burguete and Borja Latorre.
 * \copyright Copyright 2012-2016, all rights reserved.
 */
#define _GNU_SOURCE
#include "config.h"
#include <stdio.h>
#include <unistd.h>
#include <libxml/parser.h>
#include <libintl.h>
#if HAVE_GTK
#include <gtk/gtk.h>
#endif
#include "utils.h"

#if HAVE_GTK
GtkWindow *main_window;         ///< Main GtkWindow.
#endif

char *error_message; ///< Error message.

/**
 * \fn void show_message (char *title, char *msg, int type)
 * \brief Function to show a dialog with a message.
 * \param title
 * \brief Title.
 * \param msg
 * \brief Message.
 * \param type
 * \brief Message type.
 */
void
show_message (char *title, char *msg, int type)
{
#if HAVE_GTK
  GtkMessageDialog *dlg;

  // Creating the dialog
  dlg = (GtkMessageDialog *) gtk_message_dialog_new
    (main_window, GTK_DIALOG_MODAL, type, GTK_BUTTONS_OK, "%s", msg);

  // Setting the dialog title
  gtk_window_set_title (GTK_WINDOW (dlg), title);

  // Showing the dialog and waiting response
  gtk_dialog_run (GTK_DIALOG (dlg));

  // Closing and freeing memory
  gtk_widget_destroy (GTK_WIDGET (dlg));

#else
  printf ("%s: %s\n", title, msg);
#endif
}

/**
 * \fn void show_error (char *msg)
 * \brief Function to show a dialog with an error message.
 * \param msg
 * \brief Error message.
 */
void
show_error (char *msg)
{
  show_message (gettext ("ERROR!"), msg, ERROR_TYPE);
}

/**
 * \fn int xml_node_get_int (xmlNode *node, const xmlChar *prop, \
 *   int *error_code)
 * \brief Function to get an integer number of a XML node property.
 * \param node
 * \brief XML node.
 * \param prop
 * \brief XML property.
 * \param error_code
 * \brief Error code.
 * \return Integer number value.
 */
int
xml_node_get_int (xmlNode * node, const xmlChar * prop, int *error_code)
{
  int i = 0;
  xmlChar *buffer;
  buffer = xmlGetProp (node, prop);
  if (!buffer)
    *error_code = 1;
  else
    {
      if (sscanf ((char *) buffer, "%d", &i) != 1)
        *error_code = 2;
      else
        *error_code = 0;
      xmlFree (buffer);
    }
  return i;
}

/**
 * \fn int xml_node_get_uint (xmlNode *node, const xmlChar *prop, \
 *   int *error_code)
 * \brief Function to get an unsigned integer number of a XML node property.
 * \param node
 * \brief XML node.
 * \param prop
 * \brief XML property.
 * \param error_code
 * \brief Error code.
 * \return Unsigned integer number value.
 */
unsigned int
xml_node_get_uint (xmlNode * node, const xmlChar * prop, int *error_code)
{
  unsigned int i = 0;
  xmlChar *buffer;
  buffer = xmlGetProp (node, prop);
  if (!buffer)
    *error_code = 1;
  else
    {
      if (sscanf ((char *) buffer, "%u", &i) != 1)
        *error_code = 2;
      else
        *error_code = 0;
      xmlFree (buffer);
    }
  return i;
}

/**
 * \fn int xml_node_get_uint_with_default (xmlNode *node, const xmlChar *prop, \
 *   unsigned int default_value, int *error_code)
 * \brief Function to get an unsigned integer number of a XML node property with
 *   a default value.
 * \param node
 * \brief XML node.
 * \param prop
 * \brief XML property.
 * \param default_value
 * \brief default value.
 * \param error_code
 * \brief Error code.
 * \return Unsigned integer number value.
 */
unsigned int
xml_node_get_uint_with_default (xmlNode * node, const xmlChar * prop,
                                unsigned int default_value, int *error_code)
{
  unsigned int i;
  if (xmlHasProp (node, prop))
    i = xml_node_get_uint (node, prop, error_code);
  else
    {
      i = default_value;
      *error_code = 0;
    }
  return i;
}

/**
 * \fn double xml_node_get_float (xmlNode *node, const xmlChar *prop, \
 *   int *error_code)
 * \brief Function to get a floating point number of a XML node property.
 * \param node
 * \brief XML node.
 * \param prop
 * \brief XML property.
 * \param error_code
 * \brief Error code.
 * \return Floating point number value.
 */
double
xml_node_get_float (xmlNode * node, const xmlChar * prop, int *error_code)
{
  double x = 0.;
  xmlChar *buffer;
  buffer = xmlGetProp (node, prop);
  if (!buffer)
    *error_code = 1;
  else
    {
      if (sscanf ((char *) buffer, "%lf", &x) != 1)
        *error_code = 2;
      else
        *error_code = 0;
      xmlFree (buffer);
    }
  return x;
}

/**
 * \fn double xml_node_get_float_with_default (xmlNode *node, \
 *   const xmlChar *prop, double default_value, int *error_code)
 * \brief Function to get a floating point number of a XML node property with a
 *   default value.
 * \param node
 * \brief XML node.
 * \param prop
 * \brief XML property.
 * \param default_value
 * \brief default value.
 * \param error_code
 * \brief Error code.
 * \return Floating point number value.
 */
double
xml_node_get_float_with_default (xmlNode * node, const xmlChar * prop,
                                 double default_value, int *error_code)
{
  double x;
  if (xmlHasProp (node, prop))
    x = xml_node_get_float (node, prop, error_code);
  else
    {
      x = default_value;
      *error_code = 0;
    }
  return x;
}

/**
 * \fn void xml_node_set_int (xmlNode *node, const xmlChar *prop, int value)
 * \brief Function to set an integer number in a XML node property.
 * \param node
 * \brief XML node.
 * \param prop
 * \brief XML property.
 * \param value
 * \brief Integer number value.
 */
void
xml_node_set_int (xmlNode * node, const xmlChar * prop, int value)
{
  xmlChar buffer[64];
  snprintf ((char *) buffer, 64, "%d", value);
  xmlSetProp (node, prop, buffer);
}

/**
 * \fn void xml_node_set_uint (xmlNode *node, const xmlChar *prop, \
 *   unsigned int value)
 * \brief Function to set an unsigned integer number in a XML node property.
 * \param node
 * \brief XML node.
 * \param prop
 * \brief XML property.
 * \param value
 * \brief Unsigned integer number value.
 */
void
xml_node_set_uint (xmlNode * node, const xmlChar * prop, unsigned int value)
{
  xmlChar buffer[64];
  snprintf ((char *) buffer, 64, "%u", value);
  xmlSetProp (node, prop, buffer);
}

/**
 * \fn void xml_node_set_float (xmlNode *node, const xmlChar *prop, \
 *   double value)
 * \brief Function to set a floating point number in a XML node property.
 * \param node
 * \brief XML node.
 * \param prop
 * \brief XML property.
 * \param value
 * \brief Floating point number value.
 */
void
xml_node_set_float (xmlNode * node, const xmlChar * prop, double value)
{
  xmlChar buffer[64];
  snprintf ((char *) buffer, 64, "%.14lg", value);
  xmlSetProp (node, prop, buffer);
}

/**
 * \fn int cores_number ()
 * \brief Function to obtain the cores number.
 * \return Cores number.
 */
int
cores_number ()
{
#ifdef G_OS_WIN32
  SYSTEM_INFO sysinfo;
  GetSystemInfo (&sysinfo);
  return sysinfo.dwNumberOfProcessors;
#else
  return (int) sysconf (_SC_NPROCESSORS_ONLN);
#endif
}

#if HAVE_GTK

/**
 * \fn unsigned int gtk_array_get_active (GtkRadioButton * array[], \
 *   unsigned int n)
 * \brief Function to get the active GtkRadioButton.
 * \param array
 * \brief Array of GtkRadioButtons.
 * \param n
 * \brief Number of GtkRadioButtons.
 * \return Active GtkRadioButton.
 */
unsigned int
gtk_array_get_active (GtkRadioButton * array[], unsigned int n)
{
  unsigned int i;
  for (i = 0; i < n; ++i)
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (array[i])))
      break;
  return i;
}

#endif
