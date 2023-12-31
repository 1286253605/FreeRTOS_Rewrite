/*
 * @Author: Banned 1286253605@qq.com
 * @Date: 2023-07-20 13:46:54
 * @LastEditors: banned 1286253605@qq.com
 * @LastEditTime: 2023-10-11 23:42:44
 * @FilePath: \FreeRTOS_Rewrite\FreeRTOS\include\portmacro.h
 * @Description: 
 * 
 * Copyright (c) 2023 by ${git_name_email}, All Rights Reserved. 
 */

#ifndef PORTMACRO_H
#define PORTMACRO_H

#include "stdint.h"
#include "stddef.h"

// 数据类型重定义
#define portCHAR        char
#define portFLOAT       float
#define portDOUBLE      double
#define portLONG        long
#define portSHORT       short
#define portSTACK_TYPE  uint32_t
#define portBASE_TYPE   long            // 32位 long 和 int 一样是4字节

#define portFORCE_INLINE __forceinline

// ? 放这里?
void vPortEnterCritical( void );

typedef portSTACK_TYPE StackType_t;
typedef long BaseType_t;
typedef unsigned long UBaseType_t;

#if( configUSE_16_BIT_TICKS == 1 )
    typedef uint16_t TickType_t;
    #define portMAX_DELAY ( TickType_t ) 0xffff
#else
    typedef uint32_t TickType_t;
    /* 8个f 4x8=32 32个1 */
    #define portMAX_DELAY ( TickType_t ) 0xffffffffUL
#endif

#define portNVIC_INT_CTRL_REG           ( *( ( volatile uint32_t *)0xe000ed04 ) )   
#define portNVIC_PENDSVSET_BIT          ( 1UL << 28UL )

#define portSY_FULL_READ_WRITE          ( 15 )

// portYIELD的原理就是， 将PendSV的寄存器位置1， 等到所有中断执行完之后就调用PendSV中断执行上下文切换
#define portYIELD()                                     \
{                                                       \
    /* 触发PendSV， 产生上下文切换，写寄存器位为1 */          \
    portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;     \
    __dsb( portSY_FULL_READ_WRITE );                    \
    __isb( portSY_FULL_READ_WRITE );                    \
}

/* 临界段 */
#define portDISABLE_INTERRUPTS vPortRaiseBASEPRI()


void portFORCE_INLINE vPortRaiseBASEPRI( void )
{
    uint32_t ulNewBASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY;
    __asm
    {
        msr basepri, ulNewBASEPRI
        dsb
        isb
    }
}


/* 带返回值的可嵌套的关中断 */
#define portSET_INTERRUPT_MASK_FROM_ISR() ulPortRaiseBASEPRI()

static portFORCE_INLINE  uint32_t ulPortRaiseBASEPRI( void )
{
uint32_t ulReturn, ulNewBASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY;
/* mrs保存basepri寄存器的值 再给其赋新的值 返回刚才暂存的值 */
	__asm
	{
		mrs ulReturn, basepri
		msr basepri, ulNewBASEPRI
		dsb
		isb
	}

	return ulReturn;
}

/* 开中断 */
/* 不带中断保护的开中断 */
#define portENABLE_INTERRUPTS() vPortSetCLEARBASEPRI( 0 )

/* 带中断保护的开中断函数, 这是会被其他函数调用的宏定义 实现嵌套功能 */
#define portCLEAR_INTERRUPT_MASK_FROM_ISR( x ) vPortSetCLEARBASEPRI( x )

void portFORCE_INLINE  vPortSetCLEARBASEPRI( uint32_t ulBASEPRI )
{
    __asm
    {
        msr basepri, ulBASEPRI
    }
}

void portFORCE_INLINE vPortClearBASEPRIFromISR( void )
{
    __asm
    {
        msr basepri, #0
    }
    
}

#define portENTER_CRITICAL() vPortEnterCritical()

#define portDISABLE_INTERRUPTS() vPortRaiseBASEPRI()

/* 退出临界段 */
#define protEXIT_CRITICAL() vPortExitCritical()

/* 多优先级 */
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1

/* 根据传入的形参 也就是任务优先级 将变量 uxReadyPriorities 的某一位置1,  */
/* uxReadyPriorities 对应task.c 文件中的 uxTopReadyPriority  */
#define portRECORD_READY_PRIORITY( uxPriority, uxReadyPriorities )  ( uxReadyPriorities ) |=  ( 1UL << ( uxPriority ) )
#define portRESET_READY_PRIORITY( uxPriority, uxReadyPriorities )   ( uxReadyPriorities ) &= ~( 1UL << ( uxPriority ) )
#if 0
#define portGET_HIGHEST_PRIORITY( uxTopPriority, uxReadyPriorities ) uxTopPriority = ( 31UL - ( uint32_t )__clz( ( uxReadyPriorities ) ) )
#endif

#if 1
    #define portGET_HIGHEST_PRIORITY( uxTopPriority, uxReadyPriorities ) uxTopPriority = ( 31UL - ( uint32_t ) __clz( ( uxReadyPriorities ) ) )

#endif
#endif /* PORTMACRO_H */

