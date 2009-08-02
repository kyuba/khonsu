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

#include <curie/sexpr.h>
#include <curie/main.h>
#include <curie/memory.h>
#include <curie/multiplex.h>
#include <curie/network.h>
#include <curie/io.h>

define_symbol (sym_request, "request");
define_symbol (sym_reply,   "reply");
define_symbol (sym_get,     "get");

define_string (str_index,   "index.xhtml");

#define KHONSU_SOCKET_ENVIRONMENT "KHONSU_SOCKET="
#define SCRIPT_NAME_ENVIRONMENT   "PATH_INFO="
#define KHONSU_CGI_IDENTIFIER 10

#define HTTP_STATUS               "Status: "
#define HTTP_200_OK               "200 OK"
#define HTTP_CONTENT_LENGTH       "Content-Length: "
#define HTTP_CONTENT_TYPE         "Content-Type: "
#define HTTP_XHTML_MIME           "application/xhtml+xml"
#define HTTP_HTML_MIME            "text/html"

#define MAX_NUM_LENGTH            32

static const char *socket_path = "khonsu-socket";
static struct io *out;
static sexpr script_name;
static sexpr id_token;

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
            sx = cdr (sx);
            sa = car (sx);

            while (!stringp (sa) && consp (sx))
            {
                sx = cdr (sx);
                sa = car (sx);
            }

            if (stringp (sa))
            {
                output = sx_string (sa);
            }

            if (output != (const char *)0)
            {
                unsigned int n, l = (MAX_NUM_LENGTH - 1), k;
                char b[MAX_NUM_LENGTH] = { 0 };

                for (n = 0; output[n] != (char)0; n++);

                io_collect (out, HTTP_STATUS, (sizeof (HTTP_STATUS) -1));
                io_collect (out, HTTP_200_OK, (sizeof (HTTP_200_OK) -1));
                io_collect (out, "\r\n", 2);

                io_collect (out, HTTP_CONTENT_LENGTH,
                            (sizeof (HTTP_CONTENT_LENGTH) -1));

                k = n;

                while (k > 10)
                {
                    b[l] = ('0' + (k % 10));
                    k /= 10;
                    l--;
                }

                b[l] = ('0' + k);
                io_collect (out, b + l, (MAX_NUM_LENGTH) - l);
                io_collect (out, "\r\n", 2);

                io_collect (out, HTTP_CONTENT_TYPE,
                            (sizeof (HTTP_CONTENT_TYPE) -1));
                io_collect (out, HTTP_XHTML_MIME,
                            (sizeof (HTTP_XHTML_MIME) -1));
                io_collect (out, "\r\n\r\n", 4);

                io_write (out, output, n);
            }

            multiplex_del_sexpr (io);
        }
    }
}

int cmain ()
{
    int i = 0, j;
    char c;
    struct sexpr_io *io;

    terminate_on_allocation_errors ();

    script_name = str_index;

    multiplex_io ();
    multiplex_sexpr ();
    multiplex_network ();

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

    id_token = cons (make_integer (KHONSU_CGI_IDENTIFIER),
                     script_name);

    multiplex_add_io (out, (void *)0, (void *)0, (void *)0);

/*    io = sx_open_stdio ();*/
    io = sx_open_socket (socket_path);

    sx_write (io, cons (sym_request, cons (id_token,
                    cons (cons (sym_get, cons (sx_nil, cons (script_name,
                                sx_end_of_list))),
                          sx_end_of_list))));

    multiplex_add_sexpr (io, on_socket_read, (void *)0);

    while (multiplex () != mx_nothing_to_do);

    return 0;
}
