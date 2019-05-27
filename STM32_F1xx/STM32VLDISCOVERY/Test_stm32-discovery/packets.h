//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
#ifndef PACKET_HPP
#define PACKET_HPP
//--------------------------------------------------------------------------------
#pragma pack (push, 1)
//--------------------------------------------------------------------------------
union U_BYTE
{
	uint8_t value;
	struct
	{
		uint8_t bit0:1;
		uint8_t bit1:1;
		uint8_t bit2:1;
		uint8_t bit3:1;
		uint8_t bit4:1;
		uint8_t bit5:1;
		uint8_t bit6:1;
		uint8_t bit7:1;
	} bites;
};

enum CMD {
    CMD_TEST    = 0,
    CMD_READ    = 1,
    CMD_WRITE   = 2,
    CMD_RESET   = 3
};

union PELCO_PACKET
{
    struct PELCO_BODY
    {
    	uint8_t sync;
        uint8_t address;
        uint8_t cmd1;
        uint8_t cmd2;
        uint8_t data1;
        uint8_t data2;
        uint8_t crc;
    } body;
    uint8_t buf[sizeof(struct PELCO_BODY)];
};

typedef struct HEADER
{
	uint16_t    prefix_16;                  // префикс
	uint8_t     addr_8;                     // адрес модуля
	uint8_t     cmd_8;                      // команда
	uint16_t    len_16;                     // длина данных
} header;

//--------------------------------------------
union QUESTION_TEST
{
    struct QUESTION_TEST_BODY
    {
    	struct HEADER   header;
        uint16_t    data;
        uint16_t    crc16;                  // контрольная сумма
    } body;
    unsigned char buf[sizeof(struct QUESTION_TEST_BODY)];
};

union ANSWER_TEST
{
    struct ANSWER_TEST_BODY
    {
    	struct HEADER   header;
        uint16_t    data;
        uint16_t    crc16;                  // контрольная сумма
    } body;
    unsigned char buf[sizeof(struct ANSWER_TEST_BODY)];
};
//--------------------------------------------
union QUESTION_RESET
{
    struct QUESTION_RESET_BODY
    {
    	struct HEADER   header;
        uint16_t    data;
        uint16_t    crc16;                  // контрольная сумма
    } body;
    unsigned char buf[sizeof(struct QUESTION_RESET_BODY)];
};

union ANSWER_RESET
{
    struct ANSWER_RESET_BODY
    {
    	struct HEADER   header;
        uint16_t    data;
        uint16_t    crc16;                  // контрольная сумма
    } body;
    unsigned char buf[sizeof(struct ANSWER_RESET_BODY)];
};
//--------------------------------------------
union QUESTION_READ
{
    struct QUESTION_READ_BODY
    {
        struct HEADER      header;
        uint16_t    crc16;                      // контрольная сумма
    } body;
    unsigned char buf[sizeof(struct QUESTION_READ_BODY)];
};

union ANSWER_READ
{
    struct ANSWER_READ_BODY
    {
    	struct HEADER      header;

        uint32_t    addr_cam_32;                // адрес камеры
        uint16_t    time_interval_16;           // интервал дворника
        uint32_t    time_washout_32;            // время помывки
        uint32_t    time_pause_washout_32;      // время между помывками
        uint32_t    preset_washout_32;          // пресет помывки
        uint32_t    time_preset_washout_32;     // времен помывки

        uint16_t    crc16;                      // контрольная сумма
    } body;
    unsigned char buf[sizeof(struct ANSWER_READ_BODY)];
};
//--------------------------------------------
union QUESTION_WRITE
{
    struct QUESTION_WRITE_BODY
    {
    	struct HEADER      header;

        uint32_t    addr_cam_32;                // адрес камеры
        uint16_t    time_interval_16;           // интервал дворника
        uint32_t    time_washout_32;            // время помывки
        uint32_t    time_pause_washout_32;      // время между помывками
        uint32_t    preset_washout_32;          // пресет помывки
        uint32_t    time_preset_washout_32;     // времен помывки

        uint16_t    crc16;                      // контрольная сумма
    } body;
    unsigned char buf[sizeof(struct QUESTION_WRITE_BODY)];
};

union ANSWER_WRITE
{
    struct ANSWER_WRITE_BODY
    {
    	struct HEADER      header;

        uint32_t    addr_cam_32;                // адрес камеры
        uint16_t    time_interval_16;           // интервал дворника
        uint32_t    time_washout_32;            // время помывки
        uint32_t    time_pause_washout_32;      // время между помывками
        uint32_t    preset_washout_32;          // пресет помывки
        uint32_t    time_preset_washout_32;     // времен помывки

        uint16_t    crc16;                      // контрольная сумма
    } body;
    unsigned char buf[sizeof(struct ANSWER_WRITE_BODY)];
};
//--------------------------------------------
#pragma pack(pop)
//--------------------------------------------------------------------------------
#endif // PACKET_HPP
