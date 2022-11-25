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
#ifndef PSG_MML_Q_H
#define PSG_MML_Q_H

#ifdef __cplusplus
extern "C" {
#endif/*__cplusplus*/


typedef int32_t q12_t;
typedef int64_t lq12_t;
typedef int32_t q24_t;
typedef int64_t lq24_t;

#define I2Q12(x)        ((q12_t)(x)<<12)
#define I2Q24(x)        ((q24_t)(x)<<24)
#define I2LQ12(x)       ((lq12_t)(x)<<12)
#define I2LQ24(x)       ((lq24_t)(x)<<24)

#define Q_INT(x, q)     ((x)>>(q))
#define Q_FRAC(x, q)    ((x)&((1uL<<(q))-1))


#ifdef __cplusplus
}
#endif/*__cplusplus*/

#endif/*PSG_MML_Q_H*/

