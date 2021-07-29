#include <stdio.h>
#include <stdlib.h>

int isSorted(int *arr, int size)
{
    if (--size - 1 >= 0)
    {
        if (!(arr[size] >= arr[size -1]))
            return 0 * isSorted(arr, 0);
        else
            return 1 * isSorted(arr, size);
    }
    
    return 1;
}

int zero (int n)
{
    if (n != 0)
        return n / 5 + zero(n/5);

    return 0;
}

int main()
{
    int arr_1[5] = {3, 6, 7, 7, 12};
    int arr_2[4] = {3, 4, 9, 8};

    printf("%d \n", isSorted(arr_1, 5));
    printf("%d \n", isSorted(arr_2, 4));

    return 0;
}