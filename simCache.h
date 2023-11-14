#ifndef SIM_CACHE_H
#define SIM_CACHE_H

#define _CRT_SECURE_NO_WARNINGS // VS Studio comppatibliity


#include <stdio.h>
#include <math.h>
#include "configCache.h"

// Simulation function
int SimulateCache(SC_SIM_Cache* CacheArr, int CacheLevel, FILE* fd);

// Cache Access functions (core)
void ReadFromCache(SC_SIM_Cache* CacheArr, int CacheLevel, int addr, int memoryAccessCnt, FILE* fd_for_log);
void WriteToCache(SC_SIM_Cache* CacheArr, int CacheLevel, int addr, int memoryAccessCnt, FILE* fd_for_log);

// Other calculation functions
/* ------- Write your own code below  ------- */

typedef struct DataLocationInfo_st {
    int CacheIndex;
    int LineIndex;
}DataLocationInfo;

DataLocationInfo FindCache(SC_SIM_Cache* CacheArr, int CacheLevel, int addr, FILE* fd_for_log);

typedef struct IndexTag_st {
    int index;
    int tag;
}IndexTag;

IndexTag IndexTagFromDecAddr(SC_SIM_Cache Cache, int decAddr);
int DecAddrFromIndexTag(SC_SIM_Cache Cache, int index, int tag);
void VictimCache(SC_SIM_Cache* CacheArr, int CacheLevel, int CacheIndex, int LineIndex, int tag,  FILE* fd_for_log);
int GetSetIndex(SC_SIM_Cache Cache, int LineIndex, int Tag);
void RealignSetRU(SC_SIM_Cache Cache, int LineIndex, int SetIndex);

/* ------------------------------------------ */


#endif