#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define Kernel_Size     ( 3 )
#define Image_Size_x    ( 256 )
#define Image_Size_y    ( 512 )
#define Result_Size_x   ( Image_Size_x - Kernel_Size + 1 )
#define Result_Size_y   ( Image_Size_y - Kernel_Size + 1 )

void markAs__Load(int* addr, FILE* fd);
void markAs__Store(int* addr, FILE* fd);


int main()
{
    // output File open
    FILE* fd = fopen("Conv2D_trace.txt", "w");

    // array declaration
    int Image[Image_Size_x][Image_Size_y];
    int Kernel[Kernel_Size][Kernel_Size];
    int Result[Result_Size_x][Result_Size_y];

    // array initialization (random value)
    for(int i = 0; i < Image_Size_x; i++)
    {
        for(int j = 0; j < Image_Size_y; j++)
        {
            Image[i][j] = rand();
        }
    }
    printf("Image initialized!\n");

    /*    Trace Start!    */
    // 2D-Convolution
    for(int i = 0; i < Result_Size_x; i++)
    {
        markAs__Load(&i, fd);

        for(int j = 0; j < Result_Size_y; j++)
        {
            markAs__Load(&j, fd);


            Result[i][j] = 0;
            markAs__Store(&Result[i][j], fd);


            for(int k = 0; k < Kernel_Size; k++)
            {
                markAs__Load(&k, fd);

                for(int l = 0; l < Kernel_Size; l++)
                {
                    markAs__Load(&l, fd);

                    Result[i][j] += Image[i + k][j + l] * Kernel[k][l];


                    markAs__Load(&Image[i + k][j + l], fd);
                    markAs__Load(&Kernel[k][l], fd);
                    markAs__Load(&Result[i][j], fd);
                    markAs__Store(&Result[i][j], fd);

                    markAs__Store(&l, fd);
                }

                markAs__Store(&k, fd);
            }

            markAs__Store(&j, fd);
        }

        markAs__Store(&i, fd);
    }
    printf("Convolution done!\n");
    
    // termination
    fclose(fd);
    return 0;
}

void markAs__Load(int* addr, FILE* fd)
{
    fprintf(fd, "L %d\n", (int) addr);
}
void markAs__Store(int* addr, FILE* fd)
{   
    fprintf(fd, "S %d\n", (int) addr);
}
