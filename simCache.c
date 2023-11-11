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

    AccessCache(CacheArr, CacheLevel, addr, memoryAccessCnt, 'r');

    /* ------------------------------------------ */
    return;
}


void WriteToCache(SC_SIM_Cache* CacheArr, int CacheLevel, int addr, int memoryAccessCnt)
{
    /* ------- Write your own code below  ------- */

    AccessCache(CacheArr, CacheLevel, addr, memoryAccessCnt, 'w');
    // printf("%s\n%s\n%s\n%s\n", binAddr, binBlockOffset, binIndex, binTag);
    
    /* ------------------------------------------ */

    return;
}



/* ------- Write your own code below  ------- */

void AccessCache(SC_SIM_Cache* CacheArr, int CacheLevel, int addr, int memoryAccessCnt, char type)
{
    for (int i = 0; i < CacheLevel; i++) {
        char* binAddr = decimal_to_binary(addr, ADDR_SIZE);
        char* binBlockOffset = strsplit_with_index(binAddr, ADDR_SIZE - log2(CacheArr[i].blockSize) + 1, ADDR_SIZE);
        char* binIndex = strsplit_with_index(binAddr, ADDR_SIZE - log2(CacheArr[i].blockSize) - log2(CacheArr[i].num_of_lines) + 1, ADDR_SIZE - log2(CacheArr[i].blockSize));
        char* binTag = strsplit_with_index(binAddr, 1, ADDR_SIZE - log2(CacheArr[i].blockSize) - log2(CacheArr[i].num_of_lines));
        
        int decBlockOffset = binary_to_decimal(binBlockOffset);
        int decIndex = binary_to_decimal(binIndex);
        int decTag = binary_to_decimal(binTag);

        int is_exist = CacheArr[i].CacheLines[0][decIndex].tag == decTag && CacheArr[i].CacheLines[0][decIndex].valid == 1;

        if (type == 'w') {
            CacheArr[i].profiler.writeCounter += 1;
            if (is_exist) {
                CacheArr[i].profiler.writeHitCounter += 1;
                break;
            } else {
                CacheArr[i].profiler.writeMissCounter += 1;
            }
        } else {
            CacheArr[i].profiler.readCounter += 1;
            if (is_exist) {
                CacheArr[i].profiler.readHitCounter += 1;
                break;
            } else {
                CacheArr[i].profiler.readMissCounter += 1;
            }
        }

        CacheArr[i].CacheLines[0][decIndex].tag = decTag;
        CacheArr[i].CacheLines[0][decIndex].valid = 1;
    }

}

/* ------------------------------------------ */
