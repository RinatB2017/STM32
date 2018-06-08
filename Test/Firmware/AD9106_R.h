#ifndef _AD9106_R_H_
#define _AD9106_R_H_

#include "stm32f10x.h"

//#define AD9106_REG_NUM_MAX 	66
// Ðåãèñòðû
typedef enum {
	SPICONFIG = 0x00,
	POWERCONFIG,
	CLOCKCONFIG,
	REFADJ,
	DAC4AGAIN,
	DAC3AGAIN,
	DAC2AGAIN,
	DAC1AGAIN,
	DACxRANGE,
	DAC4RSET,
	DAC3RSET,
	DAC2RSET,
	DAC1RSET,
	CALCONFIG,
	COMPOFFSET = 0x0E,
	RAMUPDATE = 0x1D,
	PAT_STATUS,
	PAT_TYPE,
	PATTERN_DLY = 0x20,
	DAC4DOF = 0x22,
	DAC3DOF,
	DAC2DOF,
	DAC1DOF,
	WAV4_3CONFIG,
	WAV2_1CONFIG,
	PAT_TIMEBASE,
	PAT_PERIOD,
	DAC4_3PATx,
	DAC2_1PATx,
	DOUT_START_DLY,
	DOUT_CONFIG,
	DAC4_CST,
	DAC3_CST,
	DAC2_CST,
	DAC1_CST,
	DAC4_DGAIN,
	DAC3_DGAIN,
	DAC2_DGAIN,
	DAC1_DGAIN,
	SAW4_3CONFIG,
	SAW2_1CONFIG,
	DDS_TW32 = 0x3E,
	DDS_TW1,
	DDS4_PW,
	DDS3_PW,
	DDS2_PW,
	DDS1_PW,
	TRIG_TW_SEL,
	DDSx_CONFIG,
	TW_RAM_CONFIG = 0x47,
	START_DLY4 = 0x50,
	START_ADDR4,
	STOP_ADDR4,
	DDS_CYC4,
	START_DLY3,
	START_ADDR3,
	STOP_ADDR3,
	DDS_CYC3,
	START_DLY2,
	START_ADDR2,
	STOP_ADDR2,
	DDS_CYC2,
	START_DLY1,
	START_ADDR1,
	STOP_ADDR1,
	DDS_CYC1,
	CFG_ERROR,
	SRAMDATA = 0x6000, //0x6000 to 0x6FFF
	SRAMDATA_END = 0x6fff,
} AD9106_RegistersADDR_e;
/*
const static uint16_t AD9106_RegistersADDR[] = {
	SPICONFIG,
	POWERCONFIG,
	CLOCKCONFIG,
	REFADJ,
	DAC4AGAIN,
	DAC3AGAIN,
	DAC2AGAIN,
	DAC1AGAIN,
	DACxRANGE,
	DAC4RSET,
	DAC3RSET,
	DAC2RSET,
	DAC1RSET,
	CALCONFIG,
	COMPOFFSET,
	RAMUPDATE,
	PAT_STATUS,
	PAT_TYPE,
	PATTERN_DLY,
	DAC4DOF,
	DAC3DOF,
	DAC2DOF,
	DAC1DOF,
	WAV4_3CONFIG,
	WAV2_1CONFIG,
	PAT_TIMEBASE,
	PAT_PERIOD,
	DAC4_3PATx,
	DAC2_1PATx,
	DOUT_START_DLY,
	DOUT_CONFIG,
	DAC4_CST,
	DAC3_CST,
	DAC2_CST,
	DAC1_CST,
	DAC4_DGAIN,
	DAC3_DGAIN,
	DAC2_DGAIN,
	DAC1_DGAIN,
	SAW4_3CONFIG,
	SAW2_1CONFIG,
	DDS_TW32,
	DDS_TW1,
	DDS4_PW,
	DDS3_PW,
	DDS2_PW,
	DDS1_PW,
	TRIG_TW_SEL,
	DDSx_CONFIG,
	TW_RAM_CONFIG,
	START_DLY4,
	START_ADDR4,
	STOP_ADDR4,
	DDS_CYC4,
	START_DLY3,
	START_ADDR3,
	STOP_ADDR3,
	DDS_CYC3,
	START_DLY2,
	START_ADDR2,
	STOP_ADDR2,
	DDS_CYC2,
	START_DLY1,
	START_ADDR1,
	STOP_ADDR1,
	DDS_CYC1,
	CFG_ERROR};
*/
	// ------------------------------------- ýòî ê ÓÄÀËÅÍÈÞ!
/*
//Proper BITMASKS for each register, allows us to write/ read to/from device in easy way , not overwriting whole register
typedef enum {
	LSBFIRST = 0x8000, 	// 0 - MSB first per SPI standard (default) \ 1 - LSB first per SPI standard
	SPI3WIRE = 0x4000, 	// 0 - 4-wire SPI \ 1 - 3-wire SPI
	RESETSOFT = 0x2000, 		// Executes software reset of SPI and controllers, reloads default register values, except for Register 0x00.
	DOUBLESPI = 0x1000, // Double SPI data line.
	SPI_DRV = 0x0800,   // Double-drive ability for SPI output.
	DOUT_EN = 0x0400,   // Enables DOUT signal on SDO/SDI2/DOUT pin.
	DOUT_ENM = 0x0020,  // Enable DOUT signal on SDO/SDI2/DOUT pin.
	SPI_DRVM = 0x0010,  // Double-drive ability for SPI output.
	DOUBLESPIM = 0x0008,// Double SPI data line.
	RESETM = 0x0004, 		// Executes software reset of SPI and controllers, reloads default register values, except for Register 0x00.
	SPI3WIREM = 0x0002, // Selects if SPI is using 3-wire or 4-wire interface.
	LSBFIRSTM = 0x0001  // LSB first selection.
} SPICONFIG_Reg;
// SPICONFIG[10:15] should always be set to the mirror of SPICONFIG[5:0] 
// to allow easy recovery of the SPI operation when the LSBFIRST bit is set incorrectly. 
// Bit[15] = Bit[0], Bit[14] = Bit[1], Bit[13] = Bit[2], Bit[12] = Bit[3], Bit[11] = Bit[4] and Bit[10] = Bit[5]

typedef enum {
	CLK_LDO_STAT = 0x0800,  // Read only. Flag indicating CLKVDD_1P8 LDO is on.
	DIG1_LDO_STAT = 0x0400, // Read only. Flag indicating DVDD1 LDO is on.
	DIG2_LDO_STAT = 0x0200, // Read only. Flag indicating DVDD2 LDO is on.
	PDN_LDO_CLK = 0x0100, 	// Disables the CLKVDD_1P8 LDO. An external supply is required.
	PDN_LDO_DIG1 = 0x0080, 	// Disables the DVDD1 LDO. An external supply is required.
	PDN_LDO_DIG2 = 0x0040, 	// Disables the DVDD2 LDO. An external supply is required.
	REF_PDN = 0x0020, 			// Disables 10 kOm resistor that creates REFIO voltage. User can drive with external voltage or provide external BG resistor.
	REF_EXT = 0x0010,				// Power down main BG reference including DAC bias.
	DAC1_SLEEP = 0x0008,		// Disables DAC1 output current.
	DAC2_SLEEP = 0x0004,		// Disables DAC2 output current.
	DAC3_SLEEP = 0x0002,		// Disables DAC3 output current.
	DAC4_SLEEP = 0x0001			// Disables DAC4 output current.
} POWERCONFIG_Reg;

typedef enum {
	DIS_CLK1 = 0x0800,			// Disables the analog clock to DAC1 out of the clock distribution block.
	DIS_CLK2 = 0x0400,			// Disables the analog clock to DAC2 out of the clock distribution block.
	DIS_CLK3 = 0x0200,			// Disables the analog clock to DAC3 out of the clock distribution block.
	DIS_CLK4 = 0x0100,			// Disables the analog clock to DAC4 out of the clock distribution block.
	DIS_DCLK = 0x0080,			// Disables the clock to core digital block.
	CLK_SLEEP = 0x0040,			// Enables a very low power clock mode.
	CLK_PDN = 0x0020,				// Disables and powers down main clock receiver.
	EPS = 0x0010,						// Enables Power Save (EPS) enables a low power option for the clock receiver, but maintains low jitter performance on DAC clock rising edge.
	DAC1_INV_CLK = 0x0008,	// Cannot use EPS while using this bit. Inverts the clock inside DAC Core 1 allowing 180° phase shift in DAC1 update timing.
	DAC2_INV_CLK = 0x0004,
	DAC3_INV_CLK = 0x0002,
	DAC4_INV_CLK = 0x0001
} CLOCKCONFIG_Reg;

typedef enum {
	BGDR = 0x003F						// Adjusts the BG 10 kO resistor (nominal) to 8k to 12k, changes BG voltage from 800 mV to 1.2V, respectively.
} REFADJ_Reg;

// Äëÿ DAC1AGAIN, DAC2AGAIN, DAC3AGAIN, DAC4AGAIN
typedef enum {
	DACx_GAIN_CAL = 0x7F00, // Read only. DACx analog gain calibration output.
	DACx_GAIN = 0x007F			// DACx analog gain control while not in calibration mode—twos complement.
} DACxAGAIN_Reg;

typedef enum {
	DAC4_GAIN_RNG = 0xC0,		// DAC4 gain range control.
	DAC3_GAIN_RNG = 0x30,
	DAC2_GAIN_RNG = 0x0C,
	DAC1_GAIN_RNG = 0x03
} DACxRANGE_Reg;

// Äëÿ DAC1RSET, DAC2RSET, DAC3RSET, DAC4RSET
typedef enum {
	DACx_RSET_EN = 0x8000, 	// For write, enable the internal RSET resistor for DAC4; for read, RSET for DAC4 is enabled during calibration mode.
	DACx_RSET_CAL = 0x1F00, // Read only. Digital control value of RSET resistor for DAC4 after calibration.
	DACx_RSET = 0x1F				// Digital control to set the value of RSET resistor in DAC4.
} DACxRSET_Reg;

typedef enum {
	COMP_OFFSET_OF = 0x4000,// Read only. Compensation offset calibration value overflow
	COMP_OFFSET_UF = 0x2000,// Read only. Compensation offset calibration value underflow.
	RSET_CAL_OF = 0x1000,   // Read only. RSET calibration value overflow.
	RSET_CAL_UF = 0x0800,   // Read only. RSET calibration value underflow.
	GAIN_CAL_OF = 0x0400,   // Read only. Gain calibration value overflow.
	GAIN_CAL_UF = 0x0200,   // Read only. Gain calibration value underflow.
	CAL_RESET = 0x0100,			// Pulse this bit high and low to reset the calibration results.
	CAL_MODE = 0x0080,      // Read-only. Flag indicating calibration is being used.
	CAL_MODE_EN = 0x0040,		// Enables the gain calibration circuitry.
	COMP_CAL_RNG = 0x0030,	// Offset calibration range.
	CAL_CLK_EN = 0x0008,		// Enables the calibration clock to calibration circuitry.
	CAL_CLK_DIV = 0x0007		// Sets divider from DAC clock to calibration clock.
} CALCONFIG_Reg;

typedef enum {
	COMP_OFFSET_CAL = 0x7F00,// Read only. The result of the offset calibration for the comparator.
	CAL_FIN = 0x0002,        // Read-only. Flag indicating calibration is completed.
	START_CAL = 0x0001			 // Start a calibration cycle.
} COMPOFFSET_Reg;

typedef enum {
	RAMPUPDATE = 0x0001			 // Update all SPI setting with new configuration (self clearing).
} RAMUPDATE_Reg;

typedef enum {
	BUF_READ = 0x0008, 			 // Read back from updated buffer.
	MEM_ACCESS = 0x0004, 		 // Memory SPI access enable.
	PATTERN = 0x0002,        // Read only. Status of pattern being played.
	RUN = 0x0001						 // Allows the pattern generation and stop pattern after trigger.
} PAT_STATUS_Reg;

typedef enum {
	PATTERN_RPT = 0x0001		 // Setting this bit allows the pattern to repeat the number of times defined in DAC4_3PATx and DAC2_1PATx.
} PAT_TYPE_Reg;			// 0 - Pattern continuously runs. / 1 - Pattern repeats the number of times defined in DAC4_3PATx and DAC2_1PATx.

typedef enum {
	PATTERN_DELAY = 0xFFFF	 // Time between trigger low and pattern start in number of DAC clock cycles + 1.
} PATTERN_DLY_Reg;

// Äëÿ DAC1DOF, DAC2DOF, DAC3DOF, DAC4DOF
typedef enum {
	DACx_DIG_OFFSET = 0xFFF0	// DACx digital offset.
} DACxDOF_Reg;

typedef enum {
	PRESTORE_SEL4 = 0x3000,
	WAVE_SEL4 = 0x0300,
	PRESTORE_SEL3 = 0x30,
	WAVE_SEL3 = 0x03,
} WAV4_3CONFIG_Reg;

typedef enum {
	PRESTORE_SEL2 = 0x3000,
	MASK_DAC4 = 0x0800,
	CH2_ADD = 0x0400,
	WAVE_SEL2 = 0x0300,
	PRESTORE_SEL1 = 0x30,
	MASK_DAC3 = 0x08,
	CH1_ADD = 0x04,
	WAVE_SEL1 = 0x03,
} WAV2_1CONFIG_Reg;

typedef enum {
	HOLD = 0x0F00, 
	PAT_PERIOD_BASE = 0xF0, 
	START_DELAY_BASE = 0x0F,
} PAT_TIMEBASE_Reg;

typedef enum {
	PATTERN_PERIOD = 0xFFFF
} PAT_PERIOD_Reg;

typedef enum {
	DAC4_REPEAT_CYCLE = 0xFF00, 
	DAC3_REPEAT_CYCLE = 0xFF
} DAC4_3PATx_Reg;

typedef enum {
	DAC2_REPEAT_CYCLE = 0xFF00, 
	DAC1_REPEAT_CYCLE = 0xFF
} DAC2_1PATx_Reg;

typedef enum {
	DOUT_START = 0xFFFF,
} DOUT_START_DLY_Reg;

typedef enum {
	DOUT_VAL = 0x20, 
	DOUT_MODE = 0x10, 
	DOUT_STOP = 0x0F
} DOUT_CONFIG_Reg;

typedef enum {
	DACx_CONST = 0xFFF0
} DACx_CST_Reg;

typedef enum {
	DACx_DIG_GAIN = 0xFFF0
} DACx_DGAIN_Reg;

typedef enum {
	SAW_STEP4 = 0xFC00,  //Number of samples per step for DAC4.
	SAW_TYPE4 = 0x0300, //0- Ramp up saw wave. 1- Ramp down saw wave. 2- Triangle saw wave. 3- No wave, zero.
	SAW_STEP3 = 0x00FC,
	SAW_TYPE3 = 0x0003
} SAW4_3CONFIG_Reg;

typedef enum {
	SAW_STEP2 = 0xFC00,  //Number of samples per step for DAC2.
	SAW_TYPE2 = 0x0300, //0- Ramp up saw wave. 1- Ramp down saw wave. 2- Triangle saw wave. 3- No wave, zero.
	SAW_STEP1 = 0x00FC,
	SAW_TYPE1 = 0x0003
} SAW2_1CONFIG_Reg;

///////////////
typedef enum {
	DDSTW_MSB = 0xFFFF  //DDS tuning word MSB.
} DDS_TW32_Reg;

typedef enum {
	DDSTW_LSB = 0xFF00  //DDS tuning word LSB.
} DDS_TW1_Reg;

typedef enum {
	DDSx_PHASE = 0xFFFF //DDSx phase offset.
} DDSx_PW_Reg;

typedef enum {
	TRIG_DELAY_EN = 0x0002 //Enable start delay as trigger delay for all four channels.
	//Settings
	// 0 Delay repeats for all patterns.
	 //1 Delay is only at the start of first pattern.
	 
} TRIG_TW_SEL_Reg;

typedef enum {
	DDS_COS_EN4 = 0x8000, //Enable DDS4 cosine output of DDS instead of sine wave.
	DDS_MSB_EN4 = 0x4000, //Enable the clock for the RAM address. Increment is coming from the DDS4 MSB. Default is coming from DAC clock.
	DDS_COS_EN3 = 0x0800, //Enable DDS3 cosine output of DDS instead of sine wave.
	DDS_MSB_EN3 = 0x0400, //Enable the clock for the RAM address. Increment is coming from the DDS3 MSB. Default is coming from DAC clock.
	PHASE_MEM_EN3 = 0x0200, //Enable DDS3 phase offset input coming from RAM reading START_ADDR3. Since phase word is 8 bits and RAM data is 14 bits, only 8 MSB of RAM are taken into account. Default is coming from SPI map, DDS3_PHASE.
	DDS_COS_EN2 = 0x0080, //Enable DDS2 cosine output of DDS instead of sine wave.
	DDS_MSB_EN2 = 0x0040, //Enable the clock for the RAM address. Increment is coming from the DDS2 MSB. Default is coming from DAC clock.
	DDS_MSB_EN1 = 0x0004, //Enable the clock for the RAM address. Increment is coming from the DDS1 MSB. Default is coming from DAC clock.
	TW_MEM_EN = 0x0001 //Enable DDS tuning word input coming from RAM reading using START_ADDR1. Since tuning word is 24 bits and RAM data is 14 bits, 10 bits are set to 0s depending on the value of the TW_MEM_SHIFT bits in the TW_RAM_CONFIG register. Default is coming from SPI map, DDSTW.
} DDSx_CONFIG_Reg;

typedef enum {
	TW_MEM_SHIFT = 0x000F
//settings NOTE! TW_MEM_EN1 must be set = 1 to use this bit field.

 0x00   DDS1TW = {RAM[11:0],12'b0}
 0x01   DDS1TW = {DDS1TW[23],RAM[11:0],11'b0}
 0x02	DDS1TW = {DDS1TW[23:22],RAM[11:0],10'b0}
 0x03	DDS1TW = {DDS1TW[23:21],RAM[11:0],9'b0}
 0x04	DDS1TW = {DDS1TW[23:20],RAM[11:0],8'b0}
 0x05	DDS1TW = {DDS1TW[23:19],RAM[11:0],7'b0}
 0x06	DDS1TW = {DDS1TW[23:18],RAM[11:0],6'b0}
 0x07	DDS1TW = {DDS1TW[23:17],RAM[11:0],5'b0}
 0x08	DDS1TW = {DDS1TW[23:16],RAM[11:0],3'b0}
 0x09	DDS1TW = {DDS1TW[23:15],RAM[11:0],4'b0}
 0x0A	DDS1TW = {DDS1TW[23:14],RAM[11:0],2’b0}
 0x0B	DDS1TW = {DDS1TW[23:13],RAM[11:0],1’b0}
 0x0C	DDS1TW = {DDS1TW[23:12],RAM[11:0]}
 0x0D	DDS1TW = {DDS1TW[23:11],RAM[11:1]}
 0x0E	DDS1TW = {DDS1TW[23:10],RAM[11:2]}
 0x0F	DDS1TW = {DDS1TW[23:9],RAM[11:3]}
 0x10	DDS1TW = {DDS1TW[23:8],RAM[11:4]}
 x Reserved
 
} TW_RAM_CONFIG_Reg;

// Äëÿ START_DLY1, START_DLY2, START_DLY3, START_DLY4
typedef enum {
	START_DELAYx = 0xFFFF,  //Start delay of DACx.
} START_DLYx_REG;

typedef enum {
	START_ADDRx = 0xFFF0,  //RAM address where DACx starts to read waveform.
} START_ADDRx_Reg;

typedef enum {
	STOP_ADDRx = 0xFFF0,  //RAM address where DACx stops to read waveform.
} STOP_ADDRx_Reg;

typedef enum {
	DDS_CYCx = 0xFFFF // Number of sine wave cycles when DDS prestored waveform with start and stop delays is selected for DACx output.
} DDS_CYCx_Reg;

typedef enum {
	ERROR_CLEAR = 0x8000,  		  		// Read only. Writing this bit clears all errors.
	CFG_ERROR_ = 0x7FC0,						// Read only.
	DOUT_START_LG_ERR = 0x0020, 		// Read only. When DOUT_START is larger than pattern delay, this error is toggled.
	PAT_DLY_SHORT_ERR = 0x0010, 		// Read only. When pattern delay value is smaller than default value, this error is toggled.
	DOUT_START_SHORT_ERR = 0x0008, 	// Read only. When DOUT_START value is smaller than default value, this error is toggled.
	PERIOD_SHORT_ERR = 0x0004, 			// Read only. When period register setting value is smaller than pattern play cycle, this error is toggled.
	ODD_ADDR_ERR = 0x0002, 					// Read only. When memory pattern play is not even in length in trigger delay mode, this error flag is toggled.
	MEM_READ_ERR = 0x0001 					// Read only. When there is a memory read conflict, this error flag is toggled.
} CFG_ERROR_Reg;
*/
#endif // _AD9106_R_H_
