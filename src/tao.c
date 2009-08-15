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
#include <curie/directory.h>
#include <curie/memory.h>
#include <curie/regex.h>
#include <curie/gc.h>

define_symbol (sym_tao_map_extension,        "tao:map-extension");
define_symbol (sym_tao_transform,            "tao:transform");
define_symbol (sym_tao_match,                "tao:match");
define_symbol (sym_tao_replace,              "tao:replace");
define_symbol (sym_tao_item,                 "tao:item");
define_symbol (sym_tao_tail,                 "tao:tail");
define_symbol (sym_tao_all,                  "tao:all");

define_symbol (sym_tao_profiles,             "tao-profiles");
define_symbol (sym_tao_themes,               "tao-themes");

define_string (str_undefined_reference,      "undefined reference");

struct transformation
{
    sexpr conditions;
    sexpr replacements;
    struct transformation *next;
};

struct transformation *tao_profiles = (void *)0;
struct transformation *tao_themes   = (void *)0;

static void tao_add_transformation (struct transformation **t, sexpr sx)
{
    struct memory_pool pool
            = MEMORY_POOL_INITIALISER (sizeof (struct transformation));
    struct transformation * tr = get_pool_mem (&pool);
    sexpr c, ca, cd, cond, condx, da;

    if (tr == (void *)0)
    {
        return;
    }

    tr->conditions   = sx_end_of_list;
    tr->replacements = sx_end_of_list;
    tr->next         = *t;

    gc_add_root (&(tr->conditions));
    gc_add_root (&(tr->replacements));

    while (consp (sx))
    {
        c  = car (sx);
        ca = car (c);

        if (truep (equalp (ca, sym_tao_match)))
        {
            da = cdr (c);
            tr->conditions
                    = cons (cons (car (da), rx_compile_sx (cdr (da))),
                            tr->conditions);
        }
        else if (truep (equalp (ca, sym_tao_replace)))
        {
            cd = cdr (c);
            cond = car (cd);
            condx = sx_end_of_list;

            while (consp (cond))
            {
                condx = cons (rx_compile_sx (car (cond)), condx);

                cond = cdr (cond);
            }

            tr->replacements = cons (cons (condx, cdr (cd)), tr->replacements);
        }

        sx = cdr (sx);
    }

    tr->replacements = sx_reverse (tr->replacements);

    *t = tr;
}

static void configure_callback_work (struct transformation **t, sexpr sx)
{
    sexpr b = read_directory_sx (sx), d, da;
    struct sexpr_io *i;

    while (consp (b))
    {
        i = sx_open_io (io_open_read(sx_string (car (b))), io_open_null);

        while (!eofp (d = sx_read (i)))
        {
            if (consp (d))
            {
                da = car (d);

                if (truep (equalp (da, sym_tao_map_extension)))
                {
                    kho_configure (cons (sym_map_extension, cdr (d)));
                }
                else if (truep (equalp (da, sym_tao_transform)))
                {
                    tao_add_transformation (t, cdr (d));
                }
            }
        }

        sx_close_io (i);

        b = cdr (b);
    }
}

static void configure_callback (sexpr sx)
{
    sexpr a = car (sx);

    if (truep (equalp (a, sym_tao_profiles)))
    {
        configure_callback_work (&tao_profiles, car (cdr (sx)));
    }
    else if (truep (equalp (a, sym_tao_themes)))
    {
        configure_callback_work (&tao_themes, car (cdr (sx)));
    }
}

static sexpr apply_replacement (sexpr rx, sexpr node, sexpr path, sexpr env)
{
    sexpr c = rx, d = sx_end_of_list, type, t, ta,
          e = lx_make_environment (sx_end_of_list);

    if (!consp (rx))
    {
        return rx;
    }

    type = car (c);
    if (symbolp (type))
    {
        c = cdr (c);
    }

    while (consp (c))
    {
        t = car (c);
        if (environmentp (t))
        {
            e = lx_environment_join (e, t);
        }
        else
        {
            t = apply_replacement (t, node, sx_end_of_list, env);

            if (consp (t))
            {
                ta = car (t);

                if (symbolp (ta))
                {
                    d = cons (t, d);
                }
                else
                {
                    while (consp (t))
                    {
                        ta = car (t);

                        if (environmentp (ta))
                        {
                            e = lx_environment_join (e, ta);
                        }
                        else
                        {
                            d = cons (ta, d);
                        }

                        t = cdr (t);
                    }
                }
            }
            else if (environmentp (t))
            {
                e = lx_environment_join (e, t);
            }
            else
            {
                d = cons (t, d);
            }
        }

        c = cdr (c);
    }

    d = sx_reverse (d);

    if (symbolp (type))
    {
        if (truep (equalp (sym_tao_all, type)))
        {
            return node;
        }
        else if (truep (equalp (sym_tao_tail, type)))
        {
            int n = sx_integer (car (cdr (rx)));
            c = node;

            while ((n > 0) && consp (c))
            {
                c = cdr (c);
                n--;
            }

            return c;
        }
        else if (truep (equalp (sym_tao_item, type)))
        {
            int n = sx_integer (car (cdr (rx)));
            c = node;

            while ((n > 0) && consp (c))
            {
                c = cdr (c);
                n--;
            }

            if (consp (c))
            {
                return car (c);
            }
            else
            {
                return str_undefined_reference;
            }
        }

        return cons (type, cons (e, d));
    }
    else
    {
        return cons (e, d);
    }
}

static sexpr apply_transformation
        (struct transformation *trb, sexpr arguments, sexpr path, sexpr env)
{
    struct transformation *tr = trb;
    sexpr c, d, da, db, rc, rx, pc, type, t,
          e = lx_make_environment (sx_end_of_list);
    char good;

    if (!consp (arguments))
    {
        return arguments;
    }

    type = car (arguments);

    if (symbolp (type))
    {
        path = cons (make_string (sx_symbol (type)), path);
        arguments = cdr (arguments);
    }
    else
    {
        d  = sx_end_of_list;
        da = arguments;

        while (consp (da))
        {
            t = car (da);
            if (environmentp (t))
            {
                e = lx_environment_join (e, t);
            }
            else
            {
                d = cons (apply_transformation (trb, t, path, env), d);
            }

            da = cdr (da);
        }

        return cons (e, sx_reverse (d));
    }

    while (tr != (void *)0)
    {
        good = 1;

        for (c = tr->conditions; good && consp (c); c = cdr (c))
        {
            d  = car (c);
            da = lx_environment_lookup (env, car (d));
            db = cdr (d);

            if (!nexp (da))
            {
                if (falsep (rx_match_sx (cdr (d), da)))
                {
                    good = 0;
                }
            }
        }

        if (good)
        {
            c  = tr->replacements;
            d  = sx_end_of_list;

            for (good = 1; consp (c); (good = 1), (c = cdr (c)))
            {
                da = car (c);
                rc = car (da);
                pc = path;
                rx = cdr (da);

                while (good && consp (rc) && consp (pc))
                {
                    if (falsep (rx_match_sx (car (rc), car (pc))))
                    {
                        good = 0;
                    }

                    rc = cdr (rc);
                    pc = cdr (pc);
                }

                if (consp (rc) && !consp (pc))
                {
                    good = 0;
                }

                if (good)
                {
                    c = arguments;
                    d = sx_end_of_list;
                    e = lx_make_environment (sx_end_of_list);

                    while (consp (c))
                    {
                        t = car (c);
                        if (environmentp (t))
                        {
                            e = lx_environment_join (e, t);
                        }
                        else
                        {
                            d = cons (apply_transformation (trb, t,
                                      sx_end_of_list, env), d);
                        }

                        c = cdr (c);
                    }

                    return apply_replacement (rx, cons (e, sx_reverse (d)),
                                              path, env);
                }
            }
        }

        tr = tr->next;
    }

    d  = sx_end_of_list;
    da = arguments;

    while (consp (da))
    {
        t = car (da);
        if (environmentp (t))
        {
            e = lx_environment_join (e, t);
        }
        else
        {
            d = cons (apply_transformation (trb, t, path, env), d);
        }

        da = cdr (da);
    }

    return cons (type, cons (e, sx_reverse (d)));
}

static sexpr object_sub (sexpr arguments, sexpr path, sexpr env)
{
    return apply_transformation
            (tao_themes,
             apply_transformation (tao_profiles, arguments, path, env),
             path, env);
}

static sexpr object (sexpr arguments, sexpr *env)
{
    return cons (sym_object, object_sub (arguments, sx_end_of_list, *env));
}

int cmain ()
{
    terminate_on_allocation_errors ();

    kho_configure_callback = configure_callback;

    initialise_khonsu ();

    kho_environment = lx_environment_unbind (kho_environment, sym_object);

    kho_environment = lx_environment_bind
            (kho_environment, sym_object,
             lx_foreign_lambda (sym_object, object));

    kho_debug (make_symbol ("tao"));

    while (multiplex () != mx_nothing_to_do)
    {
//        gc_invoke();
    }

    return 0;
}
