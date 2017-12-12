// synchdisk.h
// 	Data structures to export a synchronous interface to the raw
//	disk device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#ifndef SYNCHDISK_H
#define SYNCHDISK_H

#include "disk.h"
#include "synch.h"

// The following class defines a "synchronous" disk abstraction.
// As with other I/O devices, the raw physical disk is an asynchronous device --
// requests to read or write portions of the disk return immediately,
// and an interrupt occurs later to signal that the operation completed.
// (Also, the physical characteristics of the disk device assume that
// only one operation can be requested at a time).
//
// This class provides the abstraction that for any individual thread
// making a request, it waits around until the operation finishes before
// returning.

/********************  I hava changed there ***********************/

class Cache {
    public:
        int valid;  //有效位
        int dirty;  //脏位
        int sector;
        int last_visit_time;
        char data[SectorSize];
};
/***************************  end  ***************************/


class SynchDisk {
  public:
    SynchDisk(char* name);    		// Initialize a synchronous disk,
					// by initializing the raw Disk.
    ~SynchDisk();			// De-allocate the synch disk data

    void ReadSector(int sectorNumber, char* data);
    					// Read/write a disk sector, returning
    					// only once the data is actually read
					// or written.  These call
    					// Disk::ReadRequest/WriteRequest and
					// then wait until the request is done.
    void WriteSector(int sectorNumber, char* data);

    void RequestDone();			// Called by the disk device interrupt
					// handler, to signal that the current disk operation is complete.
    /********************  I hava changed there ***********************/
    void PlusReader(int sector);   //增加读者
    void MinusReader(int sector);  //减少读者
    void BeginWrite(int sector);   //第一个读者开始读或写者开始写
    void EndWrite(int sector);     //最后一个读者读结束，或者写者写结束
    int numVisitors[NumSectors];   //记录访问某磁盘块的线程数量
    /***************************  end  ***************************/


  private:
    /********************  I hava changed there ***********************/
    Semaphore *mutex[NumSectors];   //文件访问信号量
    int numReaders[NumSectors];     //各个文件的读者数量
    Lock *rLock;                    //保证和读者数量相关操作互斥
    Cache *cache[4];
    /***************************  end  ***************************/
    Disk *disk;		  		// Raw disk device
    Semaphore *semaphore; 		// To synchronize requesting thread
					// with the interrupt handler
    Lock *lock;		  		// Only one read/write request
					// can be sent to the disk at a time
};

#endif // SYNCHDISK_H
