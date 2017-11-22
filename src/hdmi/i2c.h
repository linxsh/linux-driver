#ifndef __I2C_H__
#define __I2C_H__

extern int i2c_get_reg(unsigned int *regs, unsigned int *size);
extern int i2c_set_reg(unsigned int regs);
extern int i2c_init_reg(void);
extern int i2c_read (unsigned char IICAddr, unsigned char ByteAddr, unsigned char *Data, unsigned int Size);
extern int i2c_write(unsigned char IICAddr, unsigned char ByteAddr, unsigned char *Data, unsigned int Size);

#endif
