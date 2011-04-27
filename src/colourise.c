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
#include <sievert/sexpr.h>
#include <curie/multiplex.h>
#include <curie/memory.h>
#include <curie/gc.h>

define_string (str_colour_box,      "colour-box");
define_string (str_colour_dash,     "colour-");
define_string (str_colour_rgb_dash, "colour-rgb-");
define_string (str_zero,            "0");

define_symbol (sym_colour_box_rgb,  "colour-box-rgb");
define_symbol (sym_span,            "span");
define_symbol (sym_style,           "style");
define_symbol (sym_class,           "class");

static sexpr webroot = sx_nonexistent;

static void configure_callback (sexpr sx)
{
    sexpr a = car (sx);

    if (truep (equalp (a, sym_root)))
    {
        webroot = car (cdr (sx));
    }
}

static sexpr colour_scale (sexpr c, unsigned int max)
{
    double n, d, m = max;
    unsigned int r;

    if (integerp (c)) /* convert to rational by assuming the denominator is 1 */
    {
        n = sx_integer (c);
        d = 1.0;
    }
    else
    {
        n = sx_numerator   (c);
        d = sx_denominator (c);
    }

    r = n / d * m;

    return make_integer (r);
}

static sexpr rgb_to_html (sexpr r, sexpr g, sexpr b)
{
    const char *s;
    define_string (str_hash, "#");

    r = colour_scale (r, 255);
    g = colour_scale (g, 255);
    b = colour_scale (b, 255);

    r = sx_integer_to_string_hex (sx_integer (r));
    g = sx_integer_to_string_hex (sx_integer (g));
    b = sx_integer_to_string_hex (sx_integer (b));

    if ((s = sx_string (r)), (s[1] == 0))
    {
        r = sx_join (str_zero, r, sx_nil);
    }
    if ((s = sx_string (g)), (s[1] == 0))
    {
        g = sx_join (str_zero, g, sx_nil);
    }
    if ((s = sx_string (b)), (s[1] == 0))
    {
        b = sx_join (str_zero, b, sx_nil);
    }

    return sx_join (str_hash,
                    sx_join (r, g, b),
                    str_nil);
}

static sexpr colour_box_rgb (sexpr arguments, struct machine_state *st)
{
    define_string (str_bcc, "background-color:");

    sexpr r, g, b, v;

    r         = car (arguments);
    arguments = cdr (arguments);
    g         = car (arguments);
    arguments = cdr (arguments);
    b         = car (arguments);

    v         = rgb_to_html (r, g, b);

    return sx_list3 (sym_span,
                     lx_make_environment
                       (sx_list2 (cons (sym_class, str_colour_box),
                                  cons (sym_style,
                                        sx_join (str_bcc, v, str_nil)))),
                     v);
}

int cmain ()
{
    kho_configure_callback = configure_callback;

    initialise_khonsu ();

    kho_environment = lx_environment_bind
      (kho_environment, sym_colour_box_rgb,
       lx_foreign_lambda (sym_colour_box_rgb, colour_box_rgb));

    while (multiplex () != mx_nothing_to_do)
    {
//        gc_invoke();
    }

    return 0;
}
