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
#include <curie/sexpr.h>
#include <curie/main.h>
#include <curie/memory.h>
#include <curie/multiplex.h>
#include <curie/network.h>
#include <curie/io.h>

define_symbol (sym_post,                "post");
define_symbol (sym_put,                 "put");
define_symbol (sym_delete,              "delete");
define_symbol (sym_options,             "options");
define_symbol (sym_head,                "head");
define_symbol (sym_accept,              "accept");
define_symbol (sym_accept_language,     "accept-language");
define_symbol (sym_user_agent,          "user-agent");
define_symbol (sym_request_body_type,   "request-body-type");
define_symbol (sym_request_body_length, "request-body-length");
define_symbol (sym_method,              "method");

define_string (str_index,               "index");

define_string (str_error_transcript_not_possible_xhtml,
                                        "/error/transcript-not-possible.xhtml");
define_string (str_text_plain,          "text/plain");

#define KHONSU_SOCKET_ENVIRONMENT        "KHONSU_SOCKET="
#define SCRIPT_NAME_ENVIRONMENT          "PATH_INFO="
#define HTTP_ACCEPT_ENVIRONMENT          "HTTP_ACCEPT="
#define HTTP_ACCEPT_LANGUAGE_ENVIRONMENT "HTTP_ACCEPT_LANGUAGE="
#define HTTP_USER_AGENT_ENVIRONMENT      "HTTP_USER_AGENT="
#define REQUEST_METHOD_ENVIRONMENT       "REQUEST_METHOD="
#define CONTENT_TYPE_ENVIRONMENT         "CONTENT_TYPE="
#define CONTENT_LENGTH_ENVIRONMENT       "CONTENT_LENGTH="
#define KHONSU_CGI_IDENTIFIER 10

#define HTTP_STATUS               "Status: "
#define HTTP_200_OK               "200 OK"
#define HTTP_404_FNF              "404 File not Found"
#define HTTP_500_ISE              "500 Internal Server Error"
#define HTTP_CONTENT_LENGTH       "Content-Length: "
#define HTTP_CONTENT_TYPE         "Content-Type: "

#define BAD_OUTPUT\
   "Content-Type: text/html\r\nStatus: 503 Broken Connection\r\n\r\n<html><head><title>Broken Connection</title></head><body><h1>Broken Connection</h1><p>Khonsu's CGI redirection programme seems to have trouble connecting to the core programme. This is usually a problem with the server's configuration.</p></body></html>"

#define MAX_NUM_LENGTH            32

static const char *socket_path = "khonsu-socket";
static struct io *out;
static sexpr script_name;
static sexpr id_token;
static struct sexpr_io *io;
static sexpr rq_environment;
static sexpr rq_method;
static char no_output = 1;

static void request (sexpr env, sexpr sn)
{
    id_token = cons (make_integer (KHONSU_CGI_IDENTIFIER), sn);

    sx_write (io, cons (sym_request, cons (id_token,
                cons (cons (rq_method, cons (env, cons (sn,
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
    } while ((r != io_end_of_file) && (r != io_unrecoverable_error));

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
            sexpr mime     = sx_nonexistent;
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
                request (rq_environment,
                         str_error_transcript_not_possible_xhtml);
            }
            else
            {
                unsigned int n, ml;
                const char *m;
                const char *e1, *e2;

                if (!nexp (mime))
                {
                    m = sx_string (mime);

                    for (ml = 0; m[ml] != (char)0;     ml++);
                }

                for (n = 0; output[n] != (char)0; n++);

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

                if (!nexp (mime))
                {
                    io_collect (out, HTTP_CONTENT_TYPE,
                                (sizeof (HTTP_CONTENT_TYPE) -1));
                    io_collect (out, m, ml);
                    io_collect (out, "\r\n", 2);
                }

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

                no_output = 0;

                multiplex_del_sexpr (io);
            }
        }
    }
}

#define env_test_add(j,ev,key,env,mk)\
        for (j = 0; ((c = t[j]) != (char)0) && (c == ev[j]); j++);\
        if ((j == (sizeof (ev) - 1)) && (t[0] == ev[0]))\
        {\
            env = lx_environment_bind\
                  (env, key,\
                   mk (t + (sizeof (ev) - 1)));\
            continue;\
        }\

int cmain ()
{
    int i, j;
    char c;
    sexpr env;

    terminate_on_allocation_errors ();

    script_name = str_index;

    multiplex_io ();
    multiplex_sexpr ();
    multiplex_network ();
    initialise_seteh ();

    env = lx_make_environment (sx_end_of_list);
    rq_method = sym_get;

    out = io_open_stdout ();

    for (i = 0; curie_environment[i] != (char *)0; i++)
    {
        const char *t = curie_environment[i];

        for (j = 0;
             ((c = t[j]) != (char)0) && (c == KHONSU_SOCKET_ENVIRONMENT[j]);
             j++);

        if ((j == (sizeof (KHONSU_SOCKET_ENVIRONMENT) - 1)) &&
            (t[0] == KHONSU_SOCKET_ENVIRONMENT[0]))
        {
            socket_path = t + (sizeof (KHONSU_SOCKET_ENVIRONMENT) - 1);
            continue;
        }

        for (j = 0;
             ((c = t[j]) != (char)0) && (c == SCRIPT_NAME_ENVIRONMENT[j]);
             j++);

        if ((j == (sizeof (SCRIPT_NAME_ENVIRONMENT) - 1)) &&
            (t[0] == SCRIPT_NAME_ENVIRONMENT[0]))
        {
            script_name =
                kho_canonicalise
                    (make_string (t + (sizeof (SCRIPT_NAME_ENVIRONMENT) - 1)));

            continue;
        }

        for (j = 0;
             ((c = t[j]) != (char)0) && (c == REQUEST_METHOD_ENVIRONMENT[j]);
             j++);

        if ((j == (sizeof (REQUEST_METHOD_ENVIRONMENT) - 1)) &&
             (t[0] == REQUEST_METHOD_ENVIRONMENT[0]))
        {
            const char *b = t + (sizeof (REQUEST_METHOD_ENVIRONMENT) - 1);

            switch (b[0])
            {
                case 'P':
                    if ((b[1] == 'O') && (b[2] == 'S') && (b[3] == 'T') &&
                         (b[4] == (char)0))
                    {
                        rq_method = sym_post;
                    } else
                    if ((b[1] == 'U') && (b[2] == 'T') && (b[3] == (char)0))
                    {
                        rq_method = sym_put;
                    }
                    break;
                case 'D':
                    if ((b[1] == 'E') && (b[2] == 'L') && (b[3] == 'E') &&
                        (b[4] == 'T') && (b[5] == 'E') && (b[6] == (char)0))
                    {
                        rq_method = sym_delete;
                    }
                    break;
                case 'H':
                    if ((b[1] == 'E') && (b[2] == 'A') && (b[3] == 'D') &&
                         (b[4] == (char)0))
                    {
                        rq_method = sym_head;
                    }
                    break;
                case 'O':
                    if ((b[1] == 'P') && (b[2] == 'T') && (b[3] == 'I') &&
                        (b[4] == 'O') && (b[5] == 'N') && (b[6] == 'S') &&
                        (b[7] == (char)0))
                    {
                        rq_method = sym_options;
                    }
                    break;
            }
            continue;
        }

        env_test_add (j, CONTENT_TYPE_ENVIRONMENT, sym_request_body_type, env,
                      make_string);
        env_test_add (j, CONTENT_LENGTH_ENVIRONMENT, sym_request_body_length,
                      env, make_symbol);
        env_test_add (j, HTTP_ACCEPT_ENVIRONMENT, sym_accept, env,
                      make_string);
        env_test_add (j, HTTP_ACCEPT_LANGUAGE_ENVIRONMENT, sym_accept_language,
                      env, make_string);
        env_test_add (j, HTTP_USER_AGENT_ENVIRONMENT, sym_user_agent, env,
                      make_string);
    }

    multiplex_add_io (out, (void *)0, (void *)0, (void *)0);
    io = sx_open_socket (socket_path);
    multiplex_add_sexpr (io, on_socket_read, (void *)0);

    rq_environment = env;
    request (env, script_name);

    while (multiplex () != mx_nothing_to_do);

    if (no_output)
    {
        io_write (out, BAD_OUTPUT, sizeof(BAD_OUTPUT));
    }

    return 0;
}
