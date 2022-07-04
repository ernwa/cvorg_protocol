#ifndef COMMANDS_H
#define COMMANDS_H


// General Command Range:
#define GET_BOARD_INFO          0x00
#define GET_EPROM_INFO          0x01
//#define FIRMWARE_VERSION		0x01 // Currently not used in IB code, but may be needed later (will figure out a new value when that time comes)

// Interface Board Command Range:
#define DIS_VSE10           	0x0a
#define EN_VSE10            	0x0b
#define DIS_VSS10           	0x0c
#define EN_VSS10            	0x0d
#define UNTRIP_CMD              0x0e // Master IB
#define TRIP_CMD                0x0f // Master IB
#define RECENT_CURRENT_RAW  	0x10 // Master IB get
#define ADC_CAL                 0x11 // Master IB set
#define ADC_VALUE_RAW           0x12 // Master IB get
#define ADC_VALUE_COMP          0x13 // Master IB get
#define ADC_COUNTS2VOLTS        0x14 // Master IB get
#define BREAKER_STATUS          0x23 // Master IB get
#define BREAKER_THRES_VSP       0x24 // Master IB set
#define BREAKER_THRES_VSS       0x25 // Master IB set
#define BREAKER_THRES_VSE       0x27 // Master IB set
#define BREAKER_THRES_ALL       0x28 // Master IB set all
#define HW_BREAKER_VSP_I        0x29 // Master IB get
#define HW_BREAKER_VSE_I        0x2a // Master IB get
#define HW_BREAKER_VSS_I        0x2b // Master IB get
#define HW_BREAKER_VSD5_I       0x2c // Master IB get
#define HW_BREAKER_ALL          0x2d // Master IB get all

#define HW_BREAKER_VOLT     	0x2e //set undervoltage value
#define HW_BREAKER_VOLT_GET 	0x2f //get undervoltage value

#define SET_LCD_TEXT            0x40 // Slave IB
#define ECHO_FLOAT          	0x41
#define FORWARD_PACKET      	0x8d // this is a special case and a command coming from the computer should never be 0x0d, else this will not work
#define DAC                 	0x31
#define pos15V              	0x32
#define neg5V               	0x33
#define neg12V              	0x34
#define neg8V               	0x35
#define RefV                	0x36
#define SLAVE_BUILD_INFO    	0x37
#define START_UP_INFO       	0x38
//Used to specify the data being sent is to be shown on the LCD
#define LCD                 0x30

#define SET_SNAP_PROJECTOR_ON       0x30 // SNAP
#define SET_SNAP_PROJECTOR_OFF      0x31 // SNAP
#define GET_METADATA                0x50 // ALL
#define SET_SNAP_FRAME_RESET_ON     0x51 // SNAP
#define SET_SNAP_FRAME_RESET_OFF    0x52 // SNAP
#define SET_SNAP_ROW_SEL            0x53 // SNAP
#define SET_DAC_MODE                0x54 // ALL
#define SET_SNAP_WINDOW_SEL         0x55 // SNAP
#define SET_SNAP_DIMENSIONS         0x56 // SNAP
#define SET_PDP_WRITE_TICK          0x61 // PDP
#define SET_PDP_RESET_TICK          0x62 // PDP
#define SET_PDP_VSYNC_DAC_TICK      0x63 // PDP
#define GET_PDP_WRITE_TICK          0x64 // PDP
#define GET_PDP_RESET_TICK          0x65 // PDP
#define GET_PDP_VSYNC_DAC_TICK      0x66 // PDP
#define GET_SNAP_TICKS              0x68 // SNAP
#define GET_SNAP_FRAME_RESET        0x69 // SNAP
#define GET_PRBS_INFO               0x70 // PRBS
#define GET_HW_ID                   0x71 // ALL
#define GET_EEPROM_INFO             0x72 // ALL
#define SET_EEPROM_INFO             0x73 // ALL
#define SET_MOUT_MODE               0x74 // ALL
#define SET_PDP_AXI_MODE            0x75 // PDP
#define SET_PDP_AXI_MODE_DIMENSIONS 0x76 // PDP
#define SET_PDP_RESET_MODE          0x77 // PDP
#define GET_PDP_AXI_MODE            0x78 // PDP
#define GET_PDP_AXI_MODE_DIMENSIONS 0x79 // PDP
#define GET_SYSTEM                  0x7A // PDP
#define SET_SYSTEM                  0x7B // PDP
#define GET_PDP_DAC_SHIFT           0x7C // PDP
#define SET_PDP_DAC_SHIFT           0x7D // PDP
#define GET_PDP_VSYNC_TICKS         0x41 // PDP
#define GET_PDP_STATE               0x42 // PDP
#define SET_PDP_AXI_MODE_RESET      0x43 // PDP
#define SET_PDP_ERROR_CLEAR         0x44 // PDP

#endif /* COMMANDS_H */
