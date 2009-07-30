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

#include <khonsu/relay.h>
#include <khonsu/khonsu.h>
#include <curie/exec.h>
#include <curie/shell.h>
#include <curie/main.h>
#include <curie/multiplex.h>

static struct sexpr_io *sxio = (struct sexpr_io *)0;

static void sx_io_read (sexpr sx, struct sexpr_io *io, void *aux)
{
/*    sx_write (kho_stdio, sx);*/
    lx_eval (sx, &kho_environment);
/*    sx_write (kho_stdio, sx);*/
}

void relay_spawn (sexpr configuration)
{
    sexpr target = sx_false;

    while (consp (configuration))
    {
        sexpr a  = car (configuration);
        sexpr a1 = car (a);

        if (truep(equalp(a1, sym_pipeline)))
        {
            sexpr b1 = cdr (a);
            sexpr b2 = cdr (b1);

            target = car (b1);

            if (consp (b2))
            {
                kho_configure (cons (sym_pipeline, b2));
            }
        }
        else
        {
            kho_configure (a);
        }

        configuration = cdr (configuration);
    }

    if (stringp (target))
    {
        sexpr filename = which (target);

        if (stringp(filename))
        {
            const char *f = sx_string (filename);
            char *p[] = { (char *)f, (char *)0 };
            struct exec_context *c = execute (0, p, curie_environment);

            switch (c->pid)
            {
                case -1:
                case 0:
                    break;
                default:
                    sxio = sx_open_io (c->in, c->out);

                    multiplex_add_sexpr (sxio, sx_io_read, (void *)0);

                    sx_write (sxio, cons (sym_configure, kho_configuration));

                    sx_write (kho_stdio, filename);
                    break;
            }
        }
    }
}

void relay_sub (sexpr request)
{
    if (sxio == (struct sexpr_io *)0)
    {
        lx_eval (cons (sym_reply, request), &kho_environment);
    }
    else
    {
        sx_write (sxio, cons (sym_request, request));
    }
}
