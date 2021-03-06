// machine.cc
//	Routines for simulating the execution of user programs.
//
//  DO NOT CHANGE -- part of the machine emulation
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "machine.h"
#include "system.h"

// Textual names of the exceptions that can be generated by user program
// execution, for debugging.
static char* exceptionNames[] = { "no exception", "syscall",
				"page fault/no TLB entry", "page read only",
				"bus error", "address error", "overflow",
				"illegal instruction" };

//----------------------------------------------------------------------
// CheckEndian
// 	Check to be sure that the host really uses the format it says it
//	does, for storing the bytes of an integer.  Stop on error.
//----------------------------------------------------------------------

static
void CheckEndian()
{
    union checkit {
        char charword[4];
        unsigned int intword;
    } check;

    check.charword[0] = 1;
    check.charword[1] = 2;
    check.charword[2] = 3;
    check.charword[3] = 4;

#ifdef HOST_IS_BIG_ENDIAN
    ASSERT (check.intword == 0x01020304);
#else
    ASSERT (check.intword == 0x04030201);
#endif
}

//----------------------------------------------------------------------
// Machine::Machine
// 	Initialize the simulation of user program execution.
//
//	"debug" -- if TRUE, drop into the debugger after each user instruction
//		is executed.
//----------------------------------------------------------------------

Machine::Machine(bool debug)
{
    int i;
    for (i = 0; i < NumTotalRegs; i++)
        registers[i] = 0;
    mainMemory = new char[MemorySize];
    for (i = 0; i < MemorySize; i++)
      	mainMemory[i] = 0;
    //因为不会改makefile,直接把这里改了
//#ifdef USE_TLB
    tlb = new TranslationEntry[TLBSize];
    for (i = 0; i < TLBSize; i++){
        tlb[i].valid = FALSE;
        /*******************  I hava change here **********************/
        tlb[i].createTime=stats->totalTicks;
        tlb[i].lastUseTime=stats->totalTicks;
        /***************************  end  ***************************/
    }
/*
    pageTable = NULL;
#else	// use linear page table
    tlb = NULL;
    pageTable = NULL;
#endif
*/
    singleStep = debug;
    CheckEndian();
}

//----------------------------------------------------------------------
// Machine::~Machine
// 	De-allocate the data structures used to simulate user program execution.
//----------------------------------------------------------------------

Machine::~Machine()
{
    delete [] mainMemory;
    if (tlb != NULL)
        delete [] tlb;
}

//----------------------------------------------------------------------
// Machine::RaiseException
// 	Transfer control to the Nachos kernel from user mode, because
//	the user program either invoked a system call, or some exception
//	occured (such as the address translation failed).
//
//	"which" -- the cause of the kernel trap  系统出错陷入的类型
//	"badVaddr" -- the virtual address causing the trap, if appropriate 系统出错陷入的位置
//----------------------------------------------------------------------

void
Machine::RaiseException(ExceptionType which, int badVAddr)
{
    DEBUG('m', "Exception: %s\n", exceptionNames[which]);

//  ASSERT(interrupt->getStatus() == UserMode);
    registers[BadVAddrReg] = badVAddr;
    DelayedLoad(0, 0);			// finish anything in progress
    interrupt->setStatus(SystemMode);
    ExceptionHandler(which);		// interrupts are enabled at this point
    interrupt->setStatus(UserMode);
}

//----------------------------------------------------------------------
// Machine::Debugger
// 	Primitive debugger for user programs.  Note that we can't use
//	gdb to debug user programs, since gdb doesn't run on top of Nachos.
//	It could, but you'd have to implement *a lot* more system calls
//	to get it to work!
//
//	So just allow single-stepping, and printing the contents of memory.
//----------------------------------------------------------------------

void Machine::Debugger()
{
    char *buf = new char[80];
    int num;

    interrupt->DumpState();
    DumpState();
    printf("%d> ", stats->totalTicks);
    fflush(stdout);
    fgets(buf, 80, stdin);
    if (sscanf(buf, "%d", &num) == 1)
	runUntilTime = num;
    else {
	runUntilTime = 0;
	switch (*buf) {
	  case '\n':
	    break;

	  case 'c':
	    singleStep = FALSE;
	    break;

	  case '?':
	    printf("Machine commands:\n");
	    printf("    <return>  execute one instruction\n");
	    printf("    <number>  run until the given timer tick\n");
	    printf("    c         run until completion\n");
	    printf("    ?         print help message\n");
	    break;
	}
    }
    delete [] buf;
}

//----------------------------------------------------------------------
// Machine::DumpState
// 	Print the user program's CPU state.  We might print the contents
//	of memory, but that seemed like overkill.
//----------------------------------------------------------------------

void
Machine::DumpState()
{
    int i;

    printf("Machine registers:\n");
    for (i = 0; i < NumGPRegs; i++)
	switch (i) {
	  case StackReg:
	    printf("\tSP(%d):\t0x%x%s", i, registers[i],
		   ((i % 4) == 3) ? "\n" : "");
	    break;

	  case RetAddrReg:
	    printf("\tRA(%d):\t0x%x%s", i, registers[i],
		   ((i % 4) == 3) ? "\n" : "");
	    break;

	  default:
	    printf("\t%d:\t0x%x%s", i, registers[i],
		   ((i % 4) == 3) ? "\n" : "");
	    break;
	}

    printf("\tHi:\t0x%x", registers[HiReg]);
    printf("\tLo:\t0x%x\n", registers[LoReg]);
    printf("\tPC:\t0x%x", registers[PCReg]);
    printf("\tNextPC:\t0x%x", registers[NextPCReg]);
    printf("\tPrevPC:\t0x%x\n", registers[PrevPCReg]);
    printf("\tLoad:\t0x%x", registers[LoadReg]);
    printf("\tLoadV:\t0x%x\n", registers[LoadValueReg]);
    printf("\n");
}

//----------------------------------------------------------------------
// Machine::ReadRegister/WriteRegister
//   	Fetch or write the contents of a user program register.
//----------------------------------------------------------------------

int Machine::ReadRegister(int num)
    {
	ASSERT((num >= 0) && (num < NumTotalRegs));
	return registers[num];
    }

void Machine::WriteRegister(int num, int value)
    {
	ASSERT((num >= 0) && (num < NumTotalRegs));
	// DEBUG('m', "WriteRegister %d, value %d\n", num, value);
	registers[num] = value;
    }

/*******************  I hava change here **********************/

void Machine::FIFOSwap(int address){
    if(tlb==NULL){
        printf("There is no TLB\n");
        return;
    }
    //printf("TLB is full,using FIFO Swap.The entryTime of PTE is:");
    int min=65535;
    int index=0;
    int vpn=(unsigned)address/PageSize;
    int offset=(unsigned)address%PageSize;
    //感觉这里用个最小堆实现更合理,不过直接调用STL会有异常，自己懒得写了
    for(int i=0;i<TLBSize;i++){
        //printf("%d   ",tlb[i].createTime);
        if(tlb[i].createTime<min){
            min=tlb[i].createTime;
            index=i;
        }
    }
    //printf("%d\n");
    //printf("the entryTime of being swapped PTE is %d\n",tlb[index].createTime);
    tlb[index].virtualPage=entry->virtualPage;
    tlb[index].physicalPage=entry->physicalPage;
    tlb[index].valid=entry->valid;
    tlb[index].readOnly=entry->readOnly;
    tlb[index].dirty=entry->dirty;
    tlb[index].createTime=stats->totalTicks;
    entry->lastUseTime=stats->totalTicks;
    tlb[index].lastUseTime=entry->lastUseTime;
}

void Machine::LRUSwap(int address){
    if(tlb==NULL){
        printf("There is no TLB\n");
        return;
    }
   // printf("TLB is full,using LRU Swap.the lastUseTime of PTE is:");
    int min=65535;
    int index=0;
    int vpn=(unsigned)address/PageSize;
    int offset=(unsigned)address%PageSize;
    for(int i=0;i<TLBSize;i++){
        //printf("%d   ",tlb[i].lastUseTime);
        if(tlb[i].lastUseTime<min){
            min=tlb[i].lastUseTime;
            index=i;
        }
    }
    //printf("%d\n");
    //printf("the lastUseTime of being swapped PTE is %d\n",tlb[index].lastUseTime);
    //pageTable[i].virtualPage = i;
    tlb[index].virtualPage=entry->virtualPage;
    tlb[index].physicalPage=entry->physicalPage;
    tlb[index].valid=entry->valid;
    tlb[index].readOnly=entry->readOnly;
    tlb[index].dirty=entry->dirty;
    tlb[index].createTime=stats->totalTicks;
    entry->lastUseTime=stats->totalTicks;
    tlb[index].lastUseTime=entry->lastUseTime;
}
/***************************  end  ***************************/

/*******************  I hava change here **********************/
void Machine::Suspend_prepare()
{
    OpenFile *openfile=fileSystem->Open("vm");
    if(openfile==NULL)
        ASSERT(false);
    for (int i = 0; i < NumPhysPages; i++){
        if(machine->pageTable[i].thread_id==currentThread->getThread_id()){
            printf("%s may write PageTable[%d] to disk\n",currentThread->getName(),i);
            if(machine->pageTable[i].dirty==TRUE){
                openfile->WriteAt(&(machine->mainMemory[i*PageSize]),PageSize,machine->pageTable[i].virtualPage*PageSize);
            }
            machine->pageTable[i].valid=FALSE;
        }
    }
}

void Machine::PC_advance(){
    WriteRegister(PrevPCReg,registers[PCReg]);
    WriteRegister(PCReg,registers[PCReg]+sizeof(int));
    WriteRegister(NextPCReg,registers[NextPCReg]+sizeof(int));
}

void Machine::clear(){
    for (int i = 0; i < TLBSize; i++){
        tlb[i].valid = FALSE;
        tlb[i].createTime=stats->totalTicks;
        tlb[i].lastUseTime=stats->totalTicks;
    }
}
/***************************  end  ***************************/




















