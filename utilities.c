#include <stdio.h>
#include <string.h>
#include "utilities.h"


/*-------------- Write your own code below ---------------*/

int binary_to_decimal(const char *binary) {
    int decimal = 0;
    int base = 1;
    int length = strlen(binary);

    for (int i = length - 1; i >= 0; i--) {
        if (binary[i] == '1') {
            decimal += base;
        }
        base *= 2;
    }

    return decimal;
}

char* decimal_to_binary(int decimal, int memorySize) {
    if (decimal < 0) {
        return NULL;  // 음수 처리
    }

    // 32비트 정수를 위한 충분한 크기의 문자열 할당
    int binarySize = memorySize + 1;  // 메모리사이즈 + 널 종료 문자
    char *binary = malloc(binarySize);
    if (binary == NULL) {
        return NULL;  // 메모리 할당 실패 처리
    }

    memset(binary, '0', binarySize);
    binary[binarySize - 1] = '\0';

    int index = binarySize - 2;
    while (decimal > 0) {
        binary[index] = (decimal % 2) + '0';
        decimal /= 2;
        index--;
    }

    return binary;
}

char* strsplit_with_index(const char *str, int start, int end)
{
    char *part = malloc(end - start + 2);
    // 첫 번째 부분 복사 (0~4번째 문자)
    strncpy(part, str + start - 1, end - start + 1);
    part[end - start + 2] = '\0';  // 널 종료 문자 삽입

    return part;
}
/*--------------------------------------------------------*/