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

define_string (str_xml_declaration,
  "<?xml version=\"1.1\" encoding=\"utf-8\" ?>\n");
define_string (str_doctype_xhtml,
  "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" "
  "\"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n");

define_symbol (sym_formal_paragraph,         "formal-paragraph");
define_symbol (sym_paragraph,                "paragraph");
define_symbol (sym_page,                     "page");
define_symbol (sym_html,                     "html");
define_symbol (sym_head,                     "head");
define_symbol (sym_title,                    "title");
define_symbol (sym_body,                     "body");
define_symbol (sym_h1,                       "h1");
define_symbol (sym_h2,                       "h2");
define_symbol (sym_h3,                       "h3");
define_symbol (sym_h4,                       "h4");
define_symbol (sym_h5,                       "h5");
define_symbol (sym_h6,                       "h6");
define_symbol (sym_p,                        "p");

define_symbol (sym_application_xhtml_p_xml,  "application/xhtml+xml");

define_symbol (sym_tao_map_extension,        "tao:map-extension");
define_symbol (sym_tao_transform,            "tao:transform");
define_symbol (sym_tao_match,                "tao:match");
define_symbol (sym_tao_replace,              "tao:replace");
define_symbol (sym_tao_item,                 "tao:item");
define_symbol (sym_tao_tail,                 "tao:tail");

define_symbol (sym_tao_profiles,             "tao-profiles");
define_symbol (sym_tao_themes,               "tao-themes");

define_string (str_xhtml,                    "xhtml");
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
    sexpr c = rx, d = sx_end_of_list, type, e;

    if (!consp (rx))
    {
        return rx;
    }

    type = car (rx);
    if (symbolp (type))
    {
        c = cdr (c);
    }

    e = car (rx);
    if (environmentp (e))
    {
        c = cdr (c);
    }
    else
    {
        e = lx_make_environment (sx_end_of_list);
    }

    while (consp (c))
    {
        d = cons (apply_replacement (car (c), node, sx_end_of_list, env), d);

        c = cdr (c);
    }

    d = sx_reverse (d);

    if (symbolp (type))
    {
        if (truep (equalp (sym_tao_tail, type)))
        {
            int n = sx_integer (car (cdr (rx))) - 1;
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
            int n = sx_integer (car (cdr (rx))) -1;
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
        return d;
    }
}

static sexpr object_sub (sexpr arguments, sexpr path, sexpr env)
{
    struct transformation *tr = tao_profiles;
    sexpr c, d, da, db, rc, rx, rcc, pc, type;
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
            d  = cons (object_sub (car (da), path, env), d);

            da = cdr (da);
        }

        return sx_reverse (d);
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

/*                sx_write (kho_stdio, cons (sym_reply, rc));
                sx_write (kho_stdio, cons (sym_reply, pc));
                sx_write (kho_stdio, cons (sym_reply, rx));*/

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
                    while (consp (c))
                    {
                        d = cons (object_sub (car (c), sx_end_of_list, env), d);
                        c = cdr (c);
                    }

//                    return sx_reverse (d);
                    return apply_replacement (rx, sx_reverse (d), path, env);
                }
            }
        }

        tr = tr->next;
    }

    d  = sx_end_of_list;
    da = arguments;

    while (consp (da))
    {
        d  = cons (object_sub (car (da), path, env), d);

        da = cdr (da);
    }

    return cons (type, sx_reverse (d));
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

    while (multiplex () != mx_nothing_to_do);

    return 0;
}
