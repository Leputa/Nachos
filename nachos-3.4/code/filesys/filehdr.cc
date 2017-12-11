// filehdr.cc
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector,
//
//      Unlike in a real system, we do not keep track of file permissions,
//	ownership, last modification date, etc., in the file header.
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "filehdr.h"

//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------

bool
FileHeader::Allocate(BitMap *freeMap, int fileSize)
{
    numBytes = fileSize;
    numSectors  = divRoundUp(fileSize, SectorSize);
    if (freeMap->NumClear() < numSectors)
        return FALSE;		// not enough space

    /********************  I hava changed there ***********************/
    if(numSectors<NumDirect){
        for (int i = 0; i < numSectors; i++)
            dataSectors[i] = freeMap->Find();
    }
    else {
        for (int i = 0; i < NumDirect-1; i++)
            dataSectors[i] = freeMap->Find();
        dataSectors[NumDirect-1]=freeMap->Find();
        //printf("%d\n",dataSectors[NumDirect-1]);
        int indirect_index[32];
        for (int i = 0; i < numSectors-NumDirect+1;i++){
            indirect_index[i]=freeMap->Find();
        }
        //间接索索写回磁盘
        synchDisk->WriteSector(dataSectors[NumDirect-1],(char *)indirect_index);
        //printf("%d\n",dataSectors[NumDirect-1]);
    }
    /***************************  end  ***************************/
    return TRUE;
}

//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void
FileHeader::Deallocate(BitMap *freeMap)
{
    /********************  I hava changed there ***********************/
    if(numSectors<NumDirect){
        for (int i = 0; i < numSectors; i++) {
            //ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
            freeMap->Clear((int) dataSectors[i]);
        }
    }else{
        char *indirect_index=new char[SectorSize];
        synchDisk->ReadSector(dataSectors[NumDirect-1],indirect_index);
        for (int i = 0; i < numSectors-NumDirect+1;i++){
            //char 8bit,int 32bit
            freeMap->Clear((int)indirect_index[i * 4]);
        }
        for (int i = 0; i < NumDirect;i++){
            freeMap->Clear((int)dataSectors[i]);
        }
    }
    /***************************  end  ***************************/
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk.
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
FileHeader::FetchFrom(int sector)
{
    synchDisk->ReadSector(sector, (char *)this);
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk.
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void
FileHeader::WriteBack(int sector)
{
    synchDisk->WriteSector(sector, (char *)this);
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

int
FileHeader::ByteToSector(int offset)
{
    /********************  I hava changed there ***********************/
    //29*128=3712
    if(offset<(NumDirect-1)*128)
        return(dataSectors[offset / SectorSize]);
    else {
        int sector_position=(offset-(NumDirect-1)*128)/128;
        char *indirect_index=new char[SectorSize];
        synchDisk->ReadSector(dataSectors[NumDirect-1],indirect_index);
        return int(indirect_index[sector_position*4]);
    }
    /***************************  end  ***************************/
}

//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
FileHeader::FileLength()
{
    return numBytes;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void
FileHeader::Print()
{
    int i, j, k;
    char *data = new char[SectorSize];

    printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);
    /********************  I hava changed there ***********************/
    if (numSectors < NumDirect){
        for (i = 0; i < numSectors; i++)
            printf("%d ", dataSectors[i]);
    }else{
        printf("indirect_index: %d\n",dataSectors[NumDirect-1]);
        for (i = 0; i < NumDirect - 1; i++)
            printf("%d ", dataSectors[i]);
        char *indirect_index=new char[SectorSize];
        synchDisk->ReadSector(dataSectors[NumDirect-1],indirect_index);
        j=0;
        for (i = 0; i < numSectors-NumDirect+1;i++){
            printf("%d ",int(indirect_index[j]));
            j+=4;
        }
    }
    /***************************  end  ***************************/
    printf("\nFile contents:\n");
    /********************  I hava changed there ***********************/
    if (numSectors < NumDirect){
        for (i = k = 0; i < numSectors; i++) {
            synchDisk->ReadSector(dataSectors[i], data);
            for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
                if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
                    printf("%c", data[j]);
                else
                    printf("\\%x", (unsigned char)data[j]);
            }
        }
    }else{
        for (i = k = 0;i<NumDirect-1;i++){
            printf("Sector: %d\n",dataSectors[i]);
            synchDisk->ReadSector(dataSectors[i],data);
            for(j=0;(j<SectorSize)&&(k<numBytes);j++,k++){
                if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
                    printf("%c", data[j]);
                else
                    printf("\\%x", (unsigned char)data[j]);
            }
        }
        char *indirect_index =new char[SectorSize];
        synchDisk->ReadSector(dataSectors[NumDirect-1],indirect_index); //读一级索引
        for (i=0;i<numSectors-NumDirect+1;i++){
            printf("Sector: %d\n",int(indirect_index[i*4]));
            synchDisk->ReadSector(int(indirect_index[i*4]),data);
             for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
                if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
                    printf("%c", data[j]);
                else
                    printf("\\%x", (unsigned char)data[j]);
            }
        }
    }
    printf("\n");

    delete [] data;
    /********************  I hava changed there ***********************/
    if(fileTag==1){
        printf("filetype: %s\n",type);
        printf("create-time: %s\n",create_time);
        printf("last-visit-time: %s\n",last_visit_time);
        printf("last-modified-time: %s\n",last_modified_time);
    }
    /***************************  end  ***************************/
}


/********************  I hava changed there ***********************/
//strncpy(dest,src,n);    strncpy把src所指向以'\0'结尾的字符串的前n个字符复制到dest所指的数组中，返回指向dest的指针。
void FileHeader::set_create_time(){
    time_t timep;
    time(&timep);
    strncpy(create_time,asctime(gmtime(&timep)),25);
    create_time[24]='\0';
}

void FileHeader::set_last_visit_time(){
    time_t timep;
    time(&timep);
    strncpy(last_visit_time,asctime(gmtime(&timep)),25);
    last_visit_time[24]='\0';
}

void FileHeader::set_last_modified_time(){
    time_t timep;
    time(&timep);
    strncpy(last_modified_time,asctime(gmtime(&timep)),25);
    last_modified_time[24]='\0';
}

bool FileHeader::Extend(BitMap *freeMap,int bytes){
    numBytes=numBytes+bytes;
    int last_sector=numSectors;
    numSectors=divRoundUp(numBytes,SectorSize);
    if(last_sector==numSectors)
        return TRUE;
    if( freeMap->NumClear()<numSectors-last_sector)
        return FALSE;
    if (fileTag==4)
        printf("extend %d sectors\n",numSectors-last_sector);
    for (int i=last_sector;i<numSectors;i++)
        dataSectors[i]=freeMap->Find();
    return TRUE;
}
/***************************  end  ***************************/




