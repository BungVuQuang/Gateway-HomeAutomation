#include "IrHandle.h"
static const char *TAG = "IRHandle";
/*========================IR TX HANDLE===========================*/
const rmt_channel_t RMT_CHANNEL = RMT_CHANNEL_1;
const gpio_num_t IR_PIN = GPIO_NUM_5; // Using GPIO 5 of ESP32
const int RMT_TX_CARRIER_EN = 1;      // Enable carrier for IR transmitter test with IR led
/*========================IR RX HANDLE===========================*/
ir_parser_t *ir_parser = NULL;
uint32_t addr = 0;
bool repeat = false;
RingbufHandle_t rb = NULL;

void IRAM_ATTR rmt_isr_handler(void *arg)
{
    // read RMT interrupt status.
    uint32_t intr_st = RMT.int_st.val;

    RMT.conf_ch[0].conf1.rx_en = 0;
    RMT.conf_ch[0].conf1.mem_owner = RMT_MEM_OWNER_TX;
    volatile rmt_item32_t *item = RMTMEM.chan[0].data32;

    if (item)
    {

        if (ir_parser->input(ir_parser, item, 34) == ESP_OK)
        {
            if (ir_parser->get_scan_code(ir_parser, &addr, &cmd, &repeat) == ESP_OK)
            {
                xEventGroupSetBits(g_control_event_group, INFRATE_RECIVE_MESSAGE);
                // ESP_LOGI(TAG, "addr: 0x%04x cmd: 0x%04x  lenght: %d", addr, cmd, 34);
            }
        }
    };

    RMT.conf_ch[0].conf1.mem_wr_rst = 1;
    RMT.conf_ch[0].conf1.mem_owner = RMT_MEM_OWNER_RX;
    RMT.conf_ch[0].conf1.rx_en = 1;

    // clear RMT interrupt status.
    RMT.int_clr.val = intr_st;
}

void rmt_rx_config(void)
{
    rmt_config_t rmt_rx_config = RMT_DEFAULT_CONFIG_RX(23, RMT_CHANNEL_0);
    rmt_config(&rmt_rx_config);
    rmt_set_rx_intr_en(RMT_CHANNEL_0, true);
    rmt_isr_register(rmt_isr_handler, NULL, ESP_INTR_FLAG_IRAM, &xHandler);
    rmt_driver_install(RMT_CHANNEL_0, 3000, 0);
    ir_parser_config_t ir_parser_config = IR_PARSER_DEFAULT_CONFIG((ir_dev_t)RMT_CHANNEL_0);
    ir_parser_config.flags |= IR_TOOLS_FLAGS_PROTO_EXT; // Using extended IR protocols (both NEC and RC5 have extended version)
    ir_parser = ir_parser_rmt_new_nec(&ir_parser_config);
    rmt_get_ringbuf_handle(RMT_CHANNEL_0, &rb);
    rmt_rx_start(RMT_CHANNEL_0, true);
}

void rmt_tx_config(void)
{
    // put your setup code here, to run once:
    rmt_config_t rmtConfig;

    rmtConfig.rmt_mode = RMT_MODE_TX; // transmit mode
    rmtConfig.channel = RMT_CHANNEL;  // channel to use 0 - 7
    rmtConfig.clk_div = 80;           // clock divider 1 - 255. source clock is 80MHz -> 80MHz/80 = 1MHz -> 1 tick = 1 us
    rmtConfig.gpio_num = IR_PIN;      // pin to use
    rmtConfig.mem_block_num = 2;      // memory block size

    rmtConfig.tx_config.loop_en = 0;                            // no loop
    rmtConfig.tx_config.carrier_freq_hz = 36000;                // 36kHz carrier frequency
    rmtConfig.tx_config.carrier_duty_percent = 33;              // duty
    rmtConfig.tx_config.carrier_level = RMT_CARRIER_LEVEL_HIGH; // carrier level
    rmtConfig.tx_config.carrier_en = RMT_TX_CARRIER_EN;         // carrier enable
    rmtConfig.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;        // signal level at idle
    rmtConfig.tx_config.idle_output_en = true;                  // output if idle

    rmt_config(&rmtConfig);
    rmt_driver_install(rmtConfig.channel, 0, 0);
}

void rmt_tx_sendData(int *rtmData, int lenght)
{
    int sizeData = lenght;
    rmt_item32_t sendDataRmt[sizeData]; // Data to send the Ac turn On

    for (int i = 0, t = 0; i < (sizeData)-1; i += 2, t++)
    {
        // Odd bit High
        sendDataRmt[t].duration0 = rtmData[i];
        sendDataRmt[t].level0 = 1;

        sendDataRmt[t].duration1 = rtmData[i + 1];
        sendDataRmt[t].level1 = 0;
    }
    ESP_LOGI(TAG, "SEND RMT DATA");
    rmt_write_items(RMT_CHANNEL, sendDataRmt, sizeData, false);
}
