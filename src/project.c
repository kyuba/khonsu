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
define_symbol (sym_document,                   "document");
define_symbol (sym_icon,                       "icon");
define_symbol (sym_description,                "description");
define_symbol (sym_short_description,          "short-description");
define_symbol (sym_sub_section,                "sub-section");
define_symbol (sym_elaborate,                  "elaborate");
define_symbol (sym_project,                    "project");
define_string (str_selected,                   "selected");
define_string (str_menu,                       "menu");
define_string (str_slash,                      "/");
define_string (str_sprojects,                  "/project/");
define_string (str_project_dash,               "project-");
define_string (str_project_slash,              "project/");
define_string (str_space,                      " ");
define_string (str_dot,                        ".");
define_string (str_icon,                       "icon");
define_string (str_Project,                    "Project");
define_string (str_dot_ksu,                    ".ksu");
define_string (str_png_icon_no_picture_png,    "png/icon-no-picture.png");

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

static sexpr include_project (sexpr file, sexpr env)
{
    sexpr fn  = sx_join (webroot, str_sprojects,
                         sx_join (file, str_dot_ksu, str_nil));

    if (truep (filep (fn)))
    {
        return include_file (fn, env);
    }

    return cons (sym_project, cons (file, sx_end_of_list));
}

static sexpr project_elaborate (sexpr args, sexpr *env)
{
    sexpr pn = car (args);
    args     = cdr (args);

    if (eolp (args))
    {
        return include_project (pn, *env);
    }
    else
    {
        sexpr t, v, icon = str_png_icon_no_picture_png, desc = sx_nonexistent,
              sdesc = sx_nonexistent, a;

        while (consp (args))
        {
            a    = car (args);
            t    = car (a);
            v    = car (cdr (a));

            if (truep (equalp (t, sym_icon)))
            {
                icon = v;
            }
            else if (truep (equalp (t, sym_description)))
            {
                desc = cdr (a);
            }
            else if (truep (equalp (t, sym_short_description)))
            {
                sdesc = cdr (a);
            }

            args = cdr (args);
        }

        return cons (sym_object, cons (cons (sym_document, cons (sx_join
            (str_Project, str_space, pn), cons (cons (sym_menu, sx_end_of_list),
            cons (desc, cons (cons (sym_image, cons (icon, sx_end_of_list)),
            sx_end_of_list))))), sx_end_of_list));
    }
}

static sexpr project (sexpr args, sexpr *env)
{
    sexpr pn = car (args);
    args     = cdr (args);

    if (eolp (args))
    {
        return include_project (pn, *env);
    }
    else
    {
        sexpr t, v, icon = str_png_icon_no_picture_png, desc = sx_nonexistent,
              sdesc = sx_nonexistent, a, ext;

        if (truep (lx_environment_lookup (*env, sym_elaborate)))
        {
            return project_elaborate (cons (pn, args), env);
        }

        ext = lx_environment_lookup (*env, sym_extension);

        while (consp (args))
        {
            a    = car (args);
            t    = car (a);
            v    = car (cdr (a));

            if (truep (equalp (t, sym_icon)))
            {
                icon = v;
            }
            else if (truep (equalp (t, sym_description)))
            {
                desc = v;
            }
            else if (truep (equalp (t, sym_short_description)))
            {
                sdesc = v;
            }

            args = cdr (args);
        }

        return cons (sym_object, cons (cons (sym_icon,
                 cons (sx_join (str_project_dash, pn,
                       (nexp (ext) ? str_nil
                                   : sx_join (str_dot, ext, str_nil))),
                 cons (sx_join (str_Project, str_space, pn),
                 cons (icon,
                 cons (sdesc, sx_end_of_list))))),
                 sx_end_of_list));
    }
}

static sexpr request (sexpr arguments, sexpr *env)
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

            if (environmentp (te))
            {
                te = lx_environment_bind (te, sym_elaborate, sx_true);
            }
            else
            {
                te = lx_make_environment
                    (cons (cons (sym_elaborate, sx_true), sx_end_of_list));
            }

            if ((etarget[0]=='p') && (etarget[1]=='r') && (etarget[2]=='o') &&
                (etarget[3]=='j') && (etarget[4]=='e') && (etarget[5]=='c') &&
                (etarget[6]=='t') && (etarget[7]=='-'))
            {
                a2 = sx_join (str_project_slash,make_string(etarget+8),str_nil);
                r = cons (cons(sym_get,cons (te,cons (a2, sx_end_of_list))),r);
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

    relay_sub (sx_reverse (r));
    return sx_nonexistent;
}

int cmain ()
{
    terminate_on_allocation_errors ();

    kho_configure_callback = configure_callback;

    initialise_khonsu ();

    kho_environment = lx_environment_bind
      (kho_environment, sym_project, lx_foreign_lambda (sym_project, project));
    kho_environment = lx_environment_bind
      (kho_environment, sym_request, lx_foreign_lambda (sym_request, request));

    kho_debug (make_symbol ("sigma"));

    while (multiplex () != mx_nothing_to_do)
    {
//        gc_invoke();
    }

    return 0;
}
