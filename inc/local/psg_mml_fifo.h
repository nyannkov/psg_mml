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
#ifndef PSG_MML_FIFO_H
#define PSG_MML_FIFO_H

#ifdef __cplusplus
extern "C" {
#endif/*__cplusplus*/

#define MSG_TYPE_DECODE_STATUS             (0x10)
#define MSG_TYPE_SETTINGS_1                (0x20)
#define MSG_TYPE_SETTINGS_2                (0x21)
#define MSG_TYPE_SETTINGS_3                (0x22)
#define MSG_TYPE_NOTE_ON                   (0x30)
#define MSG_TYPE_NOTE_ON_REST              (0x31)
#define MSG_TYPE_NOTE_OFF                  (0x40)
#define MSG_TYPE_LFO_1                     (0x50)
#define MSG_TYPE_LFO_2                     (0x51)
#define MSG_TYPE_SOFT_ENV_1                (0x60)
#define MSG_TYPE_SOFT_ENV_2                (0x71)
#define MSG_TYPE_SOFT_ENV_3                (0x72)
#define MSG_TYPE_SOFT_ENV_4                (0x73)

typedef struct
{
    uint8_t status;
    uint8_t reserve[3];
} MSG_TYPE_DECODE_STATUS_t;

typedef struct
{
    uint8_t addr;
    uint8_t data;
    uint8_t reserve[2];
} MSG_TYPE_SETTINGS_1_t;

typedef struct
{
    uint8_t addr1;
    uint8_t data1;
    uint8_t addr2;
    uint8_t data2;
} MSG_TYPE_SETTINGS_2_t;

typedef struct
{
    uint8_t tp_end_hi;
    uint8_t tp_end_lo;
    uint8_t gate_time_tk_hi;
    uint8_t gate_time_tk_lo;
} MSG_TYPE_SETTINGS_3_t;

typedef struct
{
    uint8_t addr;
    uint8_t data;
    uint8_t note_on_time_tk_hi;
    uint8_t note_on_time_tk_lo;
} MSG_TYPE_NOTE_ON_t;

typedef struct
{
    uint8_t addr;
    uint8_t data;
    uint8_t reserve[2];
} MSG_TYPE_NOTE_OFF_t;

typedef struct
{
    uint8_t mode;
    uint8_t depth;
    uint8_t delay_tk_hi;
    uint8_t delay_tk_lo;
} MSG_TYPE_LFO_1_t;

typedef struct
{
    uint8_t q12_omega_hh;
    uint8_t q12_omega_hl;
    uint8_t q12_omega_lh;
    uint8_t q12_omega_ll;
} MSG_TYPE_LFO_2_t;

typedef struct
{
    uint8_t mode;
    uint8_t top_volume;
    uint8_t sus_volume;
    uint8_t legato_effect;
} MSG_TYPE_SOFT_ENV_1_t;

typedef struct
{
    uint8_t attack_tk_hi;
    uint8_t attack_tk_lo;
    uint8_t hold_tk_hi;
    uint8_t hold_tk_lo;
} MSG_TYPE_SOFT_ENV_2_t;

typedef struct
{
    uint8_t decay_tk_hi;
    uint8_t decay_tk_lo;
    uint8_t fade_tk_hi;
    uint8_t fade_tk_lo;
} MSG_TYPE_SOFT_ENV_3_t;

typedef struct
{
    uint8_t release_tk_hi;
    uint8_t release_tk_lo;
    uint8_t reserve[2];
} MSG_TYPE_SOFT_ENV_4_t;

typedef struct
{
    uint8_t     type;
    union
    {
        uint8_t byte[4];
        MSG_TYPE_DECODE_STATUS_t    decode_status;
        MSG_TYPE_SETTINGS_1_t       settings_1;
        MSG_TYPE_SETTINGS_2_t       settings_2;
        MSG_TYPE_SETTINGS_3_t       settings_3;
        MSG_TYPE_NOTE_ON_t          note_on;
        MSG_TYPE_NOTE_OFF_t         note_off;
        MSG_TYPE_LFO_1_t            lfo_1;
        MSG_TYPE_LFO_2_t            lfo_2;
        MSG_TYPE_SOFT_ENV_1_t       soft_env_1;
        MSG_TYPE_SOFT_ENV_2_t       soft_env_2;
        MSG_TYPE_SOFT_ENV_3_t       soft_env_3;
        MSG_TYPE_SOFT_ENV_4_t       soft_env_4;
    } data;
} PSG_MML_CMD_t;

#ifndef PSG_MML_FIFO_SCALE
#define PSG_MML_FIFO_SCALE          (8)
#endif

#if PSG_MML_FIFO_SCALE < 2 
#error PSG_MML_FIFO_SCALE must be greater equal to 2.
#endif

#define MAX_PSG_MML_FIFO_LENGTH     (MAX_PSG_MML_MSG_NUM*PSG_MML_FIFO_SCALE+1)

#if MAX_PSG_MML_FIFO_LENGTH > 30000 
#error MAX_PSG_MML_FIFO_LENGTH must be less equal to 30,000.
#endif
typedef struct
{
    PSG_MML_CMD_t ring[MAX_PSG_MML_FIFO_LENGTH];
    int16_t front;
    int16_t back;
} PSG_MML_FIFO_t;

void psg_mml_fifo_init(PSG_MML_FIFO_t *p_fifo);
uint16_t psg_mml_fifo_get_free_space(const PSG_MML_FIFO_t *p_fifo);
bool psg_mml_fifo_enqueue(PSG_MML_FIFO_t *p_fifo, const PSG_MML_CMD_t p_cmd_list[], const uint16_t list_count);
bool psg_mml_fifo_dequeue(PSG_MML_FIFO_t *p_fifo, PSG_MML_CMD_t *p_cmd);

#ifdef __cplusplus
}
#endif/*__cplusplus*/

#endif/*PSG_MML_FIFO_H*/
