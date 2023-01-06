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
#include "../../inc/local/psg_mml_q.h"
#include "../../inc/local/psg_mml_utils.h"
#include "../../inc/local/psg_mml_fifo.h"
#include "../../inc/local/psg_mml_ctrl.h"

static inline void init_soft_env_state_at_note_on(PSG_MML_TIME_t *p_time, PSG_MML_SOFT_ENV_t *p_soft_env)
{
    p_time->soft_env_cnt = 0;
    p_soft_env->state = E_SOFT_ENV_STATE_INIT_NOTE_ON;
}

static inline void init_soft_env_state_at_note_off(PSG_MML_TIME_t *p_time, PSG_MML_SOFT_ENV_t *p_soft_env)
{
    p_time->soft_env_cnt = 0;
    p_soft_env->state = E_SOFT_ENV_STATE_INIT_NOTE_OFF;
}

static inline void init_pitchbend(PSG_MML_TIME_t *p_time, PSG_MML_PITCHBEND_t *p_pitchbend)
{
    p_time->pitchbend_cnt = p_time->note_on;
    if ( p_time->pitchbend_cnt == 0 )
    {
        p_pitchbend->state = E_PITCHBEND_STATE_STOP;
        return;
    }

    p_pitchbend->q12_tp = p_pitchbend->q12_tp_base;

    if ( p_pitchbend->q12_tp_base < p_pitchbend->q12_tp_end )
    {
        p_pitchbend->state = E_PITCHBEND_STATE_TP_UP;
        p_pitchbend->q12_tp_delta = (p_pitchbend->q12_tp_end - p_pitchbend->q12_tp_base)/p_time->pitchbend_cnt;
    }
    else if ( p_pitchbend->q12_tp_base > p_pitchbend->q12_tp_end )
    {
        p_pitchbend->state = E_PITCHBEND_STATE_TP_DOWN;
        p_pitchbend->q12_tp_delta = (p_pitchbend->q12_tp_base - p_pitchbend->q12_tp_end)/p_time->pitchbend_cnt;
    }
    else
    {
        p_pitchbend->state = E_PITCHBEND_STATE_STOP;
        p_pitchbend->q12_tp_delta = 0;
    }
}

static inline void trans_soft_env_state(PSG_MML_TIME_t *p_time, PSG_MML_SOFT_ENV_t *p_soft_env)
{
    if ( p_time->soft_env_cnt != 0 )
    {
        /* NO CHANGE STATE */
        return;
    }

    switch ( p_soft_env->state )
    {
    case E_SOFT_ENV_STATE_INIT_NOTE_ON:
        if ( p_soft_env->attack_tk != 0 )
        {
            p_soft_env->q12_volume = 0;
            p_soft_env->q12_rate = p_soft_env->q12_top_volume/p_soft_env->attack_tk;
            p_time->soft_env_cnt = p_soft_env->attack_tk;
            p_soft_env->state = E_SOFT_ENV_STATE_ATTACK;
            break;
        }
        /*@fallthrough@*/

    case E_SOFT_ENV_STATE_ATTACK:
        if ( p_soft_env->hold_tk != 0 )
        {
            p_soft_env->q12_volume = p_soft_env->q12_top_volume;
            p_soft_env->q12_rate = 0;
            p_time->soft_env_cnt = p_soft_env->hold_tk;
            p_soft_env->state = E_SOFT_ENV_STATE_HOLD;
            break;
        }
        /*@fallthrough@*/

    case E_SOFT_ENV_STATE_HOLD:
        if ( p_soft_env->decay_tk != 0 )
        {
            p_soft_env->q12_volume = p_soft_env->q12_top_volume;
            p_soft_env->q12_rate = (p_soft_env->q12_top_volume-p_soft_env->q12_sus_volume)/p_soft_env->decay_tk;
            p_time->soft_env_cnt = p_soft_env->decay_tk;
            p_soft_env->state = E_SOFT_ENV_STATE_DECAY;
            break;
        }
        /*@fallthrough@*/

    case E_SOFT_ENV_STATE_DECAY:
        p_soft_env->q12_volume = p_soft_env->q12_sus_volume;
        p_soft_env->q12_rate = ( p_soft_env->fade_tk != 0 ) ? p_soft_env->q12_sus_volume/p_soft_env->fade_tk : 0;
        p_time->soft_env_cnt = p_soft_env->fade_tk;
        p_soft_env->state = E_SOFT_ENV_STATE_FADE;
        break;

    case E_SOFT_ENV_STATE_FADE:
        p_soft_env->state = E_SOFT_ENV_STATE_FADE;
        break;

    case E_SOFT_ENV_STATE_INIT_NOTE_OFF:
        p_soft_env->q12_rate = ( p_soft_env->release_tk != 0 ) ? p_soft_env->q12_volume/p_soft_env->release_tk : 0;
        p_time->soft_env_cnt = p_soft_env->release_tk;
        p_soft_env->state = E_SOFT_ENV_STATE_RELEASE;
        break;

    case E_SOFT_ENV_STATE_RELEASE:
        p_soft_env->state = E_SOFT_ENV_STATE_RELEASE;
        break;

    default:
        /* NO CHANGE */
        break;
    }
}

static inline void update_soft_env_q12_volume(PSG_MML_SOFT_ENV_t *p_soft_env)
{
    
    switch ( p_soft_env->state )
    {
    case E_SOFT_ENV_STATE_ATTACK:
        if ( p_soft_env->q12_rate != 0 )
        {
            p_soft_env->q12_volume += p_soft_env->q12_rate;
            if ( p_soft_env->q12_volume > p_soft_env->q12_top_volume )
            {
                p_soft_env->q12_volume = p_soft_env->q12_top_volume;
            }
        }
        else
        {
            p_soft_env->q12_volume = p_soft_env->q12_top_volume;
        }
        break;

    case E_SOFT_ENV_STATE_HOLD:
        p_soft_env->q12_volume = p_soft_env->q12_top_volume;
        break;

    case E_SOFT_ENV_STATE_DECAY:/*@fallthrough@*/
    case E_SOFT_ENV_STATE_FADE:
        if ( p_soft_env->q12_rate != 0 )
        {
            p_soft_env->q12_volume -= p_soft_env->q12_rate;
            if ( p_soft_env->q12_volume < 0 )
            {
                p_soft_env->q12_volume = 0;
            }
        }
        else
        {
            p_soft_env->q12_volume = p_soft_env->q12_sus_volume;
        }
        break;

    case E_SOFT_ENV_STATE_RELEASE:
        if ( p_soft_env->q12_rate != 0 )
        {
            p_soft_env->q12_volume -= p_soft_env->q12_rate;
            if ( p_soft_env->q12_volume < 0 )
            {
                p_soft_env->q12_volume = 0;
            }
        }
        else
        {
            p_soft_env->q12_volume = 0;
        }
        break;

    default:
        break;
    }
}

static inline void proc_command(PSG_MML_CTRL_t *p_ctrl, uint8_t ch, const PSG_MML_CMD_t *p_psg_cmd)
{
    PSG_MML_LFO_t  *p_lfo;
    PSG_MML_TIME_t *p_time;
    PSG_MML_SOFT_ENV_t  *p_soft_env;
    PSG_MML_PITCHBEND_t  *p_pitchbend;

    p_lfo  = &p_ctrl->channel[ch].lfo;
    p_time = &p_ctrl->channel[ch].time;
    p_soft_env = &p_ctrl->channel[ch].soft_env;
    p_pitchbend = &p_ctrl->channel[ch].pitchbend;

    switch ( p_psg_cmd->type )
    {
    case MSG_TYPE_DECODE_STATUS:
        if ( p_psg_cmd->data.decode_status.status == DECODE_STATUS_START )
        {
            /*DO NOTHING*/
        }
        else if ( p_psg_cmd->data.decode_status.status == DECODE_STATUS_END )
        {
            p_ctrl->channel_end_counter++;
            if ( p_ctrl->channel_end_counter >= p_ctrl->use_channel_num )
            {
                p_ctrl->channel_end_counter = p_ctrl->use_channel_num;
                if ( p_ctrl->state != E_PSG_MML_CTRL_STATE_END )
                {
                    psg_mml_ctrl_set_state(p_ctrl, E_PSG_MML_CTRL_STATE_END);
                }
            }
        }
        else
        {
        }
        break;

    case MSG_TYPE_SETTINGS:

        p_ctrl->reg[p_psg_cmd->data.settings.addr].data = p_psg_cmd->data.settings.data;
        p_ctrl->reg[p_psg_cmd->data.settings.addr].flag = 1;
        break;

    case MSG_TYPE_NOTE_ON:

        p_time->note_on = U16(p_psg_cmd->data.note_on.note_on_time_tk_hi, p_psg_cmd->data.note_on.note_on_time_tk_lo);
        p_time->gate = U16(p_psg_cmd->data.note_on.gate_time_tk_hi, p_psg_cmd->data.note_on.gate_time_tk_lo);
        if ( p_soft_env->mode != SOFT_ENVELOPE_MODE_OFF )
        {
            if ( !p_soft_env->legato_effect )
            {
                init_soft_env_state_at_note_on(p_time, p_soft_env);
                trans_soft_env_state(p_time, p_soft_env);
            }
        }

        if ( p_lfo->mode != LFO_MODE_OFF )
        {
            p_lfo->active = true;
        }

        p_pitchbend->q12_tp_base= I2Q12( U16(p_ctrl->reg[2*ch+1].data, p_ctrl->reg[2*ch+0].data) );
        p_pitchbend->q12_tp_end = I2Q12( U16(p_psg_cmd->data.note_on.tp_end_hi, p_psg_cmd->data.note_on.tp_end_lo) );
        init_pitchbend(p_time, p_pitchbend);

        p_ctrl->reg[PSG_REG_ADDR_MIXER].data &= ~(uint8_t)(0x9uL<<ch); 
        p_ctrl->reg[PSG_REG_ADDR_MIXER].data |= p_psg_cmd->data.note_on.data;
        p_ctrl->reg[PSG_REG_ADDR_MIXER].flag |= (1<<ch);


        break;

    case MSG_TYPE_NOTE_ON_REST:

        p_time->note_on = U16(p_psg_cmd->data.note_on.note_on_time_tk_hi, p_psg_cmd->data.note_on.note_on_time_tk_lo);
        p_ctrl->reg[PSG_REG_ADDR_MIXER].data &= ~(uint8_t)(0x9uL<<ch); 
        p_ctrl->reg[PSG_REG_ADDR_MIXER].data |= p_psg_cmd->data.note_on.data;
        p_ctrl->reg[PSG_REG_ADDR_MIXER].flag |= (1<<ch);

        break;

    case MSG_TYPE_NOTE_OFF:

        p_lfo->active = false;

        if ( p_soft_env->mode != SOFT_ENVELOPE_MODE_OFF )
        {
            init_soft_env_state_at_note_off(p_time, p_soft_env);
            trans_soft_env_state(p_time, p_soft_env);
        }
        else
        {
            p_ctrl->reg[PSG_REG_ADDR_MIXER].data &= (uint8_t)(~(0x9uL<<ch)); 
            p_ctrl->reg[PSG_REG_ADDR_MIXER].data |= p_psg_cmd->data.note_off.data;
            p_ctrl->reg[PSG_REG_ADDR_MIXER].flag |= (1<<ch);
        }
        break;

    case MSG_TYPE_LFO:

        p_lfo->mode = p_psg_cmd->data.lfo.mode;
        p_lfo->depth = p_psg_cmd->data.lfo.depth;
        p_lfo->delay_tk = U16(p_psg_cmd->data.lfo.delay_tk_hi, p_psg_cmd->data.lfo.delay_tk_lo);
        p_lfo->q12_omega = U32(p_psg_cmd->data.lfo.q12_omega_hh
                             , p_psg_cmd->data.lfo.q12_omega_hl
                             , p_psg_cmd->data.lfo.q12_omega_lh
                             , p_psg_cmd->data.lfo.q12_omega_ll);

        p_lfo->q12_delta = 0;
        p_lfo->theta = 0;

        p_time->lfo_delay_cnt = p_lfo->delay_tk;
        break;

    case MSG_TYPE_SOFT_ENV_1:

        p_soft_env->mode            = p_psg_cmd->data.soft_env_1.mode;
        p_soft_env->q12_top_volume  = I2Q12(p_psg_cmd->data.soft_env_1.top_volume);
        p_soft_env->q12_sus_volume  = I2Q12(p_psg_cmd->data.soft_env_1.sus_volume);
        p_soft_env->legato_effect  = (p_psg_cmd->data.soft_env_1.legato_effect != 0);
        break;

    case MSG_TYPE_SOFT_ENV_2:

        p_soft_env->attack_tk  = U16(p_psg_cmd->data.soft_env_2.attack_tk_hi , p_psg_cmd->data.soft_env_2.attack_tk_lo);
        p_soft_env->hold_tk    = U16(p_psg_cmd->data.soft_env_2.hold_tk_hi   , p_psg_cmd->data.soft_env_2.hold_tk_lo);
        p_soft_env->decay_tk   = U16(p_psg_cmd->data.soft_env_2.decay_tk_hi  , p_psg_cmd->data.soft_env_2.decay_tk_lo);
        break;

    case MSG_TYPE_SOFT_ENV_3:

        p_soft_env->fade_tk    = U16(p_psg_cmd->data.soft_env_3.fade_tk_hi    , p_psg_cmd->data.soft_env_3.fade_tk_lo);
        p_soft_env->release_tk = U16(p_psg_cmd->data.soft_env_3.release_tk_hi , p_psg_cmd->data.soft_env_3.release_tk_lo);
        break;

    default:
        break;
    }
}

static inline void proc_lfo(PSG_MML_CTRL_t *p_ctrl, uint8_t ch, PSG_MML_LFO_t *p_lfo, PSG_MML_TIME_t *p_time)
{
    uint8_t tp_hi = 0;
    uint8_t tp_lo = 0;
    uint16_t tp_next = 0;
    uint32_t lfo_delta_int = 0;

    if ( !p_lfo->active )
    {
        return;
    }

    if ( p_time->lfo_delay_cnt > 0 )
    {
        p_time->lfo_delay_cnt--;
        return;
    }

    tp_hi = p_ctrl->reg[2*ch+1].data;
    tp_lo = p_ctrl->reg[2*ch+0].data;
    tp_next = U16(tp_hi, tp_lo);

    p_lfo->q12_delta += p_lfo->q12_omega;
    lfo_delta_int = Q_INT(p_lfo->q12_delta, 12);

    if ( lfo_delta_int > 0 )
    {
        uint16_t lfo_theta = p_lfo->theta;
        uint16_t lfo_depth = p_lfo->depth;
        uint32_t i;
        lq24_t lq24_tp = 0;

        for ( i = 0; i < lfo_delta_int; i++ )
        {
            lq24_tp = ((lq24_t)tp_next)<<24;
            lq24_tp += p_lfo->q24_tp_frac;
            if ( (lfo_depth <= lfo_theta) && ( lfo_theta < (lfo_depth*3)))
            {
                lq24_tp = (lq24_tp*Q_PITCHBEND_FACTOR_N)>>24;
            }
            else
            {
                lq24_tp = (lq24_tp*Q_PITCHBEND_FACTOR)>>24;
            }
            p_lfo->q24_tp_frac = Q_FRAC(lq24_tp, 24);

            tp_next = Q_INT(lq24_tp, 24);

            lfo_theta++;
            if ( lfo_theta >= lfo_depth*4 )
            {
                lfo_theta = 0;
            }
        }
        p_lfo->theta = lfo_theta;
        p_lfo->q12_delta = Q_FRAC(p_lfo->q12_delta, 12); 

        p_ctrl->reg[2*ch+0].data = U16_LO(tp_next);
        p_ctrl->reg[2*ch+0].flag = 1;

        p_ctrl->reg[2*ch+1].data = U16_HI(tp_next);
        p_ctrl->reg[2*ch+1].flag = 1;

    }
}

static inline void proc_soft_env_gen(PSG_MML_CTRL_t *p_ctrl, uint8_t ch, PSG_MML_SOFT_ENV_t *p_soft_env, PSG_MML_TIME_t *p_time)
{
    uint8_t vol_ctrl;
    uint8_t vol_ctrl_next;
    if ( p_time->soft_env_cnt > 0 )
    {
        p_time->soft_env_cnt--;
    }

    vol_ctrl = p_ctrl->reg[0x08+ch].data;
    vol_ctrl_next = Q_INT(p_soft_env->q12_volume, 12);
    if ( vol_ctrl_next != vol_ctrl )
    {
        p_ctrl->reg[0x8+ch].data = vol_ctrl_next;
        p_ctrl->reg[0x8+ch].flag = 1;
    }

    trans_soft_env_state(p_time, p_soft_env);

    update_soft_env_q12_volume(p_soft_env);
}

static inline void proc_pitchbend(PSG_MML_CTRL_t *p_ctrl, uint8_t ch, PSG_MML_PITCHBEND_t *p_pitchbend, PSG_MML_TIME_t *p_time)
{
    uint16_t tp;
    if ( p_time->pitchbend_cnt > 0 )
    {
        p_time->pitchbend_cnt--;
    }
    else
    {
        p_pitchbend->state = E_PITCHBEND_STATE_STOP;
    }

    switch ( p_pitchbend->state )
    {
    case E_PITCHBEND_STATE_STOP:
        break;

    case E_PITCHBEND_STATE_TP_UP:

        p_pitchbend->q12_tp += p_pitchbend->q12_tp_delta;
        if ( p_pitchbend->q12_tp > I2Q12(MAX_TP) )
        {
            p_pitchbend->state = E_PITCHBEND_STATE_STOP;
        }
        tp = Q_INT(p_pitchbend->q12_tp, 12);

        p_ctrl->reg[0x0+2*ch].data = U16_LO(tp);
        p_ctrl->reg[0x0+2*ch].flag = 1;
        p_ctrl->reg[0x1+2*ch].data = U16_HI(tp);
        p_ctrl->reg[0x1+2*ch].flag = 1;
        break;

    case E_PITCHBEND_STATE_TP_DOWN:

        p_pitchbend->q12_tp -= p_pitchbend->q12_tp_delta;
        if ( p_pitchbend->q12_tp < I2Q12(0) )
        {
            p_pitchbend->state = E_PITCHBEND_STATE_STOP;
        }
        tp = Q_INT(p_pitchbend->q12_tp, 12);

        p_ctrl->reg[0x0+2*ch].data = U16_LO(tp);
        p_ctrl->reg[0x0+2*ch].flag = 1;
        p_ctrl->reg[0x1+2*ch].data = U16_HI(tp);
        p_ctrl->reg[0x1+2*ch].flag = 1;
        break;

    default:
        p_pitchbend->state = E_PITCHBEND_STATE_STOP;
        break;
    }
}

void psg_mml_ctrl_init(PSG_MML_CTRL_t *p_ctrl, uint8_t use_channel_num)
{
    uint32_t i;

    psg_mml_ctrl_set_state(p_ctrl, E_PSG_MML_CTRL_STATE_UNINITIALIZED);

    for ( i = 0; i < NUM_CHANNEL; i++ )
    {
        /* Initialize lfo */
        p_ctrl->channel[i].lfo.mode = LFO_MODE_OFF;
        p_ctrl->channel[i].lfo.depth = 0;
        p_ctrl->channel[i].lfo.speed = 0;
        p_ctrl->channel[i].lfo.active = false;
        p_ctrl->channel[i].lfo.q12_omega = I2Q12(0);
        p_ctrl->channel[i].lfo.q12_delta = I2Q12(0);
        p_ctrl->channel[i].lfo.theta = 0;
        p_ctrl->channel[i].lfo.delay_tk = 0;
        p_ctrl->channel[i].lfo.q24_tp_frac;

        /* Initialize time */
        p_ctrl->channel[i].time.note_on = 0;
        p_ctrl->channel[i].time.gate = 0;
        p_ctrl->channel[i].time.soft_env_cnt = 0;
        p_ctrl->channel[i].time.lfo_delay_cnt = 0;
        p_ctrl->channel[i].time.pitchbend_cnt = 0;

        /* Initialize software envelope generator */
        p_ctrl->channel[i].soft_env.mode = SOFT_ENVELOPE_MODE_OFF;
        p_ctrl->channel[i].soft_env.attack_tk = 0;
        p_ctrl->channel[i].soft_env.hold_tk = 0;
        p_ctrl->channel[i].soft_env.decay_tk = 0;
        p_ctrl->channel[i].soft_env.fade_tk = 0;
        p_ctrl->channel[i].soft_env.release_tk = 0;
        p_ctrl->channel[i].soft_env.q12_rate = 0;
        p_ctrl->channel[i].soft_env.q12_volume = 0;
        p_ctrl->channel[i].soft_env.q12_top_volume = 0;
        p_ctrl->channel[i].soft_env.q12_sus_volume = 0;
        p_ctrl->channel[i].soft_env.state = E_SOFT_ENV_STATE_INIT_NOTE_ON;
        p_ctrl->channel[i].soft_env.legato_effect = false;

        /* Initialize pitchbend */
        p_ctrl->channel[i].pitchbend.q12_tp = 0;
        p_ctrl->channel[i].pitchbend.q12_tp_base = 0;
        p_ctrl->channel[i].pitchbend.q12_tp_end = 0;
        p_ctrl->channel[i].pitchbend.q12_tp_delta = 0;
        p_ctrl->channel[i].pitchbend.state = E_PITCHBEND_STATE_STOP;
    }

    for ( i = MIN_PSG_REG_ADDR; i <= MAX_PSG_REG_ADDR; i++ )
    {
        p_ctrl->reg[i].data = 0;
        p_ctrl->reg[i].flag = 0;

    }

    p_ctrl->reg[PSG_REG_ADDR_MIXER].data = PSG_REG_ALL_MUTE;
    p_ctrl->reg[PSG_REG_ADDR_MIXER].flag = (1<<use_channel_num)-1;

    p_ctrl->use_channel_num = use_channel_num;
    p_ctrl->channel_end_counter = 0;

    psg_mml_ctrl_set_state(p_ctrl, E_PSG_MML_CTRL_STATE_STOP);
}

void psg_mml_ctrl_set_state(PSG_MML_CTRL_t *p_ctrl, PSG_MML_CTRL_STATE_t state)
{
    p_ctrl->state = state;
}

PSG_MML_CTRL_STATE_t psg_mml_ctrl_get_state(const PSG_MML_CTRL_t *p_ctrl )
{
    return p_ctrl->state;
}

void psg_mml_ctrl_proc(PSG_MML_CTRL_t *p_ctrl, PSG_MML_FIFO_t *p_fifo)
{
    PSG_MML_LFO_t  *p_lfo;
    PSG_MML_TIME_t *p_time;
    PSG_MML_SOFT_ENV_t  *p_soft_env;
    PSG_MML_PITCHBEND_t  *p_pitchbend;
    PSG_MML_CMD_t psg_cmd;
    uint8_t ch;

    if ( p_ctrl->state != E_PSG_MML_CTRL_STATE_PLAY )
    {
        return;
    }

    for ( ch = 0; ch < NUM_CHANNEL; ch++ )
    {
        p_lfo  = &p_ctrl->channel[ch].lfo;
        p_time = &p_ctrl->channel[ch].time;
        p_soft_env = &p_ctrl->channel[ch].soft_env;
        p_pitchbend = &p_ctrl->channel[ch].pitchbend;

        if ( p_time->note_on > 0 )
        {
            p_time->note_on--;
        }
        if ( p_time->gate > 0 )
        {
            p_time->gate--;
        }
        if ( p_time->note_on == 0 )
        {
            while ( psg_mml_fifo_dequeue(&p_fifo[ch], &psg_cmd) )
            {
                proc_command(p_ctrl, ch, &psg_cmd);
                if ( p_time->note_on > 0 )
                {
                    break;
                }
            }
        }
        if ( p_time->gate == 0 )
        {
            /* Mute tone and noise */
            if ( ((p_ctrl->reg[PSG_REG_ADDR_MIXER].data>>ch)&0x9) != 0x9 )
            {
                p_ctrl->reg[PSG_REG_ADDR_MIXER].data |= (0x9<<ch);    
                p_ctrl->reg[PSG_REG_ADDR_MIXER].flag |= (1<<ch);
            }
        }

        /* PITCHBEND BLOCK */
        proc_pitchbend(p_ctrl, ch, p_pitchbend, p_time);

        /* SOFTWARE ENVELOPE GENERATOR BLOCK */
        if ( p_soft_env->mode != SOFT_ENVELOPE_MODE_OFF )
        {
            proc_soft_env_gen(p_ctrl, ch, p_soft_env, p_time);
        }
        /* LFO BLOCK */
        if ( p_lfo->mode != LFO_MODE_OFF )
        {
            proc_lfo(p_ctrl, ch, p_lfo, p_time);
        }
    }
}
