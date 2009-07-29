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
#include <duat/9p-server.h>

static struct io *debug_data_raw;
static struct sexpr_io *debug_data;
static struct io *configuration_data_raw;
static struct sexpr_io *configuration_data;
static struct dfs *root;
static sexpr hosted_configuration = sx_end_of_list;

static void on_root_close(struct d9r_io *io, void *aux)
{
}

static void on_debug_read
        (struct d9r_io *io, int_16 tag, struct dfs_file *file, int_64 offset,
         int_32 length)
{
    if (debug_data_raw->length > offset)
    {
        int len = debug_data_raw->length - offset;
        if (len > 7000) len = 7000;

        d9r_reply_read (io, tag, len, (int_8*)debug_data_raw->buffer + offset);
    }
    else
    {
        d9r_reply_read (io, tag, 0, (int_8*)"");
    }
}

static void on_cfg_read
        (struct d9r_io *io, int_16 tag, struct dfs_file *file, int_64 offset,
         int_32 length)
{
    d9r_reply_read (io, tag, 0, (int_8*)"");
}

void initialise_khonsu ()
{
    static char initialised = 0;

    if (!initialised)
    {
        struct dfs_directory *ctl;

        multiplex_d9s ();

        root = dfs_create (on_root_close, (void *)0);
        ctl  = dfs_mk_directory (root->root, "ctl");
        (void)dfs_mk_file (ctl, "debug", (char *)0, (int_8*)0, 0, (void *)0,
                           on_debug_read, (void *)0);
        (void)dfs_mk_file (ctl, "cfg", (char *)0, (int_8*)0, 0, (void *)0,
                           on_cfg_read, (void *)0);

        debug_data_raw = io_open_special ();
        debug_data = sx_open_io (io_open_null, debug_data_raw);
        configuration_data_raw = io_open_special ();
        configuration_data = sx_open_io (io_open_null, configuration_data_raw);

        multiplex_add_d9s_stdio (root);
        multiplex_add_d9s_socket ("temporary-socket", root);

        initialised = 1;
    }
}

void kho_debug (sexpr sx)
{
    sx_write (debug_data, sx);
}

void kho_configure (sexpr sx)
{
    hosted_configuration = cons (sx, hosted_configuration);
    sx_write (configuration_data, sx);
}
