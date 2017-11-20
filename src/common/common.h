#ifndef __COMMON_H__
#define __COMMON_H__

#	include <linux/kernel.h>
#	include <linux/slab.h>
#	include <linux/mm.h>
#	include <linux/ioport.h>
#	include <linux/errno.h>
#	include <linux/vmalloc.h>
#	include <asm/io.h>
#	include <asm/uaccess.h>
#	include <asm/cacheflush.h>
#	include <linux/memory.h>
#	include <linux/interrupt.h>
#	include <linux/delay.h>
#	include <linux/kthread.h>
#	include <asm/div64.h>
#	include <linux/version.h>
#	include <linux/list.h>
#	include <linux/proc_fs.h>
#	include <linux/proc_fs.h>
#	include <linux/time.h>

#ifndef NULL
#define NULL 0
#endif

#	if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28)
#		include <linux/semaphore.h>
#		include <asm/page.h>
#		include <linux/swab.h>
#	elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
#		include <linux/semaphore.h>
#		include <linux/byteorder/swab.h>
#		include <asm-generic/page.h>
#	else
#		include <asm/semaphore.h>
#		include <linux/byteorder/swab.h>
#		include <asm-generic/page.h>
#	endif

extern void* dr_page_malloc(unsigned long size);
extern void  dr_page_free(void *p, unsigned long size);
extern int   dr_copy_to_user(void *to, const void *from, unsigned int n);
extern int   dr_copy_from_user(void *to, const void *from, unsigned int n);
#ifdef arm_linux
extern void  dr_cache_sync(const void *start, unsigned int size, int direction);
extern void  dr_dcache_inv_range(unsigned int addr, unsigned int size);
extern void  dr_dcache_clean_range(unsigned int addr, unsigned int size);
extern void  dr_dcache_flush_range(unsigned int start, unsigned int end);
#endif
extern void* dr_wait_queue_create(void);
extern void  dr_wait_queue_destory(void *queue);
extern int   dr_queue_wait(void *queue, int timeout_us);
extern int   dr_queue_wake(void *queue);
extern void* dr_malloc(unsigned int size);
extern void* dr_mallocz(unsigned int size);
extern void dr_free(void *ptr);
extern unsigned long dr_virt_to_phys(void *addr);
extern void *dr_phys_to_virt(unsigned long addr);

#define dr_memcpy                           memcpy
#define dr_memset                           memset

#define dr_printf(fmt, args...)             printk(fmt, ##args)
#define dr_sprintf(buf, fmt, args...)       sprintf(buf, fmt, ##args)
#define dr_snprintf(buf, len, fmt, args...) snprintf(buf, len, fmt, ##args)
#define dr_sscanf(buf, fmt, args...)        sscanf(buf, fmt, ##args)
#define dr_vprintf(fmt, args)               vprintk(fmt, args)

typedef struct mutex dr_mutex_t;
#define dr_mutex_init(m)                  mutex_init(m)
#define dr_mutex_lock(m)                  mutex_lock(m)
#define dr_mutex_trylock(m)               mutex_trylock(m)
#define dr_mutex_unlock(m)                mutex_unlock(m)
#define dr_mutex_destroy(m)               mutex_destroy(m)

#define dr_ioremap(offset,size)             ioremap(offset,size)
#define dr_iounmap(addr)                    iounmap((void *)(addr))

#define dr_request_mem_region(start,len)    request_mem_region(start,len,"vsp")
#define dr_release_mem_region(start,len)    release_mem_region(start,len)

#define dr_mdelay(n)                     mdelay((n))
#define dr_udelay(n)                     udelay(n)
#define dr_msleep(n)                     msleep((n))

typedef struct semaphore dr_sem_id;

extern int dr_sem_create(dr_sem_id *sem_id, unsigned int sem_init_val);
extern int dr_sem_delete(dr_sem_id *sem_id);
extern int dr_sem_post  (dr_sem_id *sem_id);
extern int dr_sem_wait  (dr_sem_id *sem_id);
extern int dr_sem_wait_timeout (dr_sem_id *sem_id,long timeout);
extern int dr_sem_trywait  (dr_sem_id *sem_id);

extern int dr_thread_create(const char *name, void **id, void(*entryFunc)(void *), void *arg);
extern int dr_thread_delete(void *id);
extern int dr_thread_delay(unsigned int millisecond);
#endif
