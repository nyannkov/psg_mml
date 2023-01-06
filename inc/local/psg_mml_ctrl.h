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
#ifndef PSG_MML_CTRL
#define PSG_MML_CTRL

#ifdef __cplusplus
extern "C" {
#endif/*__cplusplus*/

#define PSG_MML_CTRL_GET_REG_DATA(p_ctrl, addr)     ((p_ctrl)->reg[(addr)].data)
#define PSG_MML_CTRL_GET_REG_FLAG(p_ctrl, addr)     ((p_ctrl)->reg[(addr)].flag)
#define PSG_MML_CTRL_CLEAR_REG_FLAG(p_ctrl, addr)   ((p_ctrl)->reg[(addr)].flag = 0)

typedef enum
{
    E_PSG_MML_CTRL_STATE_UNINITIALIZED = 0,
    E_PSG_MML_CTRL_STATE_STOP,
    E_PSG_MML_CTRL_STATE_PLAY,
    E_PSG_MML_CTRL_STATE_END
} PSG_MML_CTRL_STATE_t;

#pragma pack(1)
typedef struct
{
    bool        active;
    uint8_t     mode;
    uint8_t     depth;
    uint8_t     speed;
    q12_t       q12_omega;
    q12_t       q12_delta;
    uint16_t    theta;
    uint16_t    delay_tk;
    q24_t       q24_tp_frac;
} PSG_MML_LFO_t;

typedef enum
{
    E_SOFT_ENV_STATE_INIT_NOTE_ON = 0,
    E_SOFT_ENV_STATE_ATTACK,
    E_SOFT_ENV_STATE_HOLD,
    E_SOFT_ENV_STATE_DECAY,
    E_SOFT_ENV_STATE_FADE,
    E_SOFT_ENV_STATE_INIT_NOTE_OFF,
    E_SOFT_ENV_STATE_RELEASE,
} SOFT_ENV_STATE_t;

typedef struct
{
    uint8_t             mode;
    uint16_t            attack_tk;
    uint16_t            hold_tk;
    uint16_t            decay_tk;
    uint16_t            fade_tk;
    uint16_t            release_tk;
    q12_t               q12_rate;
    q12_t               q12_volume;
    q12_t               q12_top_volume;
    q12_t               q12_sus_volume;
    SOFT_ENV_STATE_t    state;
    bool                legato_effect;
} PSG_MML_SOFT_ENV_t;

typedef struct
{
    uint16_t note_on;
    uint16_t gate; 
    uint16_t soft_env_cnt;
    uint16_t lfo_delay_cnt;
    uint16_t pitchbend_cnt;
} PSG_MML_TIME_t;

typedef enum
{
    E_PITCHBEND_STATE_STOP = 0,
    E_PITCHBEND_STATE_TP_UP,
    E_PITCHBEND_STATE_TP_DOWN,
} PITCHBEND_STATE_t;

typedef struct
{
    q12_t               q12_tp;
    q12_t               q12_tp_base;
    q12_t               q12_tp_end;
    q12_t               q12_tp_delta;
    PITCHBEND_STATE_t   state;
} PSG_MML_PITCHBEND_t;

typedef struct
{
    uint8_t flag;
    uint8_t data;
} PSG_MML_REG_t;

typedef struct
{
    PSG_MML_CTRL_STATE_t    state;
    PSG_MML_REG_t           reg[NUM_OF_PSG_REG];
    uint8_t                 use_channel_num;
    uint8_t                 channel_end_counter;
    struct
    {
        PSG_MML_TIME_t      time;
        PSG_MML_LFO_t       lfo;
        PSG_MML_SOFT_ENV_t  soft_env;
        PSG_MML_PITCHBEND_t pitchbend;
    } channel[NUM_CHANNEL];
} PSG_MML_CTRL_t;

#pragma pack()

void psg_mml_ctrl_init(PSG_MML_CTRL_t *p_ctrl, uint8_t use_channel_num);
void psg_mml_ctrl_proc(PSG_MML_CTRL_t *p_ctrl, PSG_MML_FIFO_t *p_fifo);

void psg_mml_ctrl_set_state(PSG_MML_CTRL_t *p_ctrl, PSG_MML_CTRL_STATE_t state);
PSG_MML_CTRL_STATE_t psg_mml_ctrl_get_state(const PSG_MML_CTRL_t *p_ctrl);


#ifdef __cplusplus
}
#endif/*__cplusplus*/

#endif/*PSG_MML_CTRL*/
