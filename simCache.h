#ifndef SIM_CACHE_H
#define SIM_CACHE_H

#define _CRT_SECURE_NO_WARNINGS // VS Studio comppatibliity


#include <stdio.h>
#include <math.h>
#include "configCache.h"

// Simulation function
int SimulateCache(SC_SIM_Cache* CacheArr, int CacheLevel, FILE* fd);

// Cache Access functions (core)
void ReadFromCache(SC_SIM_Cache* CacheArr, int CacheLevel, int addr, int memoryAccessCnt);
void WriteToCache(SC_SIM_Cache* CacheArr, int CacheLevel, int addr, int memoryAccessCnt);

// Other calculation functions
/* ------- Write your own code below  ------- */

/* ------------------------------------------ */


#endif