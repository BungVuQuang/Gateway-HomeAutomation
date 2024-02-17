#ifndef _DS1307_H_
#define _DS1307_H_

#include "driver/i2c.h"
#include <time.h>

#define I2C_FREQ_HZ 400000
#define I2CDEV_TIMEOUT 1000
#define DS1307_ADDR 0x68 //!< I2C address

#define RAM_SIZE 56

#define TIME_REG    0
#define CONTROL_REG 7
#define RAM_REG     8

#define CH_BIT      (1 << 7)
#define HOUR12_BIT  (1 << 6)
#define PM_BIT      (1 << 5)
#define SQWE_BIT    (1 << 4)
#define OUT_BIT     (1 << 7)

#define CH_MASK      0x7f
#define SECONDS_MASK 0x7f
#define HOUR12_MASK  0x1f
#define HOUR24_MASK  0x3f
#define SQWEF_MASK   0x03
#define SQWE_MASK    0xef
#define OUT_MASK     0x7

typedef struct {
    i2c_port_t port;            // I2C port number
    uint8_t addr;               // I2C address
    gpio_num_t sda_io_num;      // GPIO number for I2C sda signal
    gpio_num_t scl_io_num;      // GPIO number for I2C scl signal
        uint32_t clk_speed;             // I2C clock frequency for master mode
} i2c_dev_t;





uint8_t bcd2dec(uint8_t val);
uint8_t dec2bcd(uint8_t val);
esp_err_t update_register(i2c_dev_t *dev, uint8_t reg, uint8_t mask, uint8_t val);
esp_err_t ds1307_init_desc(i2c_dev_t *dev, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio);
esp_err_t ds1307_start(i2c_dev_t *dev, bool start);
esp_err_t ds1307_is_running(i2c_dev_t *dev, bool *running);
esp_err_t ds1307_get_time(i2c_dev_t *dev, struct tm *time);
esp_err_t ds1307_set_time(i2c_dev_t *dev, const struct tm *time);

esp_err_t i2c_master_init(i2c_port_t port, int sda, int scl);
esp_err_t i2c_dev_read(const i2c_dev_t *dev, const void *out_data, size_t out_size, void *in_data, size_t in_size);
esp_err_t i2c_dev_write(const i2c_dev_t *dev, const void *out_reg, size_t out_reg_size, const void *out_data, size_t out_size);
 esp_err_t i2c_dev_read_reg(const i2c_dev_t *dev, uint8_t reg,
        void *in_data, size_t in_size);


 esp_err_t i2c_dev_write_reg(const i2c_dev_t *dev, uint8_t reg,
        const void *out_data, size_t out_size);

#endif /* _DS1307_H_ */
