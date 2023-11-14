#include <stdio.h>
#include <string.h>
#include <math.h>
#include "configCache.h"
#include "simCache.h"
#include "utilities.h"
#include <time.h>


int SimulateCache(SC_SIM_Cache* CacheArr, int CacheLevel, FILE* fd)
{
    FILE *fd_for_log;
    fd_for_log =fopen("log.txt", "w");

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

        if (LOGGING) {
            if (memoryAccessCnt == 5000)
                break;
        }

        // Memeory Access
        switch (accessType)
        {
            case 'L':
                ReadFromCache(CacheArr, CacheLevel, addr, memoryAccessCnt, fd_for_log);
                break;
            case 'S':
                WriteToCache(CacheArr, CacheLevel, addr, memoryAccessCnt, fd_for_log);
                break;
            default :
                printf("Error: Invalid Access Type\n");
                break;
        }
        if (LOGGING) fprintf(fd_for_log, "-----------------------------------------\n");
    }
    fclose(fd_for_log);
    return memoryAccessCnt;
}


void ReadFromCache(SC_SIM_Cache* CacheArr, int CacheLevel, int addr, int memoryAccessCnt, FILE *fd_for_log)
{
    /* ------- Write your own code below  ------- */

    // 캐시 존재 여부 출력 함수, 만약 cache hit 시 해당 cache level과 line index return
    if (LOGGING) fprintf(fd_for_log, "READ - address: %d\n", addr);
    DataLocationInfo d;
    d = FindCache(CacheArr, CacheLevel, addr, fd_for_log);
    // 캐시에 존재하면 해당 캐시레벨까지만 액세스, 존재하지 않으면 최종 캐시레벨까지 액세스
    int last_cache_lv = d.CacheIndex == -1 ? CacheLevel-1 : d.CacheIndex;

    // 캐시가 존재하지 않을 때 메모리 접근 + 1
    if (d.CacheIndex == -1) {
        if (LOGGING) fprintf(fd_for_log, "LOAD FROM MEMORY!\n");
        MEM_ACCESS_COUNTER += 1;
    }
    // 하위 캐시부터 순차적으로 override
    for (int i = last_cache_lv; i >= 0; i--) {
        CacheArr[i].profiler.readCounter += 1;
        // hit가 발생한 캐시에서는
        if (i == d.CacheIndex) {
            CacheArr[i].profiler.readHitCounter += 1;
            continue;
        } else {
            // 캐시 스케일에 따라 index, tag 변환
            IndexTag indexTag = IndexTagFromDecAddr(CacheArr[i], addr);
            // 캐시에 존재하지 않는 경우 override
            VictimCache(CacheArr, CacheLevel, i, indexTag.index, indexTag.tag, fd_for_log);
        }
    }


    /* ------------------------------------------ */
    return;
}


void WriteToCache(SC_SIM_Cache* CacheArr, int CacheLevel, int addr, int memoryAccessCnt, FILE *fd_for_log)
{
    /* ------- Write your own code below  ------- */
    if (LOGGING) fprintf(fd_for_log, "WRITE - address: %d\n", addr);
    DataLocationInfo d;    
    d = FindCache(CacheArr, CacheLevel, addr, fd_for_log);

    // 캐시에 존재하면 해당 캐시레벨까지만 액세스, 존재하지 않으면 최종 캐시레벨까지 액세스
    int last_cache_lv = d.CacheIndex == -1 ? CacheLevel-1 : d.CacheIndex;

    // 캐시가 존재하지 않을 때 메모리 접근 + 1
    if (d.CacheIndex == -1) {
        if (LOGGING) fprintf(fd_for_log, "LOAD FROM MEMORY!\n");
        MEM_ACCESS_COUNTER += 1;        
    }
    // 하위 캐시부터 순차적으로 override
    for (int i = last_cache_lv; i >= 0; i--) {
        // 캐시 스케일에 따라 index, tag 변환
        IndexTag indexTag = IndexTagFromDecAddr(CacheArr[i], addr);
        // 캐시에 존재하지 않는 경우 victim 후 override
        if (i != d.CacheIndex) {
            VictimCache(CacheArr, CacheLevel, i, indexTag.index, indexTag.tag, fd_for_log);
        } else {
            // 데이터 보유 캐시에 들어왔을 때는 hit + 1
            CacheArr[i].profiler.writeHitCounter += 1;
        }
        CacheArr[i].profiler.writeCounter += 1;
    }

    switch (CacheArr[0].writePolicy) {
        case 0: {   // WRITE TRHOUGH
            // 메모리까지 전부 write하므로 메모리/모든 하위캐시 접근 +1
            for (int i = 1; i < CacheLevel; i++) {
                CacheArr[i].profiler.accessCounter += 1;
            }
            MEM_ACCESS_COUNTER += 1;
            if (LOGGING) fprintf(fd_for_log, "WRITE ON MEMORY!");
            break;
        }
        case 1: {   // WRITE BACK
            // 최상위 캐시에 데이터 작성 후 dirty bit 1로 수정 
            IndexTag L1IndexTag = IndexTagFromDecAddr(CacheArr[0], addr);
            CacheArr[0].CacheLines[0][L1IndexTag.index].dirty = 1;
            if (LOGGING) fprintf(fd_for_log, "WRITE DATA ON CACHE[%d][%d] --- TAG: %d\n", 0, L1IndexTag.index, L1IndexTag.tag);
            break;
        }
    }


    // AccessCache(CacheArr, CacheLevel, addr, memoryAccessCnt, 'w');
    
    /* ------------------------------------------ */

    return;
}


/* ------- Write your own code below  ------- */

// 캐시 존재 여부 출력 함수, 만약 존재 시 존재하는 최상위 cache level과 line index return
DataLocationInfo FindCache(SC_SIM_Cache* CacheArr, int CacheLevel, int addr, FILE* fd_for_log)
{
    DataLocationInfo d;
    d.CacheIndex = -1;
    d.LineIndex = -1;
    for (int i = 0; i < CacheLevel; i++) {
        IndexTag decIndexTag = IndexTagFromDecAddr(CacheArr[i], addr);
        int decIndex = decIndexTag.index;
        int decTag = decIndexTag.tag;

        CacheArr[i].profiler.accessCounter += 1;

        int setIndex = GetSetIndex(CacheArr[i], decIndex, decTag);
        int is_exist = setIndex != -1;
        
        if (is_exist) {
            if (LOGGING) fprintf(fd_for_log, "ACCESS CACHE[%d] --- HIT! --- INDEX: %d, TAG:%d, SET: %d\n", i, decIndex, decTag, setIndex);

            d.CacheIndex = i;
            d.LineIndex = decIndex;
            switch (CacheArr[i].replacementPolicy) {
                case 0: {
                    //  LRU일 때는 캐시에 저장될 때와 hit이 발생했을 때 모두 최신순으로 정렬
                    if (LOGGING) {
                        fprintf(fd_for_log, "---<ORIGIN TAGS>: |");
                        for (int j = 0; j < CacheArr[i].associativity; j++) {
                            fprintf(fd_for_log, "%d|", CacheArr[i].CacheLines[j][decIndex].tag);
                        }
                        fprintf(fd_for_log, "\n");
                    }
                    RealignSetRU(CacheArr[i], decIndex, setIndex);
                    if (LOGGING) {
                        fprintf(fd_for_log, "---<NEW    TAGS>: |");
                        for (int j = 0; j < CacheArr[i].associativity; j++) {
                            fprintf(fd_for_log, "%d|", CacheArr[i].CacheLines[j][decIndex].tag);
                        }
                        fprintf(fd_for_log, "\n");
                    }
                    break;
                }
                case 1: {
                    //  FIFO일 때는 캐시에 저장될 때만 최신순으로 정렬
                    break;
                }
                default: {
                    //  RANDOM, NONE일 때는 정렬 필요 없음
                    break;
                }
            }

            break;
        }
        if (LOGGING) fprintf(fd_for_log, "ACCESS CACHE[%d] --- MISS! --- INDEX: %d, TAG:%d\n", i, decIndex, decTag);
    }
    return d;

    // d: { CacheIndex, LineIndex, OriginTag }
}

// victim cache 함수, 기존에 캐시에 차지하던 데이터를 날리고 방금 사용한 데이터를 override함.
void VictimCache(SC_SIM_Cache* CacheArr, int CacheLevel, int CacheIndex, int LineIndex, int Tag, FILE* fd_for_log)
{
    CacheArr[CacheIndex].profiler.accessCounter += 1;
    int SetIndex = 0;

    switch (CacheArr[CacheIndex].replacementPolicy) {
        case 0: {
            SetIndex = CacheArr[CacheIndex].associativity - 1;
            break;
        }
        case 1: {
            SetIndex = CacheArr[CacheIndex].associativity - 1;
            break;
        }
        case 2: {
            SetIndex = rand() % CacheArr[CacheIndex].associativity;
            break;
        }
        default: {
            break;
        }
    }

    if (LOGGING) {
        fprintf(fd_for_log, "REPLACE CACHE[%d][%d]\n", CacheIndex, LineIndex);
        fprintf(fd_for_log, "---<ORIGIN TAGS>: |");
        for (int i = 0; i < CacheArr[CacheIndex].associativity; i++) {
            fprintf(fd_for_log, "%d|", CacheArr[CacheIndex].CacheLines[i][LineIndex].tag);
        }
        fprintf(fd_for_log, "\n");
    }

    if (CacheArr[0].writePolicy == 1) {         // WRITE BACK
        if (CacheArr[CacheIndex].CacheLines[SetIndex][LineIndex].dirty == 1) {
            int originAddr = DecAddrFromIndexTag(CacheArr[CacheIndex], LineIndex, Tag);
            if (CacheIndex < CacheLevel - 1) { // L1 L2인 경우
                int ParentSetIndex = 0;
                // 하위 단계의 캐시에 접근해 캐시에 데이터 수정 후 dirty bit 1로 수정
                CacheArr[CacheIndex + 1].profiler.accessCounter += 1;
                IndexTag IndexTagOfParent = IndexTagFromDecAddr(CacheArr[CacheIndex + 1], originAddr);
                CacheArr[CacheIndex + 1].CacheLines[ParentSetIndex][IndexTagOfParent.index].dirty = 1;
                if (LOGGING)fprintf(fd_for_log, "---WRITE DATA ON CACHE[%d][%d] --- TAG: %d\n", CacheIndex+1, IndexTagOfParent.index, CacheArr[CacheIndex + 1].CacheLines[ParentSetIndex][IndexTagOfParent.index].tag);
            } else { // L3인 경우
                // 메모리에 데이터 수정
                MEM_ACCESS_COUNTER += 1;
                if (LOGGING)fprintf(fd_for_log, "WRITE ON MEMORY!");
            }
        }
    }

    //  victim cache
    CacheArr[CacheIndex].CacheLines[SetIndex][LineIndex].dirty = 0;
    CacheArr[CacheIndex].CacheLines[SetIndex][LineIndex].tag = Tag;
    CacheArr[CacheIndex].CacheLines[SetIndex][LineIndex].valid = 1;
    switch (CacheArr[CacheIndex].replacementPolicy) {
        case 0: {
            //  LRU일 때는 캐시에 저장될 때와 hit이 발생했을 때 모두 최신순으로 정렬
            RealignSetRU(CacheArr[CacheIndex], LineIndex, SetIndex);
            break;
        }
        case 1: {
            //  FIFO일 때는 캐시에 저장될 때만 최신순으로 정렬
            RealignSetRU(CacheArr[CacheIndex], LineIndex, SetIndex);
            break;
        }
        default: {
            //  RANDOM, NONE일 때는 정렬 필요 없음
            break;
        }
    }
    if (LOGGING) {
        fprintf(fd_for_log, "---<NEW    TAGS>: ");
        for (int i = 0; i < CacheArr[CacheIndex].associativity; i++) {
            fprintf(fd_for_log, "|%d", CacheArr[CacheIndex].CacheLines[i][LineIndex].tag);
        }
        fprintf(fd_for_log, "|\n");
    }
}

// decimal address를 통해 cache 스케일에 따라 tag와 index를 추출함
IndexTag IndexTagFromDecAddr(SC_SIM_Cache Cache, int decAddr) {
    IndexTag indexTag;
    char* binAddr = decimal_to_binary(decAddr, ADDR_SIZE);
    char* binBlockOffset = strsplit_with_index(binAddr, ADDR_SIZE - log2(Cache.blockSize) + 1, ADDR_SIZE);
    char* binIndex = strsplit_with_index(binAddr, ADDR_SIZE - log2(Cache.blockSize) - log2(Cache.num_of_lines) + 1, ADDR_SIZE - log2(Cache.blockSize));
    char* binTag = strsplit_with_index(binAddr, 1, ADDR_SIZE - log2(Cache.blockSize) - log2(Cache.num_of_lines));
    
    int decBlockOffset = binary_to_decimal(binBlockOffset);
    int decIndex = binary_to_decimal(binIndex);
    int decTag = binary_to_decimal(binTag);
    indexTag.index = decIndex;
    indexTag.tag = decTag;

    return indexTag;
}

// index와 tag를 통해 cache 스케일에 따라 decimal address를 추출함
int DecAddrFromIndexTag(SC_SIM_Cache Cache, int index, int tag) {
    char binOriginAddr[100];
    char* binOriginIndex;
    char* binOriginTag;
    char binBlockOffset[10];
    for (int i=0; i<log2(Cache.blockSize); i++) {
        binBlockOffset[i] = "0";
    }
    int decOriginAddr;
    int tagBit = ADDR_SIZE - log2(Cache.blockSize) - log2(Cache.num_of_lines);
    int indexBit = log2(Cache.num_of_lines);
    binOriginIndex = decimal_to_binary(index, indexBit);
    binOriginTag = decimal_to_binary(tag, tagBit);
    strcat(binOriginAddr, binOriginTag);
    strcat(binOriginAddr, binOriginIndex);
    strcat(binOriginAddr, binBlockOffset);

    return binary_to_decimal(binOriginAddr);
}

// Set Index를 찾는 함수
int GetSetIndex(SC_SIM_Cache Cache, int LineIndex, int Tag)
{
    int SetIndex = -1;
    for (int i = 0; i < Cache.associativity; i++) {
        if (Cache.CacheLines[i][LineIndex].tag == Tag && Cache.CacheLines[i][LineIndex].valid == 1) {
            SetIndex = i;
            break;
        }
    }

    return SetIndex;
}


// 최신순으로 Set 정렬
void RealignSetRU(SC_SIM_Cache Cache, int LineIndex, int SetIndex)
{
    SC_SIM_CacheLine tempCacheLine = Cache.CacheLines[SetIndex][LineIndex];
    for (int i = SetIndex - 1; i >= 0; i--) {
        Cache.CacheLines[i+1][LineIndex] = Cache.CacheLines[i][LineIndex];
    }
    Cache.CacheLines[0][LineIndex] = tempCacheLine;
}

/* ------------------------------------------ */
