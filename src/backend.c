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

#include <khonsu/khonsu.h>
#include <seteh/lambda.h>
#include <curie/multiplex.h>
#include <curie/memory.h>
#include <curie/filesystem.h>
#include <curie/gc.h>
#include <curie/signal.h>

define_symbol (sym_include,          "include");
define_symbol (sym_base_name,        "base-name");
define_symbol (sym_extension,        "extension");
define_string (str_error_file_not_found_xhtml,
                                     "/error/file-not-found.xhtml");
define_string (str_dot,              ".");
define_string (str_dot_ksu,          ".ksu");

static sexpr webroot          = sx_nonexistent;
static sexpr mime_map         = sx_nonexistent;

static void configure_callback (sexpr sx)
{
    sexpr a = car (sx);

    if (truep (equalp (a, sym_root)))
    {
        webroot = car (cdr (sx));
    }
    else if (truep (equalp (a, sym_map_extension)))
    {
        a = cdr (sx);
        mime_map = lx_environment_bind (mime_map, car (a), car (cdr (a)));
    }
}

static sexpr include (sexpr arguments, sexpr *env)
{
    define_string (str_slash, "/");
    sexpr e = *env, to = car (arguments),
          t = sx_join (webroot, str_slash, to),
          data = sx_end_of_list, r, tje, lang, lcodes, te, type, tf;
    struct sexpr_io *io;

    if (truep (filep (t)))
    {
        return cons (e, cons (cons (sym_verbatim,
                     cons (t, sx_end_of_list)), sx_end_of_list));
    }
    else
    {
        int len = 0, i = 0;
        const char *ts = sx_string (to);
        char *tmp;
        while (ts[len] != (char)0) 
        {
            if (ts[len] == '.') i = len;
            len++;
        }

        if (i > 0)
        {
            len = i;
            tmp = aalloc (len + 1);
            for (i = 0; i < len; i++)
            {
                tmp[i] = ts[i];
            }
            tmp[i] = 0;
            i++;

            te   = make_string (tmp);
            type = make_string (ts + i);

            afree (i, tmp);

            lcodes = (lx_environment_lookup (e, sym_language));

            e = lx_environment_bind (e, sym_base_name, te);
            e = lx_environment_bind (e, sym_extension, type);

            te = sx_join (te, str_dot_ksu, str_nil);

            while (consp (lcodes))
            {
                lang = car (lcodes);

                t = sx_join (webroot, str_slash,
                             sx_join (lang, str_slash, te));

                if (truep (filep (t)))
                {
                    tf = lx_environment_lookup(mime_map, type);

                    if (!nexp (tf))
                    {
                        e = lx_environment_bind (e, sym_format, tf);

                        io = sx_open_io (io_open_read (sx_string (t)),
                                         io_open_null);

                        while (!eofp (r = sx_read (io)))
                        {
                            if (!nexp (r))
                            {
                                sexpr n = car (r);
                                if (truep (equalp (sym_object, n)))
                                {
                                    tje = lx_environment_join
                                            (kho_environment, *env);
                                    tje = lx_environment_join (tje, e);
                                    data = cons (lx_eval (r, &tje), data);
                                }
                                else
                                {
                                    data = cons (r, data);
                                }
                            }
                        }

                        sx_close_io (io);

                        return cons (e, sx_reverse (data));
                    }
                    else
                    {
                        return cons (e, cons (cons (sym_object,
                                     cons (sym_verbatim, cons (t,
                                           sx_end_of_list))), sx_end_of_list));
                    }
                }

                lcodes = cdr (lcodes);
            }
        }
    }

    if (!nexp (lx_environment_lookup (e, sym_error)))
    {
        return cons (sym_object,
                     cons (cons (sym_error, cons (sym_file_not_found,
                           sx_end_of_list)), sx_end_of_list));
    }
    else
    {
        return sx_nonexistent;
    }
}

static sexpr get (sexpr arguments, sexpr *env)
{
    sexpr e = car (arguments), r;

    if (!environmentp (e))
    {
        e = lx_make_environment (sx_end_of_list);
    }

    r = include (cdr (arguments), &e);

    if (nexp (r))
    {
        e = lx_environment_bind (e, sym_error, sym_file_not_found);

        return include (cons (str_error_file_not_found_xhtml,
                        sx_end_of_list), &e);
    }

    return r;
}

int cmain ()
{
    terminate_on_allocation_errors ();

    mime_map = lx_make_environment (sx_end_of_list);

    gc_add_root (&mime_map);

    kho_configure_callback = configure_callback;

    initialise_khonsu ();

    kho_environment = lx_environment_bind
            (kho_environment, sym_get, lx_foreign_lambda (sym_get, get));
    kho_environment = lx_environment_bind
            (kho_environment, sym_include,
             lx_foreign_lambda (sym_include, include));

    kho_debug (make_symbol ("khonsu-backend"));

    while (multiplex () != mx_nothing_to_do)
    {
//        gc_invoke();
    }

    return 0;
}
