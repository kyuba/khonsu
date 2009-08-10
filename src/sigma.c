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

define_symbol (sym_menu,      "menu");
define_symbol (sym_list,      "list");
define_symbol (sym_link,      "link");
define_symbol (sym_href,      "href");
define_symbol (sym_item,      "item");
define_symbol (sym_extension, "extension");
define_symbol (sym_base_name, "base-name");
define_symbol (sym_class,     "class");
define_string (str_selected,  "selected");
define_string (str_menu,      "menu");
define_string (str_dot,       ".");

static sexpr menu (sexpr arguments, sexpr *env)
{
    sexpr sx = sx_end_of_list, target, name, t, e;

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

    return cons (sym_list, cons (lx_make_environment (cons (cons (sym_id,
             str_menu), sx_end_of_list)), sx_reverse (sx)));
}

int cmain ()
{
    terminate_on_allocation_errors ();

    initialise_khonsu ();

    kho_environment = lx_environment_bind
            (kho_environment, sym_menu, lx_foreign_lambda (sym_menu, menu));

    kho_debug (make_symbol ("sigma"));

    while (multiplex () != mx_nothing_to_do);

    return 0;
}
