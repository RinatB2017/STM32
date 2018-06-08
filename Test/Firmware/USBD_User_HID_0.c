//! [code_USBD_User_HID]
#include "rl_usb.h"
#include "AD9106.h"
#include "ADT7516.h"

#include "RTE\USB\USBD_Config_HID_0.h"
#include <string.h>

#include "Leds.h"

uint8_t rep_num_done = 0;
uint8_t rep_buffer[USBD_HID0_FEAT_REPORT_MAX_SZ];

// переменная для Uptime 
uint32_t timer_100us;

// Called during USBD_Initialize to initialize the USB Device class.
void USBD_HID0_Initialize (void)
{
}

// Called during USBD_Uninitialize to de-initialize the USB Device class.
void USBD_HID0_Uninitialize (void)
{
}

// \brief Prepare HID Report data to send.
// \param[in]   rtype   report type:
//                - HID_REPORT_INPUT           = input report requested
//                - HID_REPORT_FEATURE         = feature report requested
// \param[in]   req     request type:
//                - USBD_HID_REQ_EP_CTRL       = control endpoint request
//                - USBD_HID_REQ_PERIOD_UPDATE = idle period expiration request
//                - USBD_HID_REQ_EP_INT        = previously sent report on interrupt endpoint request
// \param[in]   rid     report ID (0 if only one report exists).
// \param[out]  buf     buffer containing report data to send.
// \return              number of report data bytes prepared to send or invalid report requested.
//              - value >= 0: number of report data bytes prepared to send
//              - value = -1: invalid report requested
int32_t USBD_HID0_GetReport (uint8_t rtype, uint8_t req, uint8_t rid, uint8_t *buf)
{
	uint16_t rep_ADDR = 0, rep_DATA = 0;
	
	memcpy(buf, rep_buffer, USBD_HID0_FEAT_REPORT_MAX_SZ);
	
	switch (rtype)
	{
		case HID_REPORT_INPUT:
		/*switch (req)
			{
				case USBD_HID_REQ_EP_CTRL:
				case USBD_HID_REQ_PERIOD_UPDATE:
				break;

				case USBD_HID_REQ_EP_INT:
				break;
			}*/
		break;

		case HID_REPORT_FEATURE:
			if(rid == 0x00)
			{	
				//rep_buffer[1] = rep_num_done;
				
				if(buf[0] == 0x2a) // Общие настройки
				{	
					if(buf[1] == 0x20) // Uptime
					{	
						buf[2] = (uint8_t)(timer_100us & 0xff); 
						buf[3] = (uint8_t)(timer_100us >> 8); 
						buf[4] = (uint8_t)(timer_100us >> 16); 
						buf[5] = (uint8_t)(timer_100us >> 24); 
					}
					else if(buf[1] == 0x80) // BoardRevision
					{	
						buf[2] = 0; 
					}
					else if(buf[1] == 0x81) // SerialNumber
					{	
						buf[2] = 1; 
					}
				}
				else if(buf[0] == 0x8a) // AD9106, General Registers Read
				{
					for(rep_num_done = 0; rep_num_done < buf[1]; rep_num_done++)
					{
						// buf[] = команда | кол-во регистров в пакете | 2 байта адреса | 2 байта данных | (адрес и данные далее повторяются)...
						// Адрес вычисляем по циклу, к указателю i + 2 байта(команда\кол-во) * 4 (2 байта адреса + 2 байта данных для каждого региста)
						// вначале идет младшая часть, поэтому дополнительно + 0 и потом старшая, поэтому +1 к указателю i
						rep_ADDR = buf[(rep_num_done * 4) + 2] + (buf[(rep_num_done * 4) + 3] << 8);
						
						rep_DATA = AD9106_RegisterDataRead(rep_ADDR);
						
						// Данные вычисляем по циклу, к указателю i + 2 байта(команда\кол-во) * 4 (2 байта адреса + 2 байта данных для каждого региста)
						// вначале идет младшая часть, поэтому дополнительно + 2 и потом старшая, поэтому +3 к указателю i
						buf[(rep_num_done * 4) + 4] = (uint8_t)(rep_DATA & 0xff);
						buf[(rep_num_done * 4) + 5] = (uint8_t)(rep_DATA >> 8);
					}
				}
				else if(buf[0] == 0x8b) // AD9106, SRAM Read
				{
					AD9106_WriteReg(PAT_STATUS, 0x0C);
					AD9106_WriteReg(RAMUPDATE, 0x01);
					
					__disable_irq();
					for(rep_num_done = 0; rep_num_done < rep_buffer[1]; rep_num_done++)
					{
						rep_ADDR = rep_buffer[(rep_num_done * 4) + 2] + (rep_buffer[(rep_num_done * 4) + 3] << 8);
						
						if(rep_ADDR >=0x6000) 
						{
							rep_DATA = AD9106_ReadReg(rep_ADDR);
							if(rep_DATA == 0xffff) rep_DATA = AD9106_ReadReg(rep_ADDR);
						}
						else rep_DATA = 0;
						
						//rep_DATA = (uint16_t)(rep_DATA >> 3); // !!!! КОСТЫЛЬ
						//if(rep_DATA > 0xfff) rep_DATA = (uint16_t)(-(rep_DATA));
						buf[(rep_num_done * 4) + 4] = (uint8_t)(rep_DATA & 0xff);
						buf[(rep_num_done * 4) + 5] = (uint8_t)(rep_DATA >> 8);
					}
					__enable_irq();
					
					//AD9106_WriteReg(PAT_STATUS, 0x10);
					//AD9106_WriteReg(RAMUPDATE, 0x01);
				}
				else if(buf[0] == 0x8c) // AD9106, Update, Config Read
				{

				}
				else if(buf[0] == 0x8f) // AD9106, Register Read
				{
					if(buf[1] == 1)
					{
						rep_ADDR = rep_buffer[2] + (rep_buffer[3] << 8);
						
						rep_DATA = AD9106_RegisterDataRead(rep_ADDR);
						
						buf[4] = (uint8_t)(rep_DATA & 0xff);
						buf[5] = (uint8_t)(rep_DATA >> 8);
					}
				}
				else if(buf[0] == 0x9a) // ADT7519
				{	
					if(buf[1] == 0x10) // temp
					{
						rep_DATA = (uint16_t)(ADT7516_ReadInternalTemp() * 100);
						
						buf[2] = (uint8_t)(rep_DATA & 0xff);
						buf[3] = (uint8_t)(rep_DATA >> 8);
					}
					else if(buf[1] == 0x20) // adc
					{
						rep_DATA = ADT7516_ReadADC(buf[2]);
						
						buf[3] = (uint8_t)(rep_DATA & 0xff);
						buf[4] = (uint8_t)(rep_DATA >> 8);
					}
				}
			}
			
		
			return (128);
	}
	return (0);
}
 
 
// \brief Process received HID Report data.
// \param[in]   rtype   report type:
//                - HID_REPORT_OUTPUT    = output report received
//                - HID_REPORT_FEATURE   = feature report received
// \param[in]   req     request type:
//                - USBD_HID_REQ_EP_CTRL = report received on control endpoint
//                - USBD_HID_REQ_EP_INT  = report received on interrupt endpoint
// \param[in]   rid     report ID (0 if only one report exists).
// \param[in]   buf     buffer that receives report data.
// \param[in]   len     length of received report data.
// \return      true    received report data processed.
// \return      false   received report data not processed or request not supported.
bool USBD_HID0_SetReport(uint8_t rtype, uint8_t req, uint8_t rid, const uint8_t *buf, int32_t len) 
{
	uint16_t rep_ADDR = 0, rep_DATA = 0, rep_DATA1 = 0, rep_DATA2 = 0;
	
	memcpy(rep_buffer, buf, USBD_HID0_FEAT_REPORT_MAX_SZ);
	
	switch (rtype) 
	{
		case HID_REPORT_OUTPUT:

		break;
		case HID_REPORT_FEATURE:
			if(rid == 0x00)
			{
				StatusLeds_Blink(Led_Link_Green);
				
				if(rep_buffer[0] == 0x20) // Общие настройки
				{	

				}
				else if(rep_buffer[0] == 0x80) // AD9106, Registers Write
				{					
					for(rep_num_done = 0; rep_num_done < rep_buffer[1]; rep_num_done++)
					{
						// buf[] = команда | кол-во регистров в пакете | 2 байта адреса | 2 байта данных | (адрес и данные далее повторяются)...
						// Адрес вычисляем по циклу, к указателю i + 2 байта(команда\кол-во) * 4 (2 байта адреса + 2 байта данных для каждого региста)
						// вначале идет младшая часть, поэтому дополнительно + 0 и потом старшая, поэтому +1 к указателю i
						rep_ADDR = rep_buffer[(rep_num_done * 4) + 2] + (rep_buffer[(rep_num_done * 4) + 3] << 8);
						
						// Данные вычисляем по циклу, к указателю i + 2 байта(команда\кол-во) * 4 (2 байта адреса + 2 байта данных для каждого региста)
						// вначале идет младшая часть, поэтому дополнительно + 2 и потом старшая, поэтому +3 к указателю i
						rep_DATA = rep_buffer[(rep_num_done * 4) + 4] + (rep_buffer[(rep_num_done * 4) + 5] << 8);
						
						AD9106_RegisterDataWrite(rep_ADDR, rep_DATA);
					}
				}
				else if(rep_buffer[0] == 0x81) // AD9106, SRAM Write
				{
					AD9106_WriteReg(PAT_STATUS, 0x04);
					AD9106_WriteReg(RAMUPDATE, 0x01);
					
					for(rep_num_done = 0; rep_num_done < rep_buffer[1]; rep_num_done++)
					{
						rep_ADDR = rep_buffer[(rep_num_done * 4) + 2] + (rep_buffer[(rep_num_done * 4) + 3] << 8);
						rep_DATA = rep_buffer[(rep_num_done * 4) + 4] + (rep_buffer[(rep_num_done * 4) + 5] << 8);
						
						if(rep_ADDR >=0x6000) 
						{
							//AD9106_WriteReg(rep_ADDR, (uint16_t)(rep_DATA << 3)); // !!!! КОСТЫЛЬ
							AD9106_WriteReg(rep_ADDR, rep_DATA);
						}
					}
					//AD9106_WriteReg(PAT_STATUS, 0x10);
					//AD9106_WriteReg(RAMUPDATE, 0x01);
				}
				else if(rep_buffer[0] == 0x82) // AD9106, Update, Config Write
				{
					if(rep_buffer[1] == 0x20) // Play pattern from SRAM
					{
						//rep_buffer[2] // Channel
						rep_DATA1 = rep_buffer[3] + (rep_buffer[4] << 8); // Start ADDR
						rep_DATA2 = rep_buffer[5] + (rep_buffer[6] << 8); // Stop ADDR
						
						//AD9106_PlayPatternFromSRAM(0, 0, 1024, 10, 10);
					}
					if(rep_buffer[1] == 0x30) // усиление
					{
						StatusLeds_Blink(Led_Link_Red);
					}
					if(rep_buffer[1] == 0x31) // период паттерна
					{
						StatusLeds_Blink(Led_Link_Red);
					}
					if(rep_buffer[1] == 0x32) // фазовый сдвиг
					{
						StatusLeds_Blink(Led_Link_Red);
					}
					if(rep_buffer[1] == 0x33) // цифровой сдвиг
					{
						StatusLeds_Blink(Led_Link_Red);
					}
					if(rep_buffer[1] == 0x34) // 
					{
						StatusLeds_Blink(Led_Link_Red);
					}
					else if(rep_buffer[1] == 0x80) // Manual Reset AD9106
					{
						StatusLeds_Blink(Led_Link_Red);
						AD9106_ManualReset();
					}
					else if(rep_buffer[1] == 0x90) // Update Settings					
					{
						AD9106_UpdateSPIParams();
					}
					else if(rep_buffer[1] == 0x92) // Stop All			
					{
						AD9106_StopGeneration();
					}
				}
				else if(rep_buffer[0] == 0x85) // AD9106, Register Write
				{
					if(rep_buffer[1] == 0x01)
					{
						rep_ADDR = rep_buffer[2] + (rep_buffer[3] << 8);
						rep_DATA = rep_buffer[4] + (rep_buffer[5] << 8);	
						AD9106_RegisterDataWrite(rep_ADDR, rep_DATA);
					}
				}
				else if(rep_buffer[0] == 0x90) // ADT7519 Write
				{	
					if(rep_buffer[1] == 0x30) // dac
					{
						rep_DATA = rep_buffer[3] + (rep_buffer[4] << 8);	
						
						ADT7516_WriteDAC(buf[2], rep_DATA);
					}
					else if(rep_buffer[1] == 0x80) // reset
					{
						ADT7516_ManualReset();
					}
				}
			}
		break;
	}
	return true;
}

//! [code_USBD_User_HID]
