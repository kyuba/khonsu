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

define_string (str_xhtml,                    "xhtml");

static sexpr paragraph (sexpr arguments, sexpr *env)
{
    return cons (sym_p, arguments);
}

static sexpr page (sexpr arguments, sexpr *env)
{
    sexpr format = lx_environment_lookup (*env, sym_format);
    if (!nexp (format))
    {
        if (truep (equalp (format, sym_application_xhtml_p_xml)))
        {
            sexpr title = car (arguments), at;
            arguments = cdr (arguments);

            if (environmentp (title))
            {
                at = title;
                title = car (arguments);
                arguments = cdr (arguments);
            }

            return lx_eval
                  (cons (str_xml_declaration, cons (str_doctype_xhtml,
                       cons (
                         cons (sym_html,
                         cons (cons (sym_head,
                           cons (cons (sym_title, cons (title, sx_end_of_list)),
                             sx_end_of_list)),
                           cons (cons (sym_body, cons (cons (sym_h1,
                                 cons (title, sx_end_of_list)), arguments)),
                             sx_end_of_list))), sx_end_of_list))),
                   env);
        }
    }

    return cons (sym_page, arguments);
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

    kho_debug (make_symbol ("tao"));

    while (multiplex () != mx_nothing_to_do);

    return 0;
}
