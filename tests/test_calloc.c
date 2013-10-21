#include <niel_malloc.h>

int main(int argc, char *argv[])
{
    int *arr1, *arr2, *arr3;

    arr1 = (int *)niel_calloc(100, sizeof(int));
    arr2 = (int *)niel_calloc(100, sizeof(int));
    arr3 = (int *)niel_calloc(100, sizeof(int));

    niel_free(arr1);
    niel_free(arr2);
    niel_free(arr3);

    arr1 = (int *)niel_calloc(100, sizeof(int));
    arr2 = (int *)niel_calloc(100, sizeof(int));
    arr3 = (int *)niel_calloc(100, sizeof(int));

    niel_free(arr1);
    niel_free(arr2);
    niel_free(arr3);
}
