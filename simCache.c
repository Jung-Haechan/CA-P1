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

        // if (memoryAccessCnt == 10)
        //     break;

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

    DataLocationInfo d;
    d = FindCache(CacheArr, CacheLevel, addr);
    if (d.CacheIndex == -1) {                           // 데이터가 메모리에만 있을 때
        for (int i = 0; i < CacheLevel; i++) {
            CacheArr[i].profiler.readCounter += 1;
            CacheArr[i].profiler.accessCounter += 1;
            MEM_ACCESS_COUNTER += 1;
        }
    } else {                                            // 데이터가 캐시에 있을 때
        CacheArr[d.CacheIndex].profiler.readHitCounter += 1;
        for (int i = 0; i < d.CacheIndex + 1; i++) {
            CacheArr[i].profiler.readCounter += 1;
            CacheArr[i].profiler.accessCounter += 1;
        }
    }

    /* ------------------------------------------ */
    return;
}


void WriteToCache(SC_SIM_Cache* CacheArr, int CacheLevel, int addr, int memoryAccessCnt)
{
    /* ------- Write your own code below  ------- */
    DataLocationInfo d;
    d = FindCache(CacheArr, CacheLevel, addr);
    if (d.CacheIndex == -1) {                                   // 데이터가 메모리에만 있을 때
        for (int i = 0; i < CacheLevel; i++) {
            CacheArr[i].profiler.writeCounter += 1;
            CacheArr[i].profiler.accessCounter += 1;
            MEM_ACCESS_COUNTER += 1;
        }
    } else {                                                    // 데이터가 캐시에 있을 때
        CacheArr[d.CacheIndex].profiler.writeHitCounter += 1;
        for (int i = 0; i < d.CacheIndex + 1; i++) {
            CacheArr[i].profiler.writeCounter += 1;
            CacheArr[i].profiler.accessCounter += 1;
        }
    }

    // Write through
    for (int i = 0; i < CacheLevel; i++) {
        CacheArr[i].profiler.accessCounter += 1;
    }
    MEM_ACCESS_COUNTER += 1;

    // AccessCache(CacheArr, CacheLevel, addr, memoryAccessCnt, 'w');
    
    /* ------------------------------------------ */

    return;
}



/* ------- Write your own code below  ------- */

DataLocationInfo FindCache(SC_SIM_Cache* CacheArr, int CacheLevel, int addr)
{
    DataLocationInfo d;
    d.CacheIndex = -1;
    for (int i = 0; i < CacheLevel; i++) {
        char* binAddr = decimal_to_binary(addr, ADDR_SIZE);
        char* binBlockOffset = strsplit_with_index(binAddr, ADDR_SIZE - log2(CacheArr[i].blockSize) + 1, ADDR_SIZE);
        char* binIndex = strsplit_with_index(binAddr, ADDR_SIZE - log2(CacheArr[i].blockSize) - log2(CacheArr[i].num_of_lines) + 1, ADDR_SIZE - log2(CacheArr[i].blockSize));
        char* binTag = strsplit_with_index(binAddr, 1, ADDR_SIZE - log2(CacheArr[i].blockSize) - log2(CacheArr[i].num_of_lines));
        
        int decBlockOffset = binary_to_decimal(binBlockOffset);
        int decIndex = binary_to_decimal(binIndex);
        int decTag = binary_to_decimal(binTag);

        int is_exist = CacheArr[i].CacheLines[0][decIndex].tag == decTag && CacheArr[i].CacheLines[0][decIndex].valid == 1;
        
        CacheArr[i].CacheLines[0][decIndex].tag = decTag;
        CacheArr[i].CacheLines[0][decIndex].valid = 1;
        
        d.LineIndex = decIndex;
        if (is_exist == 1) {
            d.CacheIndex = i;
            break;
        }
    }
    return d;

    // d: { CacheIndex, LineIndex }
}

/* ------------------------------------------ */
