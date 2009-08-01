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
#include <curie/gc.h>
#include <curie/main.h>

static sexpr ddebug     = sx_end_of_list;
sexpr kho_environment   = sx_end_of_list;
sexpr kho_configuration = sx_end_of_list;

struct sexpr_io *kho_stdio;

void (*kho_configure_callback)(sexpr) = (void *)0;

static void sx_stdio_read (sexpr sx, struct sexpr_io *io, void *aux)
{
    lx_eval (sx, &kho_environment);
}

static sexpr reply (sexpr arguments, sexpr *env)
{
    sexpr rv = sx_end_of_list;
    sexpr menv = *env;

    while (consp(arguments))
    {
        sexpr a = car (arguments);
        if (environmentp (a))
        {
            menv = lx_environment_join (menv, a);
            rv = cons (a, rv);
        }
        else
        {
            sexpr r = lx_eval (a, &menv);
            if (consp (r))
            {
                sexpr ra = car (r);
                if (environmentp (ra))
                {
                    rv = cons (ra, rv);
                    for (r = cdr (r); consp (r); r = cdr (r))
                    {
                        rv = cons (car (r), rv);
                    }
                }
                else
                {
                    rv = cons (r, rv);
                }
            }
            else
            {
                rv = cons (r, rv);
            }
        }
        arguments = cdr (arguments);
    }

    sx_write (kho_stdio, cons (sym_reply, sx_reverse (rv)));

    return sx_nonexistent;
}

static sexpr request (sexpr arguments, sexpr *env)
{
    relay_sub (arguments);
    return sx_nonexistent;
}

static sexpr object_sub (sexpr arguments, sexpr env)
{
    if (consp (arguments))
    {
        sexpr t = car (arguments), r = sx_end_of_list, tx;
        if (truep (equalp (t, sym_object)))
        {
            arguments = cdr (arguments);
            t = car (arguments);
        }

        arguments = cdr (arguments);

        tx = lx_eval (t, &env);
        if (primitivep (tx))
        {
            return lx_eval (cons (tx, arguments), &env);
        }

        while (consp (arguments))
        {
            r = cons (object_sub (car (arguments), env), r);
            arguments = cdr (arguments);
        }

        t = object_sub (t, env);

        if (symbolp (t) && !nexp (lx_environment_lookup (env, t)))
        {
            return lx_eval (cons (t, sx_reverse (r)), &env);
        }

        return cons (t, sx_reverse (r));
    }

    return arguments;
}

static sexpr object (sexpr arguments, sexpr *env)
{
    sexpr r = object_sub (arguments, *env), ra;
    if (consp (r) && ((ra = car (r)), stringp (ra)))
    {
        for (r = cdr (r); consp (r); r = cdr (r))
        {
            ra = sx_join (ra, car (r), sx_nil);
        }

        return ra;
    }
    else
    {
        return cons (sym_object, r);
    }
}

static sexpr debug (sexpr arguments, sexpr *env)
{
    return cons (sym_debug, cons (ddebug, arguments));
}

static sexpr configure (sexpr arguments, sexpr *env)
{
    static char s = 0;
    if (!s)
    {
        relay_spawn (arguments);
        s = 1;
    }

    return sx_nonexistent;
}

void initialise_khonsu ()
{
    static char initialised = 0;

    if (!initialised)
    {
        multiplex_sexpr ();
        initialise_seteh ();

        kho_stdio = sx_open_stdio();

        gc_add_root (&ddebug);
        gc_add_root (&kho_configuration);
        gc_add_root (&kho_environment);

        kho_environment = lx_make_environment(sx_end_of_list);
        kho_environment = lx_environment_bind
                (kho_environment, sym_reply,
                 lx_foreign_lambda (sym_reply, reply));
        kho_environment = lx_environment_bind
                (kho_environment, sym_request,
                 lx_foreign_lambda (sym_request, request));
        kho_environment = lx_environment_bind
                (kho_environment, sym_object,
                 lx_foreign_lambda (sym_object, object));
        kho_environment = lx_environment_bind
                (kho_environment, sym_debug,
                 lx_foreign_lambda (sym_debug, debug));

        multiplex_add_sexpr (kho_stdio, sx_stdio_read, (void *)0);

        if (curie_argv[1])
        {
            struct sexpr_io * i = sx_open_io (io_open_read (curie_argv[1]),
                                              io_open_null);
            sexpr r, c = sx_end_of_list;

            while (!eofp (r = sx_read (i)))
            {
                if (!nexp (r))
                {
                    c = cons (r, c);
                }
            }

            relay_spawn (c);
        }
        else
        {
            kho_environment = lx_environment_bind
                    (kho_environment, sym_configure,
                     lx_foreign_lambda (sym_configure, configure));
        }

        initialised = 1;
    }
}

void kho_debug (sexpr sx)
{
    ddebug = cons (sx, ddebug);
}

void kho_configure (sexpr sx)
{
    kho_configuration = cons (sx, kho_configuration);
    if (kho_configure_callback != (void *)0)
    {
        kho_configure_callback (sx);
    }
}
