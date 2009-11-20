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
#include <curie/gc.h>

define_string (str_lt,        "<");
define_string (str_ltslash,   "</");
define_string (str_gt,        ">");
define_string (str_slashgt,   "/>");
define_string (str_eqquot,    "=\"");
define_string (str_quot,      "\"");
define_string (str_space,     " ");
define_string (str_text_html, "text/html");

static sexpr object_xml (sexpr arguments, struct machine_state *st)
{
    sexpr r = str_nil, t = car (arguments), a;
    char have_content = 0, symbol_t = 0;

    if (truep (equalp (sym_object, t)))
    {
        arguments = cdr (arguments);
        t = car (arguments);
    }

    symbol_t = symbolp (t);

    if (symbol_t)
    {
        r = sx_join (str_lt, t, str_nil);
        arguments = cdr (arguments);

        a = car (arguments);
        if (environmentp (a))
        {
            sexpr tb, tba, tbb;
            a = lx_environment_alist (a);

            while (consp (a))
            {
                tb = car (a);
                tba = car (tb);
                tbb = cdr (tb);

                if (symbolp (tba) && stringp (tbb))
                {
                    r = sx_join (r, str_space, tba);
                    r = sx_join (r, str_eqquot, tbb);
                    r = sx_join (r, str_quot, str_nil);
                }

                a = cdr (a);
            }

            arguments = cdr (arguments);
        }
    }

    while (consp (arguments))
    {
        a = car (arguments);

        if (consp (a))
        {
            if (symbol_t && !have_content)
            {
                r = sx_join (r, str_gt, str_nil);
            }

            r = sx_join (r, object_xml (a, st), str_nil);
            have_content = 1;
        }
        else if (stringp (a))
        {
            if (symbol_t && !have_content)
            {
                r = sx_join (r, str_gt, str_nil);
            }

            r = sx_join (r, a, str_nil);
            have_content = 1;
        }

        arguments = cdr (arguments);
    }

    if (symbol_t)
    {
        if (!have_content)
        {
            r = sx_join (r, str_slashgt, str_nil);
        }
        else
        {
            r = sx_join (r, sx_join (str_ltslash, t, str_gt), str_nil);
        }
    }

    return r;
}

static sexpr object_sgml (sexpr arguments, struct machine_state *st)
{
    sexpr r = str_nil, t = car (arguments), a;
    char have_content = 0, symbol_t = 0;

    if (truep (equalp (sym_object, t)))
    {
        arguments = cdr (arguments);
        t = car (arguments);
    }

    symbol_t = symbolp (t);

    if (symbol_t)
    {
        r = sx_join (str_lt, t, str_nil);
        arguments = cdr (arguments);

        a = car (arguments);
        if (environmentp (a))
        {
            sexpr tb, tba, tbb;
            a = lx_environment_alist (a);

            while (consp (a))
            {
                tb = car (a);
                tba = car (tb);
                tbb = cdr (tb);

                if (symbolp (tba) && stringp (tbb))
                {
                    r = sx_join (r, str_space, tba);
                    r = sx_join (r, str_eqquot, tbb);
                    r = sx_join (r, str_quot, str_nil);
                }

                a = cdr (a);
            }

            arguments = cdr (arguments);
        }
    }

    while (consp (arguments))
    {
        a = car (arguments);

        if (consp (a))
        {
            if (symbol_t && !have_content)
            {
                r = sx_join (r, str_gt, str_nil);
            }

            r = sx_join (r, object_sgml (a, st), str_nil);
            have_content = 1;
        }
        else if (stringp (a))
        {
            if (symbol_t && !have_content)
            {
                r = sx_join (r, str_gt, str_nil);
            }

            r = sx_join (r, a, str_nil);
            have_content = 1;
        }

        arguments = cdr (arguments);
    }

    if (symbol_t)
    {
        if (!have_content)
        {
            r = sx_join (r, str_gt, str_nil);
        }
        else
        {
            r = sx_join (r, sx_join (str_ltslash, t, str_gt), str_nil);
        }
    }

    return r;
}

static sexpr object (sexpr arguments, struct machine_state *st)
{
    sexpr env = st->environment;

    return truep (equalp (lx_environment_lookup (env, sym_format),
                          str_text_html)) ? object_sgml (arguments, st)
                                          : object_xml (arguments, st);
}

int cmain ()
{
    terminate_on_allocation_errors ();

    initialise_khonsu ();

    kho_environment = lx_environment_unbind (kho_environment, sym_object);

    kho_environment = lx_environment_bind
            (kho_environment, sym_object,
             lx_foreign_lambda (sym_object, object));

    while (multiplex () != mx_nothing_to_do)
    {
//        gc_invoke();
    }

    return 0;
}
