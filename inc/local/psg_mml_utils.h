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
#ifndef PSG_MML_UTILS
#define PSG_MML_UTILS


#ifdef __cplusplus
extern "C" {
#endif/*__cplusplus*/


#define U16(h,l)                (((uint16_t)(h)<<8)|(l))
#define U16_HI(x)               (((x)>>8)&0xFF)
#define U16_LO(x)               (((x)>>0)&0xFF)

#define U32(hh,hl,lh,ll)        (((uint32_t)(hh)<<24)|((uint32_t)(hl)<<16)|((uint32_t)(lh)<<8)|(ll))
#define U32_HH(x)               (((x)>>24)&0xFF)
#define U32_HL(x)               (((x)>>16)&0xFF)
#define U32_LH(x)               (((x)>> 8)&0xFF)
#define U32_LL(x)               (((x)>> 0)&0xFF)


#ifndef PSG_MML_MEMCPY
void *psg_mml_memcpy(void *p_dest, const void *p_src, size_t n);
#define PSG_MML_MEMCPY      psg_mml_memcpy
#endif/*PSG_MML_MEMCPY*/


#ifndef PSG_MML_MEMSET
void *psg_mml_memset(void *p_dest, int c, size_t n);
#define PSG_MML_MEMSET      psg_mml_memset
#endif/*PSG_MML_MEMSET*/


#ifdef __cplusplus
}
#endif/*__cplusplus*/

#endif/*PSG_MML_UTILS*/
