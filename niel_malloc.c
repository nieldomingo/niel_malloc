#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "niel_malloc.h"


mem_blk_header  *allocate_new_blk(mem_blk_header *last_blk, size_t size)
{
    size_t size_to_allocate = mem_blk_header_size + size;

    mem_blk_header *new_blk = sbrk(size_to_allocate);

    if (new_blk == (mem_blk_header *)-1)
        return NULL;
    
    new_blk->mem_blk = calc_mem_blk(new_blk);
    new_blk->blk_size = size;
    new_blk->next = NULL;
    new_blk->prev = last_blk;
    new_blk->assigned_flag = 1;
    if (last_blk != NULL)
    {
        last_blk->next = new_blk;
    }

    return new_blk->mem_blk;
}

mem_blk_header *assign_blk(mem_blk_header *blk, size_t size)
{
    size_t size_diff = blk->blk_size - size - mem_blk_header_size;

    if (size_diff > mem_blk_header_size)
    {
        /*
         * split the block
         */

        blk->blk_size = size;
        blk->assigned_flag = 1;
        mem_blk_header *new_blk = (mem_blk_header *)((char *)blk + mem_blk_header_size + size);
        new_blk->mem_blk = calc_mem_blk(new_blk);
        new_blk->blk_size = size_diff;
        new_blk->assigned_flag = 0;
        new_blk->next = blk->next;
        new_blk->prev = blk;
        blk->next = new_blk;

        return blk->mem_blk;
    }
    else
    {
        /*
         * don't split the block
         */
        blk->assigned_flag = 1;

        return blk->mem_blk;
    }
}

void *niel_malloc(size_t size)
{
    size_t aligned_size = align8(size);

    /* first call */
    if (heap_start == NULL)
    {
        heap_start = sbrk(0);
        return allocate_new_blk(NULL, aligned_size);
    }
    else
    {
        mem_blk_header *prev = heap_start;
        mem_blk_header *current = heap_start;

        while(current != NULL)
        {
            /*
             * look for a suitable niel_free block.
             * very naive algorithm, just assign the block
             * that fits.
             */
            if (current->assigned_flag == 0 && current->blk_size >= aligned_size)
            {
                return assign_blk(current, aligned_size);
            }

            prev = current;
            current = current->next;
        }

        /* 
         * did not find a suitable block, will allocate
         * new block
         * */

        return allocate_new_blk(prev, aligned_size);
    }
}

void collapse_consecutive_blks(mem_blk_header *start_blk)
{
    mem_blk_header *next_blk = start_blk->next;
    size_t new_size = start_blk->blk_size + next_blk->blk_size + mem_blk_header_size;

    start_blk->blk_size = new_size;
    start_blk->next = next_blk->next;

    if (next_blk->next != NULL)
    {
        next_blk->next->prev = start_blk;
    }
}

void niel_free(void *ptr)
{
    mem_blk_header *blk = (mem_blk_header *)((char *)ptr - mem_blk_header_size);
    
    blk->assigned_flag = 0;

    /*
     * check if succeeding blk can be combined with current block
     */
    if (blk->next != NULL && blk->next->assigned_flag == 0)
    {
        collapse_consecutive_blks(blk);
    }

    /*
     * check if preceeding blk can be combined with current block
     */
    if (blk->prev != NULL && blk->prev->assigned_flag == 0)
    {
        blk = blk->prev;
        collapse_consecutive_blks(blk);
    }

}

void *niel_calloc (size_t nr, size_t size)
{
    void *mem_ptr = NULL;
    
    mem_ptr = niel_malloc(nr * size);

    memset(mem_ptr, '\0', nr * size);

    return mem_ptr;
}

int main(int argc, char *argv[])
{
    int *arr1, *arr2, *arr3;

    arr1 = (int *)niel_malloc(sizeof(int) * 100);
    arr2 = (int *)niel_malloc(sizeof(int) * 100);
    arr3 = (int *)niel_malloc(sizeof(int) * 100);

    niel_free(arr1);
    niel_free(arr2);
    niel_free(arr3);

    arr1 = (int *)niel_malloc(sizeof(int) * 150);
    arr2 = (int *)niel_malloc(sizeof(int) * 150);

    niel_free(arr1);
    niel_free(arr2);

    arr1 = (int *)niel_calloc(100, sizeof(int));
    arr2 = (int *)niel_calloc(100, sizeof(int));
    arr3 = (int *)niel_calloc(100, sizeof(int));

    niel_free(arr1);
    niel_free(arr2);
    niel_free(arr3);

}
