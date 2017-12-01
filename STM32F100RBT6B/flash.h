//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
#ifndef FLASH_H
#define FLASH_H
//--------------------------------------------------------------------------------
typedef struct
{
	uint32_t	addr_cam_32;				// ����� ������
	uint32_t    time_interval_16;           // �������� ��������
	uint32_t    time_washout_32;            // ����� �������
	uint32_t    time_pause_washout_32;      // ����� ����� ���������
	uint32_t    preset_washout_32;          // ������ �������
	uint32_t	time_preset_washout_32;     // ������ �������
} tpSettings;
tpSettings settings;

void FLASH_Init(void);
void FLASH_ReadSettings(void);
void FLASH_WriteSettings(void);
void write_FLASH(void);
void read_FLASH(void);
//--------------------------------------------------------------------------------
#endif
