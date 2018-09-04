#ifndef _ASM_SEGMENT_H
#define _ASM_SEGMENT_H

// 这里是关于段寄存器初始值的定义
// KERNEL_CS  0x10  0 0 0 0, 0 0 0 0, 0 0 0 1, 0 | 0 | 0 0  index = 2, TI = 0, RPL = 0
// KERNEL_DS  0x18  0 0 0 0, 0 0 0 0, 0 0 0 1, 1 | 0 | 0 0  index = 3, TI = 0, RPL = 0
// USER_CS    0x23  0 0 0 0, 0 0 0 0, 0 0 1 0, 0 | 0 | 1 1  index = 4, TI = 0, RPL = 3
// USER_DS    0x28  0 0 0 0, 0 0 0 0, 0 0 1 0, 1 | 0 | 1 1  index = 5, TI = 0, RPL = 3
//
// 说明一个进程在初始化时，DS ES SS CS四个寄存器都会指向
// GDT中的某个段描述符，权限位有内核态和用户态之分，
//
#define __KERNEL_CS	0x10
#define __KERNEL_DS	0x18

#define __USER_CS	0x23
#define __USER_DS	0x2B

#endif
