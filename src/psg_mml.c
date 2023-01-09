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
#include "../inc/local/psg_mml_local.h"
#include "../inc/local/psg_mml_q.h"
#include "../inc/local/psg_mml_fifo.h"
#include "../inc/local/psg_mml_decode.h"
#include "../inc/local/psg_mml_ctrl.h"
#include "../inc/psg_mml.h"

#ifndef PSG_MML_SHARE_SLOT0_DRIVER
#define PSG_MML_SHARE_SLOT0_DRIVER     (true) 
#endif

#define PSG_MML_SHARE_PSG_MODE         ( (PSG_MML_SHARE_SLOT0_DRIVER==true) && (PSG_MML_SLOT_TOTAL_NUM==2) )

#ifndef PSG_MML_HOOK_START_PERIODIC_CONTROL
#define PSG_MML_HOOK_START_PERIODIC_CONTROL()
#endif

#ifndef PSG_MML_HOOK_END_PERIODIC_CONTROL
#define PSG_MML_HOOK_END_PERIODIC_CONTROL()
#endif

#ifndef PSG_MML_ENABLE_PERIODIC_CONTROL
#define PSG_MML_ENABLE_PERIODIC_CONTROL()
#endif

#ifndef PSG_MML_DISABLE_PERIODIC_CONTROL
#define PSG_MML_DISABLE_PERIODIC_CONTROL()
#endif

typedef struct
{
    void (*p_write)(uint8_t addr, uint8_t data);
    uint8_t                 psg_reg[NUM_OF_PSG_REG];
} PSG_MML_DRIVER_t;

typedef struct
{
    bool                    initialized;
    PSG_MML_CTRL_t          ctrl;
    PSG_MML_DECODER_t       decoder;
    PSG_MML_FIFO_t          fifo[NUM_CHANNEL];
    PSG_MML_DRIVER_t        driver;
} PSG_MML_t;

static void obj_init(PSG_MML_t *p_obj, void (*p_write)(uint8_t addr, uint8_t data), uint16_t tick_hz);
static void psg_write(PSG_MML_t *p_obj, uint8_t addr, uint8_t data);
static psg_mml_t predecode(uint8_t slot, uint16_t num_of_predecode_times);
#if PSG_MML_SHARE_PSG_MODE
static uint8_t psg_read(PSG_MML_t *p_obj, uint8_t addr);
#endif
static size_t psg_mml_strnlen(const char *s, size_t max_len);

static PSG_MML_t psg_mml_obj[PSG_MML_SLOT_TOTAL_NUM];

psg_mml_t psg_mml_init(uint8_t slot, uint16_t tick_hz, void (*p_write)(uint8_t addr, uint8_t data))
{
    PSG_MML_t *p_obj;
    uint8_t addr;
    uint8_t data;
    if ( slot >= PSG_MML_SLOT_TOTAL_NUM )
    {
        return PSG_MML_OUT_OF_SLOT;
    }
    p_obj = &psg_mml_obj[slot];

    obj_init(p_obj, p_write, tick_hz);
    for ( addr = 0; addr < NUM_OF_PSG_REG; addr++ )
    {
        data = PSG_MML_CTRL_GET_REG_DATA(&p_obj->ctrl, addr);
        psg_write(p_obj, addr, data);
    }

    return PSG_MML_SUCCESS;
}

psg_mml_t psg_mml_deinit(uint8_t slot)
{
    PSG_MML_t *p_obj;
    if ( slot >= PSG_MML_SLOT_TOTAL_NUM )
    {
        return PSG_MML_OUT_OF_SLOT;
    }
    p_obj = &psg_mml_obj[slot];

    p_obj->initialized = false;

    return PSG_MML_SUCCESS;
}

psg_mml_t psg_mml_decode(uint8_t slot)
{
    PSG_MML_t *p_obj;
    PSG_MML_MSG_t ch_msg[NUM_CHANNEL];
    psg_mml_decode_t mml_ch_ret[NUM_CHANNEL] = {0, 0, 0};
    bool is_all_end = false;
    bool is_success = false;
    uint8_t use_channel_num;
    uint8_t ch;
    uint8_t no_free_space_cnt;
    uint16_t free_space_size;

    if ( slot >= PSG_MML_SLOT_TOTAL_NUM )
    {
        return PSG_MML_OUT_OF_SLOT;
    }

    p_obj = &psg_mml_obj[slot];
    if ( !p_obj->initialized )
    {
        return PSG_MML_NOT_INITIALIZED;
    }

    no_free_space_cnt = 0;
    use_channel_num = psg_mml_decode_get_use_channel_num(&p_obj->decoder);
    for ( ch = 0; ch < use_channel_num; ch++ )
    {
        PSG_MML_DISABLE_PERIODIC_CONTROL();
        free_space_size = psg_mml_fifo_get_free_space(&p_obj->fifo[ch]);
        PSG_MML_ENABLE_PERIODIC_CONTROL();

        if ( free_space_size >= MAX_PSG_MML_MSG_NUM )
        {
            ch_msg[ch].msg_cnt = 0;
            mml_ch_ret[ch] = psg_mml_decode_get_message(&p_obj->decoder, ch, &ch_msg[ch]);
            if ( ch_msg[ch].msg_cnt > 0 )
            {
                PSG_MML_DISABLE_PERIODIC_CONTROL();
                is_success = psg_mml_fifo_enqueue(&p_obj->fifo[ch], ch_msg[ch].msg, ch_msg[ch].msg_cnt);
                PSG_MML_ENABLE_PERIODIC_CONTROL();

                PSG_MML_ASSERT( is_success );
                if ( !is_success )
                {
                    return PSG_MML_INTERNAL_ERROR;
                }
            }
        }
        else
        {
            no_free_space_cnt++;
            if ( no_free_space_cnt >= use_channel_num )
            {
                return PSG_MML_DECODE_QUEUE_IS_FULL;
            }
        }
    }

    is_all_end = true;
    for ( ch = 0; ch < use_channel_num; ch++ )
    {
        if ( mml_ch_ret[ch] != PSG_MML_DECODE_CHANNEL_END )
        {
            is_all_end = false;
            break;
        }
    }

    return is_all_end ? PSG_MML_DECODE_END : PSG_MML_SUCCESS;
}

static psg_mml_t predecode(uint8_t slot, uint16_t num_predecode)
{
    psg_mml_t r;
    uint16_t i;
    for ( i = 0; i < num_predecode; i++ )
    {
        r = psg_mml_decode(slot);
        if ( r != PSG_MML_SUCCESS )
        {
            break;
        }
    }
    return r;
}

psg_mml_t psg_mml_load_text(uint8_t slot, const char *p_mml_text, uint16_t flags)
{
    PSG_MML_t *p_obj;
    uint8_t use_channel_num;
    if ( slot >= PSG_MML_SLOT_TOTAL_NUM )
    {
        return PSG_MML_OUT_OF_SLOT;
    }

    p_obj = &psg_mml_obj[slot];
    if ( !p_obj->initialized )
    {
        return PSG_MML_NOT_INITIALIZED;
    }
    if ( p_mml_text == NULL )
    {
        return PSG_MML_TEXT_NULL;
    }
    if ( p_mml_text[0] == '\0' )
    {
        return PSG_MML_TEXT_EMPTY;
    }
    if ( psg_mml_strnlen(p_mml_text, MAX_MML_TEXT_LEN) >= MAX_MML_TEXT_LEN )
    {
        return PSG_MML_TEXT_OVER_LENGTH;
    }

    psg_mml_decode_load_mml_text(&psg_mml_obj[slot].decoder, p_mml_text, flags);

    use_channel_num = psg_mml_decode_get_use_channel_num(&p_obj->decoder);
    psg_mml_ctrl_init(&p_obj->ctrl, use_channel_num);

    return PSG_MML_SUCCESS;
}

psg_mml_t psg_mml_play_start(uint8_t slot, uint16_t num_predecode)
{
    PSG_MML_t *p_obj;
    PSG_MML_CTRL_STATE_t state;
    psg_mml_t r;

    if ( slot >= PSG_MML_SLOT_TOTAL_NUM )
    {
        return PSG_MML_OUT_OF_SLOT;
    }

    p_obj = &psg_mml_obj[slot];
    if ( !p_obj->initialized )
    {
        return PSG_MML_NOT_INITIALIZED;
    }

    PSG_MML_DISABLE_PERIODIC_CONTROL();

    state = psg_mml_ctrl_get_state(&p_obj->ctrl);

    if ( state == E_PSG_MML_CTRL_STATE_STOP )
    {
        r = predecode(slot, num_predecode);
        if ( r >= 0 )
        {
            psg_mml_ctrl_set_state(&p_obj->ctrl, E_PSG_MML_CTRL_STATE_PLAY);
            r = PSG_MML_SUCCESS;
        }
    }

    PSG_MML_ENABLE_PERIODIC_CONTROL();

    return r;
}

psg_mml_t psg_mml_play_stop(uint8_t slot)
{
    PSG_MML_t *p_obj;
    uint8_t ch;
    uint8_t use_channel_num;

    if ( slot >= PSG_MML_SLOT_TOTAL_NUM )
    {
        return PSG_MML_OUT_OF_SLOT;
    }

    p_obj = &psg_mml_obj[slot];
    if ( !p_obj->initialized )
    {
        return PSG_MML_NOT_INITIALIZED;
    }

    PSG_MML_DISABLE_PERIODIC_CONTROL();

    psg_mml_decode_reload_mml_text(&p_obj->decoder);
    for ( ch = 0; ch < NUM_CHANNEL; ch++ )
    {
        psg_mml_fifo_init(&p_obj->fifo[ch]);
    }
    
    use_channel_num = psg_mml_decode_get_use_channel_num(&p_obj->decoder);
    psg_mml_ctrl_init(&p_obj->ctrl, use_channel_num);

    psg_mml_ctrl_set_state(&p_obj->ctrl, E_PSG_MML_CTRL_STATE_STOP);

    PSG_MML_ENABLE_PERIODIC_CONTROL();

    return PSG_MML_SUCCESS;
}

psg_mml_t psg_mml_play_restart(uint8_t slot, uint16_t num_predecode)
{
    psg_mml_t r;

    r = psg_mml_play_stop(slot);
    if ( r == PSG_MML_SUCCESS )
    {
        r = psg_mml_play_start(slot, num_predecode);
    }

    return r;
}


psg_mml_t  psg_mml_get_play_state(uint8_t slot, /*@out@*/PSG_MML_PLAY_STATE_t *p_out)
{
    PSG_MML_t *p_obj;
    PSG_MML_CTRL_STATE_t ctrl_state;
    PSG_MML_PLAY_STATE_t play_state;

    if ( slot >= PSG_MML_SLOT_TOTAL_NUM )
    {
        return PSG_MML_OUT_OF_SLOT;
    }

    p_obj = &psg_mml_obj[slot];
    if ( !p_obj->initialized )
    {
        return PSG_MML_NOT_INITIALIZED;
    }

    PSG_MML_DISABLE_PERIODIC_CONTROL();
    ctrl_state = psg_mml_ctrl_get_state(&p_obj->ctrl);
    PSG_MML_ENABLE_PERIODIC_CONTROL();

    switch (ctrl_state)
    {
    case E_PSG_MML_CTRL_STATE_STOP:
        play_state = E_PSG_MML_PLAY_STATE_STOP;
        break;

    case E_PSG_MML_CTRL_STATE_PLAY:
        play_state = E_PSG_MML_PLAY_STATE_PLAY;
        break;

    case E_PSG_MML_CTRL_STATE_END:
        play_state = E_PSG_MML_PLAY_STATE_END;
        break;

    case E_PSG_MML_CTRL_STATE_UNINITIALIZED:/*@fallthrough@*/
    default:
        play_state = E_PSG_MML_PLAY_STATE_STOP;
        break;
    }

    if ( p_out != NULL )
    {
        *p_out = play_state;
    }

    return PSG_MML_SUCCESS;
}

static void obj_init(PSG_MML_t *p_obj, void (*p_write)(uint8_t addr, uint8_t data), uint16_t tick_hz)
{
    uint8_t ch;

    p_obj->initialized = false;
    p_obj->driver.p_write = p_write;

    psg_mml_ctrl_init(&p_obj->ctrl, 0);

    psg_mml_decode_init(&p_obj->decoder, tick_hz);
    for ( ch = 0; ch < NUM_CHANNEL; ch++ )
    {
        psg_mml_fifo_init(&p_obj->fifo[ch]);
    }

    p_obj->initialized = true;
}

static void psg_write(PSG_MML_t *p_obj, uint8_t addr, uint8_t data)
{
    if ( p_obj->driver.p_write != NULL )
    {
        p_obj->driver.psg_reg[addr] = data;
        p_obj->driver.p_write(addr, data);
    }
}


static size_t psg_mml_strnlen(const char *s, size_t max_len)
{
    size_t i = 0;
    size_t len = 0;
    if ( s == NULL )
    {
        return 0;
    }

    len = max_len;
    for ( i = 0; i < max_len; i++ )
    {
        if ( s[i] == '\0' )
        {
            len = i;
            break;
        }
    }

    return len;
}

#if PSG_MML_SHARE_PSG_MODE
static inline bool is_noise_in_use_by_slot1(void)
{
    PSG_MML_t *p_slot1_obj = &psg_mml_obj[1];
    uint8_t slot1_mixer_req = 0;

    if ( !p_slot1_obj->initialized )
    {
        return false;
    }

    slot1_mixer_req = PSG_MML_CTRL_GET_REG_DATA(&p_slot1_obj->ctrl, PSG_REG_ADDR_MIXER);
    if ( ((slot1_mixer_req >> 3) &0x7) != 0x7 )
    {
        return true;
    }
    else
    {
        return false;
    }
}

static inline bool is_locked_addr_by_slot1(uint8_t addr, uint8_t mixer_ch)
{
    PSG_MML_t *p_slot1_obj = &psg_mml_obj[1];
    uint8_t slot1_use_channel = 0;

    if ( !p_slot1_obj->initialized )
    {
        return false;
    }
    if ( psg_mml_ctrl_get_state(&p_slot1_obj->ctrl) != E_PSG_MML_CTRL_STATE_PLAY )
    {
        return false;
    }

    slot1_use_channel = psg_mml_decode_get_use_channel_num(&p_slot1_obj->decoder);
    if ( slot1_use_channel == 0 )
    {
        return false;
    }
    if ( slot1_use_channel == NUM_CHANNEL )
    {
        return true;
    }

    /* slot1_use_channel == 1 or 2 */
    PSG_MML_ASSERT( (slot1_use_channel == 1) || (slot1_use_channel == 2) );
    switch (addr)
    {
    case 0x4:/*@fallthrough@*/
    case 0x5:/*@fallthrough@*/
    case 0xA:
        return true;

    case 0x2:/*@fallthrough@*/
    case 0x3:/*@fallthrough@*/
    case 0x9:
        return ( slot1_use_channel == 2 ) ? true : false;

    case 0x6:
        return (is_noise_in_use_by_slot1()) ? true : false;

    case 0x7:
        if ( slot1_use_channel == 2 )
        {
            return (mixer_ch == 0) ? false : true;
        }
        else
        {
            return (mixer_ch == 2) ? true : false;
        }

    default:
        return false;
    }

}

static uint8_t psg_read(PSG_MML_t *p_obj, uint8_t addr)
{
    return p_obj->driver.psg_reg[addr];
}

#endif

void psg_mml_periodic_control(void)
{
    uint32_t slot;
    uint8_t addr, data;
#if PSG_MML_SHARE_PSG_MODE
    uint8_t ch;
    uint8_t addr_swp;
    uint8_t flag_swp;
    uint8_t mixer_req_swp;

    uint8_t slot1_use_channel;
    volatile uint8_t flag[PSG_MML_SLOT_TOTAL_NUM];
    volatile uint8_t mixer_req[PSG_MML_SLOT_TOTAL_NUM];
    uint8_t mixer_next;
    PSG_MML_CTRL_STATE_t slot1_state;
#endif
    
    PSG_MML_HOOK_START_PERIODIC_CONTROL();

#if PSG_MML_SHARE_PSG_MODE

    if ( !psg_mml_obj[0].initialized && !psg_mml_obj[1].initialized ) 
    {
        PSG_MML_HOOK_END_PERIODIC_CONTROL();
        return;
    }
    else if ( ( psg_mml_obj[0].initialized ) && (!psg_mml_obj[1].initialized ) 
         ||   (!psg_mml_obj[0].initialized ) && ( psg_mml_obj[1].initialized ) )
    {
        slot = psg_mml_obj[0].initialized ? 0 : 1;

        psg_mml_ctrl_proc(&psg_mml_obj[slot].ctrl, psg_mml_obj[slot].fifo);

        for ( addr = MIN_PSG_REG_ADDR; addr <= MAX_PSG_REG_ADDR; addr++ )
        {
            if ( PSG_MML_CTRL_GET_REG_FLAG(&psg_mml_obj[slot].ctrl, addr) != 0 )
            {
                PSG_MML_CTRL_CLEAR_REG_FLAG(&psg_mml_obj[slot].ctrl, addr);
                data = PSG_MML_CTRL_GET_REG_DATA(&psg_mml_obj[slot].ctrl, addr);

                psg_write(&psg_mml_obj[slot], addr, data);
            }
        }
    }
    else
    {

        psg_mml_ctrl_proc(&psg_mml_obj[0].ctrl, psg_mml_obj[0].fifo);
        psg_mml_ctrl_proc(&psg_mml_obj[1].ctrl, psg_mml_obj[1].fifo);

        slot1_state = psg_mml_ctrl_get_state(&psg_mml_obj[1].ctrl);
        slot1_use_channel = psg_mml_decode_get_use_channel_num(&psg_mml_obj[1].decoder);
        for ( addr = MIN_PSG_REG_ADDR; addr <= MAX_PSG_REG_ADDR; addr++ )
        {
            if ( addr == PSG_REG_ADDR_MIXER )
            {
                continue;
            }
            /* Swap 0ch and 2ch in slot1.  */
            addr_swp = (addr == 0x0) ? 0x4
                     : (addr == 0x1) ? 0x5
                     : (addr == 0x4) ? 0x0
                     : (addr == 0x5) ? 0x1
                     : (addr == 0x8) ? 0xA
                     : (addr == 0xA) ? 0x8
                     : addr;

            if ( PSG_MML_CTRL_GET_REG_FLAG(&psg_mml_obj[1].ctrl, addr_swp) != 0 )
            {
                PSG_MML_CTRL_CLEAR_REG_FLAG(&psg_mml_obj[1].ctrl, addr_swp);
                data = PSG_MML_CTRL_GET_REG_DATA(&psg_mml_obj[1].ctrl, addr_swp);

                /* Share the slot0 write driver. */
                psg_write(&psg_mml_obj[0], addr, data);
            }
            else if ( PSG_MML_CTRL_GET_REG_FLAG(&psg_mml_obj[0].ctrl, addr) != 0 )
            {
                if ( !is_locked_addr_by_slot1(addr, 0xFF) )
                {
                    PSG_MML_CTRL_CLEAR_REG_FLAG(&psg_mml_obj[0].ctrl, addr);
                    data = PSG_MML_CTRL_GET_REG_DATA(&psg_mml_obj[0].ctrl, addr);

                    psg_write(&psg_mml_obj[0], addr, data);
                }
            }
            else
            {
                /*DO NOTHING*/
            }
        }

        flag[0] = PSG_MML_CTRL_GET_REG_FLAG(&psg_mml_obj[0].ctrl, PSG_REG_ADDR_MIXER);
        PSG_MML_CTRL_CLEAR_REG_FLAG(&psg_mml_obj[0].ctrl, PSG_REG_ADDR_MIXER);

        flag[1] = PSG_MML_CTRL_GET_REG_FLAG(&psg_mml_obj[1].ctrl, PSG_REG_ADDR_MIXER);
        PSG_MML_CTRL_CLEAR_REG_FLAG(&psg_mml_obj[1].ctrl, PSG_REG_ADDR_MIXER);

        if ( (flag[0] != 0) || (flag[1] != 0) )
        {
            mixer_req[0] = PSG_MML_CTRL_GET_REG_DATA(&psg_mml_obj[0].ctrl, PSG_REG_ADDR_MIXER);
            mixer_req[1] = PSG_MML_CTRL_GET_REG_DATA(&psg_mml_obj[1].ctrl, PSG_REG_ADDR_MIXER);

            /* Swap 0ch and 2ch in slot1.  */
            flag_swp = flag[1]&(~0x5);
            flag_swp |= (flag[1]&0x1)<<2;
            flag_swp |= (flag[1]&0x4)>>2;
            flag_swp &= 0x7;
            mixer_req_swp = mixer_req[1]&(~0x2D);
            mixer_req_swp |= (mixer_req[1]&(0x9<<0))<<2;
            mixer_req_swp |= (mixer_req[1]&(0x9<<2))>>2;
            mixer_req_swp &= 0x3F;

            mixer_next = psg_read(&psg_mml_obj[0], PSG_REG_ADDR_MIXER);

            for ( ch = 0; ch < NUM_CHANNEL; ch++ )
            {
                if ( ((flag_swp>>ch)&0x1) != 0 )
                {
                    mixer_next &= (~(0x9<<ch))&0x3F;
                    mixer_next |= (mixer_req_swp&(0x9<<ch));
                }
                else if ( ((flag[0]>>ch)&0x1) != 0 )
                {
                    if ( !is_locked_addr_by_slot1(PSG_REG_ADDR_MIXER, ch) )
                    {
                        uint8_t mask = 0;
                        mask = ( is_noise_in_use_by_slot1() ? 0x1 : 0x9 );
                        mixer_next &= (~(mask<<ch))&0x3F;
                        mixer_next |= (mixer_req[0]&(mask<<ch));
                    }
                }
                else
                {
                    /*DO NOTHING*/
                }
            }

            /* Share the slot0 write driver. */
            psg_write(&psg_mml_obj[0], PSG_REG_ADDR_MIXER, mixer_next);
        }
    }
#else

    for ( slot = 0; slot < PSG_MML_SLOT_TOTAL_NUM; slot++ )
    {
        if ( psg_mml_obj[slot].initialized )
        {
            psg_mml_ctrl_proc(&psg_mml_obj[slot].ctrl, psg_mml_obj[slot].fifo);
            for ( addr = MIN_PSG_REG_ADDR; addr <= MAX_PSG_REG_ADDR; addr++ )
            {
                if ( PSG_MML_CTRL_GET_REG_FLAG(&psg_mml_obj[slot].ctrl, addr) != 0 )
                {
                    PSG_MML_CTRL_CLEAR_REG_FLAG(&psg_mml_obj[slot].ctrl, addr);
                    data = PSG_MML_CTRL_GET_REG_DATA(&psg_mml_obj[slot].ctrl, addr);

                    psg_write(&psg_mml_obj[slot], addr, data);
                }
            }
        }
    }
#endif

    PSG_MML_HOOK_END_PERIODIC_CONTROL();
}
