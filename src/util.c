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

define_string (str_undefined_o, "(undefined-object ");
define_string (str_undefined_e, ")");

sexpr kho_merge (sexpr arguments)
{
    sexpr r = str_nil;

    while (consp (arguments))
    {
        sexpr n = car (arguments);

        if (consp (n))
        {
            n = sx_join (str_undefined_o, car (n), str_undefined_e);
        }

        if (stringp (n))
        {
            r = sx_join (r, n, str_nil);
        }

        arguments = cdr (arguments);
    }

    return r;
}

sexpr kho_tagmerge (sexpr arguments, sexpr pre, sexpr post)
{
    return sx_join (pre, kho_merge (arguments), post);
}

sexpr kho_canonicalise (sexpr string)
{
    const char *s = sx_string (string);
    char st [KHO_MAX_PATH_LENGTH];
    unsigned int i = 0, j = 0, ld = 0, cc = 0;
    char t = s[0];

    while ((t != (char)0) && (j < (KHO_MAX_PATH_LENGTH - 1)))
    {
        st[j] = t;
        j++;

        switch (t)
        {
            case '.':
                ld++;
                break;
            case '/':
                if ((cc == 0) && (ld < 3))
                {
                    if (ld == 0)
                    {
                        j--;
                    }
                    else if (ld == 1)
                    {
                        j--;
                        do
                        {
                            j--;
                        }
                        while ((j > 0) && (st[j] != '/'));

                        if (j > 0)
                        {
                            j++;
                        }
                    }
                    else if (ld == 2)
                    {
                        j--;
                        do
                        {
                            j--;
                        }
                        while ((j > 0) && (st[j] != '/'));

                        if (j > 0)
                        {
                            j--;
                            do
                            {
                                j--;
                            }
                            while ((j > 0) && (st[j] != '/'));

                            if (j > 0)
                            {
                                j++;
                            }
                        }
                    }
                }

                ld = 0;
                cc = 0;
                break;
            default:
                cc++;
        }

        i++;
        t = s[i];
    }

    st[j] = 0;

    return make_string (st);
}

sexpr kho_normalise (sexpr string)
{
    sexpr sx      = kho_canonicalise (string);
    const char *s = sx_string        (sx);
    char st [KHO_MAX_PATH_LENGTH];
    char *stp = st;
    unsigned int i = 0, j = 0;
    char l = 0, t;

    while ((t = s[i]) != (char)0)
    {
        if (((t >= 'a') && (t <= 'z')) ||
            ((t >= 'A') && (t <= 'Z')) ||
            ((t >= '0') && (t <= '9')))
        {
            st[j] = t;
            l = (char)0;
            j++;
        }
        else if (l == (char)0)
        {
            st[j] = '-';
            j++;
            l = (char)1;
        }

        i++;
    }

    if (j > 0)
    {
        do
        {
            j--;
        }
        while (st[j] == '-');
        j++;
    }

    st[j] = (char)0;

    while ((*stp) == '-')
    {
        stp++;
    }

    return make_string (stp);
}
