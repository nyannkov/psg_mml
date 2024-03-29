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
#ifndef PSG_MML_CONF_H
#define PSG_MML_CONF_H

#ifdef __cplusplus
extern "C" {
#endif/*__cplusplus*/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "hardware/irq.h"

#define PSG_MML_SLOT_TOTAL_NUM                (1)
#define PSG_MML_FIFO_SCALE                    (1)
/*#define PSG_MML_SHARE_SLOT0_DRIVER            true     */
/*#define PSG_MML_USE_TP_TABLE                  (1)      */
/*#define PSG_MML_FSCLOCK_001HZ       (2000000uL*100*uL) */ /* 2.00 MHz (unit: 0.01 Hz) */

#ifdef DEBUG
#define PSG_MML_ASSERT(TF)              assert((TF))
#else
#define PSG_MML_ASSERT(TF)
#endif

#define PSG_MML_DISABLE_PERIODIC_CONTROL()           irq_set_enabled(PWM_IRQ_WRAP, false)
#define PSG_MML_ENABLE_PERIODIC_CONTROL()            irq_set_enabled(PWM_IRQ_WRAP, true)       

#define PSG_MML_HOOK_START_PERIODIC_CONTROL()
#define PSG_MML_HOOK_END_PERIODIC_CONTROL()

#ifdef __cplusplus
}
#endif/*__cplusplus*/

#endif/*PSG_MML_CONF_H*/
