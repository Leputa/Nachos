// system.h
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H
/********************  Here is my codes ***********************/
#define MaxThread 128
#include "bitmap.h"
/***************************  end  ***************************/

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"

/********************  Here is my codes ***********************/
extern int thread_id_flag[MaxThread];  //使用"0""1"来标识进程状态
extern Thread* tid_pointer[MaxThread];       //线程指针
extern int policy; //线程调度策略 1代表基于优先级的抢占式调度，2代表时间片轮转调度
extern int pageSwapPolicy;//页面调度策略，1代表FIFO,2代表LRU
extern int tlbHit;
extern int tlbUnHit;
extern BitMap *bitmap;
/***************************  end  ***************************/

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock

#ifdef USER_PROGRAM
#include "machine.h"
extern Machine* machine;	// user program memory and registers
#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk   *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;
#endif

#endif // SYSTEM_H
