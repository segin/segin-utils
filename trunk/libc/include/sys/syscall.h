/* sys/sycall.h: Syscall inteface */

#ifndef __SYS_SYSCALL_H__
#define __SYS_SYSCALL_H__

int syscall(int syscall, ...);


#define	sys_nosys	0
#define	sys_exit	1
#define	sys_fork	2
#define	sys_read	3
#define	sys_write	4
#define	sys_open	5
#define	sys_close	6


#endif /* __SYS_SYSCALL_H__ */
