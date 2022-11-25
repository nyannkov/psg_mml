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
#include "../../inc/local/psg_mml_fifo.h"

static bool is_fifo_empty(const PSG_MML_FIFO_t *p_fifo);
static void *psg_mml_memcpy(void *p_dest, const void *p_src, size_t n);

void psg_mml_fifo_init(PSG_MML_FIFO_t *p_fifo)
{
    p_fifo->front = 0;
    p_fifo->back = 0;
}

uint16_t psg_mml_fifo_get_free_space(const PSG_MML_FIFO_t *p_fifo)
{
    int32_t d;
    d = p_fifo->front - p_fifo->back;
    return (uint16_t)((d >= 0) ? (MAX_PSG_MML_FIFO_LENGTH-1 - d) : (d*-1 - 1));
}

bool psg_mml_fifo_enqueue(PSG_MML_FIFO_t *p_fifo, const PSG_MML_CMD_t p_cmd_list[], const uint16_t list_count)
{
    uint16_t i;
    int16_t next_front;
    uint16_t fifo_free_space;
    
    fifo_free_space = psg_mml_fifo_get_free_space(p_fifo);
    if ( fifo_free_space < list_count )
    {
        return false;
    }

    next_front = p_fifo->front;
    for ( i = 0; i < list_count; i++ )
    {
        psg_mml_memcpy(&p_fifo->ring[next_front], &p_cmd_list[i], sizeof(PSG_MML_CMD_t));
        next_front++;
        if ( next_front >= MAX_PSG_MML_FIFO_LENGTH )
        {
            next_front = 0;
        }
    }

    p_fifo->front = next_front;
    return true;
}

bool psg_mml_fifo_dequeue(PSG_MML_FIFO_t *p_fifo, PSG_MML_CMD_t *p_cmd)
{
    int16_t next_back;

    if ( is_fifo_empty(p_fifo) )
    {
        return false;
    }

    next_back = p_fifo->back;
    psg_mml_memcpy(p_cmd, &p_fifo->ring[next_back], sizeof(PSG_MML_CMD_t));

    next_back++;
    if ( next_back >= MAX_PSG_MML_FIFO_LENGTH )
    {
        next_back = 0;
    }

    p_fifo->back = next_back;

    return true;
}

static bool is_fifo_empty(const PSG_MML_FIFO_t *p_fifo)
{
    return (p_fifo->front == p_fifo->back);
}

static void *psg_mml_memcpy(void *p_dest, const void *p_src, size_t n)
{
    size_t i;
    for ( i = 0; i < n; i++ )
    {
        ((uint8_t *)p_dest)[i] = ((uint8_t *)p_src)[i];
    }
    return p_dest;
}

