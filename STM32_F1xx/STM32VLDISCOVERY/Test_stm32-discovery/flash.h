//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
#ifndef FLASH_H
#define FLASH_H
//--------------------------------------------------------------------------------
typedef struct
{
	uint32_t    time_interval_16;           // �������� ��������
	uint32_t    time_washout_32;            // ����� �������
	uint32_t    time_pause_washout_32;      // ����� ����� ���������
	uint32_t    preset_washout_32;          // ������ �������
	uint32_t	time_preset_washout_32;     // ������ �������
} tpSettings;
tpSettings settings;
//--------------------------------------------------------------------------------
#endif
