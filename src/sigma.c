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
define_symbol (sym_first_name,                 "first-name");
define_symbol (sym_last_name,                  "last-name");
define_symbol (sym_icon,                       "icon");
define_string (str_selected,                   "selected");
define_string (str_menu,                       "menu");
define_string (str_slash,                      "/");
define_string (str_scontacts,                  "/contact/");
define_string (str_space,                      " ");
define_string (str_dot,                        ".");
define_string (str_icon,                       "icon");
define_string (str_dot_ksu,                    ".ksu");
define_string (str_contact_dash,               "contact-");
define_string (str_png_contact_no_picture_png, "png/contact-no-picture.png");

static sexpr webroot          = sx_nonexistent;

static void configure_callback (sexpr sx)
{
    sexpr a = car (sx);

    if (truep (equalp (a, sym_root)))
    {
        webroot = car (cdr (sx));
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

static sexpr menu (sexpr arguments, sexpr *env)
{
    sexpr sx = sx_end_of_list, target, name, t, e, title;

    title     = car (arguments);
    arguments = cdr (arguments);

    if (eolp (arguments))
    {
        return include_menu (title, *env);
    }

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

    return cons (sym_wrap, cons (lx_make_environment (cons (cons (sym_id,
             str_menu), sx_end_of_list)), cons (cons (sym_section, cons (title,
             cons (cons (sym_list, sx_reverse (sx)), sx_end_of_list))),
             sx_end_of_list)));
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
        sexpr t, v, icon = str_png_contact_no_picture_png,
              fn = sx_nonexistent, ln = sx_nonexistent, n = a, te;

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

            args = cdr (args);
        }

        te = lx_environment_join (kho_environment, *env);

        return lx_eval (cons (sym_object, cons (cons (sym_icon,
                 cons (sx_join (str_contact_dash, n,
                       sx_join (str_dot,
                                lx_environment_lookup (*env, sym_extension),
                                str_nil)),
                 cons (sx_join (fn, str_space, ln),
                 cons (str_png_contact_no_picture_png, sx_end_of_list)))),
                 sx_end_of_list)), &te);
    }
}

static sexpr icon (sexpr args, sexpr *env)
{
    sexpr ta = car (args), d = cdr (args), nm = car (d), ic = car (cdr (d));

    return cons (sym_wrap, cons (lx_make_environment (cons (cons (sym_class,
      str_icon), sx_end_of_list)), cons (cons (sym_link, cons
      (lx_make_environment (cons (cons (sym_href, ta), sx_end_of_list)), cons
      (cons (sym_image, cons (lx_make_environment (cons (cons (sym_src, ic),
      sx_end_of_list)), sx_end_of_list)), sx_end_of_list))), cons (cons
      (sym_link, cons (lx_make_environment (cons (cons (sym_href, ta),
      sx_end_of_list)), cons (nm, sx_end_of_list))), sx_end_of_list))));
}

int cmain ()
{
    terminate_on_allocation_errors ();

    kho_configure_callback = configure_callback;

    initialise_khonsu ();

    kho_environment = lx_environment_bind
      (kho_environment, sym_menu, lx_foreign_lambda (sym_menu, menu));
    kho_environment = lx_environment_bind
      (kho_environment, sym_contact, lx_foreign_lambda (sym_contact, contact));
    kho_environment = lx_environment_bind
      (kho_environment, sym_icon, lx_foreign_lambda (sym_icon, icon));

    kho_debug (make_symbol ("sigma"));

    while (multiplex () != mx_nothing_to_do)
    {
//        gc_invoke();
    }

    return 0;
}
