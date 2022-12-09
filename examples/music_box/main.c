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
#include "psg_mml.h"
#include "./boards/board.h"

#define SLOT_MUSIC  0

/*
 * Yurikago no uta 
 *
 * Music by Shin Kusakawa
 * Lyrics by Hakushu Kitahara
 *
 * */
static const char *EXAMPLE_MML = 
"T93"
"[2"
    "$E1$A0$H100$D100$S90$F2500$R0"
    "$M1$J2$L40$T8."
    "V15L4O4"
    "A#>C8<A#8GD#F.G8F2"
    "CD#8C8<A#>D#GA#8G8F2"
    "G.G8GF8G8A#.A#8>C<A#G.A#8GFD#1"
"],"
"T93"
"[2"
    "$E1$A0$H100$D100$S90$F2500$R0"
    "$M1$J2$L40$T8."
    "$B1"
    "V12L4O4"
    "R32"
    "A#>C8<A#8GD#F.G8F2"
    "CD#8C8<A#>D#GA#8G8F2"
    "G.G8GF8G8A#.A#8>C<A#G.A#8GFD#2...."
"],"
"T93"
"[2"
    "$E1$A0$H100$D100$S90$F4000$R0"
    "$M1$J2$L40$T8."
    "V13L4O4"
    "D#2<A#2 >D1 <G#2 G2 A#1"
    ">D#2 D2 C2 <G#2 A#2 A#2 G1"
"]";

int main(void)
{
    uint16_t tick_hz;
    psg_mml_t r;
    PSG_MML_PLAY_STATE_t play_state;

    /* Initialize the board used in this example. */
    board_init();

    /* Get the timer rate in Hz.*/
    tick_hz = board_get_timer_rate();

    /* 
     * Initialize slot with the call rate of the psg_mml_periodic_control and
     * a PSG driver. The value of timer rate corresponds to the recipro-
     * cal of the time resolution (1 tick) when playing MML.
     */
    r = psg_mml_init(SLOT_MUSIC, tick_hz, board_psg_write);
    if ( r != PSG_MML_SUCCESS )
    {
        return (-1);
    }

    /* Register a callback function to be executed on the Tick event. */
    board_set_timer_callback(psg_mml_periodic_control);

    /* Start the timer that calls psg_mml_periodic_control. */
    board_start_timer();

    /* Load the MML text to be played for the initialized slot. */
    r = psg_mml_load_text(SLOT_MUSIC, EXAMPLE_MML, 0);
    if ( r != PSG_MML_SUCCESS )
    {
        return (-2);
    }

    /* Start playing the PSG while reading MML decode information from the queue. */
    r = psg_mml_play_start(SLOT_MUSIC, 10);
    if ( r != PSG_MML_SUCCESS )
    {
        return (-3);
    }

    /* Repeat until MML decoding is completed. */
    while (true)
    {
        r = psg_mml_decode(SLOT_MUSIC);
        if ( r == PSG_MML_DECODE_END )
        {
            break;
        }
        else if ( r < 0 )
        {
            return (-4);
        }
        else
        {
            continue;
        }
    }

    /* Wait until the playing music end. */
    while (true)
    {
        r = psg_mml_get_play_state(SLOT_MUSIC, &play_state);
        if ( r == PSG_MML_SUCCESS )
        {
            if ( play_state == E_PSG_MML_PLAY_STATE_END )
            {
                break;
            }
        }
        else
        {
            return (-5);
        }
    }

    
    while (true)
        tight_loop_contents();

	return 0;
}

