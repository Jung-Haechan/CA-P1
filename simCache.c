#include <stdio.h>
#include <math.h>
#include "configCache.h"
#include "simCache.h"
#include "utilities.h"


int SimulateCache(SC_SIM_Cache* CacheArr, int CacheLevel, FILE* fd)
{
    // Excution initialization
    int memoryAccessCnt = 0;

    // do simulation
    while(1)
    {
        // Read Access Log
        char accessType; int addr;
        fscanf(fd, "%c %d\n", &accessType, &addr);
        memoryAccessCnt++;
        if (feof(fd))
            break;

        // Memeory Access
        switch (accessType)
        {
            case 'L':
                ReadFromCache(CacheArr, CacheLevel, addr, memoryAccessCnt);
                break;
            case 'S':
                WriteToCache(CacheArr, CacheLevel, addr, memoryAccessCnt);
                break;
            default :
                printf("Error: Invalid Access Type\n");
                break;
        }
    }
    return memoryAccessCnt ;
}


void ReadFromCache(SC_SIM_Cache* CacheArr, int CacheLevel, int addr, int memoryAccessCnt)
{
    /* ------- Write your own code below  ------- */
    
    /* ------------------------------------------ */
    return;
}


void WriteToCache(SC_SIM_Cache* CacheArr, int CacheLevel, int addr, int memoryAccessCnt)
{
    /* ------- Write your own code below  ------- */
    
    /* ------------------------------------------ */

    return;
}



/* ------- Write your own code below  ------- */

/* ------------------------------------------ */
