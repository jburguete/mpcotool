/*
MPCOTool:
The Multi-Purposes Calibration and Optimization Tool. A software to perform
calibrations or optimizations of empirical parameters.

AUTHORS: Javier Burguete and Borja Latorre.

Copyright 2012-2023, AUTHORS.

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
 * \copyright Copyright 2012-2023, all rights reserved.
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
#include "jb/src/jb_win.h"
#include "tools.h"

#if HAVE_GTK
GtkWindow *main_window;         ///< Main GtkWindow.
#endif

char *error_message;            ///< Error message.
void (*show_pending) () = NULL;
///< Pointer to the function to show pending events.

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
                       GtkRadioButton * array[],        ///< Array of GtkRadioButtons.
#else
                       GtkCheckButton * array[],        ///< Array of GtkCheckButtons.
#endif
                       unsigned int n)  ///< Number of GtkRadioButtons.
{
  unsigned int i;
  for (i = 0; i < n; ++i)
    if (gtk_check_button_get_active (array[i]))
      break;
  return i;
}

#endif
