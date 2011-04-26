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
#include <sievert/sexpr.h>
#include <curie/multiplex.h>
#include <curie/memory.h>
#include <curie/filesystem.h>
#include <curie/time.h>
#include <curie/gc.h>

/*
define_symbol (sym_elaborate,                  "elaborate");

define_string (str_contact_dash,               "contact-");
define_string (str_contact_slash,              "contact/");
*/

static sexpr webroot          = sx_nonexistent;

static void configure_callback (sexpr sx)
{
    sexpr a = car (sx);

    if (truep (equalp (a, sym_root)))
    {
        webroot = car (cdr (sx));
    }
}

static sexpr request (sexpr arguments, struct machine_state *st)
{
    sexpr a = arguments, r = sx_end_of_list, a2, a3;

    while (consp (a))
    {
        a2 = car (a);
        a3 = car (a2);

        if (truep (equalp (a3, sym_get)))
        {
            sexpr t1 = cdr (a2), te = car (t1), t2 = cdr (t1), target = car(t2);
            const char *etarget = sx_string (target);

/*            if (environmentp (te))
            {
                te = lx_environment_bind (te, sym_elaborate, sx_true);
            }
            else
            {
                te = lx_make_environment
                    (sx_list1 (cons (sym_elaborate, sx_true)));
            }*/

/*            if ((etarget[0]=='c') && (etarget[1]=='o') && (etarget[2]=='n') &&
                (etarget[3]=='t') && (etarget[4]=='a') && (etarget[5]=='c') &&
                (etarget[6]=='t') && (etarget[7]=='-'))
            {
                a2 = sx_join (str_contact_slash,make_string(etarget+8),str_nil);
                r = cons (sx_list3 (sym_get, te, a2), r);
            }
            else*/
            {
                r = cons (a2, r);
            }
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
