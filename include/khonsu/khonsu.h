/*
 * This file is part of the kyuba.org Khonsu project.
 * See the appropriate repository at http://git.kyuba.org/ for exact file
 * modification records.
*/

/*
 * Copyright (c) 2009, Kyuba Project Members
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
*/

#ifndef LIBKHONSU_KHONSU_H
#define LIBKHONSU_KHONSU_H

#ifdef __cplusplus
extern "C" {
#endif

#define KHO_MAX_PATH_LENGTH         0x1000
#define KHO_MAX_HEADER_FIELD_LENGTH 0x1000

#include <curie/sexpr.h>

#define KHO_QUIT               (1<<0)
#define KHO_IGNORE_QUIT        (1<<1)
#define KHO_IGNORE_QUIT_RELOAD (1<<2)

extern struct sexpr_io *kho_stdio;
extern sexpr kho_configuration;
extern sexpr kho_environment;
extern void (*kho_configure_callback)(sexpr);
extern unsigned int kho_options;

void  initialise_khonsu     (void);
void  kho_load              (void);
void  kho_debug             (sexpr sx);
void  kho_configure         (sexpr sx);

sexpr kho_tagmerge          (sexpr arguments, sexpr pre, sexpr post);
sexpr kho_merge             (sexpr arguments);
sexpr kho_canonicalise      (sexpr string);
sexpr kho_normalise         (sexpr string);

void  kho_register_output   (struct sexpr_io *out);
void  kho_unregister_output (struct sexpr_io *out);

void  relay_spawn           (sexpr configuration);
void  relay_sub             (sexpr request);

define_symbol (sym_Content_Type,             "Content-Type");
define_symbol (sym_pipeline,                 "pipeline");
define_symbol (sym_root,                     "root");
define_symbol (sym_request,                  "request");
define_symbol (sym_reply,                    "reply");
define_symbol (sym_configure,                "configure");
define_symbol (sym_get,                      "get");
define_symbol (sym_pong,                     "pong");
define_symbol (sym_object,                   "object");
define_symbol (sym_debug,                    "debug");
define_symbol (sym_file_not_found,           "file-not-found");
define_symbol (sym_error,                    "error");
define_symbol (sym_format,                   "format");
define_symbol (sym_language,                 "language");
define_symbol (sym_map_extension,            "map-extension");
define_symbol (sym_verbatim,                 "verbatim");
define_symbol (sym_id,                       "id");
define_symbol (sym_quit,                     "quit");
define_symbol (sym_terminate,                "terminate");
define_symbol (sym_tick,                     "tick");
define_symbol (sym_reload,                   "reload");
define_symbol (sym_ok,                       "ok");
define_symbol (sym_invoke_gc,                "invoke-gc");

define_string (str_nil,                      "");

#ifdef __cplusplus
}
#endif

#endif
