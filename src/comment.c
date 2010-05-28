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
#include <curie/time.h>

struct transdata
{
    sexpr environment;
    sexpr *data;
    int done;
};

define_symbol (sym_document,         "document");
define_symbol (sym_comment,          "comment");
define_symbol (sym_class,            "class");
define_symbol (sym_method,           "method");
define_symbol (sym_action,           "action");
define_symbol (sym_extension,        "extension");
define_symbol (sym_sub_section,      "sub-section");
define_symbol (sym_sub_sub_section,  "sub-sub-section");
define_symbol (sym_wrap,             "wrap");
define_symbol (sym_form,             "form");
define_symbol (sym_name,             "name");
define_symbol (sym_post,             "post");
define_symbol (sym_div,              "div");
define_symbol (sym_comment_added,    "comment-added");
define_symbol (sym_base_name,        "base-name");
define_symbol (sym_date,             "date");
define_symbol (sym_time,             "time");
define_symbol (sym_fieldset,         "fieldset");
define_symbol (sym_legend,           "legend");
define_symbol (sym_label,            "label");
define_symbol (sym_input,            "input");
define_symbol (sym_type,             "type");
define_symbol (sym_textarea,         "textarea");
define_symbol (sym_for,              "for");
define_symbol (sym_value,            "value");
define_symbol (sym_paragraph,        "paragraph");
define_symbol (sym_comment_name,     "comment:name");
define_symbol (sym_comment_text,     "comment:text");
define_symbol (sym_comment_source,   "comment:source");
define_symbol (sym_comment_target,   "comment:target");
define_symbol (sym_original_name,    "original-name");
define_symbol (sym_Location,         "Location");
define_string (str_dot,              ".");
define_string (str_dot_ksu,          ".ksu");
define_string (str_post,             "post");
define_string (str_put,              "put");
define_string (str_comment,          "comment");
define_string (str_comment_by_s,     "Comment by ");
define_string (str_scomments,        "/comment/");
define_string (str_Comments,         "Comments");
define_string (str_Add_Comment,      "Add Comment");
define_string (str_Message,          "Message");
define_string (str_Name,             "Name");
define_string (str_Anonymous,        "Anonymous");
define_string (str_submit,           "submit");
define_string (str_text_html,        "text/html");
define_string (str_text_xhtml,       "application/xhtml+xml");
define_string (str_comment_name,     "comment:name");
define_string (str_comment_text,     "comment:text");
define_string (str_comment_source,   "comment:source");
define_string (str_comment_target,   "comment:target");
define_string (str_hidden,           "hidden");

static sexpr webroot          = sx_nonexistent;

static void configure_callback (sexpr sx)
{
    sexpr a = car (sx);

    if (truep (equalp (a, sym_root)))
    {
        webroot = car (cdr (sx));
    }
}

static sexpr comment (sexpr args, struct machine_state *st)
{
    sexpr name = car (args), tt, td;

    args = cdr (args);

    td = car (args);

    args = cdr (args);

    tt = car (args);

    args = cdr (args);

    return cons (sym_wrap, cons (lx_make_environment (cons (cons (sym_class,
         str_comment), sx_end_of_list)), cons (cons (sym_sub_sub_section,
         cons (sx_join (str_comment_by_s, name, str_nil), cons (cons (sym_date,
         cons (td, sx_end_of_list)), cons (cons (sym_time, cons (tt,
         sx_end_of_list)), args)))), sx_end_of_list)));
}

static void include_on_read (sexpr sx, struct sexpr_io *io, void *aux)
{
    struct transdata *td = (struct transdata *)aux;

    if (eofp (sx))
    {
        td->done = 1;
    }
    else if (consp (sx))
    {
        sexpr n = car (sx);

        if (truep (equalp (sym_object, n)))
        {
            (*(td->data)) =
                    cons (lx_eval (sx, (td->environment)), (*(td->data)));
        }
        else
        {
            (*(td->data)) = cons (sx, (*(td->data)));
        }
    }
}

static sexpr previous_comments (sexpr base, sexpr env)
{
    sexpr r = sx_end_of_list,
          t = sx_join
            (webroot, str_scomments,
             sx_join (kho_normalise(lx_environment_lookup (env, sym_base_name)),
                      str_dot_ksu, str_nil));

    if (truep (filep (t)))
    {
        struct transdata td = { env, &r, 0 };
        struct sexpr_io *in = sx_open_i (io_open_read (sx_string (t)));
        multiplex_add_sexpr (in, include_on_read, &td);

        do
        {
            multiplex ();
        }
        while (td.done == 0);

        return cons (sym_sub_section, cons (str_Comments, r));
    }
    else
    {
        return str_nil;
    }
}

static sexpr make_comment_box (sexpr ext, sexpr source, sexpr target)
{
    return cons (sym_form, cons (lx_make_environment (cons (cons (sym_id,
        str_comment), cons (cons (sym_action, str_comment), cons (cons
        (sym_method, str_post), sx_end_of_list)))), cons (cons (sym_fieldset,
        cons (cons (sym_legend, cons (str_Add_Comment, sx_end_of_list)), cons
        (cons (sym_label, cons (lx_make_environment(cons (cons (sym_for,
        str_comment_name), sx_end_of_list)), cons (str_Name, sx_end_of_list))),
        cons (cons (sym_input, cons (lx_make_environment(cons (cons (sym_id,
        str_comment_name), cons (cons (sym_name, str_comment_name), cons (cons
        (sym_value, str_Anonymous), sx_end_of_list)))), sx_end_of_list)), cons
        (cons (sym_label, cons (lx_make_environment(cons (cons (sym_for,
        str_comment_text), sx_end_of_list)), cons (str_Message,
        sx_end_of_list))), cons (cons (sym_textarea, cons (lx_make_environment
        (cons (cons (sym_id, str_comment_text), cons (cons (sym_name,
        str_comment_text), sx_end_of_list))), cons (str_Message,
        sx_end_of_list))), cons (cons (sym_input, cons (lx_make_environment
        (cons (cons (sym_type, str_submit), sx_end_of_list)), sx_end_of_list)),
        cons (cons (sym_input, cons (lx_make_environment (cons (cons (sym_type,
        str_hidden), cons (cons (sym_name, str_comment_source), cons (cons
        (sym_value, source), sx_end_of_list)))), sx_end_of_list)), cons (cons
        (sym_input, cons (lx_make_environment (cons (cons (sym_type,
        str_hidden), cons (cons (sym_name, str_comment_target), cons (cons
        (sym_value, target), sx_end_of_list)))), sx_end_of_list)),
        sx_end_of_list))))))))), sx_end_of_list)));
}

static sexpr document (sexpr args, struct machine_state *st)
{
    sexpr env = st->environment;
    sexpr e, en = car (args);

    if (environmentp (en))
    {
        args = cdr (args);
    }
    else
    {
        en = lx_make_environment (sx_end_of_list);
    }

    if (truep (lx_environment_lookup (en, sym_comment_added)))
    {
        return cons (sym_document, cons (en, args));
    }

    e = lx_environment_lookup (env, sym_format);

    args = sx_reverse (args);

    if (truep (equalp (e, str_text_xhtml)) || truep (equalp (e, str_text_html)))
    {
        args = cons (make_comment_box (lx_environment_lookup (env,
            sym_extension), lx_environment_lookup (env, sym_original_name),
            lx_environment_lookup (env, sym_base_name)),
            cons (previous_comments (lx_environment_lookup (env,
            sym_base_name), env), args));
    }
    else
    {
        args = cons (previous_comments
          (lx_environment_lookup (env, sym_base_name), env), args);
    }

    en = lx_environment_bind (en, sym_comment_added, sx_true);

    return cons (sym_document, cons (en, sx_reverse (args)));
}

static sexpr post (sexpr args, struct machine_state *st)
{
    sexpr vars = car (args), name = car (cdr (args));

    if (truep (equalp (name, str_comment)))
    {
        sexpr r = sx_end_of_list, t = sx_join
           (webroot, str_scomments,
            sx_join (kho_normalise(lx_environment_lookup (vars,
                                                          sym_comment_target)),
                     str_dot_ksu, str_nil)),
              target = lx_environment_lookup (vars, sym_comment_source);
        struct sexpr_io *io;
        struct datetime dtime;

        dt_get (&dtime);

        if (truep (filep (t)))
        {
            sexpr re;
            io = sx_open_i (io_open_read (sx_string (t)));

            while (!eofp (re = sx_read (io)))
            {
                if (!nexp (re))
                {
                    r = cons (re, r);
                }
            }

            sx_close_io (io);
        }

        r = cons (cons (sym_object, cons (cons (sym_comment, cons (lx_environment_lookup (vars, sym_comment_name), cons (make_integer (dtime.date), cons (make_integer (dtime.time), cons (cons (sym_paragraph, cons (lx_environment_lookup (vars, sym_comment_text), sx_end_of_list)), sx_end_of_list))))), sx_end_of_list)), r);

        io = sx_open_o (io_open_write (sx_string (t)));

        r = sx_reverse (r);

        while (consp (r))
        {
            sx_write (io, car (r));
            r = cdr (r);
        }

        sx_close_io (io);

        vars = lx_environment_bind (vars, sym_Location, target);

        return cons (vars, cons (target, sx_end_of_list));
    }
    else
    {
        return cons (sym_post, args);
    }
}

int cmain ()
{
    kho_configure_callback = configure_callback;

    initialise_khonsu ();

    kho_environment = lx_environment_bind
      (kho_environment,sym_document,lx_foreign_lambda (sym_document,document));
    kho_environment = lx_environment_bind
      (kho_environment, sym_comment, lx_foreign_lambda (sym_comment, comment));
    kho_environment = lx_environment_bind
      (kho_environment, sym_post,    lx_foreign_lambda (sym_post,    post));

    while (multiplex () != mx_nothing_to_do)
    {
//        gc_invoke();
    }

    return 0;
}
