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
#include <curie/sexpr.h>
#include <curie/main.h>

define_string (str_test_1,    "a/b/c/d");
define_string (str_test_2,    "a/b/../b/c/d");
define_string (str_test_3,    "a/b/./c/d");
define_string (str_test_4,    "a/b/./c/d/.");
define_string (str_test_5,    "a/b/./c/d/./");
define_string (str_test_6,    "./a/b/./c/d/./");
define_string (str_test_7,    "../a/b/./c/d/./");
define_string (str_test_8,    "../a/b/c/./c/../d/./");
define_string (str_test_9,    "../a/b/c/./c/.././d/./");
define_string (str_result_1,  "a/b/c/d");
define_string (str_result_2,  "a/b/c/d/.");
define_string (str_result_3,  "a/b/c/d/");

define_string (str_test_a1,   "/////a/b/c/d");
define_string (str_test_a2,   "/a/b/../b/c/d");
define_string (str_test_a3,   "/a/b/.////c/d");
define_string (str_test_a4,   "/a/b/./c/d/.");
define_string (str_test_a5,   "/a/b/./c/d/./");
define_string (str_test_a6,   "/./a/b/./c/d/./");
define_string (str_test_a7,   "/../a/b/./c/d/./");
define_string (str_test_a8,   "/../a/b/c/./c/../d/./");
define_string (str_test_a9,   "/../a/b/c/./c/.././d/./");

define_string (str_result_4,  "a-b-c-d");

int cmain ()
{
    struct sexpr_io *sio = sx_open_stdio ();
    sexpr t;

    if (!truep (equalp (t = kho_canonicalise (str_test_1), str_result_1)))
    {
        sx_write (sio, t);
        return 1;
    }

    if (!truep (equalp (t = kho_canonicalise (str_test_2), str_result_1)))
    {
        sx_write (sio, t);
        return 2;
    }

    if (!truep (equalp (t = kho_canonicalise (str_test_3), str_result_1)))
    {
        sx_write (sio, t);
        return 3;
    }

    if (!truep (equalp (t = kho_canonicalise (str_test_4), str_result_2)))
    {
        sx_write (sio, t);
        return 4;
    }

    if (!truep (equalp (t = kho_canonicalise (str_test_5), str_result_3)))
    {
        sx_write (sio, t);
        return 5;
    }

    if (!truep (equalp (t = kho_canonicalise (str_test_6), str_result_3)))
    {
        sx_write (sio, t);
        return 6;
    }

    if (!truep (equalp (t = kho_canonicalise (str_test_7), str_result_3)))
    {
        sx_write (sio, t);
        return 7;
    }

    if (!truep (equalp (t = kho_canonicalise (str_test_8), str_result_3)))
    {
        sx_write (sio, t);
        return 8;
    }

    if (!truep (equalp (t = kho_canonicalise (str_test_9), str_result_3)))
    {
        sx_write (sio, t);
        return 9;
    }

    if (truep (equalp (t = kho_canonicalise (str_test_9), str_result_2)))
    {
        sx_write (sio, t);
        return 10;
    }

    if (!truep (equalp (t = kho_canonicalise (str_test_a1), str_result_1)))
    {
        sx_write (sio, t);
        return 11;
    }

    if (!truep (equalp (t = kho_canonicalise (str_test_a2), str_result_1)))
    {
        sx_write (sio, t);
        return 12;
    }

    if (!truep (equalp (t = kho_canonicalise (str_test_a3), str_result_1)))
    {
        sx_write (sio, t);
        return 13;
    }

    if (!truep (equalp (t = kho_canonicalise (str_test_a4), str_result_2)))
    {
        sx_write (sio, t);
        return 14;
    }

    if (!truep (equalp (t = kho_canonicalise (str_test_a5), str_result_3)))
    {
        sx_write (sio, t);
        return 15;
    }

    if (!truep (equalp (t = kho_canonicalise (str_test_a6), str_result_3)))
    {
        sx_write (sio, t);
        return 16;
    }

    if (!truep (equalp (t = kho_canonicalise (str_test_a7), str_result_3)))
    {
        sx_write (sio, t);
        return 17;
    }

    if (!truep (equalp (t = kho_canonicalise (str_test_a8), str_result_3)))
    {
        sx_write (sio, t);
        return 18;
    }

    if (!truep (equalp (t = kho_canonicalise (str_test_a9), str_result_3)))
    {
        sx_write (sio, t);
        return 19;
    }

    if (truep (equalp (t = kho_canonicalise (str_test_a9), str_result_2)))
    {
        sx_write (sio, t);
        return 20;
    }

    if (!truep (equalp (t = kho_normalise (str_test_1), str_result_4)))
    {
        sx_write (sio, t);
        return 21;
    }

    if (!truep (equalp (t = kho_normalise (str_test_2), str_result_4)))
    {
        sx_write (sio, t);
        return 22;
    }

    if (!truep (equalp (t = kho_normalise (str_test_3), str_result_4)))
    {
        sx_write (sio, t);
        return 23;
    }

    if (!truep (equalp (t = kho_normalise (str_test_4), str_result_4)))
    {
        sx_write (sio, t);
        return 24;
    }

    if (!truep (equalp (t = kho_normalise (str_test_5), str_result_4)))
    {
        sx_write (sio, t);
        return 25;
    }

    if (!truep (equalp (t = kho_normalise (str_test_6), str_result_4)))
    {
        sx_write (sio, t);
        return 26;
    }

    if (!truep (equalp (t = kho_normalise (str_test_7), str_result_4)))
    {
        sx_write (sio, t);
        return 27;
    }

    if (!truep (equalp (t = kho_normalise (str_test_8), str_result_4)))
    {
        sx_write (sio, t);
        return 28;
    }

    if (!truep (equalp (t = kho_normalise (str_test_9), str_result_4)))
    {
        sx_write (sio, t);
        return 29;
    }

    if (!truep (equalp (t = kho_normalise (str_test_a1), str_result_4)))
    {
        sx_write (sio, t);
        return 30;
    }

    if (!truep (equalp (t = kho_normalise (str_test_a2), str_result_4)))
    {
        sx_write (sio, t);
        return 31;
    }

    if (!truep (equalp (t = kho_normalise (str_test_a3), str_result_4)))
    {
        sx_write (sio, t);
        return 32;
    }

    if (!truep (equalp (t = kho_normalise (str_test_a4), str_result_4)))
    {
        sx_write (sio, t);
        return 33;
    }

    if (!truep (equalp (t = kho_normalise (str_test_a5), str_result_4)))
    {
        sx_write (sio, t);
        return 34;
    }

    if (!truep (equalp (t = kho_normalise (str_test_a6), str_result_4)))
    {
        sx_write (sio, t);
        return 35;
    }

    if (!truep (equalp (t = kho_normalise (str_test_a7), str_result_4)))
    {
        sx_write (sio, t);
        return 36;
    }

    if (!truep (equalp (t = kho_normalise (str_test_a8), str_result_4)))
    {
        sx_write (sio, t);
        return 37;
    }

    if (!truep (equalp (t = kho_normalise (str_test_a9), str_result_4)))
    {
        sx_write (sio, t);
        return 38;
    }

    return 0;
}
