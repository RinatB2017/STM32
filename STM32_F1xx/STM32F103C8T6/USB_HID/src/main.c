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
// ������������ ��������
void DelayQ(uint32_t nTime)
{
	while ( --nTime)  {   }
}
//#########################################################################
//���������� ������ ���������� ��� ����������� �������� � ��
void  GetBufPar()
{
	int  p1=380;
	int  p2=241;
	int  p3=24;
	int  p4=50;
	int  p5=53500;

	Par[0]=4;     //ID
	//Mode
	Par[1]=ModeW; // 1-����� ������� (2-����������)
	Par[2]=0;     // ������
	//Alarm
	Par[3]=0x12;  //  Alarm-���  Q1-���
	Par[4]=0x31;  //Id-��� Q��-���
	Par[5]=0x23;  //R��-����� (��� ���������)
	Par[6]=0;		//������

	//Coefficients
	//Float (cf1,cf2,cf3) to byte (Par[7-18])
	Par[7]=   (((uint8_t*)&cf1)[0]);   // - ������ ����
	Par[8]=   (((uint8_t*)&cf1)[1]);   // - ������ ����
	Par[9]=   (((uint8_t*)&cf1)[2]);   // - ������ ����
	Par[10]=  (((uint8_t*)&cf1)[3]);   //-  �������� ����
	Par[11]=  (((uint8_t*)&cf2)[0]);
	Par[12]=  (((uint8_t*)&cf2)[1]);
	Par[13]=  (((uint8_t*)&cf2)[2]);
	Par[14]=  (((uint8_t*)&cf2)[3]);
	Par[15]=  (((uint8_t*)&cf3)[0]);   // - ������ ����
	Par[16]=  (((uint8_t*)&cf3)[1]);   // - ������ ����
	Par[17]=  (((uint8_t*)&cf3)[2]);   // - ������ ����
	Par[18]=  (((uint8_t*)&cf3)[3]);   //-  �������� ����

	Par[19]=0;Par[20]=0;Par[21]=0;Par[22]=0;Par[23]=0;Par[24]=0; //������
	Par[25]=0;Par[26]=0;Par[27]=0;Par[28]=0;Par[29]=0;Par[30]=0; //������

	//View
	Par[31]=(p1>>8); Par[32]=p1; Par[33]=(p2>>8); Par[34]=p2;
	Par[35]=(p3>>8); Par[36]=p3; Par[37]=(p4>>8); Par[38]=p4;
	Par[39]=(p5>>8); Par[40]=p5;
	Par[41]=(CouPacPar>>8);Par[42]=CouPacPar;
	Par[43]=0;Par[44]=0;Par[45]=0;Par[46]=0; //������
	Par[47]=0;Par[48]=0;Par[49]=0;Par[50]=0;                     //������
	//Settings
	Par[51]=(s1>>8); Par[52]=s1; Par[53]=(s2>>8); Par[54]=s2;
	Par[55]=(s3>>8); Par[56]=s3;
	Par[57]=0; Par[58]=0; 										//������

	//Signaling
	Par[59]=0x12;           //Q1-���  Q2-���
	Par[60]=0x34; 			//K1-���  K2-����
	Par[61]=0x51;			// R��-��������� �������������-�����
	Par[62]=0; 				//������
	Par[63]=0; 				//������
}
//####################################################################################
//�������� ����� ����������� ����������
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
//�������� ����� ����������� ����������
void  Tx6var()
{
	int  ii,s;
	//*******  ��������� ����� �� ������� � �������� � ��  *********
	InBuffer[0]=3;                  //ID=3- ����������� ������
	s=0;
	for (ii = 0; ii < 5; ++ii)         //���� ��� 5-� �����
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
		CFBuf++;									//  Counter of Filling buffer ������� ���������� ������ �������
		//  (1...4)������ 4�� ��������� 5 ���� 5*4=20
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
//���������� 6 ������� bTX1-6[] ����������� �� �������� � �� ����������� �� ��� ���-�� � �����������
//� �������  ������ Ua [20]..Ic[20]-��������� (� �������� 4 ������), DT[20]-�������(0..4080)� ����������� 10 ������ 1��
//������� 5 ��������� �����(��������) 6-� ���������� ������ Ua-Ic ������ ���� ���������� ��� � �������� �  �����. �������
void GetBuf6Var()
{
	uint16_t n;
	CounterPac++;
	InBuffer[1]=CounterPac;         // ����� ������-�� ������������ � ��(������ ���������)

	if (CounterPac==1) {p1=pp1; p2=pp2; p3=pp3; p4=pp4; p5=pp5; p6=pp6;}
	if (CounterPac==4) CounterPac=0;

	//������ �������� ���������� � �������� � ������ ��������
	for (n = 0; n < 5; ++n)         //���� ��� 5-� �����
	{
		bTX1[n]=*p1;         // �������� ��� �� ��������� p1 � bTX1
		bTX2[n]=*p2;
		bTX3[n]=*p3;
		bTX4[n]=*p4;
		bTX5[n]=*p5;
		bTX6[n]=*p6;
		p1++; p2++; p3++; p4++; p5++; p6++;
	}
}
//###################################################################################
//��������� ������������ �������� ��  ������� � �� - ����� ID=1
void RunCommandsRxID1()
{
	static uint8_t NV=0;     // ���������� ���������� ��� for

	if (OutBuffer[1]==0)   {ModeGr=0;ModePar=0;} // ����� �������� ������ � �� 0-��� ������;
	if (OutBuffer[1]==1)   {ModeGr=1;ModePar=0;} // 1-������ �������;
	if (OutBuffer[1]==2)   {ModeGr=0;ModePar=1;} // 2-������ ���-��;
	if (OutBuffer[1]==3)   {ModeGr=1;ModePar=1;} // 3-����-� �����

	if (OutBuffer[2]==6) {ModeDP=6;} //����� 6 ����������
	if (OutBuffer[2]==1) {ModeDP=1;}
	if (OutBuffer[2]==0) {ModeDP=0;}

	if (OutBuffer[3]==200) {Sampling=200;}
	if (OutBuffer[3]==400) {Sampling=400;}
	if (OutBuffer[3]==1)   {Sampling=1000;}

	// pv -������ ���������� �� ������� �� ����������
	// NV ���������� ���������� � ������� ����������
	// �� ��������� OutBuffer[4..9] �������� ����������� ���������� �� 6 ���-� ��. ���-� ��� �������� � ��

	for (NV=0; NV < (sizeof(pv)+1); NV+=1)
	{  if (OutBuffer[4]==NV)  {p1=pv[NV];  break;}}  // p1- ��������� �� 1-� �� ���-�

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

	OutBuffer[0]=0;						            //���������� �����

	pp1=p1; pp2=p2; pp3=p3; pp4=p4; pp5=p5; pp6=p6;      // ������� ����� ��� �������������� �����

	CounterPac=0;  Nullcycle=0;						 //
}
//###################################################################
//��������� ������ (����������) ��  ������� � �� - ����� ID=2
void RunChDataRxID2()
{
	uint8_t Buf[4];
	float *Dp; float Dc;
	uint16_t Index,Dz;

	if (OutBuffer[1]!= 0xAA)      //����� ������
	{
		if (OutBuffer[1]==1){ModeW=1;}	 // �������
		if (OutBuffer[1]==2){ModeW=2;}	 // ����������
	}
	// -----�������
	if (OutBuffer[2]!=0xAA)
	{
		Index=OutBuffer[2];
		Dz=OutBuffer[3];   // ��
		Dz=(Dz<<8);
		Dz=Dz|OutBuffer[4];
		switch (Index)
		{
		case 1  : s1=Dz; break;
		case 2  : s2=Dz; break;
		case 3  : s3=Dz;;
		}
	}
	// -----����-��  cf1
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
	OutBuffer[0]=0;            //���������� �����
}

//######################################################################
//�������� ��������� ������ ����� STM32F4-Discovery
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
	// ������� TIMxCLK= 84 ��� ���  ������ HSE=8 ���
	TIM6->PSC = 84 - 1; 							// CK_CNT = 1 MHz= 1mkc
	TIM6->ARR = 1000-1;                             //���������� ��� � 1mc

	TIM_ITConfig ( TIM6, TIM_IT_Update, ENABLE );   //���������� ���������� �� �������	�� ������������
	TIM_Cmd ( TIM6, ENABLE );                       // �������� ������

	USBD_Init(&USB_OTG_dev,  USB_OTG_FS_CORE_ID,  &USR_desc,  &USBD_HID_cb,  &USR_cb);
	NVIC_EnableIRQ(TIM6_DAC_IRQn);                  //���������� ���������� TIM6_DAC_IRQn

	/*  ���. LED3 */
	STM32F4_Discovery_LEDOn(LED4);   /*  */
	DelayQ(6000000);
	STM32F4_Discovery_LEDOff(LED4);   /*  */

	//------------------------------------------------

	while (1)
	{
		GetBufPar();       			// ���������� ������ ��� �������� � ��

		if (OutBuffer[0]==1) // ��������� ������� ������ � ��
		{
			RunCommandsRxID1();
		}

		if (OutBuffer[0]==2)  //��������� ���������� ��  ������� � ��
		{
			RunChDataRxID2();
		}
		Button_state();

		// ���������
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
/*================= ������ ���������� TIM6 1 mc =================================*/
void TIM6_DAC_IRQHandler(void)
{

	TIM6->SR &= ~TIM_SR_UIF;                     //����� ����� ����������
	STM32F4_Discovery_LEDToggle(LED5);			 //����������� LED5

	Nullcycle++;
	if (Nullcycle==2) NullcyclePar++;

	if(ModeGr==1)                                    // ����������� �����
	{
		if ((ModeDP==1)&(Nullcycle==4))             // ���� ����������
		{
			Tx1var();
		}
		// �������� � �� ����� ���������� 5 ��������� 6 ���-�
		if ((ModeDP==6)&(Nullcycle==4))              //5 �����   IP=3
		{
			STM32F4_Discovery_LEDToggle(LED6);
			GetBufDT();	                   // ���������� ������ DT- 7-� ��-� ����������
			GetBuf6Var();
			Tx6var();
		}
	}
	//---------------------------
	if(ModePar==1)
	{
		//�������� ���������� � ��  ����� �� ��������
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
// ������ ���������� OTG

void OTG_FS_IRQHandler(void)                     //Full Speed
{
	STM32F4_Discovery_LEDToggle(LED4);
	USBD_OTG_ISR_Handler (&USB_OTG_dev);        //����� ��������� USB_OTG_CORE_HANDLE
}
