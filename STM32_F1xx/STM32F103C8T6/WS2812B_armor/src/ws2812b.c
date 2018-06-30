// The MIT License (MIT)
//
// Copyright (c) 2015 Aleksandr Aleshin <silencer@quadrius.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <stdint.h>
#include <string.h>

#include "bitmap.h"

#include "ws2812b.h"
#include "ws2812b_conf.h"

//------------------------------------------------------------
// Internal
//------------------------------------------------------------

#define MIN(a, b)   ({ typeof(a) a1 = a; typeof(b) b1 = b; a1 < b1 ? a1 : b1; })

#if defined(__ICCARM__)
__packed struct PWM
#else
struct __attribute__((packed)) PWM
#endif
{
    uint16_t g[8], r[8], b[8];
};

typedef struct PWM PWM_t;
typedef void (SrcFilter_t)(void **, PWM_t **, unsigned *, unsigned);

#ifdef WS2812B_USE_GAMMA_CORRECTION
#ifdef WS2812B_USE_PRECALCULATED_GAMMA_TABLE
static const uint8_t LEDGammaTable[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10,
    10, 11, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21,
    22, 23, 23, 24, 24, 25, 26, 26, 27, 28, 28, 29, 30, 30, 31, 32, 32, 33, 34, 35, 35, 36, 37, 38,
    38, 39, 40, 41, 42, 42, 43, 44, 45, 46, 47, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 56, 57, 58,
    59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 84,
    85, 86, 87, 88, 89, 91, 92, 93, 94, 95, 97, 98, 99, 100, 102, 103, 104, 105, 107, 108, 109, 111,
    112, 113, 115, 116, 117, 119, 120, 121, 123, 124, 126, 127, 128, 130, 131, 133, 134, 136, 137,
    139, 140, 142, 143, 145, 146, 148, 149, 151, 152, 154, 155, 157, 158, 160, 162, 163, 165, 166,
    168, 170, 171, 173, 175, 176, 178, 180, 181, 183, 185, 186, 188, 190, 192, 193, 195, 197, 199,
    200, 202, 204, 206, 207, 209, 211, 213, 215, 217, 218, 220, 222, 224, 226, 228, 230, 232, 233,
    235, 237, 239, 241, 243, 245, 247, 249, 251, 253, 255 };
#endif
#endif

static inline uint8_t LEDGamma(uint8_t v)
{
#ifdef WS2812B_USE_GAMMA_CORRECTION
#ifdef WS2812B_USE_PRECALCULATED_GAMMA_TABLE
    return LEDGammaTable[v];
#else
    return (v * v + v) >> 8;
#endif
#else
    return v;
#endif
}

static volatile int DMABusy_1;
static volatile int DMABusy_2;

static PWM_t DMABuffer_1[WS2812B_BUFFER_SIZE];
static PWM_t DMABuffer_2[WS2812B_BUFFER_SIZE];

static SrcFilter_t *DMAFilter_1;
static SrcFilter_t *DMAFilter_2;

static void *DMASrc_1;
static void *DMASrc_2;

static unsigned DMACount_1;
static unsigned DMACount_2;

static void SrcFilterNull(void **src, PWM_t **pwm, unsigned *count, unsigned size)
{
    memset(*pwm, 0, size * sizeof(PWM_t));
    *pwm += size;
}

static void RGB2PWM(RGB_t *rgb, PWM_t *pwm)
{
    uint8_t r = LEDGamma(rgb->r);
    uint8_t g = LEDGamma(rgb->g);
    uint8_t b = LEDGamma(rgb->b);

    uint8_t mask = 128;

    int i;
    for (i = 0; i < 8; i++)
    {
        pwm->r[i] = r & mask ? WS2812B_PULSE_HIGH : WS2812B_PULSE_LOW;
        pwm->g[i] = g & mask ? WS2812B_PULSE_HIGH : WS2812B_PULSE_LOW;
        pwm->b[i] = b & mask ? WS2812B_PULSE_HIGH : WS2812B_PULSE_LOW;

        mask >>= 1;
    }
}

static void SrcFilterRGB(void **src, PWM_t **pwm, unsigned *count, unsigned size)
{
    RGB_t *rgb = *src;
    PWM_t *p = *pwm;

    *count -= size;

    while (size--)
    {
        RGB2PWM(rgb++, p++);
    }

    *src = rgb;
    *pwm = p;
}

static void SrcFilterHSV(void **src, PWM_t **pwm, unsigned *count, unsigned size)
{
    HSV_t *hsv = *src;
    PWM_t *p = *pwm;

    *count -= size;

    while (size--)
    {
        RGB_t rgb;

        HSV2RGB(hsv++, &rgb);
        RGB2PWM(&rgb, p++);
    }

    *src = hsv;
    *pwm = p;
}

static void DMASend_1(SrcFilter_t *filter, void *src, unsigned count)
{
    if (!DMABusy_1)
    {
        DMABusy_1 = 1;

        DMAFilter_1 = filter;
        DMASrc_1 = src;
        DMACount_1 = count;

        PWM_t *pwm = DMABuffer_1;
        PWM_t *end = &DMABuffer_1[WS2812B_BUFFER_SIZE];

        // Start sequence
        SrcFilterNull(NULL, &pwm, NULL, WS2812B_START_SIZE);

        // RGB PWM data
        DMAFilter_1(&DMASrc_1, &pwm, &DMACount_1, MIN(DMACount_1, end - pwm));

        // Rest of buffer
        if (pwm < end)
            SrcFilterNull(NULL, &pwm, NULL, end - pwm);

        // Start transfer
        DMA_SetCurrDataCounter(WS2812B_DMA_CHANNEL_1, sizeof(DMABuffer_1) / sizeof(uint16_t));

        TIM_Cmd(WS2812B_TIM, ENABLE);
        DMA_Cmd(WS2812B_DMA_CHANNEL_1, ENABLE);
    }
}

static void DMASend_2(SrcFilter_t *filter, void *src, unsigned count)
{
    if (!DMABusy_2)
    {
        DMABusy_2 = 1;

        DMAFilter_2 = filter;
        DMASrc_2 = src;
        DMACount_2 = count;

        PWM_t *pwm = DMABuffer_2;
        PWM_t *end = &DMABuffer_2[WS2812B_BUFFER_SIZE];

        // Start sequence
        SrcFilterNull(NULL, &pwm, NULL, WS2812B_START_SIZE);

        // RGB PWM data
        DMAFilter_2(&DMASrc_2, &pwm, &DMACount_2, MIN(DMACount_2, end - pwm));

        // Rest of buffer
        if (pwm < end)
            SrcFilterNull(NULL, &pwm, NULL, end - pwm);

        // Start transfer
        DMA_SetCurrDataCounter(WS2812B_DMA_CHANNEL_2, sizeof(DMABuffer_2) / sizeof(uint16_t));

        TIM_Cmd(WS2812B_TIM, ENABLE);
        DMA_Cmd(WS2812B_DMA_CHANNEL_2, ENABLE);
    }
}

static void DMASendNext_1(PWM_t *pwm, PWM_t *end)
{
    if (!DMAFilter_1)
    {
        // Stop transfer
        TIM_Cmd(WS2812B_TIM, DISABLE);
        DMA_Cmd(WS2812B_DMA_CHANNEL_1, DISABLE);

        DMABusy_1 = 0;
    }
    else if (!DMACount_1)
    {
        // Rest of buffer
        SrcFilterNull(NULL, &pwm, NULL, end - pwm);

        DMAFilter_1 = NULL;
    }
    else
    {
        // RGB PWM data
        DMAFilter_1(&DMASrc_1, &pwm, &DMACount_1, MIN(DMACount_1, end - pwm));

        // Rest of buffer
        if (pwm < end)
            SrcFilterNull(NULL, &pwm, NULL, end - pwm);
    }
}

static void DMASendNext_2(PWM_t *pwm, PWM_t *end)
{
    if (!DMAFilter_2)
    {
        // Stop transfer
        TIM_Cmd(WS2812B_TIM, DISABLE);
        DMA_Cmd(WS2812B_DMA_CHANNEL_2, DISABLE);

        DMABusy_2 = 0;
    }
    else if (!DMACount_2)
    {
        // Rest of buffer
        SrcFilterNull(NULL, &pwm, NULL, end - pwm);

        DMAFilter_2 = NULL;
    }
    else
    {
        // RGB PWM data
        DMAFilter_2(&DMASrc_2, &pwm, &DMACount_2, MIN(DMACount_2, end - pwm));

        // Rest of buffer
        if (pwm < end)
            SrcFilterNull(NULL, &pwm, NULL, end - pwm);
    }
}

void WS2812B_DMA_HANDLER_1(void)
{
    if (DMA_GetITStatus(WS2812B_DMA_IT_HT_1) != RESET)
    {
        DMA_ClearITPendingBit(WS2812B_DMA_IT_HT_1);
        DMASendNext_1(DMABuffer_1, &DMABuffer_1[WS2812B_BUFFER_SIZE / 2]);
    }

    if (DMA_GetITStatus(WS2812B_DMA_IT_TC_1) != RESET)
    {
        DMA_ClearITPendingBit(WS2812B_DMA_IT_TC_1);
        DMASendNext_1(&DMABuffer_1[WS2812B_BUFFER_SIZE / 2], &DMABuffer_1[WS2812B_BUFFER_SIZE]);
    }
}

void WS2812B_DMA_HANDLER_2(void)
{
    if (DMA_GetITStatus(WS2812B_DMA_IT_HT_2) != RESET)
    {
        DMA_ClearITPendingBit(WS2812B_DMA_IT_HT_2);
        DMASendNext_2(DMABuffer_2, &DMABuffer_2[WS2812B_BUFFER_SIZE / 2]);
    }

    if (DMA_GetITStatus(WS2812B_DMA_IT_TC_2) != RESET)
    {
        DMA_ClearITPendingBit(WS2812B_DMA_IT_TC_2);
        DMASendNext_2(&DMABuffer_2[WS2812B_BUFFER_SIZE / 2], &DMABuffer_2[WS2812B_BUFFER_SIZE]);
    }
}

//------------------------------------------------------------
// Interface
//------------------------------------------------------------
void ws2812b_Init_1(void)
{
    // Turn on peripheral clock
    RCC_APB1PeriphClockCmd(WS2812B_APB1_RCC, ENABLE);
    RCC_APB2PeriphClockCmd(WS2812B_APB2_RCC, ENABLE);

    RCC_AHBPeriphClockCmd(WS2812B_AHB_RCC_1, ENABLE);

    // Initialize GPIO pin
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.GPIO_Pin = WS2812B_GPIO_PIN_1;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;

    GPIO_Init(WS2812B_GPIO, &GPIO_InitStruct);

    // Initialize timer clock
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

    TIM_TimeBaseInitStruct.TIM_Prescaler = (SystemCoreClock / WS2812B_FREQUENCY) - 1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period = WS2812B_PERIOD - 1;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;

    TIM_TimeBaseInit(WS2812B_TIM, &TIM_TimeBaseInitStruct);

    // Initialize timer PWM
    TIM_OCInitTypeDef TIM_OCInitStruct;

    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_Pulse = 0;
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;

    WS2812B_TIM_OCINIT(WS2812B_TIM, &TIM_OCInitStruct);
    WS2812B_TIM_OCPRELOAD(WS2812B_TIM, TIM_OCPreload_Enable);

    // Initialize DMA channel
    DMA_InitTypeDef DMA_InitStruct;

    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t) & WS2812B_TIM_DMA_CCR_1;
    DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t) DMABuffer_1;
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStruct.DMA_BufferSize = sizeof(DMABuffer_1) / sizeof(uint16_t);
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStruct.DMA_Priority = DMA_Priority_High;
    DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;

    DMA_Init(WS2812B_DMA_CHANNEL_1, &DMA_InitStruct);

    // Turn on timer DMA requests
    TIM_DMACmd(WS2812B_TIM, WS2812B_TIM_DMA_CC_1, ENABLE);

    // Initialize DMA interrupt
    NVIC_InitTypeDef NVIC_InitStruct;

    NVIC_InitStruct.NVIC_IRQChannel = WS2812B_DMA_IRQ_1;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = WS2812B_IRQ_PRIO;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = WS2812B_IRQ_SUBPRIO;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStruct);

    // Enable DMA interrupt
    DMA_ITConfig(WS2812B_DMA_CHANNEL_1, DMA_IT_HT | DMA_IT_TC, ENABLE);
}
//------------------------------------------------------------
void ws2812b_Init_2(void)
{
    // Turn on peripheral clock
    RCC_APB1PeriphClockCmd(WS2812B_APB1_RCC, ENABLE);
    RCC_APB2PeriphClockCmd(WS2812B_APB2_RCC, ENABLE);

    RCC_AHBPeriphClockCmd(WS2812B_AHB_RCC_2, ENABLE);

    // Initialize GPIO pin
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.GPIO_Pin = WS2812B_GPIO_PIN_2;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;

    GPIO_Init(WS2812B_GPIO, &GPIO_InitStruct);

    // Initialize timer clock
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

    TIM_TimeBaseInitStruct.TIM_Prescaler = (SystemCoreClock / WS2812B_FREQUENCY) - 1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period = WS2812B_PERIOD - 1;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;

    TIM_TimeBaseInit(WS2812B_TIM, &TIM_TimeBaseInitStruct);

    // Initialize timer PWM
    TIM_OCInitTypeDef TIM_OCInitStruct;

    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_Pulse = 0;
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;

    WS2812B_TIM_OCINIT(WS2812B_TIM, &TIM_OCInitStruct);
    WS2812B_TIM_OCPRELOAD(WS2812B_TIM, TIM_OCPreload_Enable);

    // Initialize DMA channel
    DMA_InitTypeDef DMA_InitStruct;

    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t) & WS2812B_TIM_DMA_CCR_2;
    DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t) DMABuffer_2;
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStruct.DMA_BufferSize = sizeof(DMABuffer_2) / sizeof(uint16_t);
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStruct.DMA_Priority = DMA_Priority_High;
    DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;

    DMA_Init(WS2812B_DMA_CHANNEL_2, &DMA_InitStruct);

    // Turn on timer DMA requests
    TIM_DMACmd(WS2812B_TIM, WS2812B_TIM_DMA_CC_2, ENABLE);

    // Initialize DMA interrupt
    NVIC_InitTypeDef NVIC_InitStruct;

    NVIC_InitStruct.NVIC_IRQChannel = WS2812B_DMA_IRQ_2;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = WS2812B_IRQ_PRIO;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = WS2812B_IRQ_SUBPRIO;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStruct);

    // Enable DMA interrupt
    DMA_ITConfig(WS2812B_DMA_CHANNEL_2, DMA_IT_HT | DMA_IT_TC, ENABLE);
}
//------------------------------------------------------------
void ws2812b_Init_4(void)
{
    // Turn on peripheral clock
    RCC_APB1PeriphClockCmd(WS2812B_APB1_RCC, ENABLE);
    RCC_APB2PeriphClockCmd(WS2812B_APB2_RCC, ENABLE);

    RCC_AHBPeriphClockCmd(WS2812B_AHB_RCC_1, ENABLE);

    // Initialize GPIO pin
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.GPIO_Pin = WS2812B_GPIO_PIN_2;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;

    GPIO_Init(WS2812B_GPIO, &GPIO_InitStruct);

    // Initialize timer clock
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

    TIM_TimeBaseInitStruct.TIM_Prescaler = (SystemCoreClock / WS2812B_FREQUENCY) - 1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period = WS2812B_PERIOD - 1;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;

    TIM_TimeBaseInit(WS2812B_TIM, &TIM_TimeBaseInitStruct);

    // Initialize timer PWM
    TIM_OCInitTypeDef TIM_OCInitStruct;

    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_Pulse = 0;
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;

    WS2812B_TIM_OCINIT(WS2812B_TIM, &TIM_OCInitStruct);
    WS2812B_TIM_OCPRELOAD(WS2812B_TIM, TIM_OCPreload_Enable);

    // Initialize DMA channel
    DMA_InitTypeDef DMA_InitStruct;

    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t) & WS2812B_TIM_DMA_CCR_1;
    DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t) DMABuffer_1;
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStruct.DMA_BufferSize = sizeof(DMABuffer_1) / sizeof(uint16_t);
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStruct.DMA_Priority = DMA_Priority_High;
    DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;

    DMA_Init(DMA1_Channel4, &DMA_InitStruct);

    // Turn on timer DMA requests
    TIM_DMACmd(WS2812B_TIM, WS2812B_TIM_DMA_CC_1, ENABLE);

    // Initialize DMA interrupt
    NVIC_InitTypeDef NVIC_InitStruct;

    NVIC_InitStruct.NVIC_IRQChannel = WS2812B_DMA_IRQ_1;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = WS2812B_IRQ_PRIO;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = WS2812B_IRQ_SUBPRIO;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStruct);

    // Enable DMA interrupt
    DMA_ITConfig(WS2812B_DMA_CHANNEL_1, DMA_IT_HT | DMA_IT_TC, ENABLE);
}
//------------------------------------------------------------
inline int ws2812b_IsReady_1(void)
{
    return !DMABusy_1;
}
//------------------------------------------------------------
inline int ws2812b_IsReady_2(void)
{
	return !DMABusy_2;
}
//------------------------------------------------------------
void ws2812b_SendRGB_1(RGB_t *rgb, unsigned count)
{
    DMASend_1(&SrcFilterRGB, rgb, count);
}
//------------------------------------------------------------
void ws2812b_SendRGB_2(RGB_t *rgb, unsigned count)
{
    DMASend_2(&SrcFilterRGB, rgb, count);
}
//------------------------------------------------------------
void ws2812b_SendHSV_1(HSV_t *hsv, unsigned count)
{
    DMASend_1(&SrcFilterHSV, hsv, count);
}
//------------------------------------------------------------
void ws2812b_SendHSV_2(HSV_t *hsv, unsigned count)
{
    DMASend_2(&SrcFilterHSV, hsv, count);
}
//------------------------------------------------------------
