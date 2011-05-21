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
#include <sievert/time.h>
#include <curie/multiplex.h>
#include <curie/memory.h>
#include <curie/gc.h>

define_string (str_image_svg_xml,   "image/svg+xml");
define_string (str_dot_svg,         ".svg");
define_string (str_Analogue_Clock,  "Analogue Clock");
define_string (str_Time,            "Time");
define_string (str_xlink_url,       "http://www.w3.org/1999/xlink");
define_string (str_hface,           "#face");
define_string (str_face,            "face");
define_string (str_vB1k,            "0 0 1000 1000");
define_string (str_style_a,         "stroke:black;stroke-width:1");
define_string (str_1000,            "1000");
define_string (str_975,             "975");
define_string (str_500,             "500");
define_string (str_400,             "400");
define_string (str_300,             "300");
define_string (str_200,             "200");
define_string (str_100,             "100");
define_string (str_75,              "75");
define_string (str_60,              "60");
define_string (str_50,              "50");
define_string (str_40,              "40");
define_string (str_25,              "25");
define_string (str_1,               "1");
define_string (str_0,               "0");
define_string (str_p8,              ".8");
define_string (str_p6,              ".6");
define_string (str_p4,              ".4");
define_string (str_p2,              ".2");
define_string (str_mA,              "mA");
define_string (str_mB,              "mB");
define_string (str_mC,              "mC");
define_string (str_mD,              "mD");
define_string (str_gA,              "gA");
define_string (str_gB,              "gB");
define_string (str_gC,              "gC");
define_string (str_gD,              "gD");
define_string (str_gi,              "gi");
define_string (str_go,              "go");
define_string (str_c,               "c");
define_string (str_hmA,             "#mA");
define_string (str_hmB,             "#mB");
define_string (str_hmC,             "#mC");
define_string (str_hmD,             "#mD");
define_string (str_hgA,             "#gA");
define_string (str_hgB,             "#gB");
define_string (str_hgC,             "#gC");
define_string (str_hgD,             "#gD");
define_string (str_hgi,             "#gi");
define_string (str_hgo,             "#go");
define_string (str_hc,              "#c");
define_string (str_rotate_pre,      "rotate(");
define_string (str_rotate_post,     ",500,500)");
define_string (str_scale_pre,       "translate(");
define_string (str_scale_mid1,      ") scale(");
define_string (str_scale_mid2,      ") rotate(");
define_string (str_scale_post,      " 500 500)");
define_string (str_comma,           ",");
define_string (str_rot0,            "0 500 500");
define_string (str_rot360,          "360 500 500");
define_string (str_s,               "s");
define_string (str_rotate,          "rotate");
define_string (str_transform,       "transform");
define_string (str_indefinite,      "indefinite");

define_symbol (sym_analogue_clock,  "analogue-clock");
define_symbol (sym_section,         "section");
define_symbol (sym_image,           "image");
define_symbol (sym_menu,            "menu");
define_symbol (sym_defs,            "defs");
define_symbol (sym_use,             "use");
define_symbol (sym_xlinkchref,      "xlink:href");
define_symbol (sym_xmlnscxlink,     "xmlns:xlink");
define_symbol (sym_svg,             "svg");
define_symbol (sym_viewBox,         "viewBox");
define_symbol (sym_line,            "line");
define_symbol (sym_style,           "style");
define_symbol (sym_x1,              "x1");
define_symbol (sym_y1,              "y1");
define_symbol (sym_x2,              "x2");
define_symbol (sym_y2,              "y2");
define_symbol (sym_g,               "g");
define_symbol (sym_from,            "from");
define_symbol (sym_to,              "to");
define_symbol (sym_dur,             "dur");
define_symbol (sym_type,            "type");
define_symbol (sym_transform,       "transform");
define_symbol (sym_animateTransform,"animateTransform");
define_symbol (sym_attributeName,   "attributeName");
define_symbol (sym_repeatDur,       "repeatDur");

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

static sexpr analogue_clock (sexpr arguments, struct machine_state *st)
{
    sexpr f = lx_environment_lookup (st->environment, sym_format), r,
          b = lx_environment_lookup (st->environment, sym_base_name);

    if (truep (equalp (f, str_image_svg_xml)))
    {
        int   i, j;
        sexpr c = car (arguments);
        sexpr day_len     = make_integer ((24 * 60 * 60));
        sexpr ticks       = make_integer (12);
        sexpr sub_ticks   = make_integer (5);
        unsigned int ct   = dt_get_time ();

        sexpr facesvg     = sx_end_of_list;
        sexpr facedefs    = sx_list5
            (sx_list2 (sym_line,
                       lx_make_environment
                         (sx_list6 (cons (sym_id, str_c),
                                    cons (sym_x1, str_500),
                                    cons (sym_y1, str_0),
                                    cons (sym_x2, str_500),
                                    cons (sym_y2, str_1000),
                                    cons (sym_style, str_style_a)))),
             sx_list2 (sym_line,
                       lx_make_environment
                         (sx_list6 (cons (sym_id, str_mA),
                                    cons (sym_x1, str_500),
                                    cons (sym_y1, str_50),
                                    cons (sym_x2, str_500),
                                    cons (sym_y2, str_75),
                                    cons (sym_style, str_style_a)))),
             sx_list2 (sym_line,
                       lx_make_environment
                         (sx_list6 (cons (sym_id, str_mB),
                                    cons (sym_x1, str_500),
                                    cons (sym_y1, str_50),
                                    cons (sym_x2, str_500),
                                    cons (sym_y2, str_60),
                                    cons (sym_style, str_style_a)))),
             sx_list2 (sym_line,
                       lx_make_environment
                         (sx_list6 (cons (sym_id, str_mC),
                                    cons (sym_x1, str_500),
                                    cons (sym_y1, str_25),
                                    cons (sym_x2, str_500),
                                    cons (sym_y2, str_50),
                                    cons (sym_style, str_style_a)))),
             sx_list2 (sym_line,
                       lx_make_environment
                         (sx_list6 (cons (sym_id, str_mD),
                                    cons (sym_x1, str_500),
                                    cons (sym_y1, str_40),
                                    cons (sym_x2, str_500),
                                    cons (sym_y2, str_50),
                                    cons (sym_style, str_style_a)))));

        sexpr faceg       = sx_list1
            (sx_list2 (sym_use, lx_make_environment
                                  (sx_list1 (cons (sym_xlinkchref,
                                                   str_hgi)))));

        arguments = cdr (arguments);

        if (consp (arguments))
        {
            ticks     = car (c);
            c         = cdr (c);
            sub_ticks = car (c);
            c         = cdr (c);
            day_len   = car (c);

            if (!integerp (day_len) || (sx_integer(day_len) == 0))
            {
                day_len   = make_integer ((24*60*60));
            }
            if (!integerp (ticks) || (sx_integer(ticks) == 0))
            {
                ticks     = make_integer (12);
            }
            if (!integerp (sub_ticks) || (sx_integer(sub_ticks) == 0))
            {
                sub_ticks = make_integer (5);
            }

            c = car (arguments);
        }

        for (i = 0; i < 2; i++)
        {
            int n = sx_integer (sub_ticks), o = sx_integer (ticks);
            sexpr t1 = str_gi, t2 = str_hgB, t3 = str_hgA, q = sx_end_of_list;

            if (i == 1) { t1 = str_go; t2 = str_hgD; t3 = str_hgC; }

            for (j = 0; j < n; j++)
            {
                sexpr f = make_integer (j * 360 / (n * o));

                f = sx_join (str_rotate_pre, sx_to_string (f), str_rotate_post);

                q = cons (sx_list2
                          (sym_use,
                            lx_make_environment
                              (sx_list2 (cons (sym_xlinkchref, t2),
                                         cons (sym_transform, f)))),
                          q);
            }

            q = cons (sx_list2
                      (sym_use,
                        lx_make_environment
                          (sx_list1 (cons (sym_xlinkchref, t3)))), q);

            q = cons (sx_list2
                      (sym_line,
                       lx_make_environment
                          (sx_list5 (cons (sym_x1, str_500),
                                     cons (sym_y1, (i==0 ? str_0 : str_50)),
                                     cons (sym_x2, str_500),
                                     cons (sym_y2, (i==0 ? str_25 : str_75)),
                                     cons (sym_style, str_style_a)))), q);

            q = cons (sx_list2
                      (sym_line,
                       lx_make_environment
                          (sx_list5 (cons (sym_x1, str_500),
                                     cons (sym_y1, str_975),
                                     cons (sym_x2, str_500),
                                     cons (sym_y2, str_1000),
                                     cons (sym_style, str_style_a)))), q);

            q = cons (sx_list2
                      (sym_line,
                       lx_make_environment
                          (sx_list5 (cons (sym_x1, str_0),
                                     cons (sym_y1, str_500),
                                     cons (sym_x2, str_25),
                                     cons (sym_y2, str_500),
                                     cons (sym_style, str_style_a)))), q);

            q = cons (sx_list2
                      (sym_line,
                       lx_make_environment
                          (sx_list5 (cons (sym_x1, str_975),
                                     cons (sym_y1, str_500),
                                     cons (sym_x2, str_1000),
                                     cons (sym_y2, str_500),
                                     cons (sym_style, str_style_a)))), q);

            q = cons (sx_list2
                      (sym_line,
                       lx_make_environment
                          (sx_list5 (cons (sym_x1, str_500),
                                     cons (sym_y1, str_100),
                                     cons (sym_x2, str_500),
                                     cons (sym_y2, str_300),
                                     cons (sym_style, str_style_a)))), q);

            facedefs = cons
                (cons (sym_g, cons (lx_make_environment
                                      (sx_list1 (cons (sym_id, t1))), q)),
                 facedefs);
        }

        for (i = 0; i < 4; i++)
        {
            int n = sx_integer (ticks);
            sexpr t1 = str_gA, t2 = str_hmA, q = sx_end_of_list;

            if (i == 1) { t1 = str_gB; t2 = str_hmB; }
            if (i == 2) { t1 = str_gC; t2 = str_hmC; }
            if (i == 3) { t1 = str_gD; t2 = str_hmD; }

            for (j = 0; j < n; j++)
            {
                sexpr f = make_integer (j * 360 / n);

                f = sx_join (str_rotate_pre, sx_to_string (f), str_rotate_post);

                q = cons (sx_list2
                          (sym_use,
                            lx_make_environment
                              (sx_list2 (cons (sym_xlinkchref, t2),
                                         cons (sym_transform, f)))),
                          q);
            }

            facedefs = cons
                (cons (sym_g, cons (lx_make_environment
                                      (sx_list1 (cons (sym_id, t1))), q)),
                 facedefs);
        }

        for (i = 0; consp (c); (c = cdr (c)), i++)
        {
            sexpr a = car (c);
            int   n = sx_integer (a);
            sexpr d = sx_join (str_nil, a, str_s);
            sexpr s = str_p2, s2 = str_400, ro = make_integer(0);

            switch (i)
            {
                case 0:
                    s = str_1;
                    s2 = str_0;
                    break;
                case 1:
                case 2:
                    s  = str_p8;
                    s2 = str_100;
                    break;
                case 3:
                case 4:
                    s  = str_p6;
                    s2 = str_200;
                    break;
                case 5:
                case 6:
                    s  = str_p4;
                    s2 = str_300;
                    break;
            }

            ro = make_integer(((ct % n) * 360 / n));

            s = sx_join (str_scale_pre, s2,
                sx_join (str_comma, s2,
                sx_join (str_scale_mid1, s,
                sx_join (str_scale_mid2,
                         sx_to_string(ro), str_scale_post))));

            faceg = cons
                (sx_list3
                    (sym_g, lx_make_environment
                              (sx_list1 (cons (sym_transform, s))),
                     sx_list3
                       (sym_use,
                        lx_make_environment
                          (sx_list1 (cons (sym_xlinkchref,
                                           (i==0) ? str_hgo :
                                           (i%2==0) ? str_hgi
                                                    : str_hgo))),
                        sx_list2
                          (sym_animateTransform,
                           lx_make_environment
                             (sx_list6 (cons (sym_attributeName,
                                              str_transform),
                                        cons (sym_type, str_rotate),
                                        cons (sym_from, str_rot0),
                                        cons (sym_to, str_rot360),
                                        cons (sym_repeatDur, str_indefinite),
                                        cons (sym_dur, d)))))),
                 faceg);
        }

        facedefs = sx_reverse (facedefs);

        facesvg = sx_list1
            (sx_list4 (sym_svg, lx_make_environment
                                  (sx_list2 (cons (sym_viewBox, str_vB1k),
                                             cons (sym_id, str_face))),
                       cons (sym_defs, facedefs),
                       cons (sym_g, faceg)));

        r = sx_list4 (sym_document,
                      str_Analogue_Clock,
                      sx_list2 (sym_defs, facesvg),
                      sx_list2 (sym_use, lx_make_environment
                                          (sx_list1 (cons (sym_xlinkchref,
                                                           str_hface)))));
    }
    else
    {
        r = sx_list4 (sym_document, str_Analogue_Clock,
                      sx_list1 (sym_menu),
                      sx_list3
                        (sym_section, str_Time,
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

            if (truep (equalp (p, sym_analogue_clock)))
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
                                         cons (sym_analogue_clock,
                                               sx_reverse (ar))));
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
      (kho_environment, sym_analogue_clock,
       lx_foreign_lambda (sym_analogue_clock, analogue_clock));

    kho_environment = lx_environment_bind
      (kho_environment, sym_request, lx_foreign_lambda (sym_request, request));

    while (multiplex () != mx_nothing_to_do)
    {
//        gc_invoke();
    }

    return 0;
}
