/*
 * MIT License
 * 
 * Copyright (c) 2022 nyannkov
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "../../inc/local/psg_mml_local.h"
#include "../../inc/local/psg_mml_utils.h"

void *psg_mml_memcpy(void *p_dest, const void *p_src, size_t n)
{
    size_t i;
    for ( i = 0; i < n; i++ )
    {
        ((uint8_t *)p_dest)[i] = ((uint8_t *)p_src)[i];
    }
    return p_dest;
}

void *psg_mml_memset(void *p_dest, int c, size_t n)
{
    size_t i;
    for ( i = 0; i < n; i++ )
    {
        ((uint8_t *)p_dest)[i] = c;
    }
    return p_dest;
}
