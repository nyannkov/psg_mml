/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
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
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/pll.h"

#include "../board.h"
#include "./pin_conf.h"

#define TIMER_RATE     100 /* unit:Hz */

static void sysclk_change(void);
static void ymz294_init(void);
static void ymz294_write(uint8_t addr, uint8_t data);

static void (*p_timer_callback_handler)(void);
static void stop_timer_irq(void);
static void start_timer_irq(void);
static void timer_irq_handler(void);

void board_init(void)
{
    /* Change sysclk 125 MHz -> 124 MHz */
    sysclk_change();

    ymz294_init();
}

void board_psg_write(uint8_t addr, uint8_t data)
{
    ymz294_write(addr, data);
}

void board_set_timer_callback(void (*p_callback)(void))
{
    p_timer_callback_handler = p_callback;
}

void board_start_timer(void)
{
    start_timer_irq();
}

void board_stop_timer(void)
{
    stop_timer_irq();
}

uint16_t board_get_timer_rate(void)
{
    return TIMER_RATE;
}

static void timer_irq_handler(void)
{
    if ( p_timer_callback_handler != NULL )
    {
        p_timer_callback_handler();
    }

    pwm_clear_irq(pwm_gpio_to_slice_num(PICO_DEFAULT_LED_PIN));
}

static void ymz294_init(void)
{
    gpio_init(PIN_YMZ294_AO);
    gpio_init(PIN_YMZ294_WRCS_N);
    gpio_init(PIN_YMZ294_D0);
    gpio_init(PIN_YMZ294_D1);
    gpio_init(PIN_YMZ294_D2);
    gpio_init(PIN_YMZ294_D3);
    gpio_init(PIN_YMZ294_D4);
    gpio_init(PIN_YMZ294_D5);
    gpio_init(PIN_YMZ294_D6);
    gpio_init(PIN_YMZ294_D7);

    gpio_set_dir(PIN_YMZ294_AO, GPIO_OUT);
    gpio_set_dir(PIN_YMZ294_WRCS_N, GPIO_OUT);
    gpio_set_dir(PIN_YMZ294_D0, GPIO_OUT);
    gpio_set_dir(PIN_YMZ294_D1, GPIO_OUT);
    gpio_set_dir(PIN_YMZ294_D2, GPIO_OUT);
    gpio_set_dir(PIN_YMZ294_D3, GPIO_OUT);
    gpio_set_dir(PIN_YMZ294_D4, GPIO_OUT);
    gpio_set_dir(PIN_YMZ294_D5, GPIO_OUT);
    gpio_set_dir(PIN_YMZ294_D6, GPIO_OUT);
    gpio_set_dir(PIN_YMZ294_D7, GPIO_OUT);

    gpio_put(PIN_YMZ294_AO, 0);
    gpio_put(PIN_YMZ294_WRCS_N, 1);
    gpio_put(PIN_YMZ294_D0, 0);
    gpio_put(PIN_YMZ294_D1, 0);
    gpio_put(PIN_YMZ294_D2, 0);
    gpio_put(PIN_YMZ294_D3, 0);
    gpio_put(PIN_YMZ294_D4, 0);
    gpio_put(PIN_YMZ294_D5, 0);
    gpio_put(PIN_YMZ294_D6, 0);
    gpio_put(PIN_YMZ294_D7, 0);

    /* phiM */
    gpio_set_function(PIN_YMZ294_PHI_M, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(PIN_YMZ294_PHI_M);

    /* Set period of 31 cycles (0 to 30 inclusive) */
    pwm_set_wrap(slice_num, 30);
    pwm_set_gpio_level(PIN_YMZ294_PHI_M, 15);
    pwm_set_enabled(slice_num, true);
}

static void ymz294_write(uint8_t addr, uint8_t data)
{
    gpio_put(PIN_YMZ294_WRCS_N, true);
    gpio_put(PIN_YMZ294_AO, false);
    gpio_put(PIN_YMZ294_WRCS_N, false);
    gpio_put(PIN_YMZ294_D0, ((addr&(1u<<0))!=0) );
    gpio_put(PIN_YMZ294_D1, ((addr&(1u<<1))!=0) );
    gpio_put(PIN_YMZ294_D2, ((addr&(1u<<2))!=0) );
    gpio_put(PIN_YMZ294_D3, ((addr&(1u<<3))!=0) );
    gpio_put(PIN_YMZ294_D4, ((addr&(1u<<4))!=0) );
    gpio_put(PIN_YMZ294_D5, ((addr&(1u<<5))!=0) );
    gpio_put(PIN_YMZ294_D6, ((addr&(1u<<6))!=0) );
    gpio_put(PIN_YMZ294_D7, ((addr&(1u<<7))!=0) );
    gpio_put(PIN_YMZ294_WRCS_N, true);

    gpio_put(PIN_YMZ294_AO, true);
    gpio_put(PIN_YMZ294_WRCS_N, false);
    gpio_put(PIN_YMZ294_D0, ((data&(1u<<0))!=0) );
    gpio_put(PIN_YMZ294_D1, ((data&(1u<<1))!=0) );
    gpio_put(PIN_YMZ294_D2, ((data&(1u<<2))!=0) );
    gpio_put(PIN_YMZ294_D3, ((data&(1u<<3))!=0) );
    gpio_put(PIN_YMZ294_D4, ((data&(1u<<4))!=0) );
    gpio_put(PIN_YMZ294_D5, ((data&(1u<<5))!=0) );
    gpio_put(PIN_YMZ294_D6, ((data&(1u<<6))!=0) );
    gpio_put(PIN_YMZ294_D7, ((data&(1u<<7))!=0) );
    gpio_put(PIN_YMZ294_WRCS_N, true);
}


/* 
 * Change sysclk from 125 MHz to 124 MHz.
 * This function is created by modifying clocks_init() in pico-sdk/src/rp2_common/hardware_clocks/clocks.c.
 */
static void sysclk_change(void)
{
    // Disable resus that may be enabled from previous software
    clocks_hw->resus.ctrl = 0;

    // Before we touch PLLs, switch sys and ref cleanly away from their aux sources.
    hw_clear_bits(&clocks_hw->clk[clk_sys].ctrl, CLOCKS_CLK_SYS_CTRL_SRC_BITS);
    while (clocks_hw->clk[clk_sys].selected != 0x1)
        tight_loop_contents();
    hw_clear_bits(&clocks_hw->clk[clk_ref].ctrl, CLOCKS_CLK_REF_CTRL_SRC_BITS);
    while (clocks_hw->clk[clk_ref].selected != 0x1)
        tight_loop_contents();

    /// \tag::pll_settings[]
    // Configure PLLs
    //                   REF     FBDIV VCO            POSTDIV
    // PLL SYS: 12 / 1 = 12MHz * 124 = 1488MHZ / 6 / 2 = 124MHz
    // PLL USB: 12 / 1 = 12MHz * 40  = 480 MHz / 5 / 2 =  48MHz
    /// \end::pll_settings[]

    /// \tag::pll_init[]
    pll_init(pll_sys, 1, 1488 * MHZ, 6, 2);
    pll_init(pll_usb, 1, 480 * MHZ, 5, 2);
    /// \end::pll_init[]

    // Configure clocks
    // CLK_REF = XOSC (12MHz) / 1 = 12MHz
    clock_configure(clk_ref,
                    CLOCKS_CLK_REF_CTRL_SRC_VALUE_XOSC_CLKSRC,
                    0, // No aux mux
                    12 * MHZ,
                    12 * MHZ);

    /// \tag::configure_clk_sys[]
    // CLK SYS = PLL SYS (125MHz) / 1 = 125MHz
    clock_configure(clk_sys,
                    CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
                    CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
                    124 * MHZ,
                    124 * MHZ);
    /// \end::configure_clk_sys[]

    // CLK USB = PLL USB (48MHz) / 1 = 48MHz
    clock_configure(clk_usb,
                    0, // No GLMUX
                    CLOCKS_CLK_USB_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
                    48 * MHZ,
                    48 * MHZ);

    // CLK ADC = PLL USB (48MHZ) / 1 = 48MHz
    clock_configure(clk_adc,
                    0, // No GLMUX
                    CLOCKS_CLK_ADC_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
                    48 * MHZ,
                    48 * MHZ);

    // CLK RTC = PLL USB (48MHz) / 1024 = 46875Hz
    clock_configure(clk_rtc,
                    0, // No GLMUX
                    CLOCKS_CLK_RTC_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
                    48 * MHZ,
                    46875);

    // CLK PERI = clk_sys. Used as reference clock for Peripherals. No dividers so just select and enable
    // Normally choose clk_sys or clk_usb
    clock_configure(clk_peri,
                    0,
                    CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
                    124 * MHZ,
                    124 * MHZ);
}

static void start_timer_irq(void)
{
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    uint slice_num = pwm_gpio_to_slice_num(PICO_DEFAULT_LED_PIN);
    pwm_clear_irq(slice_num);
    pwm_set_irq_enabled(slice_num, true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, timer_irq_handler);
    irq_set_enabled(PWM_IRQ_WRAP, true);

    pwm_config config = pwm_get_default_config();
    /* 124MHz/124/(9999+1)-> 100 Hz */
    pwm_config_set_clkdiv(&config, 124.f);
    pwm_config_set_wrap(&config, 9999);
    pwm_init(slice_num, &config, true);
}

static void stop_timer_irq(void)
{
    uint slice_num = pwm_gpio_to_slice_num(PICO_DEFAULT_LED_PIN);
    pwm_set_irq_enabled(slice_num, false);
}
