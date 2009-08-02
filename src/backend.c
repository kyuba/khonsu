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

define_string (str_error_file_not_found_xhtml, "/error/file-not-found.xhtml");

static sexpr webroot               = sx_nonexistent;
static sexpr mime_map              = sx_nonexistent;

void configure_callback (sexpr sx)
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

static sexpr get (sexpr arguments, sexpr *env)
{
    define_string (str_slash, "/");
    sexpr e = car (arguments),
          t = sx_join (webroot, str_slash, car (cdr (arguments))),
          data = sx_end_of_list, r;
    struct sexpr_io *io;

    if (!environmentp (e))
    {
        e = lx_make_environment (sx_end_of_list);
    }

    if (truep (filep (t)))
    {
        return cons (e, cons (cons (sym_object, cons (sym_verbatim,
                     cons (t, sx_end_of_list))), sx_end_of_list));
    }
    else
    {
        int len = 0, i;
        const char *ts = sx_string (t);
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

            t = make_string (tmp);

            afree (i, tmp);

            if (truep (filep (t)))
            {
                sexpr tf = lx_environment_lookup(mime_map,make_string (ts + i));
                if (!nexp (tf))
                {
                    e = lx_environment_bind (e, sym_format, tf);

                    io = sx_open_io (io_open_read (sx_string (t)),io_open_null);

                    while (!eofp (r = sx_read (io)))
                    {
                        if (!nexp (r))
                        {
                            sexpr n = car (r);
                            if (truep (equalp (sym_available_languages, n)))
                            {
                                e = lx_environment_bind (e, sym_language,
                                        car (cdr (r)));
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
                    return cons (e, cons (cons (sym_object, cons (sym_verbatim,
                                 cons (t, sx_end_of_list))), sx_end_of_list));
                }
            }
        }
    }

    if (!nexp (lx_environment_lookup (e, sym_error)))
    {
        return cons (sym_object,
                     cons (cons (sym_error, sym_file_not_found),
                           sx_end_of_list));
    }
    else
    {
        e = lx_environment_bind (e, sym_error, sym_file_not_found);

        return get (cons (e, cons (str_error_file_not_found_xhtml,
                    sx_end_of_list)),
                    env);
    }
}

int cmain ()
{
    terminate_on_allocation_errors ();

    mime_map = lx_make_environment (sx_end_of_list);

    kho_configure_callback = configure_callback;

    initialise_khonsu ();

    kho_environment = lx_environment_bind
            (kho_environment, sym_get, lx_foreign_lambda (sym_get, get));

    kho_debug (make_symbol ("khonsu-backend"));

    while (multiplex () != mx_nothing_to_do);

    return 0;
}
