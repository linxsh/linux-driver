#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/agpgart.h>
#include <linux/err.h>
#include <linux/in.h>
#include <linux/netlink.h>
#include <linux/writeback.h>
#include <linux/sysctl.h>
#include <linux/gfp.h>
#include <linux/i2c.h>
#include "common.h"

void *dr_page_malloc(unsigned long size)
{
	unsigned long real_size = size <= 4096 ? 4096 : size;

	return (void *)__get_free_pages(GFP_KERNEL | __GFP_REPEAT, get_order(real_size));
}

void dr_page_free(void *p, unsigned long size)
{
	unsigned long real_size = size <= 4096 ? 4096 : size;

	free_pages((unsigned long)p, get_order(real_size));
}

int dr_copy_to_user(void *to, const void *from, unsigned int n)
{
	return copy_to_user(to, from, n);
}

int dr_copy_from_user(void *to, const void *from, unsigned int n)
{
	return copy_from_user(to, from, n);
}

#ifdef arm_linux
void dr_cache_sync(const void *start, unsigned int size, int direction)
{
	flush_cache_all();
}

void dr_dcache_inv_range(unsigned int addr, unsigned int size)
{
	void *p = (void *)addr;
	phys_addr_t phys_start = virt_to_phys(p);

	__cpuc_flush_dcache_area(p, size);
	outer_flush_range(phys_start, phys_start + size);
}

void dr_dcache_clean_range(unsigned int addr, unsigned int size)
{
	void *p = (void *)addr;
	phys_addr_t phys_start = virt_to_phys(p);

	__cpuc_flush_dcache_area(p, size);
	outer_flush_range(phys_start, phys_start + size);
}

void dr_dcache_flush_range(unsigned int start, unsigned int end)
{
	unsigned int size = end - start;
	dr_cache_sync((void*)start, size, DMA_BIDIRECTIONAL);
}
#endif

void *dr_wait_queue_create(void)
{
	wait_queue_head_t *wq = dr_malloc(sizeof(wait_queue_head_t));

	init_waitqueue_head(wq);
	return wq;
}

void dr_wait_queue_destory(void *queue)
{
	wait_queue_head_t *wq = (wait_queue_head_t *)queue;

	if (wq) {
		init_waitqueue_head(wq);
		dr_free(wq);
	}
}

int dr_queue_wait(void *queue, int timeout_us)
{
	int ret = 0;
	wait_queue_head_t *wq = (wait_queue_head_t *)queue;

	if (wq == NULL) {
		return -EFAULT;
	}

	if (timeout_us >= 0) {
		ret = wait_event_interruptible_timeout(*wq, 0, msecs_to_jiffies(timeout_us/1000));
		if (ret <= 0) {
			return -EFAULT;
		}
	} else
		wait_event_interruptible(*wq, 0);

	return 0;
}

int dr_queue_wake(void *queue)
{
	wait_queue_head_t *wq = (wait_queue_head_t *)queue;

	if (wq == NULL) {
		return -EFAULT;
	}

	wake_up_interruptible(wq);

	return 0;
}

void *dr_malloc(unsigned int size)
{
	return kmalloc(size, GFP_KERNEL | __GFP_REPEAT);
}

void *dr_mallocz(unsigned int size)
{
	void *p = kmalloc(size, GFP_KERNEL | __GFP_REPEAT);

	if (p)
		memset(p, 0, size);
	return p;
}

void dr_free(void *ptr)
{
	if (ptr)
		kfree(ptr);
}

unsigned long dr_virt_to_phys(void *addr)
{
	return virt_to_phys(addr);
}

void *dr_phys_to_virt(unsigned long addr)
{
	return phys_to_virt(addr);
}

int dr_sem_create(dr_sem_id *sem_id, unsigned int sem_init_val)
{
	sema_init(sem_id, (int)sem_init_val);

	return 0;
}

int dr_sem_delete(dr_sem_id *sem_id)
{
	return -1;
}

int dr_sem_post(dr_sem_id *sem_id)
{
	up(sem_id);

	return 0;
}

int dr_sem_wait(dr_sem_id *sem_id)
{
	down(sem_id);

	return 0;
}

int dr_sem_wait_timeout(dr_sem_id *sem_id, long timeout)
{
	return down_timeout(sem_id, timeout/10);
}

int dr_sem_trywait(dr_sem_id *sem_id)
{
	return (!down_trylock(sem_id) ? 0 : -1);
}

int dr_thread_create(const char *name, void **id, void(*entryFunc)(void *), void *arg)
{
	struct task_struct *task = NULL;

	task = kthread_create((int (*)(void *))entryFunc, arg, "%s", name);
	if(task == NULL)
		return -1;

	*id = (void*)task;
	wake_up_process(task);

	return 0;
}

int dr_thread_delete(void *id)
{
	struct task_struct *task = (struct task_struct *)id;

	if (task) {
		kthread_stop(task);
		task = NULL;
	}

	return 0;
}

int dr_thread_delay(unsigned int millisecond)
{
	unsigned int ms_loop;
	unsigned int loop;

	ms_loop = millisecond;
	while (ms_loop > 0) {
		loop = (ms_loop <= 999) ? ms_loop : 999;
		msleep(loop);
		ms_loop -= loop;
	}

	return 0;
}

