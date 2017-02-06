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
 * \file utils.h
 * \brief Header file to define some useful functions.
 * \authors Javier Burguete.
 * \copyright Copyright 2012-2016, all rights reserved.
 */
#ifndef UTILS__H
#define UTILS__H 1

/**
 * \def ERROR_TYPE
 * \brief Macro to define the error message type.
 * \def INFO_TYPE
 * \brief Macro to define the information message type.
 */
#if HAVE_GTK
#define ERROR_TYPE GTK_MESSAGE_ERROR
#define INFO_TYPE GTK_MESSAGE_INFO
extern GtkWindow *main_window;
#else
#define ERROR_TYPE 0
#define INFO_TYPE 0
#endif

extern char *error_message;
extern void (*show_pending) ();

// Public functions
void show_message (char *title, char *msg, int type);
void show_error (char *msg);
int xml_node_get_int (xmlNode * node, const xmlChar * prop, int *error_code);
unsigned int xml_node_get_uint (xmlNode * node, const xmlChar * prop,
                                int *error_code);
unsigned int xml_node_get_uint_with_default (xmlNode * node,
                                             const xmlChar * prop,
                                             unsigned int default_value,
                                             int *error_code);
double xml_node_get_float (xmlNode * node, const xmlChar * prop,
                           int *error_code);
double xml_node_get_float_with_default (xmlNode * node, const xmlChar * prop,
                                        double default_value,
                                        int *error_code);
void xml_node_set_int (xmlNode * node, const xmlChar * prop, int value);
void xml_node_set_uint (xmlNode * node, const xmlChar * prop,
                        unsigned int value);
void xml_node_set_float (xmlNode * node, const xmlChar * prop, double value);
int json_object_get_int (JsonObject * object, const char *prop,
                         int *error_code);
unsigned int json_object_get_uint (JsonObject * object, const char *prop,
                                   int *error_code);
unsigned int json_object_get_uint_with_default (JsonObject * object,
                                                const char *prop,
                                                unsigned int default_value,
                                                int *error_code);
double json_object_get_float (JsonObject * object, const char *prop,
                              int *error_code);
double json_object_get_float_with_default (JsonObject * object,
                                           const char *prop,
                                           double default_value,
                                           int *error_code);
void json_object_set_int (JsonObject * object, const char *prop, int value);
void json_object_set_uint (JsonObject * object, const char *prop,
                           unsigned int value);
void json_object_set_float (JsonObject * object, const char *prop,
                            double value);
int cores_number ();
#if HAVE_GTK
void process_pending ();
unsigned int gtk_array_get_active (GtkRadioButton * array[], unsigned int n);
#endif

#endif
