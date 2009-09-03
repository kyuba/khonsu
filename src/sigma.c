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
#include <curie/time.h>
#include <curie/gc.h>

struct transdata
{
    sexpr environment;
    sexpr *data;
    int done;
};

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
define_symbol (sym_contact,                    "contact");
define_symbol (sym_document,                   "document");
define_symbol (sym_first_name,                 "first-name");
define_symbol (sym_last_name,                  "last-name");
define_symbol (sym_icon,                       "icon");
define_symbol (sym_short_description,          "short-description");
define_symbol (sym_sub_section,                "sub-section");
define_symbol (sym_elaborate,                  "elaborate");
define_symbol (sym_date,                       "date");
define_symbol (sym_time,                       "time");
define_symbol (sym_div,                        "div");
define_symbol (sym_table,                      "table");
define_symbol (sym_tbody,                      "tbody");
define_symbol (sym_tr,                         "tr");
define_symbol (sym_td,                         "td");
define_symbol (sym_th,                         "th");
define_symbol (sym_trContactName,              "tr:ContactName");
define_symbol (sym_trDescription,              "tr:Description");
define_symbol (sym_trIcon,                     "tr:Icon");

define_string (str_selected,                   "selected");
define_string (str_menu,                       "menu");
define_string (str_slash,                      "/");
define_string (str_scontacts,                  "/contact/");
define_string (str_contact_dash,               "contact-");
define_string (str_contact_slash,              "contact/");
define_string (str_comma_space,                ", ");
define_string (str_space,                      " ");
define_string (str_dot,                        ".");
define_string (str_icon,                       "icon");
define_string (str_Contact,                    "Contact");
define_string (str_dot_ksu,                    ".ksu");
define_string (str_date,                       "date");
define_string (str_time,                       "time");
define_string (str_png_icon_no_picture_png,    "png/icon-no-picture.png");
define_string (str_text_html,                  "text/html");
define_string (str_text_xhtml,                 "application/xhtml+xml");

define_string (str_imix,     "Imix'");
define_string (str_ik,       "Ik'");
define_string (str_akbal,    "Ak'b'al");
define_string (str_kan,      "K'an");
define_string (str_chikchan, "Chikchan");
define_string (str_kimit,    "Kimi");
define_string (str_manik,    "Manik'");
define_string (str_lamat,    "Lamat");
define_string (str_muluk,    "Muluk");
define_string (str_ok,       "Ok");
define_string (str_chuwen,   "Chuwen");
define_string (str_eb,       "Eb'");
define_string (str_ben,      "B'en");
define_string (str_ix,       "Ix");
define_string (str_men,      "Men");
define_string (str_kib,      "K'ib'");
define_string (str_kaban,    "Kab'an");
define_string (str_etznab,   "Etz'nab'");
define_string (str_kawak,    "Kawak");
define_string (str_ajaw,     "Ajaw");

define_string (str_pop,      "Pop");
define_string (str_wo,       "Wo");
define_string (str_sip,      "Sip");
define_string (str_sotz,     "Sotz'");
define_string (str_sek,      "Sek");
define_string (str_xul,      "Xul");
define_string (str_yaxkin,   "Yaxk'in");
define_string (str_mol,      "Mol");
define_string (str_chen,     "Ch'en");
define_string (str_yax,      "Yax");
define_string (str_sac,      "Sac");
define_string (str_keh,      "Keh");
define_string (str_mak,      "Mak");
define_string (str_kankin,   "K'ank'in");
define_string (str_muwan,    "Muwan");
define_string (str_pax,      "Pax");
define_string (str_kayab,    "K'ayab'");
define_string (str_kumku,    "Kumk'u");
define_string (str_wayeb,    "Wayeb'");

static const sexpr tzolkin_day_names [20] =
{
    ((const sexpr)&(sexpr_payload_str_ajaw)),
    ((const sexpr)&(sexpr_payload_str_imix)),
    ((const sexpr)&(sexpr_payload_str_ik)),
    ((const sexpr)&(sexpr_payload_str_akbal)),
    ((const sexpr)&(sexpr_payload_str_kan)),
    ((const sexpr)&(sexpr_payload_str_chikchan)),
    ((const sexpr)&(sexpr_payload_str_kimit)),
    ((const sexpr)&(sexpr_payload_str_manik)),
    ((const sexpr)&(sexpr_payload_str_lamat)),
    ((const sexpr)&(sexpr_payload_str_muluk)),
    ((const sexpr)&(sexpr_payload_str_ok)),
    ((const sexpr)&(sexpr_payload_str_chuwen)),
    ((const sexpr)&(sexpr_payload_str_eb)),
    ((const sexpr)&(sexpr_payload_str_ben)),
    ((const sexpr)&(sexpr_payload_str_ix)),
    ((const sexpr)&(sexpr_payload_str_men)),
    ((const sexpr)&(sexpr_payload_str_kib)),
    ((const sexpr)&(sexpr_payload_str_kaban)),
    ((const sexpr)&(sexpr_payload_str_etznab)),
    ((const sexpr)&(sexpr_payload_str_kawak))
};

static const sexpr haab_day_names [19] =
{
    ((const sexpr)&(sexpr_payload_str_pop)),
    ((const sexpr)&(sexpr_payload_str_wo)),
    ((const sexpr)&(sexpr_payload_str_sip)),
    ((const sexpr)&(sexpr_payload_str_sotz)),
    ((const sexpr)&(sexpr_payload_str_sek)),
    ((const sexpr)&(sexpr_payload_str_xul)),
    ((const sexpr)&(sexpr_payload_str_yaxkin)),
    ((const sexpr)&(sexpr_payload_str_mol)),
    ((const sexpr)&(sexpr_payload_str_chen)),
    ((const sexpr)&(sexpr_payload_str_yax)),
    ((const sexpr)&(sexpr_payload_str_sac)),
    ((const sexpr)&(sexpr_payload_str_keh)),
    ((const sexpr)&(sexpr_payload_str_mak)),
    ((const sexpr)&(sexpr_payload_str_kankin)),
    ((const sexpr)&(sexpr_payload_str_muwan)),
    ((const sexpr)&(sexpr_payload_str_pax)),
    ((const sexpr)&(sexpr_payload_str_kayab)),
    ((const sexpr)&(sexpr_payload_str_kumku)),
    ((const sexpr)&(sexpr_payload_str_wayeb))
};

static sexpr webroot          = sx_nonexistent;

static void configure_callback (sexpr sx)
{
    sexpr a = car (sx);

    if (truep (equalp (a, sym_root)))
    {
        webroot = car (cdr (sx));
    }
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

static sexpr include_file (sexpr fn, sexpr env)
{
    struct sexpr_io *io = sx_open_io (io_open_read (sx_string (fn)),
                                      io_open_null);
    sexpr data = sx_end_of_list;

    struct transdata td =
          { lx_environment_join (kho_environment, env), &data, 0 };

    multiplex_add_sexpr (io, include_on_read, &td);

    do
    {
        multiplex ();
    }
    while (td.done == 0);

    return sx_reverse (data);
}

static sexpr include_menu (sexpr file, sexpr env)
{
    sexpr lcodes = lx_environment_lookup (env, sym_language), fn;

    while (consp (lcodes))
    {
        fn = sx_join (webroot, str_slash,
                      sx_join (car (lcodes), str_slash,
                               sx_join (file, str_dot_ksu, str_nil)));

        if (truep (filep (fn)))
        {
            return include_file (fn, env);
        }

        lcodes = cdr (lcodes);
    }

    return sx_nonexistent;
}

static sexpr menu_selected_environment (sexpr a, sexpr b)
{
    sexpr e;
    const char *as = sx_string (a), *bs = sx_string (b);
    int i = 0, j = 0;

    while (as[i] && bs[j])
    {
        if (as[i] == bs[j])
        {
            i++;
            j++;
        }
        else if ((as[i] == '/') || (as[i] == '-'))
        {
            i++;
        }
        else if ((bs[j] == '/') || (bs[j] == '-'))
        {
            j++;
        }
        else
        {
            break;
        }
    }

    if ((as[i] == bs[j]) && (as[i] == (char)0))
    {
        e  = lx_make_environment
                (cons (cons (sym_class, str_selected), sx_end_of_list));
    }
    else
    {
        e  = lx_make_environment (sx_end_of_list);
    }

    return e;
}

static sexpr sub_menu (sexpr arguments, sexpr *env)
{
    sexpr sx = sx_end_of_list, target, name, t, e, title,
          ext = lx_environment_lookup (*env, sym_extension);

    title     = car (arguments);
    arguments = cdr (arguments);

    while (consp (arguments))
    {
        t      = car (arguments);
        target = car (t);
        name   = car (cdr (t));

        e = menu_selected_environment
                (target, lx_environment_lookup (*env, sym_base_name));

        sx = cons (cons (sym_item, cons (e, cons (cons (sym_link, cons
                (lx_make_environment (cons (cons (sym_href, (nexp (ext) ? target
                 : sx_join (target, str_dot, ext))),
                 sx_end_of_list)), cons (name,
                 sx_end_of_list))), sx_end_of_list))), sx);

        arguments = cdr (arguments);
    }

    return cons (cons (sym_sub_section, cons (title,
                 cons (cons (sym_list, sx_reverse (sx)), sx_end_of_list))),
                       sx_end_of_list);
}

static sexpr menu (sexpr arguments, sexpr *env)
{
    sexpr sx = sx_end_of_list, title;

    if (eolp (arguments))
    {
        return include_menu (title, *env);
    }

    title     = car (arguments);
    arguments = cdr (arguments);

    if (eolp (arguments))
    {
        return include_menu (title, *env);
    }

    while (consp (arguments))
    {
        sx = cons (sub_menu (car (arguments), env), sx);

        arguments = cdr (arguments);
    }

    return cons (sym_wrap, cons (lx_make_environment (cons (cons (sym_id,
             str_menu), sx_end_of_list)), cons (cons (sym_section, cons (title,
             cons (sx_reverse (sx), sx_end_of_list))), sx_end_of_list)));
}

static sexpr include_contact (sexpr file, sexpr env)
{
    sexpr fn  = sx_join (webroot, str_scontacts,
                         sx_join (file, str_dot_ksu, str_nil));

    if (truep (filep (fn)))
    {
        return include_file (fn, env);
    }

    return cons (sym_contact, cons (file, sx_end_of_list));
}

static sexpr contact_elaborate (sexpr args, sexpr *env)
{
    sexpr a = car (args);
    sexpr t, v, icon = str_png_icon_no_picture_png,
          fn = sx_nonexistent, ln = sx_nonexistent, sdesc = sx_nonexistent,
          te, ae = lx_environment_lookup (*env, sym_format);
    args = cdr (args);

    if (!truep (equalp (ae, str_text_xhtml)) &&
        !truep (equalp (ae, str_text_html)))
    {
        return sx_nonexistent;
    }

    while (consp (args))
    {
        a    = car (args);
        t    = car (a);
        v    = car (cdr (a));

        if (truep (equalp (t, sym_first_name)))
        {
            fn = v;
        }
        else if (truep (equalp (t, sym_last_name)))
        {
            ln = v;
        }
        else if (truep (equalp (t, sym_icon)))
        {
            icon = v;
        }
        else if (truep (equalp (t, sym_short_description)))
        {
            sdesc = v;
        }

        args = cdr (args);
    }

    te = lx_environment_join (kho_environment, *env);

/*    cons (sym_tr, cons (sx_join (fn, str_space, ln), cons (sdesc, cons (cons
           (sym_image, cons (icon, sx_end_of_list)), sx_end_of_list)))),*/

    return lx_eval (cons (sym_object, cons (cons (sym_document, cons
        (str_Contact, cons (cons (sym_menu, sx_end_of_list), cons(cons
        (sym_table, cons (cons (sym_tbody,
     cons (
        cons (sym_tr, cons (cons (sym_th, cons (sym_trContactName,
              sx_end_of_list)), cons (cons (sym_td, cons (sx_join (fn,
              str_space, ln), sx_end_of_list)), sx_end_of_list))),
        cons (cons (sym_tr, cons (cons (sym_th, cons (sym_trDescription,
              sx_end_of_list)), cons (cons (sym_td, cons (sdesc,
              sx_end_of_list)), sx_end_of_list))),
        cons (cons (sym_tr, cons (cons (sym_th, cons (sym_trIcon,
              sx_end_of_list)), cons (cons (sym_td, cons (cons(sym_image, cons
              (icon, sx_end_of_list)), sx_end_of_list)), sx_end_of_list))),
        sx_end_of_list)))),
        sx_end_of_list)), sx_end_of_list)))), sx_end_of_list)),
     &te);
}

static sexpr contact (sexpr args, sexpr *env)
{
    sexpr a = car (args);
    args    = cdr (args);

    if (eolp (args))
    {
        return include_contact (a, *env);
    }
    else
    {
        sexpr t, v, icon = str_png_icon_no_picture_png,
              fn = sx_nonexistent, ln = sx_nonexistent, sdesc = sx_nonexistent,
              n = a, te, ext;

        if (truep (lx_environment_lookup (*env, sym_elaborate)))
        {
            return contact_elaborate (cons (a, args), env);
        }

        ext = lx_environment_lookup (*env, sym_extension);

        while (consp (args))
        {
            a    = car (args);
            t    = car (a);
            v    = car (cdr (a));

            if (truep (equalp (t, sym_first_name)))
            {
                fn = v;
            }
            else if (truep (equalp (t, sym_last_name)))
            {
                ln = v;
            }
            else if (truep (equalp (t, sym_icon)))
            {
                icon = v;
            }
            else if (truep (equalp (t, sym_short_description)))
            {
                sdesc = v;
            }

            args = cdr (args);
        }

        te = lx_environment_join (kho_environment, *env);

        return lx_eval (cons (sym_object, cons (cons (sym_icon,
                 cons (sx_join (str_contact_dash, n,
                       (nexp (ext) ? str_nil
                                   : sx_join (str_dot, ext, str_nil))),
                 cons (sx_join (fn, str_space, ln),
                 cons (icon,
                 cons (sdesc, sx_end_of_list))))),
                 sx_end_of_list)), &te);
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

            if ((etarget[0]=='c') && (etarget[1]=='o') && (etarget[2]=='n') &&
                (etarget[3]=='t') && (etarget[4]=='a') && (etarget[5]=='c') &&
                (etarget[6]=='t') && (etarget[7]=='-'))
            {
                a2 = sx_join (str_contact_slash,make_string(etarget+8),str_nil);
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

static sexpr sx_date (sexpr arguments, sexpr *env)
{
    sexpr ta = car (arguments);
    char s [15], *se;
    struct date dt;
    sexpr ts;
    int i, j;
    int_date it = sx_integer (ta);

    dt_split_kin (it, &dt);

    s[0]  = ((dt.baktun / 10) % 10) + '0';
    s[1]  =  (dt.baktun       % 10) + '0';
    s[2]  =                           '.';
    s[3]  = ((dt.katun  / 10) % 10) + '0';
    s[4]  =  (dt.katun        % 10) + '0';
    s[5]  =                           '.';
    s[6]  = ((dt.tun    / 10) % 10) + '0';
    s[7]  =  (dt.tun          % 10) + '0';
    s[8]  =                           '.';
    s[9]  = ((dt.winal  / 10) % 10) + '0';
    s[10] =  (dt.winal        % 10) + '0';
    s[11] =                           '.';
    s[12] = ((dt.kin    / 10) % 10) + '0';
    s[13] =  (dt.kin          % 10) + '0';
    s[14] = 0;

    ts = make_string (s);

    i = (it + 4) % 13;

    s[0] = ((i / 10) % 10) + '0';
    s[1] =  (i       % 10) + '0';
    s[2] = 0;

    se = (s[0] == '0') ? (s + 1) : s;

    ts = sx_join (ts, str_comma_space,
                  sx_join (make_string (se), str_space,
                           tzolkin_day_names[(it % 20)]));

    i = (it - 17) % 365;

    j = (i % 20);

    s[0] = ((j / 10) % 10) + '0';
    s[1] =  (j       % 10) + '0';
    s[2] = 0;

    se = (s[0] == '0') ? (s + 1) : s;

    ts = sx_join (ts, str_comma_space,
                  sx_join (make_string (se), str_space,
                           haab_day_names[(i / 20)]));

    return cons (sym_div, cons (lx_make_environment (cons (cons (sym_class,
         str_date), sx_end_of_list)), cons (ts, sx_end_of_list)));
}

static sexpr sx_time (sexpr arguments, sexpr *env)
{
    sexpr ta = car (arguments);
    char s [5], *se;
    char ts = (unsigned int)sx_integer (ta) / (unsigned int)1000000;

    s[0] = ((ts / 100) % 10) + '0';
    s[1] = ((ts / 10)  % 10) + '0';
    s[2] =  (ts        % 10) + '0';
    s[3] = '%';
    s[4] = 0;

    se = (s[0]  == '0') ? (s  + 1) : s;
    se = (se[0] == '0') ? (se + 1) : se;

    return cons (sym_div, cons (lx_make_environment (cons (cons (sym_class,
         str_time), sx_end_of_list)), cons (make_string (se), sx_end_of_list)));
}

int cmain ()
{
    terminate_on_allocation_errors ();

    kho_configure_callback = configure_callback;

    initialise_khonsu ();

    kho_environment = lx_environment_bind
      (kho_environment, sym_menu, lx_foreign_lambda (sym_menu, menu));
    kho_environment = lx_environment_bind
      (kho_environment, sym_contact, lx_foreign_lambda (sym_contact, contact));
    kho_environment = lx_environment_bind
      (kho_environment, sym_request, lx_foreign_lambda (sym_request, request));
    kho_environment = lx_environment_bind
      (kho_environment, sym_date, lx_foreign_lambda (sym_date, sx_date));
    kho_environment = lx_environment_bind
      (kho_environment, sym_time, lx_foreign_lambda (sym_time, sx_time));

    while (multiplex () != mx_nothing_to_do)
    {
//        gc_invoke();
    }

    return 0;
}
