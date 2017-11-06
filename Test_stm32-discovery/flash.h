//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
#ifndef FLASH_H
#define FLASH_H
//--------------------------------------------------------------------------------
typedef struct
  {
    char Parameter1;        // 1 byte
    uint8_t Parameter2;     // 1 byte
    uint16_t Parameter3;    // 2 byte
    uint32_t Parameter4;    // 4 byte
    uint32_t Parameter5;    // 4 byte

                            // 8 byte = 2  32-bits words.  It's - OK
                            // !!! Full size (bytes) must be a multiple of 4 !!!
  } tpSettings;
tpSettings settings;
//--------------------------------------------------------------------------------
#endif
