#include <stdio.h>
#include <string.h>
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

        // if (memoryAccessCnt == 100)
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
    // printf("read\n");
    // 캐시에 존재하면 해당 캐시레벨까지만 액세스, 존재하지 않으면 최종 캐시레벨까지 액세스
    int last_cache_lv = d.CacheIndex == -1 ? CacheLevel-1 : d.CacheIndex;

    // 캐시가 존재하지 않을 때 메모리 접근 + 1
    if (d.CacheIndex == -1) {
        MEM_ACCESS_COUNTER += 1;        
    }
    // 하위 캐시부터 순차적으로 override
    for (int i = last_cache_lv; i >= 0; i--) {
        // 캐시 스케일에 따라 index, tag 변환
        IndexTag indexTag = IndexTagFromDecAddr(CacheArr[i], addr);
        // 캐시에 존재하지 않는 경우 override
        if (i != d.CacheIndex) {
            VictimCache(CacheArr, CacheLevel, i, indexTag.index, indexTag.tag);
        } else {
            // 데이터 보유 캐시에 들어왔을 때는 hit + 1
            CacheArr[i].profiler.readHitCounter += 1;
        }
        CacheArr[i].profiler.readCounter += 1;
        CacheArr[i].profiler.accessCounter += 1;
    }


    /* ------------------------------------------ */
    return;
}


void WriteToCache(SC_SIM_Cache* CacheArr, int CacheLevel, int addr, int memoryAccessCnt)
{
    /* ------- Write your own code below  ------- */
    DataLocationInfo d;    
    d = FindCache(CacheArr, CacheLevel, addr);
    // printf("write\n");

    // 캐시에 존재하면 해당 캐시레벨까지만 액세스, 존재하지 않으면 최종 캐시레벨까지 액세스
    int last_cache_lv = d.CacheIndex == -1 ? CacheLevel-1 : d.CacheIndex;

    // 캐시가 존재하지 않을 때 메모리 접근 + 1
    if (d.CacheIndex == -1) {
        MEM_ACCESS_COUNTER += 1;        
    }
    // 하위 캐시부터 순차적으로 override
    for (int i = last_cache_lv; i >= 0; i--) {
        // 캐시 스케일에 따라 index, tag 변환
        IndexTag indexTag = IndexTagFromDecAddr(CacheArr[i], addr);
        // 캐시에 존재하지 않는 경우 victim 후 override
        if (i != d.CacheIndex) {
            VictimCache(CacheArr, CacheLevel, i, indexTag.index, indexTag.tag);
        } else {
            // 데이터 보유 캐시에 들어왔을 때는 hit + 1
            CacheArr[i].profiler.writeHitCounter += 1;
        }
        CacheArr[i].profiler.writeCounter += 1;
        CacheArr[i].profiler.accessCounter += 1;
    }

    switch (CacheArr[0].writePolicy) {
        case 0: {   // WRITE TRHOUGH
            // 메모리까지 전부 write하므로 메모리/모든캐시 접근 +1
            for (int i = 0; i < CacheLevel; i++) {
                CacheArr[i].profiler.accessCounter += 1;
            }
            MEM_ACCESS_COUNTER += 1;
            break;
        }
        case 1: {   // WRITE BACK
            // 최상위 캐시에 데이터 작성 후 dirty bit 1로 수정 
            CacheArr[0].profiler.accessCounter += 1;
            IndexTag L1IndexTag = IndexTagFromDecAddr(CacheArr[0], addr);
            CacheArr[0].CacheLines[0][L1IndexTag.index].dirty = 1;
            break;
        }
    }


    // AccessCache(CacheArr, CacheLevel, addr, memoryAccessCnt, 'w');
    
    /* ------------------------------------------ */

    return;
}




/* ------- Write your own code below  ------- */

// 캐시 존재 여부 출력 함수, 만약 존재 시 존재하는 최상위 cache level과 line index return
DataLocationInfo FindCache(SC_SIM_Cache* CacheArr, int CacheLevel, int addr)
{
    DataLocationInfo d;
    d.CacheIndex = -1;
    d.OriginTag = -1;
    d.LineIndex = -1;
    // printf("----------------------------\n");
    for (int i = 0; i < CacheLevel; i++) {
        IndexTag decIndexTag = IndexTagFromDecAddr(CacheArr[i], addr);
        int decIndex = decIndexTag.index;
        int decTag = decIndexTag.tag;

        int is_exist = CacheArr[i].CacheLines[0][decIndex].tag == decTag && CacheArr[i].CacheLines[0][decIndex].valid == 1;
        
        if (is_exist == 1) {
            d.CacheIndex = i;
            d.LineIndex = decIndex;
            d.OriginTag = CacheArr[i].CacheLines[0][decIndex].tag;
            break;
        }
    }
    return d;

    // d: { CacheIndex, LineIndex, OriginTag }
}

// victim cache 함수, 기존에 캐시에 차지하던 데이터를 날리고 방금 사용한 데이터를 override함.
void VictimCache(SC_SIM_Cache* CacheArr, int CacheLevel, int CacheIndex, int LineIndex, int Tag)
{
    if (CacheArr[0].writePolicy == 1) {         // WRITE BACK
        int originAddr = DecAddrFromIndexTag(CacheArr[CacheIndex], LineIndex, Tag);
        if (CacheArr[CacheIndex].CacheLines[0][LineIndex].dirty == 1) {
            if (CacheIndex < CacheLevel - 1) {
                // 하위 단계의 캐시에 접근해 캐시에 데이터 수정 후 dirty bit 1로 수정
                CacheArr[CacheIndex + 1].profiler.accessCounter += 1;
                IndexTag IndexTagOfParent = IndexTagFromDecAddr(CacheArr[CacheIndex + 1], originAddr);
                CacheArr[CacheIndex + 1].CacheLines[0][LineIndex].dirty = 1;
            } else {
                // 메모리에 데이터 수정
                MEM_ACCESS_COUNTER += 1;
            }
        }
    }

    CacheArr[CacheIndex].CacheLines[0][LineIndex].tag = Tag;
    CacheArr[CacheIndex].CacheLines[0][LineIndex].valid = 1;
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

/* ------------------------------------------ */
