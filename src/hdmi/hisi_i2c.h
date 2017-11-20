#ifndef __HISI_I2C_H__
#define __HISI_I2C_H__

extern int hisi_i2c_read (unsigned char IICAddr, unsigned char ByteAddr, unsigned char *Data, unsigned int Size);
extern int hisi_i2c_write(unsigned char IICAddr, unsigned char ByteAddr, unsigned char *Data, unsigned int Size);

#endif
