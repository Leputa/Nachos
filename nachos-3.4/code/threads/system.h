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
extern int thread_id_flag[MaxThread];  //ʹ��"0""1"����ʶ����״̬
extern Thread* tid_pointer[MaxThread];       //�߳�ָ��
extern int policy; //�̵߳��Ȳ��� 1����������ȼ�����ռʽ���ȣ�2����ʱ��Ƭ��ת����
extern int pageSwapPolicy;//ҳ����Ȳ��ԣ�1����FIFO,2����LRU
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
