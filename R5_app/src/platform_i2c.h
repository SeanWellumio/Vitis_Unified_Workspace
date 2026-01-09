/* platform_i2c.h */
#ifndef PLATFORM_I2C_H
#define PLATFORM_I2C_H
#include <stdint.h>

typedef struct {
    int initialized;   /* 0 = no, 1 = yes */
    /* you can add fields here if you want to return a handle */
} platform_i2c_handle_t;

/* Initialize platform I2C. Call once from main() before modules use it.
   xiic_baseaddr: base address of AXI IIC instance (or device id)
   use_interrupts: true -> configure XIic interrupts; false -> polled API
*/
// int platform_i2c_init(uint32_t xiic_baseaddr, int use_interrupts);
int platform_i2c_init(uint32_t axi_iic_device_id);


/* deinit if needed */
void platform_i2c_deinit(void);

/* simple blocking write (modules call this) */
int platform_i2c_write(uint8_t dev_addr, uint8_t reg, const uint8_t *data, uint8_t len);

/* simple blocking read (modules call this) */
int platform_i2c_read(uint8_t addr7, uint8_t reg, uint16_t *out);

int platform_i2c_write_mux(uint8_t bus); /* select I2C MUX channel */

int platform_i2c_read_mux(uint8_t *out);

#endif
