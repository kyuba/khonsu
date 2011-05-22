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
#include <curie/signal.h>
#include <curie/gc.h>
#include <curie/main.h>
#include <curie/memory.h>

struct stdio_list
{
    struct sexpr_io  *io;
    struct stdio_list *next;
};

static sexpr ddebug                      = sx_end_of_list;
sexpr kho_environment                    = sx_end_of_list;
sexpr kho_configuration                  = sx_end_of_list;
static sexpr kho_configuration_t         = sx_end_of_list;
struct sexpr_io *kho_stdio               = (struct sexpr_io *)0;
void (*kho_configure_callback)(sexpr)    = (void *)0;
static struct stdio_list *kho_stdio_list = (struct stdio_list *)0;
unsigned int kho_options                 = 0;

static void sx_stdio_read (sexpr sx, struct sexpr_io *io, void *aux)
{
    if (eofp (sx))
    {
        kho_unregister_output (io);
    }
    else
    {
        lx_eval (sx, kho_environment);
    }
}

static sexpr reply (sexpr arguments, struct machine_state *st)
{
    define_string (str_sb, "");
    sexpr rv = sx_end_of_list, re = lx_make_environment(sx_end_of_list), sv,
          menv = st->environment, rp, tag = car (arguments);
    struct stdio_list *l = kho_stdio_list;

    sv = str_sb;

    arguments = cdr (arguments);

    while (consp(arguments))
    {
        sexpr a = car (arguments);
        if (environmentp (a))
        {
            menv = lx_environment_join (menv, a);
            re = lx_environment_join (re, a);
        }
        else if (stringp (a))
        {
            sv = sx_join (sv, a, sx_nonexistent);
        }
        else if (consp (a))
        {
            sexpr r = car (a);

            if (environmentp (r))
            {
                menv = lx_environment_join (menv, r);
                re   = lx_environment_join (re,   r);

                rv = cons (cdr (a), rv);
            }
            else if (symbolp (r))
            {
                sexpr ra = lx_environment_lookup (menv, r);
                if (nexp (ra))
                {
                    rv = cons (a, rv);
                }
                else
                {
                    sexpr ra;
                    r = lx_eval (a, menv);

                    ra = car (r);

                    if (environmentp (ra))
                    {
                        while (consp (r))
                        {
                            ra = car (r);

                            if (environmentp (ra))
                            {
                                menv = lx_environment_join (menv, ra);
                                re   = lx_environment_join (re,   ra);
                            }
                            else
                            {
                                rv = cons (ra, rv);
                            }

                            r = cdr (r);
                        }
                    }
                    else
                    {
                        rv = cons (r, rv);
                    }
                }
            }
            else
            {
                rv = cons (lx_eval (a, menv), rv);
            }
        }
        else
        {
            rv = cons (a, rv);
        }

        arguments = cdr (arguments);
    }

    rp = cons (sym_reply, cons (tag, cons (re, cons (sv, sx_reverse (rv)))));

    while (l != (struct stdio_list *)0)
    {
        sx_write (l->io, rp);
        l = l->next;
    }

    if (kho_options & KHO_QUIT)
    {
        cexit (0);
    }

    return sx_end_of_list;
}

static sexpr request (sexpr arguments, struct machine_state *st)
{
    relay_sub (arguments);
    return sx_end_of_list;
}

static sexpr verbatim (sexpr arguments, struct machine_state *st)
{
    return cons (sym_verbatim, arguments);
}

static sexpr object_sub (sexpr arguments, sexpr env)
{
    if (consp (arguments))
    {
        sexpr t = car (arguments), r = sx_end_of_list, tx, at;

        if (truep (equalp (t, sym_object)))
        {
            arguments = cdr (arguments);
            t = car (arguments);
        }

        arguments = cdr (arguments);

        tx = lx_eval (t, env);
        if (primitivep (tx))
        {
            return lx_eval (cons (tx, arguments), env);
        }

        at = car (arguments);
        if (environmentp (at))
        {
            arguments = cdr (arguments);
        }
        else
        {
            at = sx_nonexistent;
        }

        while (consp (arguments))
        {
            r = cons (object_sub (car (arguments), env), r);
            arguments = cdr (arguments);
        }

        t = object_sub (t, env);

        if (symbolp (t) && !nexp (lx_environment_lookup (env, t)))
        {
            return lx_eval (cons (t, sx_reverse (r)), env);
        }

        if (!nexp (at))
        {
            return cons (t, cons (at, sx_reverse (r)));
        }
        else
        {
            return cons (t, sx_reverse (r));
        }
    }

    return arguments;
}

static sexpr object (sexpr arguments, struct machine_state *st)
{
    sexpr r = object_sub (arguments, st->environment), ra;
    if (consp (r) && ((ra = car (r)), stringp (ra)))
    {
        return kho_merge (r);
    }
    else
    {
        return cons (sym_object, r);
    }
}

static sexpr debug (sexpr arguments, struct machine_state *st)
{
    return cons (sym_debug, cons (ddebug, arguments));
}

static sexpr quit (sexpr arguments, struct machine_state *st)
{
    if (!(kho_options & KHO_IGNORE_QUIT))
    {
        if (kho_options & KHO_IGNORE_QUIT_RELOAD)
        {
            kho_options &= ~KHO_IGNORE_QUIT_RELOAD;

            kho_load ();

            return sym_ok;
        }

        kho_options |= KHO_QUIT;
    }

    return cons (sym_quit, sx_end_of_list);
}

static sexpr configure (sexpr arguments, struct machine_state *st)
{
    static char s = 0;
    if (!s)
    {
        relay_spawn (arguments);
        s = 1;
    }

    return sx_end_of_list;
}

static sexpr tick (sexpr arguments, struct machine_state *st)
{
    return sym_error;
}

static sexpr reload (sexpr arguments, struct machine_state *st)
{
    if (!(kho_options & KHO_IGNORE_QUIT_RELOAD))
    {
        kho_options |=  KHO_IGNORE_QUIT_RELOAD;
        relay_sub (sx_list2 (make_integer (1), sx_list1 (sym_quit)));
    }

    return sym_ok;
}

static sexpr terminate (sexpr arguments, struct machine_state *st)
{
    relay_sub (sx_list2 (make_integer (1), sx_list1 (sym_quit)));
    return sym_ok;
}

static sexpr invoke_gc (sexpr arguments, struct machine_state *st)
{
    gc_invoke ();
    return sx_list1 (sym_invoke_gc);
}


static void ik_on_read (sexpr sx, struct sexpr_io *io, void *aux)
{
    if (eofp (sx))
    {
        sexpr c = kho_configuration_t;

/*        c = cdr (c); */
        
        kho_configuration_t = sx_end_of_list;

        relay_spawn (c);
    }
    else
    {
        kho_configuration_t = cons (sx, kho_configuration_t);
    }
}

void initialise_khonsu ()
{
    static char initialised = 0;

    if (!initialised)
    {
        multiplex_sexpr ();
        initialise_seteh ();
        multiplex_signal ();

        kho_register_output (kho_stdio = sx_open_stdio());

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
        kho_environment = lx_environment_bind
                (kho_environment, sym_verbatim,
                 lx_foreign_lambda (sym_verbatim, verbatim));
        kho_environment = lx_environment_bind
                (kho_environment, sym_quit,
                 lx_foreign_lambda (sym_quit, quit));
        kho_environment = lx_environment_bind
                (kho_environment, sym_reload,
                 lx_foreign_lambda (sym_reload, reload));
        kho_environment = lx_environment_bind
                (kho_environment, sym_tick,
                 lx_foreign_lambda (sym_tick, tick));
        kho_environment = lx_environment_bind
                (kho_environment, sym_terminate,
                 lx_foreign_lambda (sym_terminate, terminate));
        kho_environment = lx_environment_bind
                (kho_environment, sym_invoke_gc,
                 lx_foreign_lambda (sym_invoke_gc, invoke_gc));

        kho_load ();

        initialised = 1;
    }
}

void kho_load ()
{
    kho_configuration = sx_end_of_list;

    if (curie_argv[1])
    {
        struct sexpr_io * i = sx_open_i (io_open_read (curie_argv[1]));

        multiplex_add_sexpr (i, ik_on_read, (void *)0);

    }
    else
    {
        sexpr s = lx_environment_lookup (kho_environment, sym_configure);

        if (nexp (s))
        {
            kho_environment = lx_environment_bind
                    (kho_environment, sym_configure,
                     lx_foreign_lambda (sym_configure, configure));
        }
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

void kho_register_output (struct sexpr_io *out)
{
    static struct memory_pool pool =
            MEMORY_POOL_INITIALISER(sizeof (struct stdio_list));
    struct stdio_list *l = get_pool_mem (&pool);

    if (l != (void *)0)
    {
        multiplex_add_sexpr (out, sx_stdio_read, (void *)0);

        l->io          = out;
        l->next        = kho_stdio_list;
        kho_stdio_list = l;
    }
}

void kho_unregister_output (struct sexpr_io *out)
{
    struct stdio_list *l  = kho_stdio_list;
    struct stdio_list **p = &kho_stdio_list;

    while (l != (struct stdio_list *)0)
    {
        if (l->io == out)
        {
            *p = l->next;
            free_pool_mem (l);
            return;
        }

        p = &(l->next);
        l = l->next;
    }
}
