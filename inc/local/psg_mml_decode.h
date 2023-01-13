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
#ifndef PSG_MML_DECODE_H
#define PSG_MML_DECODE_H

#ifdef __cplusplus
extern "C" {
#endif/*__cplusplus*/


typedef int32_t psg_mml_decode_t;
#define PSG_MML_DECODE_CHANNEL_END                  (2)
#define PSG_MML_DECODE_CHANNEL_START                (1)
#define PSG_MML_DECODE_SUCCESS                      (0)

typedef enum
{
    E_MML_TEXT_LOAD_STATE_UNINITIALIZED = 0,
    E_MML_TEXT_LOAD_STATE_UNLOAD_MML_TEXT,
    E_MML_TEXT_LOAD_STATE_READY
} MML_TEXT_LOAD_STATE_t;

typedef struct
{
    const char *p_mml_text;
    uint8_t    use_channel_num;
    struct
    {
        const char *p_mml_head;
        const char *p_mml_tail;
        const char *p_mml_pos;
        const char *p_mml_loop_head[MAX_LOOP_NESTING_DEPTH];
        bool        reset_state;
        bool        end_state;
        uint16_t    loop_nesting_depth;
        int16_t     loop_times[MAX_LOOP_NESTING_DEPTH];
    } channel[NUM_CHANNEL];
} MML_TEXT_INFO_t;

typedef struct
{
    uint16_t    ep;
    uint8_t     env_shape;
    uint8_t     np;
    struct
    {
        uint8_t     tempo;
        uint8_t     note_len;
        uint8_t     note_len_dots;
        uint8_t     vol_ctrl;
        uint8_t     octave;
        uint8_t     gate_time;
        q12_t       q12_time_frac_part;
        q24_t       q24_detune;
        q24_t       q24_pitchbend;
        uint8_t     lfo_mode;
        uint8_t     lfo_speed;
        uint8_t     lfo_depth;
        uint8_t     lfo_delay_tk;
        uint8_t     soft_env_mode;
        uint8_t     soft_env_sustain;
        uint16_t    soft_env_attack_tk;
        uint16_t    soft_env_hold_tk;
        uint16_t    soft_env_decay_tk;
        uint16_t    soft_env_fade_tk;
        uint16_t    soft_env_release_tk;
        bool        legato_effect;
    } channel[NUM_CHANNEL];
} TONE_PARAMS_t;

typedef struct
{
    uint16_t                tick_hz;
    uint16_t                decode_flags;
    MML_TEXT_LOAD_STATE_t   load_mml_text_state;
    MML_TEXT_INFO_t         mml_text_info;
    TONE_PARAMS_t           tone_params;
} PSG_MML_DECODER_t;

typedef struct
{
    uint8_t msg_cnt;
    PSG_MML_CMD_t msg[MAX_PSG_MML_MSG_NUM];
} PSG_MML_MSG_t;


void              psg_mml_decode_init(PSG_MML_DECODER_t *p_decoder, uint16_t tick_hz);
void              psg_mml_decode_load_mml_text(PSG_MML_DECODER_t *p_decoder, const char *p_mml_text, uint16_t flags);
void              psg_mml_decode_reload_mml_text(PSG_MML_DECODER_t *p_decoder);
bool              psg_mml_decode_is_EOT(PSG_MML_DECODER_t *p_decoder);
uint8_t           psg_mml_decode_get_use_channel_num(PSG_MML_DECODER_t *p_decoder);
psg_mml_decode_t  psg_mml_decode_get_message(PSG_MML_DECODER_t *p_decoder, uint8_t ch,/*@out@*/ PSG_MML_MSG_t *p_out);

#ifdef __cplusplus
}
#endif/*__cplusplus*/

#endif/*PSG_MML_DECODE_H*/
