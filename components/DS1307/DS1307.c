#include "DS1307.h"
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <esp_log.h>

#define TAG "I2CDEV"
#define CHECK_ARG(ARG)                  \
    do                                  \
    {                                   \
        if (!ARG)                       \
            return ESP_ERR_INVALID_ARG; \
    } while (0)

esp_err_t i2c_dev_read_reg(const i2c_dev_t *dev, uint8_t reg,
                           void *in_data, size_t in_size)
{
    return i2c_dev_read(dev, &reg, 1, in_data, in_size);
}

esp_err_t i2c_dev_write_reg(const i2c_dev_t *dev, uint8_t reg,
                            const void *out_data, size_t out_size)
{
    return i2c_dev_write(dev, &reg, 1, out_data, out_size);
}

esp_err_t i2c_master_init(i2c_port_t port, int sda, int scl)
{
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda,
        .scl_io_num = scl,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000};
    i2c_param_config(port, &i2c_config);
    i2c_driver_install(port, I2C_MODE_MASTER, 0, 0, 0);
    return ESP_OK;
}

esp_err_t i2c_dev_read(const i2c_dev_t *dev, const void *out_data, size_t out_size, void *in_data, size_t in_size)
{
    if (!dev || !in_data || !in_size)
        return ESP_ERR_INVALID_ARG;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (out_data && out_size)
    {
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (dev->addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write(cmd, (void *)out_data, out_size, true);
    }
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev->addr << 1) | 1, true);
    i2c_master_read(cmd, in_data, in_size, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    esp_err_t res = i2c_master_cmd_begin(dev->port, cmd, I2CDEV_TIMEOUT / portTICK_PERIOD_MS);
    if (res != ESP_OK)
        ESP_LOGE(TAG, "Could not read from device [0x%02x at %d]: %d", dev->addr, dev->port, res);
    i2c_cmd_link_delete(cmd);

    return res;
}

esp_err_t i2c_dev_write(const i2c_dev_t *dev, const void *out_reg, size_t out_reg_size, const void *out_data, size_t out_size)
{
    if (!dev || !out_data || !out_size)
        return ESP_ERR_INVALID_ARG;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev->addr << 1) | I2C_MASTER_WRITE, true);
    if (out_reg && out_reg_size)
        i2c_master_write(cmd, (void *)out_reg, out_reg_size, true);
    i2c_master_write(cmd, (void *)out_data, out_size, true);
    i2c_master_stop(cmd);
    esp_err_t res = i2c_master_cmd_begin(dev->port, cmd, I2CDEV_TIMEOUT / portTICK_PERIOD_MS);
    if (res != ESP_OK)
        ESP_LOGE(TAG, "Could not write to device [0x%02x at %d]: %d", dev->addr, dev->port, res);
    i2c_cmd_link_delete(cmd);

    return res;
}

uint8_t bcd2dec(uint8_t val)
{
    return (val >> 4) * 10 + (val & 0x0f);
}

uint8_t dec2bcd(uint8_t val)
{
    return ((val / 10) << 4) + (val % 10);
}

esp_err_t update_register(i2c_dev_t *dev, uint8_t reg, uint8_t mask, uint8_t val)
{
    CHECK_ARG(dev);

    uint8_t old;

    i2c_dev_read_reg(dev, reg, &old, 1);
    uint8_t buf = (old & mask) | val;
    esp_err_t res = i2c_dev_write_reg(dev, reg, &buf, 1);

    return res;
}

esp_err_t ds1307_init_desc(i2c_dev_t *dev, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio)
{
    CHECK_ARG(dev);

    dev->port = port;
    dev->addr = DS1307_ADDR;
    dev->sda_io_num = sda_gpio;
    dev->scl_io_num = scl_gpio;
    dev->clk_speed = I2C_FREQ_HZ;
    i2c_master_init(port, sda_gpio, scl_gpio);
    return ESP_OK;
}

esp_err_t ds1307_start(i2c_dev_t *dev, bool start)
{
    return update_register(dev, TIME_REG, CH_MASK, start ? 0 : CH_BIT);
}

esp_err_t ds1307_is_running(i2c_dev_t *dev, bool *running)
{
    CHECK_ARG(dev);
    CHECK_ARG(running);

    uint8_t val;

    i2c_dev_read_reg(dev, TIME_REG, &val, 1);

    *running = val & CH_BIT;

    return ESP_OK;
}

esp_err_t ds1307_get_time(i2c_dev_t *dev, struct tm *time)
{
    CHECK_ARG(dev);
    CHECK_ARG(time);

    uint8_t buf[7];

    i2c_dev_read_reg(dev, TIME_REG, buf, 7);

    time->tm_sec = bcd2dec(buf[0] & SECONDS_MASK);
    time->tm_min = bcd2dec(buf[1]);
    if (buf[2] & HOUR12_BIT)
    {
        // RTC in 12-hour mode
        time->tm_hour = bcd2dec(buf[2] & HOUR12_MASK) - 1;
        if (buf[2] & PM_BIT)
            time->tm_hour += 12;
    }
    else
        time->tm_hour = bcd2dec(buf[2] & HOUR24_MASK);
    time->tm_wday = bcd2dec(buf[3]) - 1;
    time->tm_mday = bcd2dec(buf[4]);
    time->tm_mon = bcd2dec(buf[5]) - 1;
    time->tm_year = bcd2dec(buf[6]) + 2000;

    return ESP_OK;
}

esp_err_t ds1307_set_time(i2c_dev_t *dev, const struct tm *time)
{
    CHECK_ARG(dev);
    CHECK_ARG(time);

    uint8_t buf[7] = {
        dec2bcd(time->tm_sec),
        dec2bcd(time->tm_min),
        dec2bcd(time->tm_hour),
        dec2bcd(time->tm_wday + 1),
        dec2bcd(time->tm_mday),
        dec2bcd(time->tm_mon + 1),
        dec2bcd(time->tm_year - 2000)};

    i2c_dev_write_reg(dev, TIME_REG, buf, sizeof(buf));

    return ESP_OK;
}