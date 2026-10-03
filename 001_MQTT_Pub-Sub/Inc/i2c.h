#ifndef I2C_H_
#define I2C_H_
/* PB8=SCL, PB9=SDA, AF4 open-drain, 100 kHz. External pull-ups required.
 * Returns 0 on success, -1 if PCLK1 is unsupported. Initialization only:
 * no START, address probe, sensor read or other transaction is performed. */
int i2c1_init(void);
#endif
