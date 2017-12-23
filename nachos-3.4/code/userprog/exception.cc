// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

/*******************  I hava change here **********************/
class Info{
    public:
        Info(){
            space=NULL;
        }
        AddrSpace *space;
        int pc;
};

void exec_func(int address){
    char name[24];
    int pos=0;
    int data;
    while(1){
        machine->ReadMem(address+pos,1,&data);
        if(data==0){
            name[pos]='\0';
            break;
        }
        name[pos++]=char(data);
    }
    OpenFile *openFile=fileSystem->Open(name);
    AddrSpace *space=new AddrSpace(openFile);
    currentThread->space=space;
    delete openFile;
    space->InitRegisters();
    space->RestoreState();
    machine->Run();
}

void fork_func(int address){
    Info *info=(Info*)address;
    AddrSpace *space=info->space;
    currentThread->space=space;
    int pc=info->pc;
    space->InitRegisters();
    space->RestoreState();
    machine->WriteRegister(PCReg,pc);
    machine->WriteRegister(NextPCReg,pc+4);
    machine->Run();
}

/***************************  end  ***************************/
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    if ((which == SyscallException) && (type == SC_Halt)) {
        DEBUG('a', "Shutdown, initiated by user program.\n");
        interrupt->Halt();
    }
    /*******************  I hava change here **********************/
    else if(which==PageFaultException){
        if(machine->tag==0){
            //处理快表失效
            DEBUG('a', "PageFault on TLB.calling TranslatePTE()\n");
            //printf("There is a TLB PageFault happening!\n");
            tlbUnHit++;
            //更新页表项地址
            machine->TranlatePTE();
            int position = -1;
            for(int i = 0; i < TLBSize; i++){
                if(machine->tlb[i].valid==FALSE){
                    position=i;
                    break;
                }
            }
            //快表还有空余，不需要换出
            if(position!=-1){
                //printf("These is unused page in TLB,and the position is %d\n",position+1);
                machine->tlb[position].valid=true;
                machine->tlb[position].virtualPage=machine->entry->virtualPage;
                machine->tlb[position].physicalPage=machine->entry->physicalPage;
                machine->tlb[position].readOnly=machine->entry->readOnly;
                machine->tlb[position].dirty=machine->entry->dirty;
                machine->tlb[position].createTime=stats->totalTicks;
                machine->entry->lastUseTime=stats->totalTicks;
                machine->tlb[position].lastUseTime=machine->entry->lastUseTime;
            }
            else{
                int address=machine->ReadRegister(BadVAddrReg);
                //LRU
                if(pageSwapPolicy==2)
                    machine->LRUSwap(address);
                //FIFO
                else if(pageSwapPolicy==1)
                    machine->FIFOSwap(address);
            }
        }
        else if(machine->tag==1){
            //处理页表失效
            //目前的情况下，因为物理页面全部进入内存，所以不会发生页表失效的问题
            OpenFile *openfile=fileSystem->Open("vm");
            if(openfile==NULL)
                ASSERT(false);
            machine->vpn=(unsigned)machine->registers[BadVAddrReg]/PageSize;
            int position=bitmap->Find();

            if(position==-1){
                position=0;
                for(int j=0;j<machine->pageTableSize;j++){
                    if(machine->pageTable[j].physicalPage==0){
                        //写回
                        if(machine->pageTable[j].dirty==TRUE){
                            openfile->WriteAt(&(machine->mainMemory[position*PageSize]),PageSize,machine->pageTable[j].virtualPage*PageSize);
                            machine->pageTable[j].valid=FALSE;
                            break;
                        }
                    }
                }
            }
            openfile->ReadAt(&(machine->mainMemory[position*PageSize]),PageSize,machine->vpn*PageSize);
            machine->pageTable[machine->vpn].valid=TRUE;
            machine->pageTable[machine->vpn].physicalPage=position;
            machine->pageTable[machine->vpn].use=TRUE;
            machine->pageTable[machine->vpn].dirty=FALSE;
            machine->pageTable[machine->vpn].readOnly=FALSE;
            delete openfile;
            machine->tag=0;
        }
        int NextPC = machine->ReadRegister(NextPCReg);
        machine->WriteRegister(PCReg, NextPC);
    }
    /***************************  end  ***************************/

    /*******************  I hava change here **********************/
    else if((which==SyscallException)&&(type==SC_Create)){
        if(testTag==6)
            printf("Syscall:Create\n");
        int address=machine->ReadRegister(4);
        char name[28];
        int pos=0;
        int data;
        while(1){
            machine->ReadMem(address+pos,1,&data);
            if(data==0){
                name[pos]='\0';
                break;
            }
            name[pos++]=char(data);
        }
        fileSystem->Create(name,28);
        machine->PC_advance();
    }
    else if((which==SyscallException)&&(type==SC_Open)){
        if(testTag==6)
            printf("Syscall:Open\n");
        int address=machine->ReadRegister(4);
        char name[28];
        int pos=0;
        int data;
        while(1){
            machine->ReadMem(address+pos,1,&data);
            if(data==0){
                name[pos]='\0';
                break;
            }
            name[pos++]=char(data);
        }
        OpenFile *openfile=fileSystem->Open(name);
        machine->WriteRegister(2,(int)openfile);
        machine->PC_advance();
    }
    else if((which==SyscallException)&&(type==SC_Close)){
        if(testTag==6)
            printf("Syscall:Close\n");
        int fd=machine->ReadRegister(4);
        OpenFile *openfile=(OpenFile*)fd;
        delete openfile;
        machine->PC_advance();
    }
    else if((which==SyscallException)&&(type==SC_Read)){
        if(testTag==6)
            printf("Syscall:Read\n");
        int position=machine->ReadRegister(4);
        int cnt=machine->ReadRegister(5);
        int fd=machine->ReadRegister(6);
        OpenFile *openfile=(OpenFile*)fd;
        char content[cnt];
        int result=openfile->Read(content,cnt);
        for(int i=0;i<result;i++)
            machine->WriteMem(position+i,1,(int)content[i]);
        machine->WriteRegister(2,result);
        machine->PC_advance();
    }
    else if((which==SyscallException)&&(type==SC_Write)){
        if(testTag==6)
            printf("Syscall:Write\n");
        int position=machine->ReadRegister(4);
        int cnt=machine->ReadRegister(5);
        int fd=machine->ReadRegister(6);
        OpenFile *openfile=(OpenFile*)fd;
        char content[cnt];
        int data;
        for (int i=0;i<cnt;i++){
            machine->ReadMem(position+i,1,&data);
            content[i]=char(data);
        }
        openfile->Write(content,cnt);
        machine->PC_advance();
    }
    else if ((which==SyscallException)&&(type==SC_Exec)){
        if(testTag==7)
            printf("Syscall:Exec\n");
        int address=machine->ReadRegister(4);
        Thread *thread2=new Thread("second Thread");
        thread2->Fork(exec_func,address);
        machine->WriteRegister(2,thread2->getThread_id());
        machine->PC_advance();
    }
    else if ((which==SyscallException)&&(type==SC_Fork)){
        if(testTag==7)
            printf("Syscall:Fork\n");
        int pc=machine->ReadRegister(4);
        Thread *thread2=new Thread("second Thread");
        OpenFile *openFile=fileSystem->Open(currentThread->fileName);
        AddrSpace *space=new AddrSpace(openFile);
        space=currentThread->space;
        Info*info=new Info;
        info->pc=pc;
        info->space=space;
        thread2->Fork(fork_func,int(info));
        machine->PC_advance();
    }
    else if((which==SyscallException)&&(type==SC_Join)){
        if(testTag==7)
            printf("Syscall:Join\n");
        int thread_ID=machine->ReadRegister(4);
        while(thread_id_flag[thread_ID])
            currentThread->Yield();
        machine->PC_advance();
    }
    else if((which==SyscallException)&&(type==SC_Yield)){
        if(testTag==7)
            printf("Syscall:Yield\n");
        machine->PC_advance();
        currentThread->Yield();
    }
    else if((which==SyscallException)&&(type==SC_Exit)){
        if(testTag==7)
            printf("Syscall:Exit\n");
        int status=machine->ReadRegister(4);
        printf("programme exit with status %d\n",status);
        machine->clear();
        machine->PC_advance();
        currentThread->Finish();
    }
    /***************************  end  ***************************/
    else {
        printf("Unexpected user mode exception %d %d\n", which, type);
        ASSERT(FALSE);
    }
}
