#ifndef __EP952_H__
#define __EP952_H__

extern int   ep952CoreInit(void);
extern int   ep952CoreDestory(void);
extern void* ep952CoreOpen(void);
extern int   ep952CoreClose(void* priv);
extern int   ep952CoreIoctl(void* priv, unsigned int cmd, void *arg);

#endif
