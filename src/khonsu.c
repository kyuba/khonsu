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
#include <khonsu/relay.h>
#include <seteh/lambda.h>
#include <curie/multiplex.h>
#include <curie/gc.h>
#include <curie/main.h>

static sexpr debug      = sx_end_of_list;
sexpr kho_environment   = sx_end_of_list;
sexpr kho_configuration = sx_end_of_list;

struct sexpr_io *kho_stdio;

static void sx_stdio_read (sexpr sx, struct sexpr_io *io, void *aux)
{
    lx_eval (sx, &kho_environment);
}

static sexpr get (sexpr arguments, sexpr *env)
{
}

static sexpr reply (sexpr arguments, sexpr *env)
{
    sx_write (kho_stdio, arguments);
}

static sexpr request (sexpr arguments, sexpr *env)
{
    relay_sub (cdr (arguments));
    return sx_nonexistent;
}

static sexpr object (sexpr arguments, sexpr *env)
{
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

        kho_stdio = sx_open_stdio();

        gc_add_root (&debug);
        gc_add_root (&kho_configuration);

        kho_environment = lx_make_environment(sx_end_of_list);
        kho_environment = lx_environment_bind
                (kho_environment, sym_get,
                 lx_foreign_lambda (sym_get, get));
        kho_environment = lx_environment_bind
                (kho_environment, sym_reply,
                 lx_foreign_lambda (sym_reply, reply));
        kho_environment = lx_environment_bind
                (kho_environment, sym_request,
                 lx_foreign_lambda (sym_request, request));
        kho_environment = lx_environment_bind
                (kho_environment, sym_object,
                 lx_foreign_lambda (sym_object, object));

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
    debug = cons (sx, debug);
}

void kho_configure (sexpr sx)
{
    kho_configuration = cons (sx, kho_configuration);
}
