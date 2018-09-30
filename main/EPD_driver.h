#ifndef __EPD_DRIVER_H_
#define __EPD_DRIVER_H_

#include "esp_event_loop.h"
#include "driver/gpio.h"

#include "driver/spi_master.h"

//struttura per tenere traccia di tutti i pin usati

typedef struct EPDPins
{
    gpio_num_t BUSY;
    gpio_num_t RST;
    gpio_num_t DC;
    gpio_num_t CS;
    gpio_num_t MISO;//DIN
    gpio_num_t SCK;//CLK
} EPDPins_t;

typedef struct EPDDriverParams
{
    EPDPins_t EPDPins;
    spi_device_handle_t handle;
} EPDDriverParams_t;

typedef enum 
{
    _10ms = 0b00,
    _20ms = 0b01,
    _30ms = 0b10,
    _40ms = 0b11
} softStartPeriod_t;

typedef enum 
{
    strength1 = 0b000,
    strength2 = 0b001,
    strength3 = 0b010,
    strength4 = 0b011,
    strength5 = 0b100,
    strength6 = 0b101,
    strength7 = 0b110,
    strength8 = 0b111
} drivingStrength_t;

typedef enum 
{
    _270ns = 0b000,
    _340ns = 0b001,
    _400ns = 0b010,
    _540ns = 0b011,
    _800ns = 0b100,
    _1540ns = 0b101,
    _3340ns = 0b110,
    _6580ns = 0b111
} maximumOffTimeGDT_t;

typedef enum
{
    external = 0b0,
    internal = 0b1
} sourcePower_t;

typedef enum
{
    VDHplusVCOMDC = 0b00,
    VGH = 0b01
} VCOMVoltageLEvel_t;

typedef enum
{
    _16V = 0b00,
    _15V = 0b01,
    _14V = 0b10,
    _13V = 0b11
} VGHVGL_t;

typedef enum
{
    _24dV = 0b000000,
    _26dV = 0b000001,
    _28dV = 0b000010,
    _30dV = 0b000011,
    _32dV = 0b000100,
    _34dV = 0b000101,
    _36dV = 0b000110,
    _38dV = 0b000111,
    _100dV = 0b100110,
    _102dV = 0b100111,
    _104dV = 0b101000,
    _106dV = 0b101001,
    _108dV = 0b101010,
    _110dV = 0b101011

} VDXPowerSelection_t;

typedef enum 
{
    _96x230 = 0b00,
    _96x252 = 0b01,
    _128x296 = 0b10,
    _160x296 = 0b11
} resolution_t;

typedef enum 
{
    LUT_OTP = 0b0,
    LUT_register = 0b1
} RegEN_t;

typedef enum
{
    tricolor = 0b0,
    bicolor = 0b1
} BWR_t;

typedef enum
{
    down = 0b0,
    up = 0b1
} gateScanDirection_t;

typedef enum
{
    left = 0b0,
    right = 0b1
} sourceShiftDirection_t;

typedef enum
{
    OFF = 0b0,
    ON = 0b1
} boosterSwitch_t;

typedef enum
{
    reset = 0b0,
    none = 0b1
} softReset_t;

typedef enum
{
    _200Hz = 0b111001,
    _100Hz = 0b111010,
    _67Hz = 0b111011,
    _50Hz = 0b111100,
    _40Hz = 0b111101,
    _33Hz = 0b111110,
    _29Hz = 0b111111,
    _171Hz = 0b110001,
    _86Hz = 0b110010,
    _57Hz = 0b110011,
    _43Hz = 0b110100,
    _34Hz = 0b110101,
    _24Hz = 0b110111,
    _150Hz = 0b101001,
    _72Hz = 0b101010,
    _48Hz = 0b101011,
    _36Hz = 0b101100,
    _20Hz = 0b101111
    //implementare anche gli altri...
} PLLFramerate_t;

typedef enum 
{
    __10dV = 0b000000,
    __15dV = 0b000001,
    __20dV = 0b000010,
    __25dV = 0b000011,
    __30dV = 0b000100,
    __35dV = 0b000101,
    __40dV = 0b000110,
    __45dV = 0b000111,
    __50dV = 0b001000,
    __55dV = 0b001001,
    __60dV = 0b001010,
    __65dV = 0b001011,
    __70dV = 0b001100,
    __75dV = 0b001101,
    __80dV = 0b001110,
    __85dV = 0b001111,
    __90dV = 0b010000,
    __95dV = 0b010001,
    __100dV = 0b010010,
    __105dV = 0b010011,
    __110dV = 0b010100,
    __115dV = 0b010101,
    __120dV = 0b010110,
    __125dV = 0b010111,
    __130dV = 0b011000,
    __135dV = 0b011001,
    __140dV = 0b011010,
    __145dV = 0b011011,
    __150dV = 0b011100,
    __155dV = 0b011101,
    __160dV = 0b011110,
    __165dV = 0b011111,
    __170dV = 0b100000,
    __175dV = 0b100001,
    __180dV = 0b100010,
    __185dV = 0b100011,
    __190dV = 0b100100,
    __195dV = 0b100101,
    __200dV = 0b100110,
    __205dV = 0b100111,
    __210dV = 0b101000,
    __215dV = 0b101001,
    __220dV = 0b101010,
    __225dV = 0b101011,
    __230dV = 0b101100,
    __235dV = 0b101101,
    __240dV = 0b101110,
    __245dV = 0b101111,
    __250dV = 0b110000,
    __255dV = 0b110001,
    __260dV = 0b110010,
    __265dV = 0b110011,
    __270dV = 0b110100,
    __275dV = 0b110101,
    __280dV = 0b110110,
    __285dV = 0b110111,
    __290dV = 0b111000,
    __295dV = 0b111001,
    __300dV = 0b111010

} VCOM_settings_t;

typedef enum
{
    zero = 0b00,
    one = 0b01,
    two = 0b10,
    three = 0b11
} VBD_DDX_t;

typedef enum 
{
    _17 = 0b0000,
    _16 = 0b0001,
    _15 = 0b0010,
    _14 = 0b0011,
    _13 = 0b0100,
    _12 = 0b0101,
    _11 = 0b0110,
    _10 = 0b0111,
    _9 = 0b1000,
    _8 = 0b1001,
    _7 = 0b1010,
    _6 = 0b1011,
    _5 = 0b1100,
    _4 = 0b1101,
    _3 = 0b1110,
    _2 = 0b1111
} VcomDataInterval_t;

EPDDriverParams_t initEPDDriver(gpio_num_t BUSY, gpio_num_t RST, gpio_num_t DC, gpio_num_t CS, gpio_num_t MISO, gpio_num_t SCK);
void EPDResetbyPIN(gpio_num_t resetPIN);
void EPDReset(EPDDriverParams_t *driver);
void EPDBoosterSoftStart(EPDDriverParams_t *driver,softStartPeriod_t periodA, drivingStrength_t strengthA, maximumOffTimeGDT_t offGDRA, softStartPeriod_t periodB, drivingStrength_t strengthB, maximumOffTimeGDT_t offGDRB, drivingStrength_t strengthC, maximumOffTimeGDT_t offGDRC);
void EPDPowerSetting(EPDDriverParams_t *driver, sourcePower_t VDS_EN, sourcePower_t VDG_EN, VCOMVoltageLEvel_t VCOM_HV, VGHVGL_t VGHL_LV, VDXPowerSelection_t VDH, VDXPowerSelection_t VDL, VDXPowerSelection_t VDHL);
void EPDPowerOn(EPDDriverParams_t *driver);
void EPDPanelSetting(EPDDriverParams_t *driver, resolution_t resolution, RegEN_t registers, BWR_t colorType, gateScanDirection_t scanGate, sourceShiftDirection_t sourceShift, boosterSwitch_t booster, softReset_t reset);
void EPDPLLControl(EPDDriverParams_t *driver, PLLFramerate_t framerate);
///
/// HRES sarà automaticamente << 3
///
void EPDResolution(EPDDriverParams_t *driver, int8_t HRES, int8_t VRESH, int8_t VRESL);
void EPDVCM_DC(EPDDriverParams_t *driver, VCOM_settings_t VCOM);
void EPDVCOMDataInerval(EPDDriverParams_t *driver, VBD_DDX_t VBD, VBD_DDX_t DDX, VcomDataInterval_t CDI);
void EPDRefresh(EPDDriverParams_t *driver);
void EPDPoweroff(EPDDriverParams_t *driver);
void EPDDeepSleep(EPDDriverParams_t *driver);
void EPDStartTransmission1(EPDDriverParams_t *driver, int8_t value[], int32_t length);
void EPDStartTransmission2(EPDDriverParams_t *driver, int8_t value[], int32_t length);
void EPDStopTransmission(EPDDriverParams_t *driver, int8_t value);
bool isBusy(gpio_num_t busyPIN);
int8_t initEPDDisplay(EPDDriverParams_t *driver);


#endif