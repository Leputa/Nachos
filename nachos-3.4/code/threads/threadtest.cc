//// threadtest.cc
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield,
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "elevatortest.h"
/********************  I hava changed there ***********************/
#include "synch.h"
#define bufferPoolSize 5
/***************************  end  ***************************/

// testnum is set in main.cc

/********************  I hava changed there ***********************/
int testnum=9;
/***************************  end  ***************************/





//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------


/********************   What I do below is for testnum==6(验证抢占性) ***********************/
void SimpleThread4(int which){
    for(int i=0;i<5;i++){
			printf("***thread name %s threadId %d priority %d looped %d times\n",currentThread->getName(),currentThread->getThread_id(),currentThread->getPriority(),i);
    }
}

void SimpleThread3(int which){
    for(int i=0;i<5;i++){
			printf("***thread name %s threadId %d priority %d looped %d times\n",currentThread->getName(),currentThread->getThread_id(),currentThread->getPriority(),i);
			if(i==3){
                Thread *t4 = new Thread("thread3",-8);
				t4->Fork(SimpleThread4,t4->getThread_id());
            }
    }
}

void SimpleThread2(int which){
    for(int i=0;i<5;i++){
			printf("***thread name %s threadId %d priority %d looped %d times\n",currentThread->getName(),currentThread->getThread_id(),currentThread->getPriority(),i);
			if(i==2){
                Thread *t3 = new Thread("thread3",2);
				t3->Fork(SimpleThread3,t3->getThread_id());
            }
    }
}
/********************   What I do up is for testnum==6(验证抢占性) ***********************/





/********************   What I do below is for testnum==8(生产者、消费者) ***********************/
class BufferPool{
    private:
        Lock *w;      //在读者写者问题中，用于锁定“读、写操作”
        Lock *lock;   //在读者写者问题中，用与锁定"修改count"
        Condition *condition;
        int bufferCount;
    public:
        BufferPool(){
            lock=new Lock("Lock");
            w=new Lock("w/r");
            condition =new Condition("Condition");
            bufferCount=0;
        }
        ~BufferPool(){
            delete lock;
            delete condition;
            bufferCount=0;
        }

        Lock* getLock(){
            return lock;
        }
        Lock *getLockW(){
            return w;
        }
        Condition* getCondition(){
            return condition;
        }
        int getbufferCount(){
            return bufferCount;
        }
        void addBuffer(){
            ++bufferCount;
        }
        void subBuffer(){
            --bufferCount;
        }
        void setBufferCount(int num){
            bufferCount=num;
        }
};


void Producer(BufferPool *bufferPool){
    for (int i=0;i<10;i++){
        //生产一个产品
        printf("Thread %s produce an item.\n",currentThread->getName());
        Lock *lock=bufferPool->getLock();
        lock->Acquire();
        Condition *condition=bufferPool->getCondition();
        if(bufferPool->getbufferCount()>=bufferPoolSize){
            printf("There is no empty buffer to get\n");
            condition->Wait(lock);
        }
        bufferPool->addBuffer();
        printf("Thread %s use an empty buffer,There are %d buffers used \n",currentThread->getName(),bufferPool->getbufferCount());
        if(bufferPool->getbufferCount()==1){
            condition->Signal(lock);
        }
        lock->Release();
    }
}

void Consumer(BufferPool *bufferPool){
    for (int i=0;i<6;i++){
        Lock *lock=bufferPool->getLock();
        lock->Acquire();
        Condition *condition=bufferPool->getCondition();
        printf("There are %d items in bufferPool\n",bufferPool->getbufferCount());
        if(bufferPool->getbufferCount()==0){
            printf("There is no item.\n");
            condition->Wait(lock);
        }
        //消费一个产品
        bufferPool->subBuffer();
        printf("Thread %s consume an item.There are %d items in bufferPool\n",currentThread->getName(),bufferPool->getbufferCount());

        if(bufferPool->getbufferCount()==bufferPoolSize-1){
            condition->Signal(lock);
        }
        lock->Release();
    }
}

/********************   What I do up is for testnum==8(生产者、消费者) ***********************/


/********************   What I do below is for testnum==9(barrier) ***********************/
void barrierTest(BufferPool *bufferPool){
    for(int i=0;i<3;i++){
        Lock *lock=bufferPool->getLock();
        Condition *condition=bufferPool->getCondition();
        lock->Acquire();
        bufferPool->addBuffer();
        printf("Thread %s produce an item,There are %d items in bufferPool\n",currentThread->getName(),bufferPool->getbufferCount());
        if(bufferPool->getbufferCount()==bufferPoolSize){
            printf("The bufferPool is full,wake up all threads.\n");
            bufferPool->setBufferCount(0);
            condition->Broadcast(lock);

        }
        else{
            //释放资源
            condition->Wait(lock);
        }
        lock->Release();
    }
}


/********************   What I do up is for testnum==9(barrier) ***********************/


/********************   What I do below is for testnum==10(write/lock) ***********************/
void Reader(BufferPool *buffPool){
    for(int i=0;i<10;i++){
        Lock *lock=buffPool->getLock();
        Lock *w=buffPool->getLockW();

        lock->Acquire();
        if(buffPool->getbufferCount()==0){
            w->Acquire();
            buffPool->addBuffer();   //加入一个读者
            //printf("A reader release lockW\n");
        }
        else{
            buffPool->addBuffer();
        }
        lock->Release();

        printf("Thread %s is Reading and there are %d readers in total\n",currentThread->getName(),buffPool->getbufferCount());

        lock->Acquire();
        buffPool->subBuffer();
        if(buffPool->getbufferCount()==0){
            w->Release();
            //printf("A reader release lockW\n");
        }
        lock->Release();
    }
}


void Writer(BufferPool *buffPool){
    for(int i=0;i<10;i++){
        Lock *w=buffPool->getLockW();
        w->Acquire();
        //printf("A writer get lockW\n");
        printf("Thread %s is Writing and there are %d readers in total\n",currentThread->getName(),buffPool->getbufferCount());
        w->Release();
        //printf("A writer release lockW\n");
    }
}
/********************   What I do up is for testnum==10(write/lock) ***********************/


void
SimpleThread(int which)
{
    /********************  I hava changed there ***********************/
    switch(testnum){
        case 2:
            for (int num = 0; num < 5; num++) {
                printf("*** thread name %s userId %d threadId %d \n",currentThread->getName(),currentThread->getUser_id(),currentThread->getThread_id());
                currentThread->Yield();
            }
            break;

        case 4:
            currentThread->Yield();
            break;

        case 5:
            for(int i=0;i<5;i++)
                printf("***thread name %s threadId %d priority %d looped %d times\n",currentThread->getName(),currentThread->getThread_id(),currentThread->getPriority(),i);
            break;

        case 6:
            for(int i=0;i<5;i++){
                printf("***thread name %s threadId %d priority %d looped %d times\n",currentThread->getName(),currentThread->getThread_id(),currentThread->getPriority(),i);
                if(i==1){
                    Thread *t2 = new Thread("thread2",4);
                    t2->Fork(SimpleThread2,t2->getThread_id());
                }
            }
            break;
        case 7:
            for (int num = 0; num < 10; num++) {
                printf("*** thread name %s  threadId %d looped %d times\n",currentThread->getName(),currentThread->getThread_id(),num);
                interrupt->SetLevel(IntOn);
                interrupt->SetLevel(IntOff);
            }
            break;
    }
    if(testnum==3){
        int id=currentThread->getThread_id();
            if(id==-1)
                printf("All threads have been allocated!\n");
            ASSERT(id!=-1);
            //printf("*** thread %d looped %d times\n", which, num);
            printf("*** thread name %s userId %d threadId %d \n",currentThread->getName(),currentThread->getUser_id(),id);
            currentThread->Yield();
    }
}




//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, (void*)1);
    SimpleThread(0);
}

/********************  Here is my codes ***********************/
void ThreadTest2(){    //test thread_id,user_id
	DEBUG('t', "Entering ThreadTest2");
	Thread *t1 = new Thread("forked thread");
	Thread *t2 = new Thread("forked thread");
	Thread *t3 = new Thread("forked thread");
	Thread *t4 = new Thread("forked thread");

	t1->Fork(SimpleThread,t1->getThread_id());
	t2->Fork(SimpleThread,t2->getThread_id());
	t3->Fork(SimpleThread,t3->getThread_id());
	t4->Fork(SimpleThread,t4->getThread_id());
}

void ThreadTest3(){ //test overflow 120
	DEBUG('t', "Entering ThreadTest3");
	Thread *t[MaxThread+5];

	for (int i=0;i<MaxThread+5;i++)
        t[i] = new Thread("forked thread");

    for (int i=0;i<MaxThread+5;i++)
        t[i]->Fork(SimpleThread,t[i]->getThread_id());

}

void ThreadTest4(){ //test TS
	DEBUG('t', "Entering ThreadTest4");

	Thread *t1 = new Thread("forked thread");
	Thread *t2 = new Thread("forked thread");
	Thread *t3 = new Thread("forked thread");
	Thread *t4 = new Thread("forked thread");

	t1->Fork(SimpleThread,t1->getThread_id());
	t2->Fork(SimpleThread,t2->getThread_id());
	t3->Fork(SimpleThread,t3->getThread_id());
	t4->Fork(SimpleThread,t4->getThread_id());

	currentThread->TS();

}

void ThreadTest5(){  //test PRI
	DEBUG('t', "Entering ThreadTest5");
	Thread *t1 = new Thread("thread1",16);
	Thread *t2 = new Thread("thread2",4);
	Thread *t3 = new Thread("thread3",-8);//测试-8时优先级是否为1
	Thread *t4 = new Thread("thread4");   //测试优先级是否为32

	t1->Fork(SimpleThread,t1->getThread_id());
	t2->Fork(SimpleThread,t2->getThread_id());
	t3->Fork(SimpleThread,t3->getThread_id());
	t4->Fork(SimpleThread,t4->getThread_id());
}

void ThreadTest6(){ //test PRI(preemptive)
	DEBUG('t', "Entering ThreadTest6");
	Thread *t1 = new Thread("thread1",16);
	t1->Fork(SimpleThread,t1->getThread_id());

}

void ThreadTest7(){ //test RR
	DEBUG('t', "Entering ThreadTest7");
	Thread *t1 = new Thread("thread1",8);
	Thread *t2 = new Thread("thread2",12);
	Thread *t3 = new Thread("thread3",1);//测试-8时优先级是否为1
	Thread *t4 = new Thread("thread4",3);   //测试优先级是否为32

	t1->Fork(SimpleThread,t1->getThread_id());
	t2->Fork(SimpleThread,t2->getThread_id());
	t3->Fork(SimpleThread,t3->getThread_id());
	t4->Fork(SimpleThread,t4->getThread_id());
}

void ThreadTest8(){ //test producer/consumer
    DEBUG('t', "Entering ThreadTest8");
    Thread *producer=new Thread("Producer");
    Thread *consumer=new Thread("Consumer1");

    BufferPool *bufferPool=new BufferPool();
    producer->Fork(Producer,bufferPool);
    consumer->Fork(Consumer,bufferPool);
}

void ThreadTest9(){  //test broadcast
    DEBUG('t', "Entering ThreadTest9");
    Thread *t1 = new Thread("thread1");
    Thread *t2 = new Thread("thread2");
    Thread *t3 = new Thread("thread3");
    Thread *t4 = new Thread("thread4");
    Thread *t5 = new Thread("thread5");

    BufferPool *bufferPool=new BufferPool();

    t1->Fork(barrierTest,bufferPool);
    t2->Fork(barrierTest,bufferPool);
    t3->Fork(barrierTest,bufferPool);
    t4->Fork(barrierTest,bufferPool);
    t5->Fork(barrierTest,bufferPool);
}

void ThreadTest10(){  //test write/lock
    DEBUG('t', "Entering ThreadTest10");
    Thread *writer=new Thread("Writer");
    Thread *reader1=new Thread("Reader1");
    Thread *reader2=new Thread("Reader2");
    Thread *reader3=new Thread("Reader3");
    Thread *reader4=new Thread("Reader4");
    Thread *reader5=new Thread("Reader5");


    BufferPool *bufferPool=new BufferPool();

    writer->Fork(Writer,bufferPool);

    reader1->Fork(Reader,bufferPool);
    reader2->Fork(Reader,bufferPool);
    reader3->Fork(Reader,bufferPool);
    reader4->Fork(Reader,bufferPool);
    reader5->Fork(Reader,bufferPool);
}

/***************************  end  ***************************/

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    printf("if you want to test Lab1 Exercise3 'userId、threadId',please input '2':\n");
    printf("if you want to test Lab1 Exercise4 'threads overflow',please input '3':\n");
    printf("if you want to test Lab1 Exercise4 'Threads Status',please input '4':");
    printf("if you want to test Lab2 Exercise3 'Pri scheduler',please input '5':\n");
    printf("if you want to test Lab2 Exercise3 'Pri scheduler(preemptive)',please input '6':\n");
    printf("if you want to test Lab2 Challenge1 'RR scheduler',please input '7':\n");
    printf("if you want to test Lab3 Exercise4 'Producer/Consumer',please input '8':\n");
    printf("if you want to test Lab3 Challenge1 'Barrier',please input '9':\n");
    printf("if you want to test Lab3 Challenge2 'write/read',please input '10':\n");
    scanf("%d",&testnum);
    switch (testnum) {
        case 1:
            ThreadTest1();
            break;
/********************  Here is my codes ***********************/
        case 2:
            ThreadTest2();
            break;
        case 3:
            ThreadTest3();
            break;
        case 4:
            ThreadTest4();
            break;
        case 5:
            ThreadTest5();
            break;
        case 6:
            ThreadTest6();
            break;
        case 7:
            ThreadTest7();
            break;
        case 8:
            ThreadTest8();
            break;
        case 9:
            ThreadTest9();
            break;
        case 10:
            ThreadTest10();
            break;
/***************************  end  ***************************/

    default:
        printf("No test specified.\n");
        break;
    }
}

