//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
#include <stm32f10x_flash.h>
#include "flash.h"
//--------------------------------------------------------------------------------
#define MY_FLASH_PAGE_ADDR 0x800FC00
#define SETTINGS_WORDS sizeof(settings)/4

//--------------------------------------------------------------------------------
extern uint32_t    addr_cam_32;                // адрес камеры
extern uint16_t    time_interval_16;           // интервал дворника
extern uint32_t    time_washout_32;            // время помывки
extern uint32_t    time_pause_washout_32;      // время между помывками
extern uint32_t    preset_washout_32;          // пресет помывки
extern uint32_t    time_preset_washout_32;     // времен помывки
//--------------------------------------------------------------------------------
void FLASH_Init(void)
{
    /* Next commands may be used in SysClock initialization function
       In this case using of FLASH_Init is not obligatorily */
    /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd( FLASH_PrefetchBuffer_Enable);
    /* Flash 2 wait state */
    FLASH_SetLatency( FLASH_Latency_2);
}
//--------------------------------------------------------------------------------
void FLASH_ReadSettings(void)
{
    //Read settings
    uint32_t *source_addr = (uint32_t *)MY_FLASH_PAGE_ADDR;
    uint32_t *dest_addr = (void *)&settings;
    for (uint16_t i=0; i<SETTINGS_WORDS; i++) {
        *dest_addr = *(__IO uint32_t*)source_addr;
        source_addr++;
        dest_addr++;
    }
}
//--------------------------------------------------------------------------------
void FLASH_WriteSettings(void)
{
    FLASH_Unlock();
    FLASH_ErasePage(MY_FLASH_PAGE_ADDR);

    // Write settings
    uint32_t *source_addr = (void *)&settings;
    uint32_t *dest_addr = (uint32_t *) MY_FLASH_PAGE_ADDR;
    for (uint16_t i=0; i<SETTINGS_WORDS; i++)
    {
        FLASH_ProgramWord((uint32_t)dest_addr, *source_addr);
        source_addr++;
        dest_addr++;
    }

    FLASH_Lock();
}
//--------------------------------------------------------------------------------
void write_FLASH(void)
{
    FLASH_Init();
    FLASH_ReadSettings();

    settings.time_interval_16 = time_interval_16;
    settings.time_washout_32 = time_washout_32;
    settings.time_pause_washout_32 = time_pause_washout_32;
    settings.preset_washout_32 = preset_washout_32;
    settings.time_preset_washout_32 = time_preset_washout_32;

    FLASH_WriteSettings();
}
//--------------------------------------------------------------------------------
void read_FLASH(void)
{
    FLASH_Init();
    FLASH_ReadSettings();

    time_interval_16 = settings.time_interval_16;
    time_washout_32 = settings.time_washout_32;
    time_pause_washout_32 = settings.time_pause_washout_32;
    preset_washout_32 = settings.preset_washout_32;
    time_preset_washout_32 = settings.time_preset_washout_32;
}
//--------------------------------------------------------------------------------
