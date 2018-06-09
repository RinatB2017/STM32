/*******************************************************************************
    main.c
    STM32F4-Discovery_USB_HID
 *******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usbd_hid_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "core_cm4.h"

USB_OTG_CORE_HANDLE  USB_OTG_dev;

//#########################################################################
// Подпрограмма задержки
void DelayQ(uint32_t nTime)
{
	while ( --nTime)  {   }
}
//#########################################################################
//Заполнение буфера параметров для последующей передачи в ПК
void  GetBufPar()
{
	int  p1=380;
	int  p2=241;
	int  p3=24;
	int  p4=50;
	int  p5=53500;

	Par[0]=4;     //ID
	//Mode
	Par[1]=ModeW; // 1-режим рабочий (2-наладочный)
	Par[2]=0;     // Резерв
	//Alarm
	Par[3]=0x12;  //  Alarm-нет  Q1-нет
	Par[4]=0x31;  //Id-нет Qсн-нет
	Par[5]=0x23;  //Rиз-норма (нет параметра)
	Par[6]=0;		//Резерв

	//Coefficients
	//Float (cf1,cf2,cf3) to byte (Par[7-18])
	Par[7]=   (((uint8_t*)&cf1)[0]);   // - первый байт
	Par[8]=   (((uint8_t*)&cf1)[1]);   // - второй байт
	Par[9]=   (((uint8_t*)&cf1)[2]);   // - третий байт
	Par[10]=  (((uint8_t*)&cf1)[3]);   //-  четвёртый байт
	Par[11]=  (((uint8_t*)&cf2)[0]);
	Par[12]=  (((uint8_t*)&cf2)[1]);
	Par[13]=  (((uint8_t*)&cf2)[2]);
	Par[14]=  (((uint8_t*)&cf2)[3]);
	Par[15]=  (((uint8_t*)&cf3)[0]);   // - первый байт
	Par[16]=  (((uint8_t*)&cf3)[1]);   // - второй байт
	Par[17]=  (((uint8_t*)&cf3)[2]);   // - третий байт
	Par[18]=  (((uint8_t*)&cf3)[3]);   //-  четвёртый байт

	Par[19]=0;Par[20]=0;Par[21]=0;Par[22]=0;Par[23]=0;Par[24]=0; //Резерв
	Par[25]=0;Par[26]=0;Par[27]=0;Par[28]=0;Par[29]=0;Par[30]=0; //Резерв

	//View
	Par[31]=(p1>>8); Par[32]=p1; Par[33]=(p2>>8); Par[34]=p2;
	Par[35]=(p3>>8); Par[36]=p3; Par[37]=(p4>>8); Par[38]=p4;
	Par[39]=(p5>>8); Par[40]=p5;
	Par[41]=(CouPacPar>>8);Par[42]=CouPacPar;
	Par[43]=0;Par[44]=0;Par[45]=0;Par[46]=0; //Резерв
	Par[47]=0;Par[48]=0;Par[49]=0;Par[50]=0;                     //Резерв
	//Settings
	Par[51]=(s1>>8); Par[52]=s1; Par[53]=(s2>>8); Par[54]=s2;
	Par[55]=(s3>>8); Par[56]=s3;
	Par[57]=0; Par[58]=0; 										//Резерв

	//Signaling
	Par[59]=0x12;           //Q1-вкл  Q2-вкл
	Par[60]=0x34; 			//K1-вкл  K2-откл
	Par[61]=0x51;			// Rиз-НижеНорма Синхронизация-норма
	Par[62]=0; 				//Резерв
	Par[63]=0; 				//Резерв
}
//####################################################################################
//Передача одной графической переменной
void  Tx1var()
{
	STM32F4_Discovery_LEDOn(LED6);
	//		 Nullcycle=0;

	if (nt<10) nt++;
	if(nt<9)
	{
		if ((nt==1)||(nt==3)||(nt==5)||(nt==7))
		{
			int i=1;
			while (i<65)
			{
				InBuffer[i]=i;
				i++;
			}
			InBuffer[0]=3;
			USBD_HID_SendReport (&USB_OTG_dev, InBuffer, 64);
		}

		if ((nt==2)||(nt==4)||(nt==6)||(nt==8))
		{
			int i=1;
			while (i<64)
			{
				InBuffer[i]=64-i;
				i++;
			}
			InBuffer[0]=3;
			USBD_HID_SendReport (&USB_OTG_dev, InBuffer, 64);
		}
	}
}
//####################################################################################
//Передача шести графических переменных
void  Tx6var()
{
	int  ii,s;
	//*******  Заполнили буфер гр данными и передали в ПК  *********
	InBuffer[0]=3;                  //ID=3- графические данные
	s=0;
	for (ii = 0; ii < 5; ++ii)         //Цикл для 5-и точек
	{
		InBuffer[2+s] =(bTX1[ii]>>8); InBuffer[3+s]= bTX1[ii];
		InBuffer[12+s]=(bTX2[ii]>>8); InBuffer[13+s]=bTX2[ii];
		InBuffer[22+s]=(bTX3[ii]>>8); InBuffer[23+s]=bTX3[ii];
		InBuffer[32+s]=(bTX4[ii]>>8); InBuffer[33+s]=bTX4[ii];
		InBuffer[42+s]=(bTX5[ii]>>8); InBuffer[43+s]=bTX5[ii];
		InBuffer[52+s]=(bTX6[ii]>>8); InBuffer[53+s]=bTX6[ii];
		s+=2;
	}
	USBD_HID_SendReport (&USB_OTG_dev, InBuffer, 64);
}

//####################################################################################
void  GetBufDT()
{
	{
		if (CounterInt >=4090) {CounterInt =0;}
		CFBuf++;									//  Counter of Filling buffer счетчик Заполнения буфера данными
		//  (1...4)каждые 4мс заносится 5 слов 5*4=20
		if (CFBuf==1)
		{
			DT[0]=CounterInt; DT[1]=CounterInt+10; DT[2]=CounterInt+20; DT[3]=CounterInt+30;
			DT[4]=CounterInt+40;  CounterInt+=50;
		}
		if (CFBuf==2)
		{
			DT[5]=CounterInt; DT[6]=CounterInt+10; DT[7]=CounterInt+20; DT[8]=CounterInt+30;
			DT[9]=CounterInt+40; CounterInt+=50;
		}
		if (CFBuf==3)
		{
			DT[10]=CounterInt; DT[11]=CounterInt+10; DT[12]=CounterInt+20; DT[13]=CounterInt+30;
			DT[14]=CounterInt+40; CounterInt+=50;
		}
		if (CFBuf==4)
		{
			DT[15]=CounterInt; DT[16]=CounterInt+10; DT[17]=CounterInt+20; DT[18]=CounterInt+30;
			DT[19]=CounterInt+40; CounterInt+=50; CFBuf=0;
		}
	}
}
//#############################################################################################
//Заполнение 6 буферов bTX1-6[] переменными по заданным с ПК указателямт на имя пер-ой и очередность
//В примере  данные Ua [20]..Ic[20]-константы (с периодом 4 пакета), DT[20]-счетчик(0..4080)с инкрементом 10 каждую 1мс
//Реально 5 очередных точек(графиков) 6-и аналоговых входов Ua-Ic должны быть оцифрованы АЦП и помещены в  соотв. массивы
void GetBuf6Var()
{
	uint16_t n;
	CounterPac++;
	InBuffer[1]=CounterPac;         // Номер пакета-не используется в ПК(только индикация)

	if (CounterPac==1) {p1=pp1; p2=pp2; p3=pp3; p4=pp4; p5=pp5; p6=pp6;}
	if (CounterPac==4) CounterPac=0;

	//Читаем заданные переменные и помещаем в буферы передачи
	for (n = 0; n < 5; ++n)         //Цикл для 5-и точек
	{
		bTX1[n]=*p1;         // значение пер по указателю p1 в bTX1
		bTX2[n]=*p2;
		bTX3[n]=*p3;
		bTX4[n]=*p4;
		bTX5[n]=*p5;
		bTX6[n]=*p6;
		p1++; p2++; p3++; p4++; p5++; p6++;
	}
}
//###################################################################################
//Настройка конфигурации передачи по  команде с ПК - пакет ID=1
void RunCommandsRxID1()
{
	static uint8_t NV=0;     // объявление переменной для for

	if (OutBuffer[1]==0)   {ModeGr=0;ModePar=0;} // Режим передачи данных в ПК 0-нет обмена;
	if (OutBuffer[1]==1)   {ModeGr=1;ModePar=0;} // 1-Только графика;
	if (OutBuffer[1]==2)   {ModeGr=0;ModePar=1;} // 2-Только пар-ры;
	if (OutBuffer[1]==3)   {ModeGr=1;ModePar=1;} // 3-Смеш-й режим

	if (OutBuffer[2]==6) {ModeDP=6;} //Режим 6 переменных
	if (OutBuffer[2]==1) {ModeDP=1;}
	if (OutBuffer[2]==0) {ModeDP=0;}

	if (OutBuffer[3]==200) {Sampling=200;}
	if (OutBuffer[3]==400) {Sampling=400;}
	if (OutBuffer[3]==1)   {Sampling=1000;}

	// pv -массив указателей на массивы гр переменных
	// NV колтчество переменных в массиве указателей
	// по значениям OutBuffer[4..9] получаем очередность указателей на 6 мас-в гр. пер-х для отправки в ПК

	for (NV=0; NV < (sizeof(pv)+1); NV+=1)
	{  if (OutBuffer[4]==NV)  {p1=pv[NV];  break;}}  // p1- указатель на 1-ю гр пер-ю

	for (NV=0; NV < (sizeof(pv)+1); NV+=1)
	{  if (OutBuffer[5]==NV)  {p2=pv[NV];  break;}}

	for (NV=0; NV < (sizeof(pv)+1); NV+=1)
	{  if (OutBuffer[6]==NV)  {p3=pv[NV];  break;}}

	for (NV=0; NV < (sizeof(pv)+1); NV+=1)
	{  if (OutBuffer[7]==NV)  {p4=pv[NV];  break;}}

	for (NV=0; NV < (sizeof(pv)+1); NV+=1)
	{  if (OutBuffer[8]==NV)  {p5=pv[NV];  break;}}

	for (NV=0; NV < (sizeof(pv)+1);NV +=1)
	{  if (OutBuffer[9]==NV)  {p6=pv[NV];  break;}}

	OutBuffer[0]=0;						            //Обработали пакет

	pp1=p1; pp2=p2; pp3=p3; pp4=p4; pp5=p5; pp6=p6;      // сделали копию для воостановления цикла

	CounterPac=0;  Nullcycle=0;						 //
}
//###################################################################
//Изменение данных (переменных) по  команде с ПК - пакет ID=2
void RunChDataRxID2()
{
	uint8_t Buf[4];
	float *Dp; float Dc;
	uint16_t Index,Dz;

	if (OutBuffer[1]!= 0xAA)      //Смена режима
	{
		if (OutBuffer[1]==1){ModeW=1;}	 // Рабочий
		if (OutBuffer[1]==2){ModeW=2;}	 // Наладочный
	}
	// -----Задание
	if (OutBuffer[2]!=0xAA)
	{
		Index=OutBuffer[2];
		Dz=OutBuffer[3];   // ст
		Dz=(Dz<<8);
		Dz=Dz|OutBuffer[4];
		switch (Index)
		{
		case 1  : s1=Dz; break;
		case 2  : s2=Dz; break;
		case 3  : s3=Dz;;
		}
	}
	// -----Коэф-ты  cf1
	if (OutBuffer[5]!=0xAA)
	{
		Index=OutBuffer[5];
		Buf[0]=OutBuffer[6];
		Buf[1]=OutBuffer[7];
		Buf[2]=OutBuffer[8];
		Buf[3]=OutBuffer[9];
		Dp=(float *)Buf;
		Dc=*Dp;
		switch (Index)
		{
		case 1  : cf1=Dc; break;
		case 2  : cf2=Dc; break;
		case 3  : cf3=Dc;
		}
	}
	OutBuffer[0]=0;            //Обработали пакет
}

//######################################################################
//Проверка состояния кнопки платы STM32F4-Discovery
void  Button_state()
{
	if (STM32F4_Discovery_PBGetState(BUTTON_USER) == Bit_SET)
	{
		if (ButtonPressed !=1)
		{
			nt=0;
			//				  CounterPac=0;                        //
			ButtonPressed = 1;
			DelayQ(800000);
		}
		else
		{
			if (ButtonPressed != 0)
			{
				ButtonPressed = 0;
				DelayQ(800000);
			}
		}
	}
}
//######################################################################

int main(void)
{
	SystemInit();

	p1=pv[0]; p2=pv[1]; p3=pv[2]; p4=pv[3]; p5=pv[4]; p6=pv[5];
	pp1=pv[0]; pp2=pv[1]; pp3=pv[2]; pp4=pv[3];pp5=pv[4]; pp6=pv[5];

	STM32F4_Discovery_LEDInit(LED3);
	STM32F4_Discovery_LEDInit(LED4);
	STM32F4_Discovery_LEDInit(LED5);
	STM32F4_Discovery_LEDInit(LED6);
	STM32F4_Discovery_PBInit(BUTTON_USER, BUTTON_MODE_GPIO);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,ENABLE);
	/*
	HCLK: 168 MHz
			APB1 Prescaler: 4 => CK_PSC = 2 * 42 MHz = 84 MHz
	 */
	// Частота TIMxCLK= 84 МГц при  кварце HSE=8 МГц
	TIM6->PSC = 84 - 1; 							// CK_CNT = 1 MHz= 1mkc
	TIM6->ARR = 1000-1;                             //Прерывание раз в 1mc

	TIM_ITConfig ( TIM6, TIM_IT_Update, ENABLE );   //Разрешения прерывание от таймера	по переполнению
	TIM_Cmd ( TIM6, ENABLE );                       // Включаем таймер

	USBD_Init(&USB_OTG_dev,  USB_OTG_FS_CORE_ID,  &USR_desc,  &USBD_HID_cb,  &USR_cb);
	NVIC_EnableIRQ(TIM6_DAC_IRQn);                  //Разрешение прерывания TIM6_DAC_IRQn

	/*  Вкл. LED3 */
	STM32F4_Discovery_LEDOn(LED4);   /*  */
	DelayQ(6000000);
	STM32F4_Discovery_LEDOff(LED4);   /*  */

	//------------------------------------------------

	while (1)
	{
		GetBufPar();       			// Заполнение буфера для передачи в ПК

		if (OutBuffer[0]==1) // Получение команды режима с ПК
		{
			RunCommandsRxID1();
		}

		if (OutBuffer[0]==2)  //Изменение переменных по  команде с ПК
		{
			RunChDataRxID2();
		}
		Button_state();

		// Индикация
		if(ModeGr==0)
			STM32F4_Discovery_LEDOff(LED6);
		if(ModePar==0)
			STM32F4_Discovery_LEDOff(LED3);

		if((ModeGr==0)&(ModePar==0))
		{
			if (OutBuffer[10]==1)
				STM32F4_Discovery_LEDOn(LED3);
			if (OutBuffer[10]==2)
				STM32F4_Discovery_LEDOn(LED6);
			if (OutBuffer[10]>2)
			{
				STM32F4_Discovery_LEDOff(LED6);
				STM32F4_Discovery_LEDOff(LED3);
			}
		}
	}
}

//############################################################################
/*================= Вектор прерывания TIM6 1 mc =================================*/
void TIM6_DAC_IRQHandler(void)
{

	TIM6->SR &= ~TIM_SR_UIF;                     //Сброс флага прерывания
	STM32F4_Discovery_LEDToggle(LED5);			 //Переключаем LED5

	Nullcycle++;
	if (Nullcycle==2) NullcyclePar++;

	if(ModeGr==1)                                    // Графический режим
	{
		if ((ModeDP==1)&(Nullcycle==4))             // Одна переменная
		{
			Tx1var();
		}
		// Передаем в ПК пакет содержащий 5 измерений 6 пер-х
		if ((ModeDP==6)&(Nullcycle==4))              //5 точек   IP=3
		{
			STM32F4_Discovery_LEDToggle(LED6);
			GetBufDT();	                   // Заполнение буфера DT- 7-я гр-я переменная
			GetBuf6Var();
			Tx6var();
		}
	}
	//---------------------------
	if(ModePar==1)
	{
		//Передача параметров в ПК  между гр пакетами
		if (NullcyclePar==100)
		{
			CouPacPar++;
			STM32F4_Discovery_LEDToggle(LED3);
			USBD_HID_SendReport (&USB_OTG_dev, Par, 64);
		}
	} // if(ModePar==1)

	if (Nullcycle==5) Nullcycle=0;
	if (NullcyclePar==100) NullcyclePar=0;
}
//###########################################################################################################
// Вектор Прерывания OTG

void OTG_FS_IRQHandler(void)                     //Full Speed
{
	STM32F4_Discovery_LEDToggle(LED4);
	USBD_OTG_ISR_Handler (&USB_OTG_dev);        //адрес структуры USB_OTG_CORE_HANDLE
}
