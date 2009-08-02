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




define_string (str_xhtml_o,
  "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" "
  "\"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n"
  "<html xmlns=\"http://www.w3.org/1999/xhtml\">");
define_string (str_html_o,                   "<html>");
define_string (str_html_e,                   "</html>");
define_string (str_head_o,                   "<head>");
define_string (str_head_e,                   "</head>");
define_string (str_title_o,                  "<title>");
define_string (str_title_e,                  "</title>");
define_string (str_body_o,                   "<body>");
define_string (str_body_e,                   "</body>");
define_string (str_h1_o,                     "<h1>");
define_string (str_h1_e,                     "</h1>");
define_string (str_h2_o,                     "<h2>");
define_string (str_h2_e,                     "</h2>");
define_string (str_h3_o,                     "<h3>");
define_string (str_h3_e,                     "</h3>");
define_string (str_h4_o,                     "<h4>");
define_string (str_h4_e,                     "</h4>");
define_string (str_h5_o,                     "<h5>");
define_string (str_h5_e,                     "</h5>");
define_string (str_h6_o,                     "<h6>");
define_string (str_h6_e,                     "</h6>");
define_string (str_p_o,                      "<p>");
define_string (str_p_e,                      "</p>");

define_symbol (sym_formal_paragraph,         "formal-paragraph");
define_symbol (sym_paragraph,                "paragraph");
define_symbol (sym_page,                     "page");
define_symbol (sym_html,                     "html");
define_symbol (sym_html4,                    "html4");
define_symbol (sym_xhtml,                    "xhtml");
define_symbol (sym_xhtml1,                   "xhtml1");
define_symbol (sym_head,                     "head");
define_symbol (sym_title,                    "title");
define_symbol (sym_body,                     "body");
define_symbol (sym_h1,                       "h1");
define_symbol (sym_h2,                       "h2");
define_symbol (sym_h3,                       "h3");
define_symbol (sym_h4,                       "h4");
define_symbol (sym_h5,                       "h5");
define_symbol (sym_h6,                       "h6");

define_symbol (sym_application_xhtml_p_xml,  "application/xhtml+xml");

define_string (str_xhtml,                    "xhtml");
define_string (str_html,                     "html");

static sexpr paragraph (sexpr arguments, sexpr *env)
{
    sexpr r = str_nil;

    while (consp (arguments))
    {
        sexpr n = car (arguments);

        if (stringp (n))
        {
            r = sx_join (r, sx_join (str_p_o, n, str_p_e), str_nil);
        }

        arguments = cdr (arguments);
    }

    return r;
}

static sexpr page (sexpr arguments, sexpr *env)
{
    sexpr format = lx_environment_lookup (*env, sym_format);
    if (!nexp (format))
    {
        if (truep (equalp (format, sym_application_xhtml_p_xml)))
        {
            sexpr title = car (arguments);
            arguments = cdr (arguments);

            return lx_eval
                  (cons (sym_object,
                       cons (
                         cons (sym_xhtml,
                         cons (cons (sym_head,
                           cons (cons (sym_title, cons (title, sx_end_of_list)),
                             sx_end_of_list)),
                           cons (cons (sym_body, cons (cons (sym_h1,
                                 cons (title, sx_end_of_list)), arguments)),
                             sx_end_of_list))), sx_end_of_list)),
                   env);
        }
    }

    return cons (sym_page, arguments);
}

static sexpr body (sexpr arguments, sexpr *env)
{
    return kho_tagmerge (arguments, str_body_o, str_body_e);
}

static sexpr title (sexpr arguments, sexpr *env)
{
    return kho_tagmerge (arguments, str_title_o, str_title_e);
}

static sexpr head (sexpr arguments, sexpr *env)
{
    return kho_tagmerge (arguments, str_head_o, str_head_e);
}

static sexpr xhtml (sexpr arguments, sexpr *env)
{
    return kho_tagmerge (arguments, str_xhtml_o, str_html_e);
}

static sexpr html (sexpr arguments, sexpr *env)
{
    return kho_tagmerge (arguments, str_html_o, str_html_e);
}

static sexpr h1 (sexpr arguments, sexpr *env)
{
    return kho_tagmerge (arguments, str_h1_o, str_h1_e);
}

static sexpr h2 (sexpr arguments, sexpr *env)
{
    return kho_tagmerge (arguments, str_h2_o, str_h2_e);
}

static sexpr h3 (sexpr arguments, sexpr *env)
{
    return kho_tagmerge (arguments, str_h3_o, str_h3_e);
}

static sexpr h4 (sexpr arguments, sexpr *env)
{
    return kho_tagmerge (arguments, str_h4_o, str_h4_e);
}

static sexpr h5 (sexpr arguments, sexpr *env)
{
    return kho_tagmerge (arguments, str_h5_o, str_h5_e);
}

static sexpr h6 (sexpr arguments, sexpr *env)
{
    return kho_tagmerge (arguments, str_h6_o, str_h6_e);
}

int cmain ()
{
    terminate_on_allocation_errors ();

    kho_configure (cons (sym_map_extension,
                   cons (str_xhtml,
                   cons (sym_application_xhtml_p_xml, sx_end_of_list))));

    initialise_khonsu ();

    kho_environment = lx_environment_bind
            (kho_environment, sym_paragraph,
             lx_foreign_lambda (sym_paragraph, paragraph));
    kho_environment = lx_environment_bind
            (kho_environment, sym_page,
             lx_foreign_lambda (sym_page, page));
    kho_environment = lx_environment_bind
            (kho_environment, sym_body,
             lx_foreign_lambda (sym_body, body));
    kho_environment = lx_environment_bind
            (kho_environment, sym_title,
             lx_foreign_lambda (sym_title, title));
    kho_environment = lx_environment_bind
            (kho_environment, sym_head,
             lx_foreign_lambda (sym_head, head));
    kho_environment = lx_environment_bind
            (kho_environment, sym_xhtml,
             lx_foreign_lambda (sym_xhtml, xhtml));
    kho_environment = lx_environment_bind
            (kho_environment, sym_html,
             lx_foreign_lambda (sym_html, html));
    kho_environment = lx_environment_bind
            (kho_environment, sym_h1,
             lx_foreign_lambda (sym_h1, h1));
    kho_environment = lx_environment_bind
            (kho_environment, sym_h2,
             lx_foreign_lambda (sym_h2, h2));
    kho_environment = lx_environment_bind
            (kho_environment, sym_h3,
             lx_foreign_lambda (sym_h3, h3));
    kho_environment = lx_environment_bind
            (kho_environment, sym_h4,
             lx_foreign_lambda (sym_h4, h4));
    kho_environment = lx_environment_bind
            (kho_environment, sym_h5,
             lx_foreign_lambda (sym_h5, h5));
    kho_environment = lx_environment_bind
            (kho_environment, sym_h6,
             lx_foreign_lambda (sym_h6, h6));

    kho_debug (make_symbol ("tao"));

    while (multiplex () != mx_nothing_to_do);

    return 0;
}
