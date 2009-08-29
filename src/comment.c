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

static sexpr webroot          = sx_nonexistent;

static void configure_callback (sexpr sx)
{
    sexpr a = car (sx);

    if (truep (equalp (a, sym_root)))
    {
        webroot = car (cdr (sx));
    }
}

static sexpr comment (sexpr args, sexpr *env)
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
                    cons (lx_eval (sx, &(td->environment)), (*(td->data)));
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
        struct sexpr_io *in = sx_open_io (io_open_read (sx_string (t)),
                                          io_open_null);
        multiplex_add_sexpr (in, include_on_read, &td);

        do
        {
            multiplex ();
        }
        while (td.done == 0);
    }

    return cons (sym_sub_section, cons (str_Comments, r));
}

static sexpr make_comment_box (sexpr ext)
{
    return cons (sym_form, cons (lx_make_environment (cons (cons (sym_id,
        str_comment), cons (cons (sym_action, (nexp (ext) ? str_comment :
        sx_join (str_comment, str_dot, ext))), cons (cons (sym_method, str_put),
        sx_end_of_list)))), cons (cons (sym_fieldset, cons (cons (sym_legend,
        cons (str_Add_Comment, sx_end_of_list)), cons (cons (sym_label, cons
        (lx_make_environment(cons (cons (sym_for, str_comment_name),
        sx_end_of_list)), cons (str_Name, sx_end_of_list))), cons (cons
        (sym_input, cons (lx_make_environment(cons (cons (sym_id,
        str_comment_name), cons (cons (sym_name, str_comment_name), cons (cons
        (sym_value, str_Anonymous), sx_end_of_list)))), sx_end_of_list)), cons
        (cons (sym_label, cons (lx_make_environment(cons (cons (sym_for,
        str_comment_text), sx_end_of_list)), cons (str_Message,
        sx_end_of_list))), cons (cons (sym_textarea, cons (lx_make_environment
        (cons (cons (sym_id, str_comment_text), cons (cons (sym_name,
        str_comment_text), sx_end_of_list))), cons (str_Message,
        sx_end_of_list))), cons (cons (sym_input, cons (lx_make_environment
        (cons (cons (sym_type, str_submit), sx_end_of_list)), sx_end_of_list)),
        sx_end_of_list))))))), sx_end_of_list)));
}

static sexpr document (sexpr args, sexpr *env)
{
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

    e = lx_environment_lookup (*env, sym_format);

    args = sx_reverse (args);

    if (truep (equalp (e, str_text_xhtml)) || truep (equalp (e, str_text_html)))
    {
        args = cons (make_comment_box (lx_environment_lookup (*env,
            sym_extension)), cons (previous_comments (lx_environment_lookup
            (*env, sym_base_name), *env), args));
    }
    else
    {
        args = cons (previous_comments
          (lx_environment_lookup (*env, sym_base_name), *env), args);
    }

    en = lx_environment_bind (en, sym_comment_added, sx_true);

    return cons (sym_document, cons (en, sx_reverse (args)));
}

int cmain ()
{
    terminate_on_allocation_errors ();

    kho_configure_callback = configure_callback;

    initialise_khonsu ();

    kho_environment = lx_environment_bind
      (kho_environment,sym_document,lx_foreign_lambda (sym_document,document));
    kho_environment = lx_environment_bind
      (kho_environment, sym_comment, lx_foreign_lambda (sym_comment, comment));

    while (multiplex () != mx_nothing_to_do)
    {
//        gc_invoke();
    }

    return 0;
}
