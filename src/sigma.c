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

define_symbol (sym_menu,                       "menu");
define_symbol (sym_list,                       "list");
define_symbol (sym_link,                       "link");
define_symbol (sym_src,                        "src");
define_symbol (sym_href,                       "href");
define_symbol (sym_class,                      "class");
define_symbol (sym_item,                       "item");
define_symbol (sym_image,                      "image");
define_symbol (sym_extension,                  "extension");
define_symbol (sym_base_name,                  "base-name");
define_symbol (sym_section,                    "section");
define_symbol (sym_wrap,                       "wrap");
define_symbol (sym_contact,                    "contact");
define_symbol (sym_contact_elaborate,          "contact-elaborate");
define_symbol (sym_document,                   "document");
define_symbol (sym_first_name,                 "first-name");
define_symbol (sym_last_name,                  "last-name");
define_symbol (sym_icon,                       "icon");
define_symbol (sym_short_description,          "short-description");
define_symbol (sym_sub_section,                "sub-section");
define_string (str_selected,                   "selected");
define_string (str_menu,                       "menu");
define_string (str_slash,                      "/");
define_string (str_scontacts,                  "/contact/");
define_string (str_contact_dash,               "contact-");
define_string (str_space,                      " ");
define_string (str_dot,                        ".");
define_string (str_icon,                       "icon");
define_string (str_Contact,                    "Contact");
define_string (str_dot_ksu,                    ".ksu");
define_string (str_png_icon_no_picture_png,    "png/icon-no-picture.png");

static sexpr webroot          = sx_nonexistent;
static sexpr mime_map         = sx_nonexistent;

static void configure_callback (sexpr sx)
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

static sexpr include_file (sexpr fn, sexpr env)
{
    struct sexpr_io *io = sx_open_io (io_open_read (sx_string (fn)),
                                      io_open_null);
    sexpr data = sx_end_of_list, n, r,
          tje = lx_environment_join (kho_environment, env);

    while (!eofp (r = sx_read (io)))
    {
        if (!nexp (r))
        {
            n = car (r);
            if (truep (equalp (sym_object, n)))
            {
                tje = lx_environment_join
                        (kho_environment, env);
                data = cons (lx_eval (r, &tje), data);
            }
            else
            {
                data = cons (r, data);
            }
        }
    }

    sx_close_io (io);

    return sx_reverse (data);
}

static sexpr include_menu (sexpr file, sexpr env)
{
    sexpr lcodes = lx_environment_lookup (env, sym_language), fn;

    while (consp (lcodes))
    {
        fn = sx_join (webroot, str_slash,
                      sx_join (car (lcodes), str_slash,
                               sx_join (file, str_dot_ksu, str_nil)));

        if (truep (filep (fn)))
        {
            return include_file (fn, env);
        }

        lcodes = cdr (lcodes);
    }

    return sx_nonexistent;
}

static sexpr sub_menu (sexpr arguments, sexpr *env)
{
    sexpr sx = sx_end_of_list, target, name, t, e, title;

    title     = car (arguments);
    arguments = cdr (arguments);

    while (consp (arguments))
    {
        t      = car (arguments);
        target = car (t);
        name   = car (cdr (t));

        if (truep (equalp (target,
            lx_environment_lookup (*env, sym_base_name))))
        {
            e  = lx_make_environment
                    (cons (cons (sym_class, str_selected), sx_end_of_list));
        }
        else
        {
            e  = lx_make_environment (sx_end_of_list);
        }

        sx = cons (cons (sym_item, cons (e, cons (cons (sym_link, cons
                (lx_make_environment (cons (cons (sym_href, sx_join (target,
                 str_dot, lx_environment_lookup (*env, sym_extension))),
                        sx_end_of_list)), cons (name, sx_end_of_list))),
                        sx_end_of_list))), sx);

        arguments = cdr (arguments);
    }

    return cons (cons (sym_sub_section, cons (title,
                 cons (cons (sym_list, sx_reverse (sx)), sx_end_of_list))),
                       sx_end_of_list);
}

static sexpr menu (sexpr arguments, sexpr *env)
{
    sexpr sx = sx_end_of_list, title;

    if (eolp (arguments))
    {
        return include_menu (title, *env);
    }

    title     = car (arguments);
    arguments = cdr (arguments);

    if (eolp (arguments))
    {
        return include_menu (title, *env);
    }

    while (consp (arguments))
    {
        sx = cons (sub_menu (car (arguments), env), sx);

        arguments = cdr (arguments);
    }

    return cons (sym_wrap, cons (lx_make_environment (cons (cons (sym_id,
             str_menu), sx_end_of_list)), cons (cons (sym_section, cons (title,
             cons (sx_reverse (sx), sx_end_of_list))), sx_end_of_list)));
}

static sexpr include_contact (sexpr file, sexpr env)
{
    sexpr fn  = sx_join (webroot, str_scontacts,
                         sx_join (file, str_dot_ksu, str_nil));

    if (truep (filep (fn)))
    {
        return include_file (fn, env);
    }

    return cons (sym_contact, cons (file, sx_end_of_list));
}

static sexpr contact (sexpr args, sexpr *env)
{
    sexpr a = car (args);
    args    = cdr (args);

    if (eolp (args))
    {
        return include_contact (a, *env);
    }
    else
    {
        sexpr t, v, icon = str_png_icon_no_picture_png,
              fn = sx_nonexistent, ln = sx_nonexistent, sdesc = sx_nonexistent,
              n = a, te;

        while (consp (args))
        {
            a    = car (args);
            t    = car (a);
            v    = car (cdr (a));

            if (truep (equalp (t, sym_first_name)))
            {
                fn = v;
            }
            else if (truep (equalp (t, sym_last_name)))
            {
                ln = v;
            }
            else if (truep (equalp (t, sym_icon)))
            {
                icon = v;
            }
            else if (truep (equalp (t, sym_short_description)))
            {
                sdesc = v;
            }

            args = cdr (args);
        }

        te = lx_environment_join (kho_environment, *env);

        return lx_eval (cons (sym_object, cons (cons (sym_icon,
                 cons (sx_join (str_contact_dash, n,
                       sx_join (str_dot,
                                lx_environment_lookup (*env, sym_extension),
                                str_nil)),
                 cons (sx_join (fn, str_space, ln),
                 cons (icon,
                 cons (sdesc, sx_end_of_list))))),
                 sx_end_of_list)), &te);
    }
}

static sexpr contact_elaborate (sexpr args, sexpr *env)
{
    sexpr a = car (args);
    args    = cdr (args);

    if (eolp (args))
    {
        return include_contact (a, *env);
    }
    else
    {
        sexpr t, v, icon = str_png_icon_no_picture_png,
              fn = sx_nonexistent, ln = sx_nonexistent, sdesc = sx_nonexistent,
              te;

        while (consp (args))
        {
            a    = car (args);
            t    = car (a);
            v    = car (cdr (a));

            if (truep (equalp (t, sym_first_name)))
            {
                fn = v;
            }
            else if (truep (equalp (t, sym_last_name)))
            {
                ln = v;
            }
            else if (truep (equalp (t, sym_icon)))
            {
                icon = v;
            }
            else if (truep (equalp (t, sym_short_description)))
            {
                sdesc = v;
            }

            args = cdr (args);
        }

        te = lx_environment_join (kho_environment, *env);

        return lx_eval (cons (sym_object, cons (cons (sx_join (fn, str_space,
            ln), cons (sdesc, cons (cons (sym_image, cons (icon,
            sx_end_of_list)), sx_end_of_list))), sx_end_of_list)), &te);
    }
}

static sexpr include_contact_document (sexpr to, sexpr e, sexpr re)
{
    sexpr te, type, tf, res = sx_end_of_list, tje;
    int len = 0, i = 0;
    const char *ts = sx_string (to);
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

        te   = make_string (tmp);
        type = make_string (ts + i);

        afree (i, tmp);

        e  = lx_environment_bind
                (e, sym_base_name,
                 sx_join (str_contact_dash, te, str_nil));
        e  = lx_environment_bind (e, sym_extension, type);

        tf = lx_environment_lookup(mime_map, type);
        e  = lx_environment_bind (e, sym_format, tf);
    }

    tje = lx_environment_join (kho_environment, e);

    tje = lx_environment_unbind (tje, sym_contact);
    tje = lx_environment_bind
            (tje, sym_contact,
             lx_foreign_lambda (sym_contact_elaborate, contact_elaborate));

    res = lx_eval
            (cons (sym_object, cons (cons (sym_document, cons (str_Contact, cons
                (cons (sym_menu, sx_end_of_list),
                       cons (cons (sym_contact, cons (te, sx_end_of_list)),
                             sx_end_of_list)))), sx_end_of_list)),
             &tje);

    return cons (res, cons (e, re));
}

static sexpr request (sexpr arguments, sexpr *env)
{
    sexpr a = arguments, r = sx_end_of_list, a2, a3;
    char get_handled = 0;

    while (consp (a))
    {
        a2 = car (a);
        a3 = car (a2);

        if (truep (equalp (a3, sym_get)))
        {
            sexpr t1 = cdr (a2), te = car (t1), t2 = cdr (t1), target = car(t2);
            const char *etarget = sx_string (target);

            while (etarget[0] == '/')
            {
                etarget++;
            }

            if ((etarget[0]=='c') && (etarget[1]=='o') && (etarget[2]=='n') &&
                (etarget[3]=='t') && (etarget[4]=='a') && (etarget[5]=='c') &&
                (etarget[6]=='t') && (etarget[7]=='-'))
            {
                get_handled = 1;
                r = include_contact_document (make_string(etarget + 8), te, r);
            }
            else
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

    if (!get_handled)
    {
        relay_sub (arguments);
        return sx_nonexistent;
    }

    return lx_eval (cons (sym_reply, sx_reverse (r)), &kho_environment);
}

int cmain ()
{
    terminate_on_allocation_errors ();

    kho_configure_callback = configure_callback;

    initialise_khonsu ();

    gc_add_root (&mime_map);

    kho_environment = lx_environment_bind
      (kho_environment, sym_menu, lx_foreign_lambda (sym_menu, menu));
    kho_environment = lx_environment_bind
      (kho_environment, sym_contact, lx_foreign_lambda (sym_contact, contact));
    kho_environment = lx_environment_bind
      (kho_environment, sym_request, lx_foreign_lambda (sym_request, request));

    kho_debug (make_symbol ("sigma"));

    while (multiplex () != mx_nothing_to_do)
    {
//        gc_invoke();
    }

    return 0;
}
