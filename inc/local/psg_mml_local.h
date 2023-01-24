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
#ifndef PSG_MML_LOCAL_H
#define PSG_MML_LOCAL_H

#ifdef __cplusplus
extern "C" {
#endif/*__cplusplus*/

#include "psg_mml_conf.h"


#ifndef PSG_MML_ASSERT
#define PSG_MML_ASSERT(TF)
#endif

#define NUM_CHANNEL                             (3)

#define MAX_TICK_HZ                             (1000)
#define MIN_TICK_HZ                             (1)    

#define MAX_TP                                  (4095)

#define MIN_PSG_REG_ADDR                        (0x0)
#define MAX_PSG_REG_ADDR                        (0xD)
#define NUM_OF_PSG_REG                          (MAX_PSG_REG_ADDR-MIN_PSG_REG_ADDR+1)
#define PSG_REG_ADDR_MIXER                      (0x7)
#define PSG_REG_ALL_MUTE                        (0x3F)


#define DECODE_STATUS_START                     (0)
#define DECODE_STATUS_END                       (1)

#define MAX_PSG_MML_MSG_NUM                     (14)

#define MAX_MML_TEXT_LEN                        ((size_t)(~(size_t)2))


#define MIN_TEMPO                               (32)
#define MAX_TEMPO                               (255)
#define DEFAULT_TEMPO                           (120)

#define MIN_NOTE_LENGTH                         (1)
#define MAX_NOTE_LENGTH                         (64)
#define DEFAULT_NOTE_LENGTH                     (4)

#define MIN_OCTAVE                              (1)
#define MAX_OCTAVE                              (8)
#define DEFAULT_OCTAVE                          (4)

#define MIN_ENVELOP_SHAPE                       (0x0)
#define MAX_ENVELOP_SHAPE                       (0xF)
#define DEFAULT_ENVELOP_SHAPE                   (0x0)

#define MIN_ENVELOP_EP                          (0x0000)
#define MAX_ENVELOP_EP                          (0xFFFF)
#define DEFAULT_ENVELOP_EP                      (0x0000)

#define MIN_VOLUME_LEVEL                        (0)
#define MAX_VOLUME_LEVEL                        (15)
#define DEFAULT_VOLUME_LEVEL                    (15)

#define MIN_NOTE_NUMBER                         (0)
#define MAX_NOTE_NUMBER                         (95)
#define DEFAULT_NOTE_NUMBER                     (0)

#define MIN_REST_LENGTH                         (1)
#define MAX_REST_LENGTH                         (64)
#define DEFAULT_REST_LENGTH                     (4)

#define MIN_GATE_TIME                           (1)
#define MAX_GATE_TIME                           (8)
#define DEFAULT_GATE_TIME                       MAX_GATE_TIME

#define MAX_REPEATING_DOT_LENGTH                (10)

#define MIN_DETUNE_LEVEL                        (-360*8)
#define MAX_DETUNE_LEVEL                        (360*8)
#define DEFAULT_DETUNE_LEVEL                    (0)

#define Q_PITCHBEND_FACTOR                      (q24_t)(16809550)/* POW(2, 1/360) << 24 */
#define Q_PITCHBEND_FACTOR_N                    (q24_t)(16744944)/* POW(2,-1/360) << 24 */

#define Q_CALCTP_FACTOR                         (q24_t)(17774841)/* POW(2, 1/12) <<24  */ 
#define Q_CALCTP_FACTOR_N                       (q24_t)(15835583)/* POW(2,-1/12) <<24  */ 

#define MIN_PITCHBEND_LEVEL                     (-360*8)
#define MAX_PITCHBEND_LEVEL                     (360*8)
#define DEFAULT_PITCHBEND_LEVEL                 (0)

#define MIN_NOISE_NP                            (0)
#define MAX_NOISE_NP                            (31)
#define DEFAULT_NOISE_NP                        (16)

#define MIN_NOISE_LENGTH                        (1)
#define MAX_NOISE_LENGTH                        (64)
#define DEFAULT_NOISE_LENGTH                    (4)

#define LFO_MODE_OFF                            (0)
#define LFO_MODE_TRIANGLE                       (1)
/*#define LFO_MODE_SAWTOOTH                       (2)*/
/*#define LFO_MODE_SINE                           (3)*/
#define MIN_LFO_MODE                            LFO_MODE_OFF
#define MAX_LFO_MODE                            LFO_MODE_TRIANGLE
#define DEFAULT_LFO_MODE                        LFO_MODE_OFF

#define MIN_LFO_DEPTH                           (0)
#define MAX_LFO_DEPTH                           (360)
#define DEFAULT_LFO_DEPTH                       (0)

/* unit: 0.1 Hz */
#define MIN_LFO_SPEED                           (0)
#define MAX_LFO_SPEED                           (200)
#define DEFAULT_LFO_SPEED                       (40)

/* unit: sec. */
#define MAX_LFO_PERIOD                          (10) 

#define MIN_LFO_DELAY                           (0)
#define MAX_LFO_DELAY                           (64)
#define DEFAULT_LFO_DELAY                       (0)

#define SOFT_ENVELOPE_MODE_OFF                  (0)
#define SOFT_ENVELOPE_MODE_ON                   (1)
#define MIN_SOFT_ENVELOPE_MODE                  SOFT_ENVELOPE_MODE_OFF
#define MAX_SOFT_ENVELOPE_MODE                  SOFT_ENVELOPE_MODE_ON
#define DEFAULT_SOFT_ENVELOPE_MODE              SOFT_ENVELOPE_MODE_OFF


/* unit: 1msec. */
#define MIN_SOFT_ENVELOPE_ATTACK                (0)
#define MAX_SOFT_ENVELOPE_ATTACK                (10000)
#define DEFAULT_SOFT_ENVELOPE_ATTACK            (0)

/* unit: 1msec. */
#define MIN_SOFT_ENVELOPE_HOLD                  (0)
#define MAX_SOFT_ENVELOPE_HOLD                  (10000)
#define DEFAULT_SOFT_ENVELOPE_HOLD              (0)

/* unit: 1msec. */
#define MIN_SOFT_ENVELOPE_DECAY                 (0)
#define MAX_SOFT_ENVELOPE_DECAY                 (10000)
#define DEFAULT_SOFT_ENVELOPE_DECAY             (0)

/* unit: 1 pct. */
#define MIN_SOFT_ENVELOPE_SUSTAIN               (0)
#define MAX_SOFT_ENVELOPE_SUSTAIN               (100)
#define DEFAULT_SOFT_ENVELOPE_SUSTAIN           (100)

/* unit: 1msec. */
#define MIN_SOFT_ENVELOPE_FADE                  (0)
#define MAX_SOFT_ENVELOPE_FADE                  (10000)
#define DEFAULT_SOFT_ENVELOPE_FADE              (0)

/* unit: 1msec. */
#define MIN_SOFT_ENVELOPE_RELEASE               (0)
#define MAX_SOFT_ENVELOPE_RELEASE               (10000)
#define DEFAULT_SOFT_ENVELOPE_RELEASE           (0)

#define MIN_LOOP_TIMES                          (0)
#define MAX_LOOP_TIMES                          (255)
#define DEFAULT_LOOP_TIMES                      (1)

#define MAX_LOOP_NESTING_DEPTH                  (5)

#ifdef __cplusplus
}
#endif/*__cplusplus*/

#endif/*PSG_MML_LOCAL_H*/
