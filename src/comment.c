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

define_symbol (sym_document,         "document");
define_symbol (sym_method,           "method");
define_symbol (sym_action,           "action");
define_symbol (sym_extension,        "extension");
define_symbol (sym_sub_section,      "sub-section");
define_symbol (sym_form,             "form");
define_symbol (sym_div,              "div");
define_symbol (sym_comment_added,    "comment-added");
define_symbol (sym_base_name,        "base-name");
define_string (str_dot,              ".");
define_string (str_post,             "post");
define_string (str_comment,          "comment");
define_string (str_Comments,         "Comments");
define_string (str_text_html,        "text/html");
define_string (str_text_xhtml,       "application/xhtml+xml");

static sexpr webroot          = sx_nonexistent;

static void configure_callback (sexpr sx)
{
    sexpr a = car (sx);

    if (truep (equalp (a, sym_root)))
    {
        webroot = car (cdr (sx));
    }
}

static sexpr previous_comments (sexpr base)
{
    return cons (sym_sub_section, cons (str_Comments, sx_end_of_list));
}

static sexpr make_comment_box (sexpr ext)
{
    return cons (sym_form, cons (lx_make_environment (cons (cons (sym_id, str_comment), cons (cons (sym_action, (nexp (ext) ? str_comment : sx_join (str_comment, str_dot, ext))), cons (cons (sym_method, str_post), sx_end_of_list)))), sx_end_of_list));
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
        args = cons (previous_comments
           (lx_environment_lookup (*env, sym_base_name)),
           cons (make_comment_box (lx_environment_lookup (*env, sym_extension)),
                 args));
    }
    else
    {
        args = cons (previous_comments
          (lx_environment_lookup (*env, sym_base_name)), args);
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

    while (multiplex () != mx_nothing_to_do)
    {
//        gc_invoke();
    }

    return 0;
}
