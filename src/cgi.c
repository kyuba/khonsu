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

#include <seteh/lambda.h>
#include <curie/sexpr.h>
#include <curie/main.h>
#include <curie/memory.h>
#include <curie/multiplex.h>
#include <curie/network.h>
#include <curie/io.h>

define_symbol (sym_request,        "request");
define_symbol (sym_reply,          "reply");
define_symbol (sym_get,            "get");
define_symbol (sym_verbatim,       "verbatim");
define_symbol (sym_format,         "format");
define_symbol (sym_error,          "error");
define_symbol (sym_language,       "language");
define_symbol (sym_file_not_found, "file-not-found");

define_string (str_index,          "index.xhtml");
define_string (str_nil,            "");

define_string (str_error_transcript_not_possible_xhtml,
               "/error/transcript-not-possible.xhtml");
define_string (str_text_plain,
               "text/plain");

#define KHONSU_SOCKET_ENVIRONMENT "KHONSU_SOCKET="
#define SCRIPT_NAME_ENVIRONMENT   "PATH_INFO="
#define KHONSU_CGI_IDENTIFIER 10

#define HTTP_STATUS               "Status: "
#define HTTP_200_OK               "200 OK"
#define HTTP_404_FNF              "404 File not Found"
#define HTTP_500_ISE              "500 Internal Server Error"
#define HTTP_CONTENT_LENGTH       "Content-Length: "
#define HTTP_CONTENT_TYPE         "Content-Type: "
#define HTTP_XHTML_MIME           "application/xhtml+xml"
#define HTTP_HTML_MIME            "text/html"

#define MAX_NUM_LENGTH            32

static const char *socket_path = "khonsu-socket";
static struct io *out;
static sexpr script_name;
static sexpr id_token;
static struct sexpr_io *io;

static void request (sexpr sn)
{
    id_token = cons (make_integer (KHONSU_CGI_IDENTIFIER), sn);

    sx_write (io, cons (sym_request, cons (id_token,
                cons (cons (sym_get, cons (sx_nil, cons (sn,
                            sx_end_of_list))),
                      sx_end_of_list))));
}

static void write_content_length (unsigned long len)
{
    unsigned int l = (MAX_NUM_LENGTH - 1), k;
    char b[MAX_NUM_LENGTH] = { 0 };
    io_collect (out, HTTP_CONTENT_LENGTH,
                (sizeof (HTTP_CONTENT_LENGTH) -1));

    k = len;

    while (k > 10)
    {
        b[l] = ('0' + (k % 10));
        k /= 10;
        l--;
    }

    b[l] = ('0' + k);
    io_collect (out, b + l, (MAX_NUM_LENGTH) - l);
    io_collect (out, "\r\n", 2);
}

static void verbatim_reply (sexpr sxfile)
{
    struct io *in = io_open_read (sx_string (sxfile));
    enum io_result r;

    do
    {
        r = io_read (in);
    } while (r != io_end_of_file);

    write_content_length (in->length);
    io_collect (out, "\r\n", 2);

    io_collect (out, in->buffer, in->length - in->position);

    io_close (in);
}

static void on_socket_read (sexpr sx, struct sexpr_io *io, void *aux)
{
    sexpr sa = car (sx);

    if (truep (equalp (sa, sym_reply)))
    {
        sx = cdr (sx);
        sa = car (sx);

        if (truep (equalp (sa, id_token)))
        {
            const char *output = (const char *)0;
            sexpr sxoutput = str_nil;
            sexpr env      = lx_make_environment (sx_end_of_list);
            sexpr mime     = str_text_plain;
            sexpr verbatim = sx_nonexistent;
            sexpr t, ta, tb;

            sx = cdr (sx);

            while (consp (sx))
            {
                sa = car (sx);
                if (environmentp (sa))
                {
                    env = lx_environment_join (env, sa);
                }
                else if (stringp (sa))
                {
                    sxoutput = sx_join (sxoutput, sa, str_nil);
                }
                else if (consp (sa))
                {
                    if (truep (equalp (car (sa), sym_verbatim)))
                    {
                        verbatim = car (cdr (sa));
                    }
                }
                sx = cdr (sx);
            }

            if (!nexp (t = lx_environment_lookup (env, sym_format)))
            {
                mime = t;
            }

            output = sx_string (sxoutput);

            if (output == (const char *)0)
            {
                request (str_error_transcript_not_possible_xhtml);
            }
            else
            {
                unsigned int n, ml;
                const char *m = sx_string (mime);
                const char *e1, *e2;

                for (ml = 0; m[ml] != (char)0;     ml++);
                for (n = 0;  output[n] != (char)0; n++);

                io_collect (out, HTTP_STATUS, (sizeof (HTTP_STATUS) -1));

                if (!nexp (t = lx_environment_lookup (env, sym_error)))
                {
                    if (truep (equalp (t, sym_file_not_found)))
                    {
                        io_collect (out, HTTP_404_FNF,
                                    (sizeof (HTTP_404_FNF) -1));
                    }
                    else
                    {
                        io_collect (out, HTTP_500_ISE,
                                    (sizeof (HTTP_500_ISE) -1));
                    }
                }
                else
                {
                    io_collect (out, HTTP_200_OK, (sizeof (HTTP_200_OK) -1));
                }
                io_collect (out, "\r\n", 2);

                io_collect (out, HTTP_CONTENT_TYPE,
                            (sizeof (HTTP_CONTENT_TYPE) -1));
                io_collect (out, m, ml);
                io_collect (out, "\r\n", 2);

                if (!nexp (verbatim))
                {
                    verbatim_reply (verbatim);
                }
                else
                {
                    write_content_length (n);

                    t = lx_environment_alist (env);
                    while (consp (t))
                    {
                        ta = car (t);
                        tb = cdr (ta);
                        ta = car (ta);

                        if (truep (equalp (ta, sym_error)) ||
                            truep (equalp (ta, sym_format)))
                        {
                            t = cdr (t);
                            continue;
                        }

                        e1 = sx_symbol (ta);
                        e2 = sx_string (tb);

                        for (ml = 0; e1[ml] != (char)0; ml++);
                        io_collect (out, e1, ml);
                        io_collect (out, ": ", 2);
                        for (ml = 0; e2[ml] != (char)0; ml++);
                        io_collect (out, e2, ml);
                        io_collect (out, "\r\n", 2);

                        t  = cdr (t);
                    }

                    io_collect (out, "\r\n", 2);

                    io_write (out, output, n);
                }

                multiplex_del_sexpr (io);
            }
        }
    }
}

int cmain ()
{
    int i = 0, j;
    char c;

    terminate_on_allocation_errors ();

    script_name = str_index;

    multiplex_io ();
    multiplex_sexpr ();
    multiplex_network ();
    initialise_seteh ();

    out = io_open_stdout ();

    while (curie_environment[i] != (char *)0)
    {
        const char *t = curie_environment[i];

        for (j = 0;
             ((c = t[j]) != (char)0) && (c == KHONSU_SOCKET_ENVIRONMENT[j]);
             j++);

        if ((j == (sizeof (KHONSU_SOCKET_ENVIRONMENT) - 1)) &&
            (t[0] == KHONSU_SOCKET_ENVIRONMENT[0]))
        {
            socket_path = t + (sizeof (KHONSU_SOCKET_ENVIRONMENT) - 1);
        }
        else
        {
            for (j = 0;
                 ((c = t[j]) != (char)0) && (c == SCRIPT_NAME_ENVIRONMENT[j]);
                 j++);

            if ((j == (sizeof (SCRIPT_NAME_ENVIRONMENT) - 1)) &&
                (t[0] == SCRIPT_NAME_ENVIRONMENT[0]))
            {
                script_name =
                    make_string (t + (sizeof (SCRIPT_NAME_ENVIRONMENT) - 1));
            }
        }

        i++;
    }

    multiplex_add_io (out, (void *)0, (void *)0, (void *)0);
    io = sx_open_socket (socket_path);
    multiplex_add_sexpr (io, on_socket_read, (void *)0);

    request (script_name);

    while (multiplex () != mx_nothing_to_do);

    return 0;
}
