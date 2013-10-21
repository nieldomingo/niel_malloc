#include <niel_malloc.h>
#include <assert.h>

int main(int argc, char *argv[])
{
    int *arr1, *arr2, *arr3;

    arr1 = (int *)niel_malloc(sizeof(int) * 100);
    arr2 = (int *)niel_malloc(sizeof(int) * 100);
    arr3 = (int *)niel_malloc(sizeof(int) * 100);

    for (int i = 0; i < 100; i++)
    {
        arr1[i] = i;
        arr2[i] = i;
        arr3[i] = i;
    }
    
    for (int i = 0; i < 100; i++)
    {
        assert(arr1[i] == i);
        assert(arr2[i] == i);
        assert(arr3[i] == i);
    }

    niel_free(arr1);
    niel_free(arr2);
    niel_free(arr3);

    arr1 = (int *)niel_malloc(sizeof(int) * 150);
    arr2 = (int *)niel_malloc(sizeof(int) * 150);
    
    for (int i = 0; i < 150; i++)
    {
        arr1[i] = i;
        arr2[i] = i;
    }
    
    for (int i = 0; i < 150; i++)
    {
        assert(arr1[i] == i);
        assert(arr2[i] == i);
    }

    niel_free(arr1);
    niel_free(arr2);
}
