#include "EPD_driver.h"

EPDDriverParams_t initEPDDriver(gpio_num_t BUSY, gpio_num_t RST, gpio_num_t DC, gpio_num_t CS, gpio_num_t MISO, gpio_num_t SCK)
{
    esp_err_t ret;
  
    EPDPins_t epdTmp = {
        .BUSY = BUSY,
        .RST = RST,
        .DC = DC,
        .CS = CS,
        .MISO = MISO,
        .SCK = SCK,
    };

    //Devo inizializzare i pin
    gpio_set_direction(epdTmp.BUSY, GPIO_MODE_INPUT);
    gpio_set_pull_mode(epdTmp.BUSY, GPIO_PULLUP_ONLY);//se il pin è alto EPD è busy
    gpio_set_direction(epdTmp.RST, GPIO_MODE_OUTPUT);
    gpio_set_level(epdTmp.RST, 1);
    gpio_set_direction(epdTmp.DC, GPIO_MODE_OUTPUT);
    gpio_set_level(epdTmp.DC, 1);

    //Devo inizializzare SPI

    spi_bus_config_t spiCfg = {
        .mosi_io_num = epdTmp.MISO,
        .miso_io_num = -1,
        .sclk_io_num = epdTmp.SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 8,
        .flags = SPICOMMON_BUSFLAG_MASTER | SPICOMMON_BUSFLAG_SCLK | SPICOMMON_BUSFLAG_MOSI
    };

    ret = spi_bus_initialize(VSPI_HOST, &spiCfg, 0);

    if(ret != ESP_OK) printf("Fail to initialize spi master with error %d\r\n", ret);

    //comunico al SPI che c'è un device connesso
    
    spi_device_interface_config_t spiDeviceCfg = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .mode = 0,
        .duty_cycle_pos = 128,
        .cs_ena_pretrans = 0, //indica quanto prima attivare CS
        .cs_ena_posttrans = 0, //indica quanto dopo mantenere CS
        .clock_speed_hz = (APB_CLK_FREQ/80),//4MHz
        .input_delay_ns = 0,
        .spics_io_num = epdTmp.CS,
        .flags = SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_NO_DUMMY | SPI_TRANS_USE_TXDATA,
        .queue_size = 1,
        .pre_cb = NULL,
        .post_cb = NULL
    };

    spi_device_handle_t spiDevice;

    ret = spi_bus_add_device(VSPI_HOST, &spiDeviceCfg, &spiDevice);

    if(ret != ESP_OK) printf("Fail to configure spi device with error %d\r\n", ret);

    // ====================================================================================================================


	vTaskDelay(500 / portTICK_RATE_MS);
	printf("\r\n=================================\r\n");
    printf("ePaper display DEMO library\r\n");
	printf("=================================\r\n\r\n");

	// ==================================================================
	// ==== Initialize the SPI bus and attach the EPD to the SPI bus ====

	printf("SPI: display device added to spi bus\r\n");
    EPDDriverParams_t epdDriverTmp = {
        .EPDPins = epdTmp,
        .handle = spiDevice
    };
    return epdDriverTmp;
}

void EPDResetbyPIN(gpio_num_t resetPIN)
{
    printf("EPDResetbyPIN: start reset on PIN %d\r\n", resetPIN);
    gpio_set_level(resetPIN, 0);
    vTaskDelay(1000 / portTICK_RATE_MS);
    gpio_set_level(resetPIN, 1);
    printf("EPDResetbyPIN: end reset\r\n");
}

void EPDReset(EPDDriverParams_t *driver)
{
    printf("EPDReset: start reset on PIN %d\r\n", driver->EPDPins.RST);
    EPDResetbyPIN(driver->EPDPins.RST);
}

bool isBusy(gpio_num_t busyPIN)
{
    for(int i = 0; i < 4; i++) //TODO: settare correttamente retry e tempistiche
    {
        if(gpio_get_level(busyPIN) == 1)
        {
            return false;
        }
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
    return true;
}

// Send one byte to display
//-------------------------------------
void IRAM_ATTR SPI_Write(uint8_t value, EPDDriverParams_t *driver)
{
    esp_err_t ret;

    spi_device_handle_t disp_spi = (spi_device_handle_t)driver->handle;
	//spi_device_acquire_bus(disp_spi, portMAX_DELAY);
    //creo la transazione da spedire
    spi_transaction_t valueToSend = 
    {
        .flags = SPI_TRANS_USE_TXDATA,
        .cmd = 0,
        .addr = 0,
        .length = 8,
        .rxlength = 0,
        .user = NULL,
        .tx_data = {value}
    };
    ret = spi_device_polling_transmit(disp_spi, &valueToSend);
    //spi_device_release_bus(disp_spi);
    if(ret != ESP_OK) printf("Fail to send SPI data to device with error %d\r\n", ret);
}

void EPDBoosterSoftStart(EPDDriverParams_t *driver, softStartPeriod_t periodA, drivingStrength_t strengthA, maximumOffTimeGDT_t offGDRA, softStartPeriod_t periodB, drivingStrength_t strengthB, maximumOffTimeGDT_t offGDRB, drivingStrength_t strengthC, maximumOffTimeGDT_t offGDRC)
{
    printf("EPDBoosterSoftStart: start send DC PIN %d\r\n", driver->EPDPins.DC);
    gpio_set_level(driver->EPDPins.DC, 0);
    SPI_Write(6, driver);
    gpio_set_level(driver->EPDPins.DC, 1);
    SPI_Write(periodA << 6 | strengthA << 3 | offGDRA, driver);
    SPI_Write(periodB << 6 | strengthB << 3 | offGDRB, driver);
    SPI_Write(strengthB << 3 | offGDRB, driver);
    printf("EPDBoosterSoftStart: stop send\r\n");

}

void EPDPowerSetting(EPDDriverParams_t *driver, sourcePower_t VDS_EN, sourcePower_t VDG_EN, VCOMVoltageLEvel_t VCOM_HV, VGHVGL_t VGHL_LV, VDXPowerSelection_t VDH, VDXPowerSelection_t VDL, VDXPowerSelection_t VDHL)
{
    gpio_set_level(driver->EPDPins.DC, 0);
    SPI_Write(1, driver);
    gpio_set_level(driver->EPDPins.DC, 1);
    SPI_Write(VDS_EN << 1 | VDG_EN << 0, driver);
    SPI_Write(VCOM_HV << 2 | VGHL_LV, driver);
    SPI_Write(VDH, driver);
    SPI_Write(VDL, driver);
    SPI_Write(VDHL, driver);
}

void EPDPowerOn(EPDDriverParams_t *driver)
{
    gpio_set_level(driver->EPDPins.DC, 0);
    SPI_Write(4, driver);
}

void EPDPanelSetting(EPDDriverParams_t *driver, resolution_t resolution, RegEN_t registers, BWR_t colorType, gateScanDirection_t scanGate, sourceShiftDirection_t sourceShift, boosterSwitch_t booster, softReset_t reset)
{
    gpio_set_level(driver->EPDPins.DC, 0);
    SPI_Write(0, driver);
    gpio_set_level(driver->EPDPins.DC, 1);
    SPI_Write(resolution << 6 | registers << 5 | colorType << 4 | scanGate << 3 | sourceShift << 2 | booster << 1 | reset, driver);
}

void EPDPLLControl (EPDDriverParams_t *driver, PLLFramerate_t framerate)
{
    gpio_set_level(driver->EPDPins.DC, 0);
    SPI_Write(48, driver);
    gpio_set_level(driver->EPDPins.DC, 1);
    SPI_Write(framerate, driver);
}

void EPDResolution (EPDDriverParams_t *driver, int8_t HRES, int8_t VRESH, int8_t VRESL)
{
    gpio_set_level(driver->EPDPins.DC, 0);
    SPI_Write(97, driver);
    gpio_set_level(driver->EPDPins.DC, 1);
    SPI_Write(HRES << 3, driver);
    SPI_Write(VRESH, driver);
    SPI_Write(VRESL, driver);
}

void EPDVCM_DC(EPDDriverParams_t *driver, VCOM_settings_t VCOM)
{
    gpio_set_level(driver->EPDPins.DC, 0);
    SPI_Write(130, driver);
    gpio_set_level(driver->EPDPins.DC, 1);
    SPI_Write(VCOM, driver);
}

void EPDVCOMDataInerval(EPDDriverParams_t *driver, VBD_DDX_t VBD, VBD_DDX_t DDX, VcomDataInterval_t CDI)
{
    gpio_set_level(driver->EPDPins.DC, 0);
    SPI_Write(80, driver);
    gpio_set_level(driver->EPDPins.DC, 1);
    SPI_Write(VBD << 6 | DDX << 4 | CDI, driver);
}

void EPDRefresh(EPDDriverParams_t *driver)
{
    gpio_set_level(driver->EPDPins.DC, 0);
    SPI_Write(18, driver);
}

void EPDPoweroff(EPDDriverParams_t *driver)
{
    gpio_set_level(driver->EPDPins.DC, 0);
    SPI_Write(2, driver);
}

void EPDDeepSleep(EPDDriverParams_t *driver)
{
    gpio_set_level(driver->EPDPins.DC, 0);
    SPI_Write(7, driver);
    gpio_set_level(driver->EPDPins.DC, 1);
    SPI_Write(165, driver);
}

void EPDStartTransmission1(EPDDriverParams_t *driver, int8_t value[], int32_t length)
{
    gpio_set_level(driver->EPDPins.DC, 0);
    SPI_Write(16, driver);
    gpio_set_level(driver->EPDPins.DC, 1);
    //for(int i = 0; i < length; i++)
    for(int i = 0; i < 4736; i++)
    {
        //SPI_Write(value[i], driver);
        SPI_Write(0, driver);
    }
}

void EPDStartTransmission2(EPDDriverParams_t *driver, int8_t value[], int32_t length)
{
    gpio_set_level(driver->EPDPins.DC, 0);
    SPI_Write(19, driver);
    gpio_set_level(driver->EPDPins.DC, 1);
    //for(int i = 0; i < length; i++)
    for(int i = 0; i < 4736; i++)
    {
        //SPI_Write(value[i], driver);
        SPI_Write(0, driver);
    }
}

void EPDStopTransmission(EPDDriverParams_t *driver, int8_t value)
{
    gpio_set_level(driver->EPDPins.DC, 0);
    SPI_Write(17, driver);
    gpio_set_level(driver->EPDPins.DC, 1);
    SPI_Write(value << 7, driver);
}

int8_t initEPDDisplay(EPDDriverParams_t *driver)
{
    printf("\r\n=================================\r\n");
    printf("EPDPins\r\n");
    printf("- BUSY %d\r\n", driver->EPDPins.BUSY);
    printf("- RST %d\r\n", driver->EPDPins.RST);
    printf("- DC %d\r\n", driver->EPDPins.DC);
    printf("- MISO %d\r\n", driver->EPDPins.MISO);
    printf("- SCK %d\r\n", driver->EPDPins.SCK);
	printf("=================================\r\n\r\n");
    //Reset EPDIC
    //EPDResetbyPIN(driver->EPDPins->RST);
    EPDReset(driver);
    //Booster soft start
    EPDBoosterSoftStart(driver,  _10ms,  strength3,  _6580ns,  _10ms,  strength3,  _6580ns,  strength3,  _6580ns);
    //Power setting 
    EPDPowerSetting(driver, internal, internal, VDHplusVCOMDC, _16V, _110dV, _110dV, _30dV);
    //Power on 
    EPDPowerOn(driver);
    //Check BUSY pin
    if(isBusy(driver->EPDPins.BUSY))
    {
        return -1;
    }
    //Panel setting
    EPDPanelSetting(driver, _128x296, LUT_register, tricolor, up, right, ON, none);
    //PLL control 
    EPDPLLControl (driver, _100Hz);
    //Resolution setting 
    EPDResolution (driver, 16, 1, 40);
    //VCM_DC setting
    EPDVCM_DC(driver, __100dV);
    //Vcom and data interval setting 
    EPDVCOMDataInerval(driver, two, zero, _10);
    //LUT(Lock Up Table)???

    //---QUESTA PARTE SERVE PER INVIARE I DATI DELL'IMMAGINE---//
    //Data start transmission 1 
    int8_t data[100];
    for(int i = 0; i < 100; i++)
    {
        data[i] = 1;
    }
    EPDStartTransmission1(driver, data, 100);
    //  Transport B/W data 
    //Data start transmission 2 
    for(int i = 0; i < 100; i++)
    {
        data[i] = 1;
    }
    EPDStopTransmission(driver, 0);
    EPDStartTransmission2(driver, data, 100);
    EPDStopTransmission(driver, 1);
    //  Transport red data 
    //---FINE INVIO DATI DELL'IMMAGINE---//

    //Display refresh
     EPDRefresh(driver);
    //Check BUSY pin 
    if(isBusy(driver->EPDPins.BUSY))
    {
        return -1;
    }
    //Vcom and data interval setting 
    EPDVCOMDataInerval(driver, zero, zero, _10);
    //Power off
    EPDPoweroff(driver);
    //Deep sleep
    EPDDeepSleep(driver);
    gpio_set_level(driver->EPDPins.DC, 0);
    return 0;
}
