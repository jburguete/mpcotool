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
 * \file tools.c
 * \brief Source file to define some useful functions.
 * \authors Javier Burguete and Borja Latorre.
 * \copyright Copyright 2012-2022, all rights reserved.
 */
#define _GNU_SOURCE
#include "config.h"
#include <stdio.h>
#include <unistd.h>
#include <libxml/parser.h>
#include <libintl.h>
#include <glib.h>
#include <json-glib/json-glib.h>
#ifdef G_OS_WIN32
#include <windows.h>
#endif
#if HAVE_GTK
#include <gtk/gtk.h>
#endif
#include "tools.h"

#if HAVE_GTK
GtkWindow *main_window;         ///< Main GtkWindow.
#endif

char *error_message;            ///< Error message.
void (*show_pending) () = NULL;
///< Pointer to the function to show pending events.

/**
 * Function to show a dialog with a message.
 */
void
show_message (char *title,      ///< Title.
              char *msg,        ///< Message.
              int type
#if !HAVE_GTK
              __attribute__((unused))
#endif
              ///< Message type.
  )
{
#if HAVE_GTK
  GtkMessageDialog *dlg;
  GMainLoop *loop;

  // Creating the dialog
  dlg = (GtkMessageDialog *)
    gtk_message_dialog_new (main_window, GTK_DIALOG_MODAL,
                            (GtkMessageType) type, GTK_BUTTONS_OK, "%s", msg);

  // Setting the dialog title
  gtk_window_set_title (GTK_WINDOW (dlg), title);

  // Showing the dialog and waiting response
#if !GTK4
  gtk_widget_show_all (GTK_WIDGET (dlg));
  g_signal_connect (dlg, "response", G_CALLBACK (gtk_widget_destroy), NULL);
#else
  gtk_widget_show (GTK_WIDGET (dlg));
  g_signal_connect (dlg, "response", G_CALLBACK (gtk_window_destroy), NULL);
#endif
  loop = g_main_loop_new (NULL, 0);
  g_signal_connect_swapped (dlg, "destroy", G_CALLBACK (g_main_loop_quit),
                            loop);
  g_main_loop_run (loop);
  g_main_loop_unref (loop);

  // Closing and freeing memory
  gtk_window_destroy (GTK_WINDOW (dlg));

#else
  printf ("%s: %s\n", title, msg);
#endif
}

/**
 * Function to show a dialog with an error message.
 */
void
show_error (char *msg)          ///< Error message.
{
  show_message (_("ERROR!"), msg, ERROR_TYPE);
}

/**
 * Function to get an integer number of a XML node property.
 *
 * \return Integer number value.
 */
int
xml_node_get_int (xmlNode * node,       ///< XML node.
                  const xmlChar * prop, ///< XML property.
                  int *error_code)      ///< Error code.
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
 * Function to get an unsigned integer number of a XML node property.
 *
 * \return Unsigned integer number value.
 */
unsigned int
xml_node_get_uint (xmlNode * node,      ///< XML node.
                   const xmlChar * prop,        ///< XML property.
                   int *error_code)     ///< Error code.
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
 * Function to get an unsigned integer number of a XML node property with a
 *   default value.
 *
 * \return Unsigned integer number value.
 */
unsigned int
xml_node_get_uint_with_default (xmlNode * node, ///< XML node.
                                const xmlChar * prop,   ///< XML property.
                                unsigned int default_value,
                                ///< default value.
                                int *error_code)        ///< Error code.
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
 * Function to get a floating point number of a XML node property.
 *
 * \return Floating point number value.
 */
double
xml_node_get_float (xmlNode * node,     ///< XML node.
                    const xmlChar * prop,       ///< XML property.
                    int *error_code)    ///< Error code.
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
 * Function to get a floating point number of a XML node property with a default
 *   value.
 *
 * \return Floating point number value.
 */
double
xml_node_get_float_with_default (xmlNode * node,        ///< XML node.
                                 const xmlChar * prop,  ///< XML property.
                                 double default_value,  ///< default value.
                                 int *error_code)       ///< Error code.
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
 * Function to set an integer number in a XML node property.
 */
void
xml_node_set_int (xmlNode * node,       ///< XML node.
                  const xmlChar * prop, ///< XML property.
                  int value)    ///< Integer number value.
{
  xmlChar buffer[64];
  snprintf ((char *) buffer, 64, "%d", value);
  xmlSetProp (node, prop, buffer);
}

/**
 * Function to set an unsigned integer number in a XML node property.
 */
void
xml_node_set_uint (xmlNode * node,      ///< XML node.
                   const xmlChar * prop,        ///< XML property.
                   unsigned int value)  ///< Unsigned integer number value.
{
  xmlChar buffer[64];
  snprintf ((char *) buffer, 64, "%u", value);
  xmlSetProp (node, prop, buffer);
}

/**
 * Function to set a floating point number in a XML node property.
 */
void
xml_node_set_float (xmlNode * node,     ///< XML node.
                    const xmlChar * prop,       ///< XML property.
                    double value)       ///< Floating point number value.
{
  xmlChar buffer[64];
  snprintf ((char *) buffer, 64, "%.14lg", value);
  xmlSetProp (node, prop, buffer);
}

/**
 * Function to get an integer number of a JSON object property.
 *
 * \return Integer number value.
 */
int
json_object_get_int (JsonObject * object,       ///< JSON object.
                     const char *prop,  ///< JSON property.
                     int *error_code)   ///< Error code.
{
  const char *buffer;
  int i = 0;
  buffer = json_object_get_string_member (object, prop);
  if (!buffer)
    *error_code = 1;
  else
    {
      if (sscanf (buffer, "%d", &i) != 1)
        *error_code = 2;
      else
        *error_code = 0;
    }
  return i;
}

/**
 * Function to get an unsigned integer number of a JSON object property.
 *
 * \return Unsigned integer number value.
 */
unsigned int
json_object_get_uint (JsonObject * object,      ///< JSON object.
                      const char *prop, ///< JSON property.
                      int *error_code)  ///< Error code.
{
  const char *buffer;
  unsigned int i = 0;
  buffer = json_object_get_string_member (object, prop);
  if (!buffer)
    *error_code = 1;
  else
    {
      if (sscanf (buffer, "%u", &i) != 1)
        *error_code = 2;
      else
        *error_code = 0;
    }
  return i;
}

/**
 * Function to get an unsigned integer number of a JSON object property with a
 *   default value.
 *
 * \return Unsigned integer number value.
 */
unsigned int
json_object_get_uint_with_default (JsonObject * object, ///< JSON object.
                                   const char *prop,    ///< JSON property.
                                   unsigned int default_value,
                                   ///< default value.
                                   int *error_code)     ///< Error code.
{
  unsigned int i;
  if (json_object_get_member (object, prop))
    i = json_object_get_uint (object, prop, error_code);
  else
    {
      i = default_value;
      *error_code = 0;
    }
  return i;
}

/**
 * Function to get a floating point number of a JSON object property.
 *
 * \return Floating point number value.
 */
double
json_object_get_float (JsonObject * object,     ///< JSON object.
                       const char *prop,        ///< JSON property.
                       int *error_code) ///< Error code.
{
  const char *buffer;
  double x = 0.;
  buffer = json_object_get_string_member (object, prop);
  if (!buffer)
    *error_code = 1;
  else
    {
      if (sscanf (buffer, "%lf", &x) != 1)
        *error_code = 2;
      else
        *error_code = 0;
    }
  return x;
}

/**
 * Function to get a floating point number of a JSON object property with a 
 *   default value.
 *
 * \return Floating point number value.
 */
double
json_object_get_float_with_default (JsonObject * object,
                                    ///< JSON object.
                                    const char *prop,   ///< JSON property.
                                    double default_value,
                                    ///< default value.
                                    int *error_code)    ///< Error code.
{
  double x;
  if (json_object_get_member (object, prop))
    x = json_object_get_float (object, prop, error_code);
  else
    {
      x = default_value;
      *error_code = 0;
    }
  return x;
}

/**
 * Function to set an integer number in a JSON object property.
 */
void
json_object_set_int (JsonObject * object,       ///< JSON object.
                     const char *prop,  ///< JSON property.
                     int value) ///< Integer number value.
{
  char buffer[64];
  snprintf (buffer, 64, "%d", value);
  json_object_set_string_member (object, prop, buffer);
}

/**
 * Function to set an unsigned integer number in a JSON object property.
 */
void
json_object_set_uint (JsonObject * object,      ///< JSON object.
                      const char *prop, ///< JSON property.
                      unsigned int value)
                      ///< Unsigned integer number value.
{
  char buffer[64];
  snprintf (buffer, 64, "%u", value);
  json_object_set_string_member (object, prop, buffer);
}

/**
 * Function to set a floating point number in a JSON object property.
 */
void
json_object_set_float (JsonObject * object,     ///< JSON object.
                       const char *prop,        ///< JSON property.
                       double value)    ///< Floating point number value.
{
  char buffer[64];
  snprintf (buffer, 64, "%.14lg", value);
  json_object_set_string_member (object, prop, buffer);
}

/**
 * Function to obtain the cores number.
 *
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
 * Function to process events on long computation.
 */
void
process_pending ()
{
  GMainContext *context = g_main_context_default ();
  while (g_main_context_pending (context))
    g_main_context_iteration (context, 0);
}

/**
 * Function to get the active GtkRadioButton.
 *
 * \return Active GtkRadioButton.
 */
unsigned int
gtk_array_get_active (
#if !GTK4
		      GtkRadioButton * array[], ///< Array of GtkRadioButtons.
#else
		      GtkCheckButton * array[], ///< Array of GtkCheckButtons.
#endif
		      unsigned int n)   ///< Number of GtkRadioButtons.
{
  unsigned int i;
  for (i = 0; i < n; ++i)
    if (gtk_check_button_get_active (array[i]))
      break;
  return i;
}

#if GTK4

/**
 * function to set a text on a GtkEntry struct as in GTK3.
 */
void
gtk_entry_set_text (GtkEntry * entry,   ///< GtkEntry struct.
                    const char *text)   ///< text.
{
  GtkEntryBuffer *buffer;
  buffer = gtk_entry_get_buffer (entry);
  gtk_entry_buffer_set_text (buffer, text, -1);
}

/**
 * function to get the text of a GtkEntry widget as in GTK3.
 *
 * \return text.
 */
const char *
gtk_entry_get_text (GtkEntry * entry)   ///< GtkEntry struct.
{
  GtkEntryBuffer *buffer;
  buffer = gtk_entry_get_buffer (entry);
  return gtk_entry_buffer_get_text (buffer);
}

#endif

#endif
