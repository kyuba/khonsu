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
define_string (str_dot_svg,         ".svg");
define_string (str_rgb_map,         "RGB Map");
define_string (str_image_svg_xml,   "image/svg+xml");
define_string (str_hue,             "hue");
define_string (str_luminance,       "luminance");
define_string (str_Primaries,       "Primaries");
define_string (str_Map      ,       "Map");

define_symbol (sym_colour_box_rgb,  "colour-box-rgb");
define_symbol (sym_span,            "span");
define_symbol (sym_image,           "image");
define_symbol (sym_style,           "style");
define_symbol (sym_class,           "class");
define_symbol (sym_defs,            "defs");
define_symbol (sym_stop,            "stop");
define_symbol (sym_x1,              "x1");
define_symbol (sym_x2,              "x2");
define_symbol (sym_y1,              "y1");
define_symbol (sym_y2,              "y2");
define_symbol (sym_linearGradient,  "linearGradient");
define_symbol (sym_g,               "g");
define_symbol (sym_rect,            "rect");
define_symbol (sym_offset,          "offset");
define_symbol (sym_width,           "width");
define_symbol (sym_height,          "height");
define_symbol (sym_section,         "section");
define_symbol (sym_menu,            "menu");

define_symbol (sym_rgb_map,         "rgb-map");

define_symbol (sym_document,        "document");
define_symbol (sym_original_name,   "original-name");
define_symbol (sym_base_name,       "base-name");

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

static sexpr colour_box_rgb_i (sexpr arguments)
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

static sexpr colour_box_rgb (sexpr arguments, struct machine_state *st)
{
    return colour_box_rgb_i (arguments);
}

static sexpr rgb_map (sexpr arguments, struct machine_state *st)
{
    define_string (str_100p,           "100%");
//    define_string (str_zero_dot,       "0.");
    define_string (str_percent,        "%");
    define_string (str_dot_5,          "0.5");
    define_string (str_one,            "1");
    define_string (str_stop_color,     "stop-color:");
    define_string (str_stop_color_b,   "stop-color:#000000");
    define_string (str_stop_color_b_t, "stop-color:#000000;stop-opacity:0");
    define_string (str_stop_color_w,   "stop-color:#ffffff");
    define_string (str_stop_color_w_t, "stop-color:#ffffff;stop-opacity:0");
    define_string (str_fill_hue,       "fill:url(#hue)");
    define_string (str_fill_luminance, "fill:url(#luminance)");

    sexpr f = lx_environment_lookup (st->environment, sym_format), r,
          b = lx_environment_lookup (st->environment, sym_base_name);

    if (truep (equalp (f, str_image_svg_xml)))
    {
        int nstops = 0, n = 0;
        sexpr stops = sx_end_of_list, cur, last = sx_nonexistent;

        for (cur = arguments; consp (cur); cur = cdr (cur))
        {
            nstops++;
        }

        for (cur = arguments; consp (cur); cur = cdr (cur))
        {
            sexpr v = car (cur);
            sexpr r = car (v), g, b;
            v = cdr (v);
            g = car (v);
            v = cdr (v);
            b = car (v);

            v = sx_list2
                  (sym_stop,
                   lx_make_environment
                     (sx_list2
                        (cons (sym_style,
                               sx_join (str_stop_color,
                                        rgb_to_html (r, g, b),
                                        str_nil)),
                         cons (sym_offset,
                               sx_join (str_nil,
                                        sx_to_string
                                          (colour_scale
                                             (make_rational (n, nstops),
                                              100)),
                                        str_percent)))));

            n++;

            if (nexp (last))
            {
                last = sx_list2
                          (sym_stop,
                           lx_make_environment
                             (sx_list2
                                (cons (sym_style,
                                       sx_join (str_stop_color,
                                                rgb_to_html (r, g, b),
                                                str_nil)),
                                 cons (sym_offset, str_one))));
            }

            stops = cons (v, stops);
        }

        stops = sx_reverse (cons (last, stops));

        r = sx_list4
              (sym_document, str_rgb_map,
               sx_list3
                 (sym_defs,
                  cons
                    (sym_linearGradient,
                     cons (lx_make_environment
                             (sx_list5
                                (cons (sym_x1, str_zero),
                                 cons (sym_y1, str_zero),
                                 cons (sym_x2, str_100p),
                                 cons (sym_y2, str_zero),
                                 cons (sym_id, str_hue))),
                           stops)),
                  sx_list6
                    (sym_linearGradient,
                     lx_make_environment
                       (sx_list5
                          (cons (sym_x1, str_zero),
                           cons (sym_y1, str_zero),
                           cons (sym_x2, str_zero),
                           cons (sym_y2, str_100p),
                           cons (sym_id, str_luminance))),
                     sx_list2
                       (sym_stop,
                        lx_make_environment
                          (sx_list2
                             (cons (sym_style,  str_stop_color_b),
                              cons (sym_offset, str_zero)))),
                     sx_list2
                       (sym_stop,
                        lx_make_environment
                          (sx_list2
                             (cons (sym_style,  str_stop_color_b_t),
                              cons (sym_offset, str_dot_5)))),
                     sx_list2
                       (sym_stop,
                        lx_make_environment
                          (sx_list2
                             (cons (sym_style,  str_stop_color_w_t),
                              cons (sym_offset, str_dot_5)))),
                     sx_list2
                       (sym_stop,
                        lx_make_environment
                          (sx_list2
                             (cons (sym_style,  str_stop_color_w),
                              cons (sym_offset, str_one)))))),
               sx_list3
                 (sym_g,
                  sx_list2
                    (sym_rect,
                     lx_make_environment
                       (sx_list3 (cons (sym_width, str_100p),
                                  cons (sym_height, str_100p),
                                  cons (sym_style, str_fill_hue)))),
                  sx_list2
                    (sym_rect,
                     lx_make_environment
                       (sx_list3 (cons (sym_width, str_100p),
                                  cons (sym_height, str_100p),
                                  cons (sym_style, str_fill_luminance))))));
    }
    else
    {
        sexpr primaries = sx_end_of_list, cur;

        for (cur = arguments; consp (cur); cur = cdr (cur))
        {
            primaries = cons (colour_box_rgb_i (car (cur)), primaries);
        }
        primaries = sx_reverse (primaries);

        r = sx_list5 (sym_document, str_rgb_map,
                      sx_list1 (sym_menu),
                      cons (sym_section, cons (str_Primaries, primaries)),
                      sx_list3
                        (sym_section, str_Map,
                         sx_list2
                           (sym_image,
                            sx_join (str_nil, b, str_dot_svg))));
    }

    return r;
}

static sexpr request (sexpr arguments, struct machine_state *st)
{
    sexpr a = arguments, r = sx_end_of_list, a2, a3;

    while (consp (a))
    {
        a2 = car (a);
        a3 = car (a2);

        if (truep (equalp (a3, sym_get)))
        {
            sexpr t1 = cdr (a2), te = car (t1), t2 = cdr (t1), target = car(t2),
                  p;
            const char *etarget = sx_string (target);
            int len;
            struct sexpr_io *sxio;

            if (!environmentp (te))
            {
                te = lx_make_environment (sx_end_of_list);
            }

            for (len = 0; etarget[len] != 0; len++);

            sxio = sx_open_i (io_open_buffer ((void *)etarget, len));

            p = sx_read (sxio);

            if (truep (equalp (p, sym_rgb_map)))
            {
                sexpr ar = sx_end_of_list;

                while ((p = sx_read (sxio)), consp (p))
                {
                    sexpr cv = sx_end_of_list;
                    sexpr cp = p;

                    while (consp (cp))
                    {
                        sexpr cva = car (cp);

                        if (integerp (cva) || rationalp (cva))
                        {
                            cv = cons (cva, cv);
                        }

                        cp = cdr (cp);
                    }

                    ar = cons (sx_reverse (cv), ar);
                }

                te = lx_environment_bind (te, sym_original_name, target);

                a2 = sx_list3 (sym_pong,
                               te,
                               sx_list2 (sym_object,
                                         cons (sym_rgb_map, sx_reverse (ar))));
            }

            r = cons (a2, r);

            sx_close_io (sxio);
        }
        else
        {
            r = cons (a2, r);
        }

        a  = cdr (a);
    }

    relay_sub (sx_reverse (r));
    return sx_end_of_list;
}

int cmain ()
{
    kho_configure_callback = configure_callback;

    initialise_khonsu ();

    kho_environment = lx_environment_bind
      (kho_environment, sym_colour_box_rgb,
       lx_foreign_lambda (sym_colour_box_rgb, colour_box_rgb));

    kho_environment = lx_environment_bind
      (kho_environment, sym_rgb_map,
       lx_foreign_lambda (sym_rgb_map, rgb_map));

    kho_environment = lx_environment_bind
      (kho_environment, sym_request, lx_foreign_lambda (sym_request, request));

    while (multiplex () != mx_nothing_to_do)
    {
//        gc_invoke();
    }

    return 0;
}
