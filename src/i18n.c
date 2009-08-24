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

define_symbol (sym_accept_language,  "accept-language");
define_symbol (sym_Vary,             "Vary");
define_symbol (sym_default_language, "default-language");
define_string (str_Accept_Language,  "Accept-Language");

static sexpr default_language = sx_nonexistent;

static void configure_callback (sexpr sx)
{
    sexpr a = car (sx);

    if (truep (equalp (a, sym_default_language)))
    {
        default_language = car (cdr (sx));
    }
}

static sexpr get_acceptable_languages (sexpr lq)
{
    sexpr lcodes = sx_end_of_list;
    char have_default = 0;

    if (!nexp (lq))
    {
        const char *lqs = sx_string (lq);
        char lqsm [KHO_MAX_HEADER_FIELD_LENGTH], semicolon;
        sexpr n;
        int i = 0, il = 0;

        while (lqs[i] != (char)0)
        {
            lqsm[i] = lqs[i];
            if (i >= (KHO_MAX_HEADER_FIELD_LENGTH - 1)) break;
            i++;
        }

        lqsm[i] = (char)0;

        for (i = 0; lqsm[i] != (char)0; i++)
        {
            if ((lqsm[i] == ',') || (semicolon = (lqsm[i] == ';')))
            {
                lqsm[i] = (char)0;

                n = make_string (lqsm + il);
                if (!have_default && truep (equalp (n, default_language)))
                {
                    have_default = 1;
                }

                lcodes = cons (n, lcodes);

                if (semicolon)
                {
                    i++;
                    while ((lqsm[i] != 0) && (lqsm[i] != ','))
                    {
                        if (lqsm[i] == 0) break;
                        i++;
                    }
                }

                i++;

                while ((lqsm[i] == ' ') || (lqsm[i] == '\t'))
                {
                    i++;
                }

                if (lqsm[i] == 0) break;

                il = i;
            }
        }

        if (il < i)
        {
            lcodes = cons (make_string (lqsm + il), lcodes);
        }
    }


    if (!have_default)
    {
        lcodes = cons (default_language, lcodes);
    }

    return lcodes;
}

static sexpr request (sexpr arguments, sexpr *env)
{
    sexpr a = arguments, r = sx_end_of_list, a2, a3;

    while (consp (a))
    {
        a2 = car (a);
        a3 = car (a2);

        if (truep (equalp (a3, sym_get)))
        {
            sexpr t1 = cdr (a2), e = car (t1);

            if (environmentp (e))
            {
                e = lx_environment_bind
                    (e, sym_language,
                     get_acceptable_languages
                         (lx_environment_lookup (e, sym_accept_language)));
            }
            else
            {
                e = lx_make_environment
                    (cons (cons (sym_language,
                                 get_acceptable_languages (sx_nonexistent)),
                     sx_end_of_list));
            }

            e = lx_environment_bind (e, sym_Vary, str_Accept_Language);

            r = cons (cons (sym_get, cons (e, cdr (t1))), r);
        }
        else
        {
            r = cons (a2, r);
        }

        a = cdr (a);
    }

    relay_sub (sx_reverse (r));
    return sx_nonexistent;
}

int cmain ()
{
    terminate_on_allocation_errors ();

    kho_configure_callback = configure_callback;

    initialise_khonsu ();

    kho_environment = lx_environment_bind
      (kho_environment, sym_request, lx_foreign_lambda (sym_request, request));

    while (multiplex () != mx_nothing_to_do)
    {
//        gc_invoke();
    }

    return 0;
}
