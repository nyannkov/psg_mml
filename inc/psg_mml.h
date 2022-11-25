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
#ifndef PSG_MML_H
#define PSG_MML_H

#ifdef __cplusplus
extern "C" {
#endif/*__cplusplus*/

#include "psg_mml_conf.h"

#define PSG_MML_SLOT_TOTAL_NUM                  (2)

typedef enum
{
    E_PSG_MML_PLAY_STATE_STOP = 0,
    E_PSG_MML_PLAY_STATE_PLAY,
    E_PSG_MML_PLAY_STATE_END
} PSG_MML_PLAY_STATE_t;

/* RETURN CODE */
typedef int32_t psg_mml_t;
#define PSG_MML_PLAY_END                        (psg_mml_t)(3)
#define PSG_MML_DECODE_END                      (psg_mml_t)(2)
#define PSG_MML_DECODE_QUEUE_IS_FULL            (psg_mml_t)(1)
#define PSG_MML_SUCCESS                         (psg_mml_t)(0)

#define PSG_MML_NOT_INITIALIZED                 (psg_mml_t)(-1)
#define PSG_MML_OUT_OF_SLOT                     (psg_mml_t)(-2)
#define PSG_MML_TEXT_NULL                       (psg_mml_t)(-3)
#define PSG_MML_TEXT_OVER_LENGTH                (psg_mml_t)(-4)
#define PSG_MML_TEXT_EMPTY                      (psg_mml_t)(-5)
#define PSG_MML_INTERNAL_ERROR                  (psg_mml_t)(-100)

void       psg_mml_periodic_control(void);

psg_mml_t  psg_mml_init(uint8_t slot, uint16_t tick_hz, void (*p_write)(uint8_t addr, uint8_t data));
psg_mml_t  psg_mml_deinit(uint8_t slot);
psg_mml_t  psg_mml_load_text(uint8_t slot, const char *p_mml_text, uint16_t flags);
psg_mml_t  psg_mml_decode(uint8_t slot);
psg_mml_t  psg_mml_play_start(uint8_t slot, uint16_t num_predecode);
psg_mml_t  psg_mml_play_restart(uint8_t slot, uint16_t num_predecode);
psg_mml_t  psg_mml_play_stop(uint8_t slot);
psg_mml_t  psg_mml_get_play_state(uint8_t slot, /*@out@*/PSG_MML_PLAY_STATE_t *p_out);


#ifdef __cplusplus
}
#endif/*__cplusplus*/

#endif/*PSG_MML_H*/

