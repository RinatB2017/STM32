#include "cmsis_os.h"
#include "rl_usb.h"
#include "SSD1306.h"
#include "AD9106.h"
#include "ADT7516.h"
#include "Leds.h"
#include "stdio.h"

// переменная для Uptime 
extern uint32_t timer_100us;

// ---------------------------------------------------------------- Debug Output
struct __FILE { int handle; };
FILE __stdout;
FILE __stdin;
int fputc(int ch, FILE *f) 
{
	SSD1306_Putc(ch, &Font_7x10, SSD1306_COLOR_WHITE);
	return(ch);
}
// ---------------------------------------------------------------- 

// ---------------------------------------------------------------- LCD Update Thread
void Thread_LCD_Update(void const *argument)
{
	osDelay(100); // для корректной инициализации дисплея
	SSD1306_Init();
	
	SSD1306_GotoXY(8, 0);
	SSD1306_Puts("RDC Vector", &Font_11x18, SSD1306_COLOR_WHITE);
	SSD1306_DrawLine(0, 18, SSD1306_WIDTH, 18, SSD1306_COLOR_WHITE);
	
	SSD1306_GotoXY(0, 32);
	SSD1306_Puts("AB424T-Imitator", &Font_7x10, SSD1306_COLOR_WHITE);
	SSD1306_GotoXY(0, 42);
	SSD1306_Puts("BMCT.000000.000", &Font_7x10, SSD1306_COLOR_WHITE);
	SSD1306_GotoXY(0, 52);
	SSD1306_Puts("Firmware v1.12", &Font_7x10, SSD1306_COLOR_WHITE);
	
	SSD1306_UpdateScreen(); 
	
	osDelay(4000);
	
	SSD1306_Fill(SSD1306_COLOR_BLACK);
	SSD1306_UpdateScreen(); 
	
  for(;;) 
	{
		SSD1306_GotoXY(0, 0);
		printf("AVCC: +%2.2fV", (float)(0.00226 * 2 * (float)ADT7516_ReadADC(0)));
		osDelay(10);
		SSD1306_GotoXY(0, 10);
		printf("AVSS: -%2.2fV", (float)(0.00226 * 2 * (float)ADT7516_ReadADC(1)));
		osDelay(10);
		SSD1306_GotoXY(0, 20);
		printf("BIAS: +%2.2fV", (float)(0.00226 * 16 * (float)ADT7516_ReadADC(2)));
		osDelay(10);
		SSD1306_GotoXY(0, 30);
		printf("VCCIN: +%2.2fV", (float)(0.00226 * 8 * (float)ADT7516_ReadADC(3)));
		osDelay(10);
		SSD1306_GotoXY(0, 40);
		printf("Tempreture:+%2.2fC", ADT7516_ReadInternalTemp());
		osDelay(10);
		
		
		SSD1306_GotoXY(0, 50);
		printf("Uptime: %02u:%02u:%02u.%1u", timer_100us/36000, (timer_100us/600)%60, (timer_100us/10)%60, timer_100us%10);
		
		SSD1306_UpdateScreen(); 
		osDelay(100);
  }
}
osThreadId tid_Thread_LCD_Update;
osThreadDef(Thread_LCD_Update, osPriorityNormal, 1, 0);
// ---------------------------------------------------------------- 

// ---------------------------------------------------------------- USB Routine Thread
void Thread_USB_Update(void const *argument)
{
	AD9106_Init();
	ADT7516_Init();
		
	osDelay(500);	
	USBD_Initialize(0);              
  USBD_Connect(0);
	StatusLeds_Set(Led_Work_Red, Led_OFF);
	StatusLeds_Set(Led_Work_Green, Led_ON);
	
  for(;;) 
	{
		timer_100us++;
		osDelay(100);		
  }
}
osThreadId tid_Thread_USB_Update;
osThreadDef(Thread_USB_Update, osPriorityHigh, 1, 0);
// ---------------------------------------------------------------- 

// ---------------------------------------------------------------- Leds Update Thread
void Thread_LEDS_Update(void const *argument)
{
	StatusLeds_Init();
	StatusLeds_Set(Led_Work_Red, Led_ON);
	
	for(;;) 
	{
		osDelay(10);	
		StatusLeds_Routine();
	}
}
osThreadId tid_Thread_LEDS_Update;
osThreadDef(Thread_LEDS_Update, osPriorityHigh, 1, 0);
// ---------------------------------------------------------------- 

// ---------------------------------------------------------------- MAIN
int main(void)
{
	osKernelInitialize();
	tid_Thread_USB_Update = osThreadCreate(osThread(Thread_USB_Update), NULL);
	tid_Thread_LCD_Update = osThreadCreate(osThread(Thread_LCD_Update), NULL);
	tid_Thread_LEDS_Update = osThreadCreate(osThread(Thread_LEDS_Update), NULL);	
	osKernelStart();
}
// ---------------------------------------------------------------- 
