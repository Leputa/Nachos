// progtest.cc
//	Test routines for demonstrating that Nachos can load
//	a user program and execute it.
//
//	Also, routines for testing the Console hardware device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "console.h"
#include "addrspace.h"
#include "synch.h"

//----------------------------------------------------------------------
// StartProcess
// 	Run a user program.  Open the executable, load it into
//	memory, and jump to it.
//----------------------------------------------------------------------
/*******************  I hava change here **********************/

void
StartProcess2(char *filename)
{
    OpenFile *executable = fileSystem->Open(filename); //打开相应文件
    AddrSpace *space;

    if (executable == NULL) {
        printf("Unable to open file %s\n", filename);
        return;
    }
    space = new AddrSpace(executable);    //建立用户空间、装载文件、初始化用户寄存器
    currentThread->space = space;

    delete executable;			// close file

    space->InitRegisters();		// set the initial register values
    space->RestoreState();		// load page table register
    if(testTag==5)
        currentThread->Suspend();
    machine->Run();			// jump to the user progam
    ASSERT(FALSE);			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}


void testMulThreads(char *filename){
    printf("%s is running\n",currentThread->getName());
    StartProcess2(filename);
}

void testSuspendThread(char *filename){
    printf("%s is running\n",currentThread->getName());
    StartProcess2(filename);
}

void StartPTEPageFaultProcess(char *filename){
    printf("Running PTEPageFaultTest()...\n");
    Thread *t1 = new Thread("thread 1");
    Thread *t2 = new Thread("thread 2");
    Thread *t3 = new Thread("thread 3");
    Thread *t4 = new Thread("thread 4");
    Thread *t5 = new Thread("thread 5");

    t1->Fork(testMulThreads,filename);
    t2->Fork(testMulThreads,filename);
    t3->Fork(testMulThreads,filename);
    t4->Fork(testMulThreads,filename);
    t5->Fork(testMulThreads,filename);
}

void StartMulThreadsProcess(char *filename){
    printf("Running MulThreadTest()...\n");
    Thread *t1 = new Thread("thread 1");
    Thread *t2 = new Thread("thread 2");
    Thread *t3 = new Thread("thread 3");

    t1->Fork(testMulThreads,filename);
    t2->Fork(testMulThreads,filename);
    t3->Fork(testMulThreads,filename);
}

void StartSuspendThreadProcess(char *filename){
    Thread *t1 = new Thread("thread 1");
    Thread *t2 = new Thread("thread 2");
    Thread *t3 = new Thread("thread 3");

    t1->Fork(testSuspendThread,filename);
    t2->Fork(testSuspendThread,filename);
    t3->Fork(testSuspendThread,filename);
}

/***************************  end  ***************************/

void
StartProcess(char *filename)
{
    /*******************  I hava change here **********************/
    printf("if you would use FIFO,please input '1';if you would use LRU,please input '2': ");
    scanf("%d",&pageSwapPolicy);
    printf("if you'd like to test lab4 Exercise3 'TLB PageFault',please input '1':\n");
    printf("if you'd like to test lab4 Exercise4 'BitMap',please input '2':\n");
    printf("if you'd like to test lab4 Exercise5 'MulThreads for memory',please input '3':\n");
    printf("if you'd like to test lab4 Exercise6 'PTE PageFault',please input '4':\n");
    printf("if you'd like to test lab4 Challenge1 'Suspend',please input '5':\n");
    scanf("%d",&testTag);

    if(testTag==3){
        StartMulThreadsProcess(filename);
    }

    if(testTag==4){
        StartPTEPageFaultProcess(filename);
    }
    if(testTag==5){
        StartSuspendThreadProcess(filename);
    }
    /***************************  end  ***************************/
    OpenFile *executable = fileSystem->Open(filename); //打开相应文件
    AddrSpace *space;

    if (executable == NULL) {
        printf("Unable to open file %s\n", filename);
        return;
    }
    space = new AddrSpace(executable);    //建立用户空间、装载文件、初始化用户寄存器
    currentThread->space = space;

    delete executable;			// close file

    space->InitRegisters();		// set the initial register values
    space->RestoreState();		// load page table register
    machine->Run();			// jump to the user progam

    ASSERT(FALSE);			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}



// Data structures needed for the console test.  Threads making
// I/O requests wait on a Semaphore to delay until the I/O completes.

static Console *console;
static Semaphore *readAvail;
static Semaphore *writeDone;

//----------------------------------------------------------------------
// ConsoleInterruptHandlers
// 	Wake up the thread that requested the I/O.
//----------------------------------------------------------------------

static void ReadAvail(int arg) { readAvail->V(); }
static void WriteDone(int arg) { writeDone->V(); }

//----------------------------------------------------------------------
// ConsoleTest
// 	Test the console by echoing characters typed at the input onto
//	the output.  Stop when the user types a 'q'.
//----------------------------------------------------------------------

void
ConsoleTest (char *in, char *out)
{
    char ch;

    console = new Console(in, out, ReadAvail, WriteDone, 0);
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);

    for (;;) {
	readAvail->P();		// wait for character to arrive
	ch = console->GetChar();
	console->PutChar(ch);	// echo it!
	writeDone->P() ;        // wait for write to finish
	if (ch == 'q') return;  // if q, quit
    }
}
