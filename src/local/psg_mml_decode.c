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
#include "../../inc/local/psg_mml_decode.h"


#ifndef PSG_MML_USE_TP_TABLE
#define PSG_MML_USE_TP_TABLE                  (1)
#endif/*PSG_MML_USE_TP_TABLE*/


#if (PSG_MML_USE_TP_TABLE==0) 
/* Not use tp_table. */

#ifndef PSG_MML_FSCLOCK_001HZ
#define PSG_MML_FSCLOCK_001HZ       (2000000uL*100uL)   /* 2.00 MHz (unit: 0.01 Hz) */
#endif/*PSG_MML_FSCLOCK_001HZ*/

#if (PSG_MML_FSCLOCK_001HZ <= 0)
#error PSG_MML_FSCLOCK_001HZ must be greater than 0.
#endif

static uint16_t calc_tp(int16_t n)
{
    lq24_t lq24_hertz;
    lq24_t lq24_clock;
    q24_t  q24_f;
    int32_t r;
    int32_t q;
    uint32_t tp;

    n -= 45;
    q = n/12;

    lq24_hertz = I2LQ24(440);
    if ( n >= 0 )
    {
        lq24_hertz <<= q;
        q24_f = Q_CALCTP_FACTOR;
    }
    else
    {
        lq24_hertz >>= q;
        q24_f = Q_CALCTP_FACTOR_N;
        n *= -1;
    }

    r = n%12;
    while ( r-- > 0 )
    {
        lq24_hertz *= q24_f;
        lq24_hertz >>= 24;
    }

    lq24_clock = I2LQ24(PSG_MML_FSCLOCK_001HZ);

    tp = lq24_clock/(1600*lq24_hertz);

    if ( tp > MAX_TP )
    {
        tp = MAX_TP;
    }
    
    return (uint16_t)tp;
}

#define GET_TP(n)       calc_tp((n))

#else
/* Use tp_table. */


#define NUM_TP_TABLE        (MAX_NOTE_NUMBER - MIN_NOTE_NUMBER + 1)

#if (PSG_MML_USE_TP_TABLE==1)
/* Use default table. */

/* fsclock = 2 MHz */
static const uint16_t tp_table[NUM_TP_TABLE] = 
{
    0xEEE, 0xE17, 0xD4D, 0xC8E, 0xBD9, 0xB2F, 0xA8E, 0x9F7, 0x967, 0x8E0, 0x861, 0x7E8,
    0x777, 0x70B, 0x6A6, 0x647, 0x5EC, 0x597, 0x547, 0x4FB, 0x4B3, 0x470, 0x430, 0x3F4,
    0x3BB, 0x385, 0x353, 0x323, 0x2F6, 0x2CB, 0x2A3, 0x27D, 0x259, 0x238, 0x218, 0x1FA,
    0x1DD, 0x1C2, 0x1A9, 0x191, 0x17B, 0x165, 0x151, 0x13E, 0x12C, 0x11C, 0x10C, 0x0FD,
    0x0EE, 0x0E1, 0x0D4, 0x0C8, 0x0BD, 0x0B2, 0x0A8, 0x09F, 0x096, 0x08E, 0x086, 0x07E,
    0x077, 0x070, 0x06A, 0x064, 0x05E, 0x059, 0x054, 0x04F, 0x04B, 0x047, 0x043, 0x03F,
    0x03B, 0x038, 0x035, 0x032, 0x02F, 0x02C, 0x02A, 0x027, 0x025, 0x023, 0x021, 0x01F,
    0x01D, 0x01C, 0x01A, 0x019, 0x017, 0x016, 0x015, 0x013, 0x012, 0x011, 0x010, 0x00F,
};

#else
/* Use user defined tp_table. */

#include "psg_mml_usr_tp_table.h"

#endif

#define GET_TP(n)       tp_table[(n)]

#endif

static void mml_decode_operate_mixer(PSG_MML_DECODER_t *p_decoder, uint8_t ch, PSG_MML_MSG_t *p_out);
static void mml_decode_dollar(PSG_MML_DECODER_t *p_decoder, uint8_t ch);

static uint16_t msec2tick(uint16_t time_ms, uint16_t tick_hz);
static uint16_t notelen2tick(uint8_t note_len, uint8_t tempo, uint16_t tick_hz, uint8_t dot_cnt);
static int32_t clip_int32(int32_t x, int32_t min, int32_t max);
static const char * count_dot_repetition(const char *p_pos, const char *p_tail, uint8_t *p_out);
static const char * shift_half_note_number(const char *p_pos, const char *p_tail, int32_t note_num, int32_t *p_out);
static const char * get_legato_end_note_num(const char *p_pos, const char *p_tail, int32_t default_octave, int32_t *p_out);

static q12_t    get_note_on_time(uint8_t note_len, uint16_t tick_hz, uint8_t tempo, uint8_t dot_cnt);
static uint8_t  get_tp_table_column_number(const char note_name);
static const char * read_number(const char *p_pos, const char *p_tail, int32_t min, int32_t max, int32_t default_value, int32_t *p_out);
static const char * read_number_ex(const char *p_pos, const char *p_tail, int32_t min, int32_t max, int32_t default_value, int32_t *p_out, /*@null@*/bool *p_is_omitted);
static uint8_t  read_number_uint8(PSG_MML_DECODER_t *p_decoder, uint8_t ch,  int32_t min, int32_t max, int32_t default_value);
static int16_t  read_number_int16(PSG_MML_DECODER_t *p_decoder, uint8_t ch,  int32_t min, int32_t max, int32_t default_value);
static uint16_t read_number_uint16(PSG_MML_DECODER_t *p_decoder, uint8_t ch,  int32_t min, int32_t max, int32_t default_value);
static uint16_t read_number_msec2tick(PSG_MML_DECODER_t *p_decoder, uint8_t ch,  int32_t min, int32_t max, int32_t default_value);
static uint16_t read_number_notelen2tick(PSG_MML_DECODER_t *p_decoder, uint8_t ch,  int32_t min, int32_t max, int32_t default_value);
static q24_t    read_number_pitchbend(PSG_MML_DECODER_t *p_decoder, uint8_t ch,  int32_t min, int32_t max, int32_t default_value);

static void skip_white_space(const char **pp_text);
static uint16_t shift_tp(uint16_t tp, q24_t q24_factor);
static void mml_params_init(PSG_MML_DECODER_t *p_decoder);



static inline char to_upper_case(char c)
{
    if ( ('a' <= c) && (c <= 'z') )
    {
        c &= (uint8_t)~0x20UL;
    }
    return c;
}

static inline bool is_white_space(const char c)
{
    switch(c)
    {
    case ' ': /*@fallthrough@*/
    case '\t':/*@fallthrough@*/
    case '\n':/*@fallthrough@*/
    case '\r':
        return true;
    default:
        return false;
    }
}

static void skip_white_space(const char **pp_text)
{
    size_t n = MAX_MML_TEXT_LEN;
    while ( is_white_space(**pp_text) && (n != 0) )
    {
        if ( **pp_text == '\0' )
        {
            break;
        }

        (*pp_text)++;
        n--;
    }
}

static uint16_t shift_tp(uint16_t tp, q24_t q24_factor)
{
    uint64_t u64_tp;
    
    u64_tp = (uint64_t)tp;
    u64_tp *= q24_factor;

    u64_tp += 1<<23;

    tp = (uint16_t)(u64_tp>>24);
    
    return ( (tp < MAX_TP) ? tp : MAX_TP );
}

static uint16_t msec2tick(uint16_t time_ms, uint16_t tick_hz)
{
    uint16_t time_tick;

    time_tick = ((uint32_t)time_ms*tick_hz+500)/1000;
    
    return time_tick;
}

static uint16_t notelen2tick(uint8_t note_len, uint8_t tempo, uint16_t tick_hz, uint8_t dot_cnt)
{
    uint16_t time_tk;
    q12_t q12_time_tk, q12_time_tk_delta;

    if ( note_len == 0 )
    {
        return 0;
    }

    q12_time_tk_delta = I2Q12((uint32_t)tick_hz*4*60)/((uint32_t)tempo*note_len);
    q12_time_tk = q12_time_tk_delta;
    while ( dot_cnt > 0 )
    {
        q12_time_tk_delta >>= 1;
        q12_time_tk += q12_time_tk_delta; 
        dot_cnt--;
    }

    q12_time_tk += (I2Q12(1)>>1);
    time_tk = Q_INT(q12_time_tk, 12);

    return time_tk;
}

static int32_t clip_int32(int32_t x, int32_t min, int32_t max)
{
    return( (x <= min ) ? min
          : (x >= max ) ? max
          :  x
          );
}

static const char * count_dot_repetition(const char *p_pos, const char *p_tail, uint8_t *p_out)
{
    const char *p;
    uint8_t dot_cnt;
    PSG_MML_ASSERT(p_out != NULL); 

    dot_cnt = 0;
    for ( p = p_pos; p < p_tail; p++ )
    {
        if ( *p == '.' )
        {
            dot_cnt = (dot_cnt < MAX_REPEATING_DOT_LENGTH)
                    ? (dot_cnt+1)
                    : MAX_REPEATING_DOT_LENGTH;
        }
        else
        {
            break;
        }
    }

    *p_out = dot_cnt;

    return p;
}

static const char * shift_half_note_number(const char *p_pos, const char *p_tail, int32_t note_num, int32_t *p_out)
{
    const char *p;
    int32_t n;
    PSG_MML_ASSERT(p_out != NULL); 

    n = note_num;

    for ( p = p_pos; p < p_tail; p++ )
    {
        if ( ( *p == '+' ) || ( *p == '#' ) )
        {
            n = ( n < MAX_NOTE_NUMBER )
              ? (n+1)
              : MAX_NOTE_NUMBER;
        }
        else if ( *p == '-' )
        {
            n = ( n > MIN_NOTE_NUMBER )
              ? (n-1)
              : MIN_NOTE_NUMBER;
        }
        else
        {
            break;
        }
    }

    n = clip_int32(n, MIN_NOTE_NUMBER, MAX_NOTE_NUMBER);
    
    *p_out = n;

    return p;
}

static const char * get_legato_end_note_num(const char *p_pos, const char *p_tail, int32_t default_octave, int32_t *p_out)
{
    const char *p;
    int32_t octave;
    uint8_t col_num;
    int32_t note_num;

    PSG_MML_ASSERT(p_pos != NULL); 
    PSG_MML_ASSERT(p_tail != NULL); 
    PSG_MML_ASSERT(p_out != NULL); 

    /* Change octave */
    octave = clip_int32(default_octave, MIN_OCTAVE, MAX_OCTAVE);
    for ( p = p_pos; p < p_tail; p++ )
    {
        if ( *p == 'O' )
        {
            const char *p_tmp;
            p_tmp = read_number(
                    p+1
                  , p_tail
                  , MIN_OCTAVE
                  , MAX_OCTAVE
                  , DEFAULT_OCTAVE
                  , &octave
                  );
            p = (p_tmp - 1);
        }
        else if ( *p == '<' )
        {
            octave = (octave > MIN_OCTAVE) ? (octave-1) : MIN_OCTAVE;
        }
        else if ( *p == '>' )
        {
            octave = (octave < MAX_OCTAVE) ? (octave+1) : MAX_OCTAVE;
        }
        else if ( is_white_space(*p) )
        {
            continue;
        }
        else
        {
            break;
        }
    }

    col_num = get_tp_table_column_number(*p);
    note_num = (int32_t)((uint32_t)col_num&0xFFu) + (octave-1)*12;
    /* Shift Note-Number */
    p = shift_half_note_number( p+1
                              , p_tail
                              , note_num
                              , &note_num);

    *p_out = note_num;

    return p_pos;
}

static const char * read_number(const char *p_pos, const char *p_tail, int32_t min, int32_t max, int32_t default_value, int32_t *p_out)
{
    return read_number_ex(p_pos, p_tail, min, max, default_value, p_out, NULL);
}

static const char * read_number_ex(const char *p_pos, const char *p_tail, int32_t min, int32_t max, int32_t default_value, int32_t *p_out, /*@null@*/bool *p_is_omitted)
{
    int32_t n;
    int32_t sign;
    bool is_omitted;

    PSG_MML_ASSERT(p_pos != NULL); 
    PSG_MML_ASSERT(p_tail != NULL); 
    PSG_MML_ASSERT(p_out != NULL); 

    if ( p_pos >= p_tail )
    {
        *p_out = default_value;
        return p_tail;
    }

    is_omitted = true;
    switch ( *p_pos )
    {
    case '+':
        sign = 1;
        p_pos++;
        is_omitted = false;
        break;

    case '-':
        sign = (-1);
        p_pos++;
        is_omitted = false;
        break;

    default:
        sign = 1;
        /*Leave pp_pos as is.*/
        break;
    }

    n = 0;
    for (/*DO NOTHING*/; p_pos < p_tail ; p_pos++ )
    {
        if ( ('0' <= *p_pos) && (*p_pos <= '9') )
        {
            is_omitted = false;
            n = clip_int32( 
                10*n + (int32_t)(*p_pos-'0')*sign
              , -100000000
              ,  100000000
              );
        }
        else
        {
            break;
        }
    }

    if ( is_omitted )
    {
        n = default_value;
    }

    *p_out = clip_int32(n, min, max);

    if ( p_is_omitted != NULL )
    {
        *p_is_omitted = is_omitted;
    }

    return p_pos;
}

static uint8_t read_number_uint8(PSG_MML_DECODER_t *p_decoder, uint8_t ch,  int32_t min, int32_t max, int32_t default_value)
{
    int32_t r = 0;

    p_decoder->mml_text_info.channel[ch].p_mml_pos = read_number(
                                    p_decoder->mml_text_info.channel[ch].p_mml_pos+1
                                  , p_decoder->mml_text_info.channel[ch].p_mml_tail
                                  , min
                                  , max
                                  , default_value
                                  , &r
                                  );

    return (uint8_t)(r&0xFFu);
}

static int16_t read_number_int16(PSG_MML_DECODER_t *p_decoder, uint8_t ch,  int32_t min, int32_t max, int32_t default_value)
{
    int32_t r = 0;

    p_decoder->mml_text_info.channel[ch].p_mml_pos = read_number(
                                    p_decoder->mml_text_info.channel[ch].p_mml_pos+1
                                  , p_decoder->mml_text_info.channel[ch].p_mml_tail
                                  , min
                                  , max
                                  , default_value
                                  , &r
                                  );

    return (int16_t)(r&0xFFFFu);
}

static uint16_t read_number_uint16(PSG_MML_DECODER_t *p_decoder, uint8_t ch,  int32_t min, int32_t max, int32_t default_value)
{
    int32_t r = 0;

    p_decoder->mml_text_info.channel[ch].p_mml_pos = read_number(
                                    p_decoder->mml_text_info.channel[ch].p_mml_pos+1
                                  , p_decoder->mml_text_info.channel[ch].p_mml_tail
                                  , min
                                  , max
                                  , default_value
                                  , &r
                                  );

    return (uint16_t)(r&0xFFFFu);
}

static uint16_t read_number_msec2tick(PSG_MML_DECODER_t *p_decoder, uint8_t ch,  int32_t min, int32_t max, int32_t default_value)
{
    int32_t r = 0;

    p_decoder->mml_text_info.channel[ch].p_mml_pos = read_number(
                                    p_decoder->mml_text_info.channel[ch].p_mml_pos+1
                                  , p_decoder->mml_text_info.channel[ch].p_mml_tail
                                  , min
                                  , max
                                  , default_value
                                  , &r
                                  );

    return msec2tick((uint16_t)r, p_decoder->tick_hz);
}

static uint16_t read_number_notelen2tick(PSG_MML_DECODER_t *p_decoder, uint8_t ch,  int32_t min, int32_t max, int32_t default_value)
{
    int32_t r = 0;
    uint8_t dot_cnt = 0;

    p_decoder->mml_text_info.channel[ch].p_mml_pos = read_number(
                                    p_decoder->mml_text_info.channel[ch].p_mml_pos+1
                                  , p_decoder->mml_text_info.channel[ch].p_mml_tail
                                  , min
                                  , max
                                  , default_value
                                  , &r
                                  );

    if ( *p_decoder->mml_text_info.channel[ch].p_mml_pos == '.' )
    {
        p_decoder->mml_text_info.channel[ch].p_mml_pos = count_dot_repetition(
              p_decoder->mml_text_info.channel[ch].p_mml_pos
            , p_decoder->mml_text_info.channel[ch].p_mml_tail
            , &dot_cnt
            );
    }

    return notelen2tick((uint16_t)r, p_decoder->tone_params.channel[ch].tempo, p_decoder->tick_hz, dot_cnt);
}

static q24_t read_number_pitchbend(PSG_MML_DECODER_t *p_decoder, uint8_t ch,  int32_t min, int32_t max, int32_t default_value)
{
    int32_t r = 0;
    uint32_t r_abs;
    uint32_t i;
    lq24_t lq24_detune;
    q24_t  q24_factor;

    p_decoder->mml_text_info.channel[ch].p_mml_pos = read_number(
                                    p_decoder->mml_text_info.channel[ch].p_mml_pos+1
                                  , p_decoder->mml_text_info.channel[ch].p_mml_tail
                                  , min
                                  , max
                                  , default_value
                                  , &r
                                  );

    lq24_detune = I2LQ24(1);
    if ( r >= 0 )
    {
        q24_factor = Q_PITCHBEND_FACTOR_N;
        r_abs = r;
    }
    else
    {
        q24_factor = Q_PITCHBEND_FACTOR;
        r_abs = r*-1;
    }

    for ( i = 0; i < r_abs; i++ )
    {
        lq24_detune *= q24_factor;
        lq24_detune >>= 24;
    }
    p_decoder->tone_params.channel[ch].q24_detune = (q24_t)lq24_detune; 

    return (q24_t)(lq24_detune&0xFFFFFFFFu);
}

static q12_t get_note_on_time(uint8_t note_len, uint16_t tick_hz, uint8_t tempo, uint8_t dot_cnt)
{
    lq12_t lq12_time, lq12_time_delta;
    PSG_MML_ASSERT(tempo >= MIN_TEMPO);

    if ( note_len == 0 )
    {
        return 0;
    }

    lq12_time_delta = I2LQ12((uint64_t)tick_hz*4*60) / ((uint64_t)tempo*note_len);
    lq12_time = lq12_time_delta;

    while ( dot_cnt > 0 )
    {
        lq12_time_delta >>= 1;
        lq12_time += lq12_time_delta;
        dot_cnt--;
    }

    return (q12_t)lq12_time;
}

static uint8_t get_tp_table_column_number(const char note_name)
{
    uint8_t col_num = -1;
    switch ( to_upper_case(note_name) )
    {
    case 'C':
        col_num = 0;
        break;
    case 'D':
        col_num = 2;
        break;
    case 'E':
        col_num = 4;
        break;
    case 'F':
        col_num = 5;
        break;
    case 'G':
        col_num = 7;
        break;
    case 'A':
        col_num = 9;
        break;
    case 'B':
        col_num = 11;
        break;
    default:
        col_num = 0;
        break;
    }
    return col_num;
}

static void mml_params_init(PSG_MML_DECODER_t *p_decoder)
{
    uint32_t i;

    PSG_MML_MEMSET(&p_decoder->mml_text_info, 0, sizeof(p_decoder->mml_text_info));
    PSG_MML_MEMSET(&p_decoder->tone_params, 0, sizeof(p_decoder->tone_params));

    for ( i = 0; i < NUM_CHANNEL; i++ )
    {
        p_decoder->tone_params.channel[i].tempo = DEFAULT_TEMPO;
        p_decoder->tone_params.channel[i].vol_ctrl = DEFAULT_VOLUME_LEVEL&0xF;
        p_decoder->tone_params.channel[i].octave = DEFAULT_OCTAVE;
        p_decoder->tone_params.channel[i].note_len = DEFAULT_NOTE_LENGTH;
        p_decoder->tone_params.channel[i].q24_detune = I2Q24(1);
        p_decoder->tone_params.channel[i].q24_pitchbend = I2Q24(1);
        p_decoder->tone_params.channel[i].gate_time = DEFAULT_GATE_TIME;
    }
}

void psg_mml_decode_init(PSG_MML_DECODER_t *p_decoder, uint16_t tick_hz)
{
    p_decoder->load_mml_text_state = E_MML_TEXT_LOAD_STATE_UNINITIALIZED;
    p_decoder->tick_hz = tick_hz;
    p_decoder->decode_flags = 0;

    mml_params_init(p_decoder);

    p_decoder->load_mml_text_state = E_MML_TEXT_LOAD_STATE_UNLOAD_MML_TEXT;
}

void psg_mml_decode_load_mml_text(PSG_MML_DECODER_t *p_decoder, const char *p_mml_text, uint16_t flags)
{
    const char *p = NULL;
    uint8_t ch = 0;

    PSG_MML_ASSERT( p_decoder != NULL );
    PSG_MML_ASSERT( p_mml_text != NULL );
    PSG_MML_ASSERT( p_decoder->load_mml_text_state != E_MML_TEXT_LOAD_STATE_UNINITIALIZED );
    p_decoder->decode_flags = flags;

    mml_params_init(p_decoder);

    p_decoder->load_mml_text_state = E_MML_TEXT_LOAD_STATE_UNLOAD_MML_TEXT;

    p = p_mml_text;
    skip_white_space(&p);

    p_decoder->mml_text_info.use_channel_num = 0;
    for ( ch = 0; ch < NUM_CHANNEL; ch++ )
    {
        p_decoder->mml_text_info.channel[ch].p_mml_head = p;
        p_decoder->mml_text_info.channel[ch].p_mml_pos = p;
        while ((*p != ',') && (*p != '\0')) p++;

        p_decoder->mml_text_info.channel[ch].p_mml_tail = p;

        p_decoder->mml_text_info.channel[ch].reset_state = true;
        p_decoder->mml_text_info.channel[ch].end_state = false;
        p_decoder->mml_text_info.use_channel_num++;

        if ( *p == '\0' )
        {
            break;
        }

        p++;
    }

    p_decoder->mml_text_info.p_mml_text = p_mml_text;

    p_decoder->load_mml_text_state = E_MML_TEXT_LOAD_STATE_READY;
}

bool psg_mml_decode_is_EOT(PSG_MML_DECODER_t *p_decoder)
{
    bool is_EOT;
    uint8_t ch;

    PSG_MML_ASSERT(p_decoder != NULL );
    PSG_MML_ASSERT(p_decoder->load_mml_text_state != E_MML_TEXT_LOAD_STATE_UNINITIALIZED);
    PSG_MML_ASSERT(p_decoder->load_mml_text_state != E_MML_TEXT_LOAD_STATE_UNLOAD_MML_TEXT);

    is_EOT = true;
    for ( ch = 0; ch < p_decoder->mml_text_info.use_channel_num; ch++ )
    {
        if ( p_decoder->mml_text_info.channel[ch].p_mml_pos < p_decoder->mml_text_info.channel[ch].p_mml_tail )
        {
            is_EOT = false;
            break;
        }
    }

    return is_EOT;
}

void psg_mml_decode_reload_mml_text(PSG_MML_DECODER_t *p_decoder)
{
    PSG_MML_ASSERT(p_decoder != NULL);
    psg_mml_decode_load_mml_text(p_decoder, p_decoder->mml_text_info.p_mml_text, p_decoder->decode_flags);
}

uint8_t psg_mml_decode_get_use_channel_num(PSG_MML_DECODER_t *p_decoder)
{
    PSG_MML_ASSERT(p_decoder != NULL);
    return p_decoder->mml_text_info.use_channel_num;
}

psg_mml_decode_t psg_mml_decode_get_message(PSG_MML_DECODER_t *p_decoder, uint8_t ch,/*@out@*/ PSG_MML_MSG_t *p_out)
{
    PSG_MML_ASSERT(p_decoder != NULL);
    PSG_MML_ASSERT(ch < NUM_CHANNEL);
    PSG_MML_ASSERT(p_out != NULL);

    /* Clear message counter */
    p_out->msg_cnt = 0;

    if ( p_decoder->mml_text_info.channel[ch].reset_state )
    {
        p_decoder->mml_text_info.channel[ch].reset_state = false;

        p_out->msg[0].type = MSG_TYPE_DECODE_STATUS;
        p_out->msg[0].data.decode_status.status = DECODE_STATUS_START;
        p_out->msg_cnt = 1;

        return PSG_MML_DECODE_CHANNEL_START;
    }

    if ( p_decoder->mml_text_info.channel[ch].p_mml_pos >= p_decoder->mml_text_info.channel[ch].p_mml_tail )
    {
        if ( !p_decoder->mml_text_info.channel[ch].end_state )
        {
            p_decoder->mml_text_info.channel[ch].end_state = true;
            p_out->msg[0].type = MSG_TYPE_DECODE_STATUS;
            p_out->msg[0].data.decode_status.status = DECODE_STATUS_END;
            p_out->msg_cnt = 1;
        }

        return PSG_MML_DECODE_CHANNEL_END;
    }

    switch ( to_upper_case(p_decoder->mml_text_info.channel[ch].p_mml_pos[0]) )
    {
    case 'A':/*@fallthrough@*/
    case 'B':/*@fallthrough@*/
    case 'C':/*@fallthrough@*/
    case 'D':/*@fallthrough@*/
    case 'E':/*@fallthrough@*/
    case 'F':/*@fallthrough@*/
    case 'G':/*@fallthrough@*/
    case 'H':/*@fallthrough@*/
    case 'N':/*@fallthrough@*/
    case 'R':
        mml_decode_operate_mixer(p_decoder, ch, p_out);
        break;
    case '$':
        mml_decode_dollar(p_decoder, ch);
        break;
    case 'T':
        p_decoder->tone_params
        .channel[ch]
        .tempo 
        = read_number_uint8(
            p_decoder
          , ch 
          , MIN_TEMPO
          , MAX_TEMPO
          , DEFAULT_TEMPO
          );
        break;
    case 'V':
        p_decoder->tone_params
        .channel[ch]
        .vol_ctrl
        = read_number_uint8(
            p_decoder
          , ch
          , MIN_VOLUME_LEVEL
          , MAX_VOLUME_LEVEL
          , DEFAULT_VOLUME_LEVEL
          );
        break;
    case 'S':
        p_decoder->tone_params
        .env_shape
        = read_number_uint8(
            p_decoder
          , ch
          , MIN_ENVELOP_SHAPE
          , MAX_ENVELOP_SHAPE
          , DEFAULT_ENVELOP_SHAPE
          );
        p_decoder->tone_params.channel[ch].vol_ctrl = (1<<4);/* 4bit:M, 3-0bit:L3-0 */

        break;
    case 'M':
        p_decoder->tone_params
        .ep
        = read_number_uint16(
            p_decoder
          , ch
          , MIN_ENVELOP_EP
          , MAX_ENVELOP_EP
          , DEFAULT_ENVELOP_EP
          );
        break;
    case 'L':

        /* Clear global dot counter */
        p_decoder->tone_params.channel[ch].note_len_dots = 0;

        p_decoder->tone_params
        .channel[ch]
        .note_len
        = read_number_uint8(
            p_decoder
          , ch
          , MIN_NOTE_LENGTH
          , MAX_NOTE_LENGTH
          , DEFAULT_NOTE_LENGTH
          );

        if ( *p_decoder->mml_text_info.channel[ch].p_mml_pos == '.' )
        {
            p_decoder->mml_text_info.channel[ch].p_mml_pos = count_dot_repetition(
                  p_decoder->mml_text_info.channel[ch].p_mml_pos
                , p_decoder->mml_text_info.channel[ch].p_mml_tail
                , &p_decoder->tone_params.channel[ch].note_len_dots
                );
        }
        break;
    case 'O':
        p_decoder->tone_params
        .channel[ch]
        .octave
        = read_number_uint8(
            p_decoder
          , ch
          , MIN_OCTAVE
          , MAX_OCTAVE
          , DEFAULT_OCTAVE
          );
        break;
    case 'Q':
        p_decoder->tone_params
        .channel[ch]
        .gate_time
        = read_number_uint8(
            p_decoder
          , ch
          , MIN_GATE_TIME
          , MAX_GATE_TIME 
          , DEFAULT_GATE_TIME
          );
        break;
    case 'I':
        p_decoder->tone_params
        .np
        = read_number_uint8(
            p_decoder
          , ch 
          , MIN_NOISE_NP
          , MAX_NOISE_NP
          , DEFAULT_NOISE_NP
        );
        break;
    case '<':
        if ( p_decoder->tone_params.channel[ch].octave > MIN_OCTAVE )
        {
            p_decoder->tone_params.channel[ch].octave--;
        }
        p_decoder->mml_text_info.channel[ch].p_mml_pos++;
        break;
    case '>':
        if ( p_decoder->tone_params.channel[ch].octave < MAX_OCTAVE )
        {
            p_decoder->tone_params.channel[ch].octave++;
        }
        p_decoder->mml_text_info.channel[ch].p_mml_pos++;
        break;

    case '[':
        if ( p_decoder->mml_text_info.channel[ch].loop_nesting_depth < MAX_LOOP_NESTING_DEPTH )
        {
            uint16_t loop_index = 0;
            loop_index = p_decoder->mml_text_info.channel[ch].loop_nesting_depth;

            p_decoder->mml_text_info
            .channel[ch]
            .loop_times[loop_index]
                = read_number_int16( p_decoder, ch
                    , MIN_LOOP_TIMES
                    , MAX_LOOP_TIMES
                    , DEFAULT_LOOP_TIMES);

            p_decoder->mml_text_info
            .channel[ch]
            .p_mml_loop_head[loop_index]
                = p_decoder->mml_text_info.channel[ch].p_mml_pos;

            p_decoder->mml_text_info.channel[ch].loop_nesting_depth = loop_index + 1;
        }
        else
        {
            p_decoder->mml_text_info.channel[ch].p_mml_pos++;
        }
        break;
    case ']':
        if ( p_decoder->mml_text_info.channel[ch].loop_nesting_depth > 0 )
        {
            uint16_t loop_index = 0;
            loop_index = p_decoder->mml_text_info.channel[ch].loop_nesting_depth - 1;

            if ( p_decoder->mml_text_info.channel[ch].loop_times[loop_index] == 1 )
            {
                /* Loop end */
                p_decoder->mml_text_info.channel[ch].loop_times[loop_index] = 0;
                p_decoder->mml_text_info.channel[ch].loop_nesting_depth = loop_index;
                p_decoder->mml_text_info.channel[ch].p_mml_pos++;
            }
            else
            {
                if ( p_decoder->mml_text_info.channel[ch].loop_times[loop_index] > 1 )
                {
                    p_decoder->mml_text_info.channel[ch].loop_times[loop_index]--;
                }
                else
                {
                    /* Infinite loop */
                }

                p_decoder->mml_text_info.channel[ch].p_mml_pos = p_decoder->mml_text_info.channel[ch].p_mml_loop_head[loop_index];
            }
        }
        else
        {
            p_decoder->mml_text_info.channel[ch].p_mml_pos++;
        }
        break;
    default:
        p_decoder->mml_text_info.channel[ch].p_mml_pos++;
        break;
    }

    return PSG_MML_DECODE_SUCCESS;
}

static void mml_decode_operate_mixer(PSG_MML_DECODER_t *p_decoder, uint8_t ch, PSG_MML_MSG_t *p_out)
{
    const char *p_pos;
    const char *p_tail;
    int32_t note_num;
    int32_t legato_end_note_num;
    int32_t note_len;
    uint16_t note_on_time_tk;
    uint16_t gate_time_tk;
    uint16_t tp;
    uint16_t tp_end;
    char head;
    uint8_t col_num;
    uint8_t dot_cnt;
    uint8_t index;
    uint8_t req_mixer;
    q12_t q12_note_on_time;
    q12_t q12_gate_time;
    q12_t q12_omega;
    bool is_note_len_omitted;
    bool is_use_global_note_len;
    bool is_start_legato_effect;
    enum
    {
        E_NOTE_TYPE_TONE = 0,
        E_NOTE_TYPE_NOISE,
        E_NOTE_TYPE_REST,
        E_NOTE_TYPE_OTHER
    } note_type;

    PSG_MML_ASSERT(p_decoder != NULL);
    PSG_MML_ASSERT(ch < NUM_CHANNEL);
    PSG_MML_ASSERT(p_out != NULL);

    p_pos = p_decoder->mml_text_info.channel[ch].p_mml_pos;
    p_tail = p_decoder->mml_text_info.channel[ch].p_mml_tail;

    head = to_upper_case(*p_pos);

    note_num = 0;
    legato_end_note_num = 0;
    note_len = 0;
    dot_cnt = 0;
    is_start_legato_effect = false;

    if ( ('A' <= head) && (head <= 'G') )
    {
        /* Set Note-Type */
        note_type = E_NOTE_TYPE_TONE;

        /* Get Note-Number */
        col_num = get_tp_table_column_number(head);
        note_num= col_num + (p_decoder->tone_params.channel[ch].octave-1)*12;

        /* Shift Note-Number */
        p_pos = shift_half_note_number( p_pos+1
                                      , p_tail
                                      , note_num
                                      , &note_num);

        /* Get Note-Length */
        is_note_len_omitted = false;
        p_pos = read_number_ex( p_pos
                           , p_tail
                           , 0 /* special case */
                           , MAX_NOTE_LENGTH
                           , p_decoder->tone_params.channel[ch].note_len
                           , &note_len
                           , &is_note_len_omitted);

        is_use_global_note_len = is_note_len_omitted;

        /* Count Dot-Repetition */
        dot_cnt = 0;
        p_pos = count_dot_repetition( p_pos
                                    , p_tail
                                    , &dot_cnt);
        if ( is_use_global_note_len )
        {
            dot_cnt += p_decoder->tone_params.channel[ch].note_len_dots;
        }


        /* Legato */
        if ( *p_pos == '&' )
        {
            is_start_legato_effect = true;
            p_pos = get_legato_end_note_num( p_pos+1
                                           , p_tail
                                           , p_decoder->tone_params.channel[ch].octave
                                           , &legato_end_note_num );
        }
        else
        {
            is_start_legato_effect = false;
        }
    }
    else if ( head == 'N' )
    {
        /* Set Note-Type */
        note_type = E_NOTE_TYPE_TONE;

        /* Get Note-Number */
        p_pos = read_number(p_pos+1
                          , p_tail
                          , MIN_NOTE_NUMBER
                          , MAX_NOTE_NUMBER
                          , DEFAULT_NOTE_NUMBER
                          , &note_num);

        /* Count Dot-Repetition */
        dot_cnt = 0;
        p_pos = count_dot_repetition( p_pos
                                    , p_tail
                                    , &dot_cnt);

        note_len = p_decoder->tone_params.channel[ch].note_len;
        dot_cnt += p_decoder->tone_params.channel[ch].note_len_dots;
    }
    else if ( (head == 'H') || (head == 'R') )
    {
        /* Set Note-Type */
        note_type = ( head == 'R' ) 
                  ? E_NOTE_TYPE_REST 
                  : E_NOTE_TYPE_NOISE;

        /* Get Note-Length */
        if ( ((p_decoder->decode_flags>>0)&0x01) != 0 )
        {
            is_note_len_omitted = false;
            p_pos = read_number_ex( p_pos+1
                               , p_tail
                               , MIN_NOTE_LENGTH
                               , MAX_NOTE_LENGTH
                               , p_decoder->tone_params.channel[ch].note_len 
                               , &note_len
                               , &is_note_len_omitted);

            is_use_global_note_len = is_note_len_omitted;
        }
        else
        {
            is_use_global_note_len = false;
            p_pos = read_number( p_pos+1
                               , p_tail
                               , MIN_NOTE_LENGTH
                               , MAX_NOTE_LENGTH
                               , DEFAULT_NOTE_LENGTH
                               , &note_len
                               );
        }

        /* Count Dot-Repetition */
        dot_cnt = 0;
        p_pos = count_dot_repetition( p_pos
                                    , p_tail
                                    , &dot_cnt);

        if ( is_use_global_note_len )
        {
            dot_cnt += p_decoder->tone_params.channel[ch].note_len_dots;
        }
    }
    else
    {
        note_type = E_NOTE_TYPE_OTHER;
        PSG_MML_ASSERT(true);
    }

    /* Update current MML pos */
    p_decoder->mml_text_info.channel[ch].p_mml_pos = p_pos;

    /* Create message */
    p_out->msg_cnt = 0;

    index = 0;

    if ( note_type == E_NOTE_TYPE_TONE ) 
    {
        /* Apply detune-level to tp. */
        tp = shift_tp(GET_TP(note_num), p_decoder->tone_params.channel[ch].q24_detune);
        if ( is_start_legato_effect )
        {
            tp_end = shift_tp(GET_TP(legato_end_note_num), p_decoder->tone_params.channel[ch].q24_detune);
        }
        else
        {
            tp_end = shift_tp(tp, p_decoder->tone_params.channel[ch].q24_pitchbend);
        }

        p_out->msg[index].type = MSG_TYPE_SETTINGS_2;
        p_out->msg[index].data.settings_2.addr1 = 2*ch;
        p_out->msg[index].data.settings_2.data1 = U16_LO(tp);
        p_out->msg[index].data.settings_2.addr2 = 2*ch + 1;
        p_out->msg[index].data.settings_2.data2 = U16_HI(tp);
        index++;
    }
    else
    {
        tp = 0;
        tp_end = 0;
    }

    /* Set NP */
    if ( note_type == E_NOTE_TYPE_NOISE )
    {
        p_out->msg[index].type = MSG_TYPE_SETTINGS_1;
        p_out->msg[index].data.settings_1.addr = 0x06;
        p_out->msg[index].data.settings_1.data = p_decoder->tone_params.np;
        index++;
    }

    if ( ( note_type == E_NOTE_TYPE_TONE ) 
      || ( note_type == E_NOTE_TYPE_NOISE ) )
    {
        p_out->msg[index].type = MSG_TYPE_SETTINGS_1;
        p_out->msg[index].data.settings_1.addr = ch + 0x08;
        p_out->msg[index].data.settings_1.data = p_decoder->tone_params.channel[ch].vol_ctrl;
        index++;

        if ( (p_decoder->tone_params.channel[ch].vol_ctrl & (1<<4)) != 0 )
        {
            /* To reload 5bit counter of the envelope generator. */
            p_out->msg[index].type = MSG_TYPE_SETTINGS_2;
            p_out->msg[index].data.settings_2.addr1 = 0x0B;
            p_out->msg[index].data.settings_2.data1 = U16_LO(p_decoder->tone_params.ep);
            p_out->msg[index].data.settings_2.addr2 = 0x0C;
            p_out->msg[index].data.settings_2.data2 = U16_HI(p_decoder->tone_params.ep);
            index++;

            if ( !p_decoder->tone_params.channel[ch].legato_effect )
            {
                /* Set envelope shape */
                p_out->msg[index].type = MSG_TYPE_SETTINGS_1;
                p_out->msg[index].data.settings_1.addr = 0x0D;
                p_out->msg[index].data.settings_1.data = p_decoder->tone_params.env_shape;
                index++;
            }

            /* Software-Envelope generator(Turn OFF regardless of the set value.) */
            p_out->msg[index].type = MSG_TYPE_SOFT_ENV_1;
            p_out->msg[index].data.soft_env_1.mode = SOFT_ENVELOPE_MODE_OFF;
            index++;
        }
        else
        {
            /* Software-Envelope generator */
            p_out->msg[index].type = MSG_TYPE_SOFT_ENV_1;
            p_out->msg[index].data.soft_env_1.mode      = p_decoder->tone_params.channel[ch].soft_env_mode;
            p_out->msg[index].data.soft_env_1.top_volume= p_decoder->tone_params.channel[ch].vol_ctrl&0xF;
            p_out->msg[index].data.soft_env_1.sus_volume= (uint8_t)(((uint16_t)p_out->msg[index].data.soft_env_1.top_volume*p_decoder->tone_params.channel[ch].soft_env_sustain+50)/100); 
            p_out->msg[index].data.soft_env_1.legato_effect = (p_decoder->tone_params.channel[ch].legato_effect ? 1 : 0);
            index++;

            p_out->msg[index].type = MSG_TYPE_SOFT_ENV_2;
            p_out->msg[index].data.soft_env_2.attack_tk_hi = U16_HI(p_decoder->tone_params.channel[ch].soft_env_attack_tk);
            p_out->msg[index].data.soft_env_2.attack_tk_lo = U16_LO(p_decoder->tone_params.channel[ch].soft_env_attack_tk);
            p_out->msg[index].data.soft_env_2.hold_tk_hi   = U16_HI(p_decoder->tone_params.channel[ch].soft_env_hold_tk);
            p_out->msg[index].data.soft_env_2.hold_tk_lo   = U16_LO(p_decoder->tone_params.channel[ch].soft_env_hold_tk);
            index++;

            p_out->msg[index].type = MSG_TYPE_SOFT_ENV_3;
            p_out->msg[index].data.soft_env_3.decay_tk_hi  = U16_HI(p_decoder->tone_params.channel[ch].soft_env_decay_tk);
            p_out->msg[index].data.soft_env_3.decay_tk_lo  = U16_LO(p_decoder->tone_params.channel[ch].soft_env_decay_tk);
            p_out->msg[index].data.soft_env_3.fade_tk_hi   = U16_HI(p_decoder->tone_params.channel[ch].soft_env_fade_tk);
            p_out->msg[index].data.soft_env_3.fade_tk_lo   = U16_LO(p_decoder->tone_params.channel[ch].soft_env_fade_tk);
            index++;

            p_out->msg[index].type = MSG_TYPE_SOFT_ENV_4;
            p_out->msg[index].data.soft_env_4.release_tk_hi = U16_HI(p_decoder->tone_params.channel[ch].soft_env_release_tk);
            p_out->msg[index].data.soft_env_4.release_tk_lo = U16_LO(p_decoder->tone_params.channel[ch].soft_env_release_tk);
            index++;
        }
    }

    if ( note_type == E_NOTE_TYPE_TONE )
    {
        if ( !p_decoder->tone_params.channel[ch].legato_effect )
        {
            p_out->msg[index].type = MSG_TYPE_LFO_1;
            p_out->msg[index].data.lfo_1.mode  = p_decoder->tone_params.channel[ch].lfo_mode;
            p_out->msg[index].data.lfo_1.depth = p_decoder->tone_params.channel[ch].lfo_depth;
            p_out->msg[index].data.lfo_1.delay_tk_hi = U16_HI(p_decoder->tone_params.channel[ch].lfo_delay_tk);
            p_out->msg[index].data.lfo_1.delay_tk_lo = U16_LO(p_decoder->tone_params.channel[ch].lfo_delay_tk);
            index++;

            p_out->msg[index].type = MSG_TYPE_LFO_2;
            q12_omega = I2Q12(1);
            q12_omega *= (uint32_t)p_decoder->tone_params.channel[ch].lfo_depth*4*p_decoder->tone_params.channel[ch].lfo_speed;
            q12_omega /= ((uint32_t)p_decoder->tick_hz*MAX_LFO_PERIOD);
            p_out->msg[index].data.lfo_2.q12_omega_hh = U32_HH(q12_omega);
            p_out->msg[index].data.lfo_2.q12_omega_hl = U32_HL(q12_omega);
            p_out->msg[index].data.lfo_2.q12_omega_lh = U32_LH(q12_omega);
            p_out->msg[index].data.lfo_2.q12_omega_ll = U32_LL(q12_omega);
            index++;
        }
    }

    /* Note-ON */
    if ( note_len != 0 )
    {
        q12_note_on_time  = get_note_on_time(
                                    (uint8_t)(note_len&0xFFu)
                                  , p_decoder->tick_hz
                                  , p_decoder->tone_params.channel[ch].tempo
                                  , dot_cnt
                                  );
        q12_note_on_time += p_decoder->tone_params.channel[ch].q12_time_frac_part;
        p_decoder->tone_params.channel[ch].q12_time_frac_part = Q_FRAC(q12_note_on_time, 12);
        note_on_time_tk = Q_INT(q12_note_on_time, 12);

        q12_gate_time = (q12_note_on_time * (q12_t)p_decoder->tone_params.channel[ch].gate_time)/MAX_GATE_TIME;
        gate_time_tk = Q_INT(q12_gate_time, 12);

        p_out->msg[index].type = MSG_TYPE_SETTINGS_3;
        p_out->msg[index].data.settings_3.gate_time_tk_hi = U16_HI(gate_time_tk);
        p_out->msg[index].data.settings_3.gate_time_tk_lo = U16_LO(gate_time_tk);
        p_out->msg[index].data.settings_3.tp_end_hi = U16_HI(tp_end);
        p_out->msg[index].data.settings_3.tp_end_lo = U16_LO(tp_end);
        index++;

        req_mixer = 0x00;
        req_mixer = ( note_type == E_NOTE_TYPE_TONE  ) ? (0x08)
                  : ( note_type == E_NOTE_TYPE_NOISE ) ? (0x01)
                  : (0x09);

        p_out->msg[index].type = (note_type == E_NOTE_TYPE_REST)
                               ? MSG_TYPE_NOTE_ON_REST
                               : MSG_TYPE_NOTE_ON;

        p_out->msg[index].data.note_on.addr = PSG_REG_ADDR_MIXER;
        p_out->msg[index].data.note_on.data = req_mixer<<ch;
        p_out->msg[index].data.note_on.note_on_time_tk_hi = U16_HI(note_on_time_tk);
        p_out->msg[index].data.note_on.note_on_time_tk_lo = U16_LO(note_on_time_tk);
        index++;
    }

    if ( !is_start_legato_effect )
    {
        /* Note-OFF */
        p_out->msg[index].type = MSG_TYPE_NOTE_OFF;
        p_out->msg[index].data.note_off.addr = PSG_REG_ADDR_MIXER;
        p_out->msg[index].data.note_off.data = 0x09<<ch;
        index++;
    }

    PSG_MML_ASSERT(index <= MAX_PSG_MML_MSG_NUM);

    p_decoder->tone_params.channel[ch].legato_effect = is_start_legato_effect;

    p_out->msg_cnt = index;
}

static void mml_decode_dollar(PSG_MML_DECODER_t *p_decoder, uint8_t ch)
{
    PSG_MML_ASSERT(p_decoder != NULL);
    PSG_MML_ASSERT(ch < NUM_CHANNEL);

    p_decoder->mml_text_info.channel[ch].p_mml_pos++;

    switch ( to_upper_case(p_decoder->mml_text_info.channel[ch].p_mml_pos[0]) )
    {
        case 'A':
            p_decoder->tone_params
            .channel[ch]
            .soft_env_attack_tk
            = read_number_msec2tick(
                p_decoder
              , ch
              , MIN_SOFT_ENVELOPE_ATTACK
              , MAX_SOFT_ENVELOPE_ATTACK
              , DEFAULT_SOFT_ENVELOPE_ATTACK
            );
            break;
        case 'D':
            p_decoder->tone_params
            .channel[ch]
            .soft_env_decay_tk
            = read_number_msec2tick(
                p_decoder
              , ch 
              , MIN_SOFT_ENVELOPE_DECAY
              , MAX_SOFT_ENVELOPE_DECAY
              , DEFAULT_SOFT_ENVELOPE_DECAY
            );
            break;
        case 'E':
            p_decoder->tone_params
            .channel[ch]
            .soft_env_mode 
            = read_number_uint8(
                p_decoder
              , ch 
              , MIN_SOFT_ENVELOPE_MODE
              , MAX_SOFT_ENVELOPE_MODE
              , DEFAULT_SOFT_ENVELOPE_MODE
            );
            break;
        case 'F':
            p_decoder->tone_params
            .channel[ch]
            .soft_env_fade_tk
            = read_number_msec2tick(
                p_decoder
              , ch 
              , MIN_SOFT_ENVELOPE_FADE
              , MAX_SOFT_ENVELOPE_FADE
              , DEFAULT_SOFT_ENVELOPE_FADE
            );
            break;
        case 'H':
            p_decoder->tone_params
            .channel[ch]
            .soft_env_hold_tk
            = read_number_msec2tick(
                p_decoder
              , ch 
              , MIN_SOFT_ENVELOPE_HOLD
              , MAX_SOFT_ENVELOPE_HOLD
              , DEFAULT_SOFT_ENVELOPE_HOLD
            );
            break;
        case 'S':
            p_decoder->tone_params
            .channel[ch]
            .soft_env_sustain
            = read_number_uint8(
                p_decoder
              , ch 
              , MIN_SOFT_ENVELOPE_SUSTAIN
              , MAX_SOFT_ENVELOPE_SUSTAIN
              , DEFAULT_SOFT_ENVELOPE_SUSTAIN
            );
            break;
        case 'R':
            p_decoder->tone_params
            .channel[ch]
            .soft_env_release_tk
            = read_number_msec2tick(
                p_decoder
              , ch 
              , MIN_SOFT_ENVELOPE_RELEASE
              , MAX_SOFT_ENVELOPE_RELEASE
              , DEFAULT_SOFT_ENVELOPE_RELEASE
            );
            break;
        case 'M':
            p_decoder->tone_params
            .channel[ch]
            .lfo_mode
            = read_number_uint8(
                p_decoder
              , ch 
              , MIN_LFO_MODE
              , MAX_LFO_MODE
              , DEFAULT_LFO_MODE
            );
            break;
        case 'B':
            p_decoder->tone_params
            .channel[ch]
            .q24_detune
            = read_number_pitchbend(
                p_decoder
              , ch 
              , MIN_DETUNE_LEVEL
              , MAX_DETUNE_LEVEL
              , DEFAULT_DETUNE_LEVEL
            );
            break;
        case 'L':
            p_decoder->tone_params
            .channel[ch]
            .lfo_speed
            = read_number_uint8(
                p_decoder
              , ch 
              , MIN_LFO_SPEED
              , MAX_LFO_SPEED
              , DEFAULT_LFO_SPEED
            );
            break;
        case 'J':
            p_decoder->tone_params
            .channel[ch]
            .lfo_depth
            = read_number_uint8(
                p_decoder
              , ch 
              , MIN_LFO_DEPTH
              , MAX_LFO_DEPTH
              , DEFAULT_LFO_DEPTH
            );
            break;
        case 'T':
            p_decoder->tone_params
            .channel[ch]
            .lfo_delay_tk
            = read_number_notelen2tick(
                p_decoder
              , ch 
              , MIN_LFO_DELAY
              , MAX_LFO_DELAY
              , DEFAULT_LFO_DELAY
            );
            break;
        case 'P':
            p_decoder->tone_params
            .channel[ch]
            .q24_pitchbend
            = read_number_pitchbend(
                p_decoder
              , ch 
              , MIN_PITCHBEND_LEVEL
              , MAX_PITCHBEND_LEVEL
              , DEFAULT_PITCHBEND_LEVEL
            );
            break;
        default:
            p_decoder->mml_text_info.channel[ch].p_mml_pos++;
            break;
    }
}

