#include "configCache.h"
#include <stdio.h>

int MEM_ACCESS_COUNTER = 0;

SC_SIM_Cache init_Cache(int cacheSize, int blockSize, SC_SIM_WritePolicy writePolicy, int associativity, SC_SIM_ReplacementPolicy replacementPolicy, int accessCycle)
{
    // cache declaration
    SC_SIM_Cache cache;

    // fundamental parameters initialization
    cache.cacheSize = cacheSize;
    cache.blockSize = blockSize;
    cache.associativity = associativity;
    cache.writePolicy = writePolicy;
    cache.replacementPolicy = replacementPolicy;
    cache.accessCycle = accessCycle;

    // derived parameters initialization
    cache.num_of_lines = cacheSize / (blockSize * associativity);
    
    // cache memory allocation
    cache.CacheLines = (SC_SIM_CacheLine**)malloc( (int)sizeof(SC_SIM_CacheLine*) * cache.associativity);
    for (int i = 0; i < cache.associativity; i++)
    {
        cache.CacheLines[i] = (SC_SIM_CacheLine*)malloc( (int)sizeof(SC_SIM_CacheLine) * cache.num_of_lines);
    }

    // memory data initialization
    for (int setIdx = 0; setIdx < cache.associativity; setIdx++)
    {
        for (int LineIdx = 0; LineIdx < cache.num_of_lines; LineIdx++)
        {
            cache.CacheLines[setIdx][LineIdx].tag = 0;
            cache.CacheLines[setIdx][LineIdx].valid = 0;
            cache.CacheLines[setIdx][LineIdx].dirty = 0;
        }
    }

    // profiler initialization
    cache.profiler.readCounter = 0;
    cache.profiler.readHitCounter = 0;
    cache.profiler.writeCounter = 0;
    cache.profiler.writeHitCounter = 0;


    /*--------------      Write your own code below   --------------*/
    //                                                              //
    //  The initialization of member that you added in SC_SIM_Cache //
    // will be done here.                                           //
    cache.profiler.writeMissCounter = 0;
    cache.profiler.readMissCounter = 0;

    cache.profiler.accessCounter = 0;
    /*--------------------------------------------------------------*/


    return cache;
}

void killCache(SC_SIM_Cache cache)
{
    // cache memory deallocation
    for (int i = 0; i < cache.associativity; i++)
    {
        free(cache.CacheLines[i]);
    }
    free(cache.CacheLines);

    return;
}

int calc_TotalAccessCycle(SC_SIM_Cache* CacheArr, int CacheLevel)
{
    int TotalAccessCycle = 0;
    /*--------------     Write your own code below    --------------*/
    //                                                              //
    //  The code for calculating total access cycle should be       //
    //  written here.                                               //

    // 2단계
    for (int i = 0; i < CacheLevel; i++)
    {
        TotalAccessCycle += CacheArr[i].accessCycle * CacheArr[i].profiler.accessCounter;
    }

    TotalAccessCycle += SC_SIM_MEM_ACCESS_CYCLE * MEM_ACCESS_COUNTER;
    // 2단계
    
    /*--------------------------------------------------------------*/

    return TotalAccessCycle;
}

float calc_GlobalHitRatio(SC_SIM_Cache* CacheArr, int CacheLevel)
{
    float totalAccess;
    float totalMiss;
    /*--------------     Write your own code below    --------------*/
    //                                                              //
    //  The code for the calculation of global hit ratio should be  //
    //  written here.                                               //

    // 2단계
    totalAccess = (CacheArr[0].profiler.readCounter + CacheArr[0].profiler.writeCounter);
    totalMiss = (CacheArr[CacheLevel - 1].profiler.readMissCounter + CacheArr[CacheLevel - 1].profiler.writeMissCounter);
    // 2단계

    return 1 - (totalMiss / totalAccess);
    /*--------------------------------------------------------------*/
}